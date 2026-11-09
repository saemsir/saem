import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class ExamCorpusDataTests(unittest.TestCase):
    def test_static_corpus_accounts_for_every_inventory_hash_without_question_text(self):
        inventory = json.loads(
            (ROOT / "dynamic-site" / "data" / "paper_inventory.json").read_text(encoding="utf-8")
        )
        corpus = json.loads((ROOT / "data" / "exam-corpus.json").read_text(encoding="utf-8"))
        self.assertEqual(corpus["sourceCount"], inventory["sourceCount"])
        self.assertEqual(len(corpus["papers"]), inventory["sourceCount"])
        self.assertEqual(corpus["indexMethodVersion"], 1)
        self.assertFalse(corpus["topicMethod"]["storesExtractedQuestionText"])

        inventory_by_id = {source["id"]: source for source in inventory["sources"]}
        corpus_ids = set()
        for paper in corpus["papers"]:
            self.assertNotIn(paper["id"], corpus_ids)
            corpus_ids.add(paper["id"])
            source = inventory_by_id[paper["id"]]
            self.assertEqual(paper["sha256"], source["sha256"])
            self.assertEqual(paper["examSeries"], source["examSeries"])
            self.assertEqual(
                paper["ownerVerificationStatus"],
                source.get("ownerVerification", {}).get("status", "not_reviewed"),
            )
            self.assertEqual(paper["topicReviewStatus"], "machine_candidates_unreviewed")
            self.assertNotIn("text", paper)
            self.assertNotIn("questions", paper)
        self.assertEqual(corpus_ids, set(inventory_by_id))

    def test_series_summaries_cover_all_indexed_paper_files(self):
        corpus = json.loads((ROOT / "data" / "exam-corpus.json").read_text(encoding="utf-8"))
        self.assertEqual(
            sum(summary["paperFiles"] for summary in corpus["summary"].values()),
            corpus["sourceCount"],
        )


if __name__ == "__main__":
    unittest.main()
