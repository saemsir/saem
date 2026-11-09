#!/usr/bin/env python3
"""Loopback-only development website and versioned C17 kernel API."""

import argparse
import ctypes
import json
import os
import shlex
import subprocess
import sys
import tempfile
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlsplit

from inventory_exam_papers import EXAM_FOLDERS, write_inventory


ROOT = Path(__file__).resolve().parent
KERNEL_ROOT = ROOT.parent / "saem-unified" / "core" / "c17"
WEB_ROOT = ROOT / "web"
MAX_BODY_BYTES = 2048
MAX_OPERAND_CHARS = 64
MAX_OUTPUT_BYTES = 64
STATUS_NAMES = {
    0: "ok",
    8: "invalid",
    15: "range",
    18: "unknown",
}
OPERATIONS = {"add", "subtract", "multiply", "divide"}
WEB_FILES = {
    "/": ("index.html", "text/html; charset=utf-8"),
    "/app.js": ("app.js", "text/javascript; charset=utf-8"),
    "/style.css": ("style.css", "text/css; charset=utf-8"),
}
INVENTORY_POLL_SECONDS = 2


def paper_folder_signature(root):
    signature = []
    root = Path(root)
    for folder in EXAM_FOLDERS:
        exam_dir = root / folder
        if not exam_dir.is_dir():
            raise FileNotFoundError(f"Exam source folder is missing: {folder}")
        for path in exam_dir.iterdir():
            if path.is_symlink():
                raise ValueError(f"Symbolic links are not allowed in the exam corpus: {path}")
            if not path.is_file() or path.suffix.casefold() != ".pdf":
                continue
            stat = path.stat()
            signature.append((path.relative_to(root).as_posix(), stat.st_size, stat.st_mtime_ns))
    return tuple(sorted(signature))


class PaperInventoryWatcher:
    def __init__(self, root, output, interval=INVENTORY_POLL_SECONDS):
        self.root = Path(root)
        self.output = Path(output)
        self.interval = interval
        self.stop_event = threading.Event()
        self.signature = paper_folder_signature(self.root)
        self.thread = threading.Thread(
            target=self._watch,
            name="exam-paper-inventory",
            daemon=True,
        )

    def start(self):
        self.thread.start()

    def stop(self):
        self.stop_event.set()
        self.thread.join()

    def _watch(self):
        last_error = None
        while not self.stop_event.wait(self.interval):
            try:
                signature = paper_folder_signature(self.root)
                if signature == self.signature:
                    last_error = None
                    continue
                inventory, _ = write_inventory(self.root, self.output)
            except (OSError, ValueError) as error:
                message = str(error)
                if message != last_error:
                    print(f"Paper inventory refresh failed: {message}", file=sys.stderr)
                    last_error = message
                continue
            self.signature = signature
            last_error = None
            print(f"Paper inventory refreshed: {inventory['sourceCount']} PDFs.")


def build_kernel_library(output_dir):
    compiler = shlex.split(os.environ.get("CC", "cc"))
    if not compiler:
        raise RuntimeError("CC must name a C compiler.")
    output_path = Path(output_dir) / "libsaemkernel.so"
    command = compiler + [
        "-std=c17",
        "-O2",
        "-Wall",
        "-Wextra",
        "-Werror",
        "-pedantic",
        "-fPIC",
        "-shared",
        "-I",
        str(KERNEL_ROOT),
        str(ROOT / "kernel_bridge.c"),
        str(KERNEL_ROOT / "cos_rat.c"),
        "-o",
        str(output_path),
    ]
    try:
        subprocess.run(command, check=True, capture_output=True, text=True)
    except subprocess.CalledProcessError as error:
        detail = error.stderr.strip() or error.stdout.strip() or "No compiler details were provided."
        raise RuntimeError(f"C17 bridge compilation failed: {detail}") from error
    return output_path


class C17Kernel:
    def __init__(self, library_path):
        self.library = ctypes.CDLL(str(library_path))
        self.library.saem_kernel_rational.argtypes = [
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.POINTER(ctypes.c_char),
            ctypes.c_size_t,
        ]
        self.library.saem_kernel_rational.restype = ctypes.c_int

    def calculate(self, operation, left, right):
        output = ctypes.create_string_buffer(MAX_OUTPUT_BYTES)
        status = self.library.saem_kernel_rational(
            operation.encode("ascii"),
            left.encode("ascii"),
            right.encode("ascii"),
            output,
            len(output),
        )
        return status, output.value.decode("ascii") if status == 0 else None


def reject_duplicate_keys(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError("Duplicate JSON keys are not accepted.")
        result[key] = value
    return result


def reject_json_constant(value):
    raise ValueError(f"Invalid JSON value: {value}")


def make_handler(kernel, inventory_path):
    class Handler(BaseHTTPRequestHandler):
        server_version = "SaemDynamicDev/1"
        sys_version = ""

        def end_headers(self):
            if self.close_connection:
                self.send_header("Connection", "close")
            self.send_header("X-Content-Type-Options", "nosniff")
            self.send_header("X-Frame-Options", "DENY")
            self.send_header("Referrer-Policy", "no-referrer")
            self.send_header("Cross-Origin-Resource-Policy", "same-origin")
            self.send_header("Content-Security-Policy",
                             "default-src 'self'; base-uri 'none'; frame-ancestors 'none'; "
                             "form-action 'self'; object-src 'none'; script-src 'self'; "
                             "style-src 'self'; connect-src 'self'")
            self.send_header("Cache-Control", "no-store")
            super().end_headers()

        def send_json(self, status_code, payload):
            body = json.dumps(payload, separators=(",", ":"), ensure_ascii=True).encode("utf-8")
            self.send_response(status_code)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def send_error(self, code, message=None, explain=None):
            phrases = {
                400: ("bad_request", "The request is invalid."),
                404: ("not_found", "The route was not found."),
                405: ("method_not_allowed", "The method is not allowed."),
                411: ("length_required", "Content-Length is required."),
                413: ("request_too_large", "The request exceeds the size limit."),
                415: ("unsupported_media_type", "Send an application/json request."),
                422: ("kernel_refused", "The exact kernel operation was refused."),
                500: ("internal_error", "The server could not complete the request."),
                501: ("not_implemented", "The method is not implemented."),
            }
            error, detail = phrases.get(code, ("http_error", "The request could not be completed."))
            self.send_json(code, {"error": error, "message": detail})

        def do_GET(self):
            path = urlsplit(self.path)
            if path.query:
                self.send_error(404)
                return
            if path.path == "/api/v1/health":
                self.send_json(200, {
                    "apiVersion": 1,
                    "kernel": "C17 cos_rat",
                    "status": "ok",
                })
                return
            if path.path == "/api/v1/papers":
                try:
                    inventory = json.loads(Path(inventory_path).read_text(encoding="utf-8"))
                except (OSError, json.JSONDecodeError):
                    self.send_error(500)
                    return
                self.send_json(200, inventory)
                return
            asset = WEB_FILES.get(path.path)
            if asset is None:
                self.send_error(404)
                return
            name, content_type = asset
            body = (WEB_ROOT / name).read_bytes()
            self.send_response(200)
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def do_POST(self):
            self.close_connection = True
            path = urlsplit(self.path)
            if path.path != "/api/v1/kernel/rational" or path.query:
                self.send_error(404)
                return
            if self.headers.get_content_type() != "application/json":
                self.send_error(415)
                return

            content_length = self.headers.get("Content-Length")
            if content_length is None:
                self.send_error(411)
                return
            if not content_length.isascii() or not content_length.isdecimal():
                self.send_error(400)
                return
            try:
                size = int(content_length)
            except ValueError:
                self.send_error(400)
                return
            if size > MAX_BODY_BYTES:
                self.send_error(413)
                return

            raw_body = self.rfile.read(size)
            if len(raw_body) != size:
                self.send_error(400)
                return
            try:
                payload = json.loads(raw_body.decode("utf-8"),
                                     object_pairs_hook=reject_duplicate_keys,
                                     parse_constant=reject_json_constant)
            except (UnicodeDecodeError, json.JSONDecodeError, ValueError):
                self.send_error(400)
                return
            if not isinstance(payload, dict) or set(payload) != {"operation", "left", "right"}:
                self.send_error(400)
                return

            operation = payload["operation"]
            left = payload["left"]
            right = payload["right"]
            if (not isinstance(operation, str) or operation not in OPERATIONS or
                    not isinstance(left, str) or not isinstance(right, str)):
                self.send_error(400)
                return
            if any("\x00" in value or len(value) > MAX_OPERAND_CHARS
                   for value in (left, right)):
                self.send_error(400)
                return
            try:
                left.encode("ascii")
                right.encode("ascii")
            except UnicodeEncodeError:
                self.send_error(400)
                return

            status, result = kernel.calculate(operation, left, right)
            if status != 0:
                self.send_json(422, {
                    "error": STATUS_NAMES.get(status, "kernel_error"),
                    "message": "The C17 kernel refused this exact operation.",
                })
                return
            self.send_json(200, {
                "apiVersion": 1,
                "operation": operation,
                "result": result,
            })

    return Handler


def create_server(kernel, port, inventory_path=None):
    if inventory_path is None:
        inventory_path = ROOT / "data" / "paper_inventory.json"
    return ThreadingHTTPServer(
        ("127.0.0.1", port),
        make_handler(kernel, inventory_path),
    )


def main():
    parser = argparse.ArgumentParser(description="Run the local Saem C17-backed dynamic site.")
    parser.add_argument("--port", type=int, default=8000)
    args = parser.parse_args()
    if args.port < 1 or args.port > 65535:
        parser.error("--port must be between 1 and 65535")

    inventory, inventory_path = write_inventory()
    print(f"Refreshed paper inventory: {inventory['sourceCount']} PDFs in {inventory_path}")

    watcher = PaperInventoryWatcher(ROOT.parent, inventory_path)
    watcher.start()
    server = None
    try:
        with tempfile.TemporaryDirectory(prefix="saem-c17-") as build_dir:
            library_path = build_kernel_library(build_dir)
            kernel = C17Kernel(library_path)
            server = create_server(kernel, args.port)
            print(f"Saem dynamic development site: http://127.0.0.1:{args.port}")
            print("Loopback-only prototype; do not expose this development server publicly.")
            server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        watcher.stop()
        if server is not None:
            server.server_close()


if __name__ == "__main__":
    main()
