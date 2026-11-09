#!/usr/bin/env python3
"""Create a deterministic, non-destructive manifest for the supplied PDFs."""

import argparse
import hashlib
import json
import re
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
EXAM_FOLDERS = {
    "neet_exam": "neet",
    "jee_exams": "jee_main",
    "jeeadv_exams": "jee_advanced",
}
OWNER_REVIEWED_ON = "2026-11-09"
YEAR_PATTERN = re.compile(r"(?<!\d)((?:19|20)\d{2})(?!\d)")
SUBJECTS = ("physics", "chemistry", "mathematics", "biology")


def filename_hints(path, exam_series):
    name = path.name
    year_match = YEAR_PATTERN.search(name)
    year_hint = int(year_match.group(1)) if year_match else None
    session_hint = None
    if exam_series == "jee_advanced":
        paper_match = re.search(r"_(1|2)(?:_English)?\.pdf\Z", name, re.IGNORECASE)
        if paper_match:
            session_hint = f"paper_{paper_match.group(1)}"
    subject_hint = next(
        (subject for subject in SUBJECTS
         if re.search(rf"(?<![A-Za-z]){subject}(?![A-Za-z])", name, re.IGNORECASE)),
        None,
    )
    language_hint = "en" if re.search(r"(?<![A-Za-z])English(?![A-Za-z])", name, re.IGNORECASE) else None
    code_match = re.search(r"\bCode\s+([A-Za-z0-9-]+)", name, re.IGNORECASE)
    return {
        "year": year_hint,
        "session": session_hint,
        "subject": subject_hint,
        "language": language_hint,
        "paperCode": code_match.group(1) if code_match else None,
        "status": "filename_hints_only",
    }


def inventory_sources(root=ROOT, previous_inventory=None):
    sources = []
    seen_ids = set()
    previous_sources = {
        source["id"]: source
        for source in (previous_inventory or {}).get("sources", [])
        if isinstance(source, dict) and isinstance(source.get("id"), str)
    }
    previous_owner_verification = (previous_inventory or {}).get(
        "ownerVerification", {}
    )
    for folder, exam_series in EXAM_FOLDERS.items():
        exam_dir = Path(root) / folder
        if not exam_dir.is_dir():
            raise FileNotFoundError(f"Exam source folder is missing: {folder}")
        for path in sorted(exam_dir.iterdir(), key=lambda item: item.name.casefold()):
            if path.is_symlink():
                raise ValueError(f"Symbolic links are not allowed in the exam corpus: {path}")
            if not path.is_file() or path.suffix.casefold() != ".pdf":
                continue
            relative_path = path.relative_to(root).as_posix()
            digest = hashlib.sha256()
            byte_count = 0
            header_window = bytearray()
            with path.open("rb") as source_file:
                while True:
                    chunk = source_file.read(1024 * 1024)
                    if not chunk:
                        break
                    if len(header_window) < 1024:
                        header_window.extend(chunk[:1024 - len(header_window)])
                    digest.update(chunk)
                    byte_count += len(chunk)
            sha256 = digest.hexdigest()
            source_id = f"src-{hashlib.sha256(relative_path.encode('utf-8')).hexdigest()[:20]}"
            if source_id in seen_ids:
                raise ValueError(f"Duplicate source ID/hash encountered: {relative_path}")
            seen_ids.add(source_id)
            previous_source = previous_sources.get(source_id)
            previously_attested = (
                previous_source is not None
                and previous_source.get("sha256") == sha256
                and (
                    (previous_source.get("ownerVerification") or {}).get("status")
                    == "owner_verified"
                    or previous_owner_verification.get("status") == "owner_verified"
                )
            )
            previous_attestation = (previous_source or {}).get("ownerVerification") or {}
            if previously_attested:
                owner_verification = {
                    "status": "owner_verified",
                    "reviewer": previous_attestation.get(
                        "reviewer", previous_owner_verification.get("reviewer", "project owner")
                    ),
                    "reviewedOn": previous_attestation.get(
                        "reviewedOn", previous_owner_verification.get("reviewedOn")
                    ),
                }
            else:
                owner_verification = {"status": "not_reviewed"}
            sources.append({
                "id": source_id,
                "examSeries": exam_series,
                "path": relative_path,
                "originalName": path.name,
                "mediaType": "application/pdf",
                "sizeBytes": byte_count,
                "sha256": sha256,
                "machineChecks": {
                    "pdfHeaderPresentInFirst1024Bytes": b"%PDF-" in header_window,
                    "readable": True,
                },
                "indexing": filename_hints(path, exam_series),
                "rightsStatus": "not_recorded",
                "ownerVerification": owner_verification,
            })
    return {
        "inventoryVersion": 1,
        "ownerVerification": {
            "status": "source_specific",
            "reviewer": "project owner",
            "reviewedOn": OWNER_REVIEWED_ON,
            "scope": (
                "Owner verification is recorded per source. Existing sources "
                "retain a prior paper-level attestation only while their "
                "file hashes match; newly discovered sources are not "
                "implicitly attested. This is separate from extraction "
                "mapping, metadata, and redistribution permissions."
            ),
        },
        "sourceCount": len(sources),
        "sources": sources,
    }


def write_inventory(root=ROOT, output=None):
    if output is None:
        output = Path(root) / "dynamic-site" / "data" / "paper_inventory.json"
    output = Path(output)
    previous_inventory = None
    if output.is_file():
        previous_inventory = json.loads(output.read_text(encoding="utf-8"))
    inventory = inventory_sources(root, previous_inventory)
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary_path = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="w",
            encoding="utf-8",
            dir=output.parent,
            prefix=f".{output.name}.",
            suffix=".tmp",
            delete=False,
        ) as temporary_file:
            temporary_path = Path(temporary_file.name)
            temporary_file.write(json.dumps(inventory, ensure_ascii=False, indent=2) + "\n")
        temporary_path.replace(output)
    finally:
        if temporary_path is not None:
            temporary_path.unlink(missing_ok=True)
    return inventory, output


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output",
        type=Path,
        default=ROOT / "dynamic-site" / "data" / "paper_inventory.json",
        help="Output manifest path (default: dynamic-site/data/paper_inventory.json)",
    )
    args = parser.parse_args()
    inventory, output = write_inventory(ROOT, args.output)
    invalid = [
        source["path"] for source in inventory["sources"]
        if not source["machineChecks"]["pdfHeaderPresentInFirst1024Bytes"]
    ]
    print(f"Recorded {inventory['sourceCount']} source files in {output}")
    print(f"PDF header failures: {len(invalid)}")
    for path in invalid:
        print(f"  {path}")
    print("Metadata and redistribution permissions remain unrecorded; originals were not changed.")


if __name__ == "__main__":
    main()
