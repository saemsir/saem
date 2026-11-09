import json
import tempfile
import threading
import time
import unittest
import urllib.error
import urllib.request
from pathlib import Path

from server import (
    C17Kernel,
    PaperInventoryWatcher,
    build_kernel_library,
    create_server,
)
from inventory_exam_papers import inventory_sources, write_inventory


class DynamicKernelSiteTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.build_dir = tempfile.TemporaryDirectory(prefix="saem-test-c17-")
        library = build_kernel_library(cls.build_dir.name)
        cls.kernel = C17Kernel(library)
        cls.server = create_server(cls.kernel, 0)
        cls.thread = threading.Thread(target=cls.server.serve_forever, daemon=True)
        cls.thread.start()
        cls.base_url = f"http://127.0.0.1:{cls.server.server_port}"

    @classmethod
    def tearDownClass(cls):
        cls.server.shutdown()
        cls.server.server_close()
        cls.thread.join(timeout=2)
        cls.build_dir.cleanup()

    def request(self, path, method="GET", body=None, content_type="application/json"):
        data = body
        headers = {}
        if data is not None:
            headers["Content-Type"] = content_type
        request = urllib.request.Request(
            self.base_url + path, data=data, headers=headers, method=method
        )
        try:
            response = urllib.request.urlopen(request, timeout=3)
        except urllib.error.HTTPError as error:
            response = error
        return response.status, response.headers, response.read()

    def post_json(self, payload):
        return self.request(
            "/api/v1/kernel/rational",
            method="POST",
            body=json.dumps(payload).encode("utf-8"),
        )

    def test_exact_kernel_result(self):
        status, headers, body = self.post_json({
            "operation": "add",
            "left": "0.1",
            "right": "0.2",
        })
        self.assertEqual(status, 200)
        self.assertEqual(headers.get_content_type(), "application/json")
        self.assertEqual(json.loads(body)["result"], "3/10")

    def test_all_rational_operations(self):
        cases = [
            ("subtract", "1", "0.75", "1/4"),
            ("multiply", "0.2", "0.5", "1/10"),
            ("divide", "1", "8", "1/8"),
        ]
        for operation, left, right, expected in cases:
            with self.subTest(operation=operation):
                status, _, body = self.post_json({
                    "operation": operation,
                    "left": left,
                    "right": right,
                })
                self.assertEqual(status, 200)
                self.assertEqual(json.loads(body)["result"], expected)

    def test_kernel_refusal_is_explicit(self):
        status, _, body = self.post_json({
            "operation": "divide",
            "left": "1",
            "right": "0",
        })
        self.assertEqual(status, 422)
        self.assertEqual(json.loads(body)["error"], "invalid")

        status, _, body = self.post_json({
            "operation": "add",
            "left": "9223372036854775807",
            "right": "1",
        })
        self.assertEqual(status, 422)
        self.assertEqual(json.loads(body)["error"], "range")

    def test_invalid_request_shapes_and_operands_are_rejected(self):
        invalid_payloads = [
            [],
            {"operation": "add", "left": "1"},
            {"operation": "power", "left": "2", "right": "3"},
            {"operation": "add", "left": 1, "right": "2"},
            {"operation": "add", "left": "1\u00002", "right": "1"},
            {"operation": "add", "left": "1" * 65, "right": "1"},
        ]
        for payload in invalid_payloads:
            with self.subTest(payload=payload):
                status, _, _ = self.post_json(payload)
                self.assertEqual(status, 400)
        status, _, body = self.post_json({
            "operation": "add",
            "left": "1/2",
            "right": "1",
        })
        self.assertEqual(status, 422)
        self.assertEqual(json.loads(body)["error"], "invalid")

    def test_duplicate_json_keys_are_rejected(self):
        status, _, body = self.request(
            "/api/v1/kernel/rational",
            method="POST",
            body=b'{"operation":"add","operation":"divide","left":"1","right":"2"}',
        )
        self.assertEqual(status, 400)
        self.assertEqual(json.loads(body)["error"], "bad_request")

    def test_body_size_and_media_type_are_bounded(self):
        status, _, body = self.request(
            "/api/v1/kernel/rational",
            method="POST",
            body=b" " * 2049,
        )
        self.assertEqual(status, 413)
        self.assertEqual(json.loads(body)["error"], "request_too_large")

        status, _, _ = self.request(
            "/api/v1/kernel/rational",
            method="POST",
            body=b"{}",
            content_type="text/plain",
        )
        self.assertEqual(status, 415)

    def test_health_and_dynamic_page_are_served_with_safety_headers(self):
        status, headers, body = self.request("/api/v1/health")
        self.assertEqual(status, 200)
        self.assertEqual(json.loads(body)["apiVersion"], 1)
        self.assertIn("default-src 'self'", headers["Content-Security-Policy"])

        status, headers, body = self.request("/")
        self.assertEqual(status, 200)
        self.assertEqual(headers.get_content_type(), "text/html")
        self.assertIn(b"Kernel lab", body)

    def test_paper_inventory_endpoint_returns_current_manifest(self):
        status, headers, body = self.request("/api/v1/papers")
        self.assertEqual(status, 200)
        self.assertEqual(headers.get_content_type(), "application/json")
        inventory = json.loads(body)
        self.assertEqual(inventory["sourceCount"], 202)
        self.assertTrue(all(
            source["ownerVerification"]["status"] == "owner_verified"
            for source in inventory["sources"]
        ))

    def test_server_can_bind_to_explicit_cloud_runtime_host(self):
        server = create_server(self.kernel, 0, host="0.0.0.0")
        try:
            self.assertEqual(server.server_address[0], "0.0.0.0")
        finally:
            server.server_close()

    def test_unknown_routes_and_query_variants_are_not_served(self):
        for path in ("/missing", "/api/v1/health?debug=1", "/../server.py"):
            with self.subTest(path=path):
                status, _, _ = self.request(path)
                self.assertEqual(status, 404)


class PaperInventoryWatcherTests(unittest.TestCase):
    def test_watcher_refreshes_inventory_when_papers_are_added_or_removed(self):
        with tempfile.TemporaryDirectory(prefix="saem-paper-watcher-") as directory:
            root = Path(directory)
            for folder in ("neet_exam", "jee_exams", "jeeadv_exams"):
                (root / folder).mkdir()
            existing = root / "neet_exam" / "existing.pdf"
            existing.write_bytes(b"%PDF-1.7\nexisting")
            output = root / "paper_inventory.json"
            original_source = inventory_sources(root)["sources"][0]
            output.write_text(json.dumps({
                "ownerVerification": {"status": "owner_verified"},
                "sources": [original_source],
            }), encoding="utf-8")
            initial, _ = write_inventory(root, output)
            self.assertEqual(initial["sourceCount"], 1)
            self.assertEqual(
                initial["sources"][0]["ownerVerification"]["status"],
                "owner_verified",
            )

            watcher = PaperInventoryWatcher(root, output, interval=0.05)
            watcher.start()
            try:
                added = root / "jee_exams" / "new.pdf"
                added.write_bytes(b"%PDF-1.7\nnew")
                deadline = time.monotonic() + 3
                while time.monotonic() < deadline:
                    current = json.loads(output.read_text(encoding="utf-8"))
                    if current["sourceCount"] == 2:
                        break
                    time.sleep(0.02)
                self.assertEqual(current["sourceCount"], 2)
                by_name = {source["originalName"]: source for source in current["sources"]}
                self.assertEqual(
                    by_name["existing.pdf"]["ownerVerification"]["status"],
                    "owner_verified",
                )
                self.assertEqual(
                    by_name["new.pdf"]["ownerVerification"]["status"],
                    "not_reviewed",
                )

                existing.unlink()
                deadline = time.monotonic() + 3
                while time.monotonic() < deadline:
                    current = json.loads(output.read_text(encoding="utf-8"))
                    if current["sourceCount"] == 1:
                        break
                    time.sleep(0.02)
                self.assertEqual(current["sourceCount"], 1)
                self.assertEqual(current["sources"][0]["rightsStatus"], "not_recorded")
                self.assertEqual(
                    current["sources"][0]["ownerVerification"]["status"],
                    "not_reviewed",
                )
            finally:
                watcher.stop()


if __name__ == "__main__":
    unittest.main()
