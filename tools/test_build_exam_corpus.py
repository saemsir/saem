import hashlib
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from build_exam_corpus import build_corpus, detect_topics, detect_year


class ExamCorpusIndexerTests(unittest.TestCase):
    def test_year_uses_filename_hint_before_title_candidate(self):
        self.assertEqual(
            detect_year(2024, "jee_main", "JEE Main Online Exam 2020"),
            (2024, "filename_hint"),
        )

    def test_year_extracts_exam_title_but_refuses_ambiguous_matches(self):
        self.assertEqual(
            detect_year(None, "jee_main", "JEE Main Online Paper Examination 2020"),
            (2020, "title_text_candidate"),
        )
        self.assertEqual(
            detect_year(None, "neet", "NEET Exam 2020. AIPMT 2015."),
            (None, "needs_mapping"),
        )
        self.assertEqual(detect_year(None, "jee_advanced", "scanned paper"), (None, "needs_mapping"))

    def test_topics_are_only_emitted_after_distinct_keyword_matches(self):
        text = (
            "Electric field and electric potential in a capacitor. "
            "Gauss law gives the electric field for this charge distribution."
        )
        signals = detect_topics(text)
        electrostatics = next(item for item in signals if item["label"] == "Electrostatics")
        self.assertGreaterEqual(electrostatics["keywordMatches"], 2)
        self.assertEqual(detect_topics("A single capacitor"), [])

    def test_unchanged_sources_reuse_cached_index_and_refresh_review_status(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source_path = root / "neet_exam" / "paper.pdf"
            source_path.parent.mkdir()
            source_path.write_bytes(b"unchanged source")
            digest = hashlib.sha256(source_path.read_bytes()).hexdigest()
            source = {
                "id": "paper-1",
                "examSeries": "neet",
                "path": "neet_exam/paper.pdf",
                "originalName": "paper.pdf",
                "sha256": digest,
                "indexing": {"year": 2024},
                "ownerVerification": {"status": "not_reviewed"},
            }
            cached_record = {
                "id": "paper-1",
                "examSeries": "neet",
                "sourceName": "paper.pdf",
                "sha256": digest,
                "ownerVerificationStatus": "owner_verified",
                "year": 2024,
                "yearSource": "filename_hint",
                "pages": 12,
                "searchableCharacters": 500,
                "extractionStatus": "text_extracted",
                "topicReviewStatus": "machine_candidates_unreviewed",
                "topicSignals": [],
            }

            corpus = build_corpus(
                root,
                {"inventoryVersion": 1, "sources": [source]},
                {
                    "corpusVersion": 1,
                    "indexMethodVersion": 1,
                    "topicMethod": {"status": "heuristic_unreviewed"},
                    "papers": [cached_record],
                },
            )

        self.assertEqual(corpus["papers"][0]["pages"], 12)
        self.assertEqual(corpus["papers"][0]["ownerVerificationStatus"], "not_reviewed")


if __name__ == "__main__":
    unittest.main()
