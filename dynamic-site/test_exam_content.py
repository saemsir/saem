import copy
import hashlib
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from inventory_exam_papers import inventory_sources
from validate_exam_content import (
    ContractError,
    is_kernel_rational,
    question_is_mock_eligible,
    validate_dataset,
    validate_schema,
)


class ExamContentContractTests(unittest.TestCase):
    def setUp(self):
        self.root_dir = tempfile.TemporaryDirectory(prefix="saem-exam-contract-")
        self.root = Path(self.root_dir.name)
        source_path = self.root / "neet_exam" / "sample.pdf"
        source_path.parent.mkdir()
        source_path.write_bytes(b"%PDF-1.7\nverified test source")
        source_hash = hashlib.sha256(source_path.read_bytes()).hexdigest()

        self.inventory = {
            "inventoryVersion": 1,
            "ownerVerification": {
                "status": "owner_verified",
                "reviewer": "project owner",
                "reviewedOn": "2026-11-09",
                "scope": "Test fixture.",
            },
            "sourceCount": 1,
            "sources": [{
                "id": "src-sample",
                "examSeries": "neet",
                "path": "neet_exam/sample.pdf",
                "originalName": "sample.pdf",
                "mediaType": "application/pdf",
                "sizeBytes": source_path.stat().st_size,
                "sha256": source_hash,
                "machineChecks": {
                    "pdfHeaderPresentInFirst1024Bytes": True,
                    "readable": True,
                },
                "indexing": {"status": "needs_metadata"},
                "rightsStatus": "permission_confirmed",
            }],
        }
        review = {
            "status": "owner_verified",
            "reviewer": "project owner",
            "reviewedOn": "2026-11-09",
        }
        self.dataset = {
            "schemaVersion": 1,
            "datasetId": "neet-physics-test",
            "ownerVerification": {
                "status": "owner_verified",
                "reviewer": "project owner",
                "reviewedOn": "2026-11-09",
                "scope": "A test fixture verified by its creator.",
            },
            "sources": [{
                "id": "src-sample",
                "examSeries": "neet",
                "path": "neet_exam/sample.pdf",
                "sha256": source_hash,
                "rightsStatus": "permission_confirmed",
                "citation": "Local test fixture",
            }],
            "ruleProfiles": [{
                "id": "neet-physics-2026",
                "examSeries": "neet",
                "year": 2026,
                "session": "main",
                "durationSeconds": 180,
                "questionTypes": ["single_choice"],
                "scoringByType": {
                    "single_choice": {
                        "correct": "4",
                        "incorrect": "-1",
                        "unanswered": "0",
                    },
                },
                "sourceIds": ["src-sample"],
                "review": review.copy(),
            }],
            "papers": [{
                "id": "neet-physics-2026-main",
                "examSeries": "neet",
                "year": 2026,
                "session": "main",
                "subject": "physics",
                "language": {"tag": "en", "script": "Latn"},
                "sourceIds": ["src-sample"],
                "ruleProfileId": "neet-physics-2026",
                "review": review.copy(),
            }],
            "topics": [{
                "id": "mechanics",
                "label": "Mechanics",
                "subject": "physics",
                "parentId": None,
            }],
            "questions": [{
                "id": "neet-physics-2026-main-q1",
                "paperId": "neet-physics-2026-main",
                "sourceId": "src-sample",
                "sourceLocation": {"page": 1, "label": "Question 1"},
                "type": "single_choice",
                "topicIds": ["mechanics"],
                "prompt": {"sourceText": "Which option is correct?"},
                "options": [
                    {"id": "a", "text": "First option"},
                    {"id": "b", "text": "Second option"},
                ],
                "answerKey": {"choiceId": "a"},
                "solution": {
                    "steps": ["Apply the stated rule."],
                    "finalAnswer": "First option",
                    "review": review.copy(),
                },
                "review": review.copy(),
            }],
        }

    def tearDown(self):
        self.root_dir.cleanup()

    def validate(self, dataset=None, inventory=None):
        return validate_dataset(
            dataset or self.dataset,
            inventory or self.inventory,
            repository_root=self.root,
        )

    def test_valid_dataset_and_schema(self):
        self.assertTrue(self.validate())
        validate_schema(self.dataset)

    def test_paper_owner_attestation_does_not_replace_item_and_rights_gates(self):
        question = self.dataset["questions"][0]
        self.assertTrue(question_is_mock_eligible(question, self.dataset))

        pending = copy.deepcopy(self.dataset)
        pending["questions"][0]["review"] = {"status": "pending"}
        self.assertFalse(question_is_mock_eligible(
            pending["questions"][0], pending
        ))

        unreviewed_solution = copy.deepcopy(self.dataset)
        unreviewed_solution["questions"][0]["solution"]["review"] = {
            "status": "pending"
        }
        self.assertFalse(question_is_mock_eligible(
            unreviewed_solution["questions"][0], unreviewed_solution
        ))

        no_rights = copy.deepcopy(self.dataset)
        no_rights["sources"][0]["rightsStatus"] = "not_recorded"
        self.assertFalse(question_is_mock_eligible(
            no_rights["questions"][0], no_rights
        ))

        unreviewed_rules = copy.deepcopy(self.dataset)
        unreviewed_rules["ruleProfiles"][0]["review"] = {"status": "pending"}
        self.assertFalse(question_is_mock_eligible(
            unreviewed_rules["questions"][0], unreviewed_rules
        ))

    def test_scoring_values_are_canonical_and_fit_the_c17_rational_range(self):
        self.assertTrue(is_kernel_rational("3/10"))
        self.assertTrue(is_kernel_rational("-9223372036854775807"))
        self.assertFalse(is_kernel_rational("2/4"))
        self.assertFalse(is_kernel_rational("-0"))
        self.assertFalse(is_kernel_rational("9223372036854775808"))
        self.assertFalse(is_kernel_rational("1/9223372036854775808"))

        invalid = copy.deepcopy(self.dataset)
        invalid["ruleProfiles"][0]["scoringByType"]["single_choice"][
            "correct"
        ] = "2/4"
        with self.assertRaisesRegex(ContractError, "reduced rational"):
            self.validate(invalid)
    def test_schema_rejects_float_marks_wrong_answer_shape_and_extra_fields(self):
        for mutate in (
            lambda data: data["ruleProfiles"][0]["scoringByType"][
                "single_choice"
            ].update(correct=4.0),
            lambda data: data["questions"][0]["answerKey"].update(
                choiceId="missing", extra="not allowed"
            ),
            lambda data: data.update(unexpected=True),
        ):
            with self.subTest(mutate=mutate):
                invalid = copy.deepcopy(self.dataset)
                mutate(invalid)
                with self.assertRaises(ContractError):
                    self.validate(invalid)

    def test_references_and_exam_rule_profile_must_be_consistent(self):
        invalid = copy.deepcopy(self.dataset)
        invalid["papers"][0]["ruleProfileId"] = "missing-profile"
        with self.assertRaisesRegex(ContractError, "unknown rule profile"):
            self.validate(invalid)

        invalid = copy.deepcopy(self.dataset)
        invalid["ruleProfiles"][0]["year"] = 2025
        with self.assertRaisesRegex(ContractError, "does not match"):
            self.validate(invalid)

        invalid = copy.deepcopy(self.dataset)
        invalid["questions"][0]["topicIds"] = ["missing-topic"]
        with self.assertRaisesRegex(ContractError, "unknown topic"):
            self.validate(invalid)

        invalid = copy.deepcopy(self.dataset)
        invalid["topics"][0]["parentId"] = "mechanics"
        with self.assertRaisesRegex(ContractError, "parent topic cycle"):
            self.validate(invalid)

    def test_question_source_and_answer_options_must_resolve(self):
        invalid = copy.deepcopy(self.dataset)
        invalid["questions"][0]["sourceId"] = "not-on-paper"
        with self.assertRaisesRegex(ContractError, "not attached to paper"):
            self.validate(invalid)

        invalid = copy.deepcopy(self.dataset)
        invalid["questions"][0]["answerKey"]["choiceId"] = "missing"
        with self.assertRaisesRegex(ContractError, "unknown option"):
            self.validate(invalid)

    def test_paths_hashes_and_machine_invalid_pdfs_are_rejected(self):
        invalid = copy.deepcopy(self.dataset)
        invalid["sources"][0]["path"] = "../outside.pdf"
        with self.assertRaises(ContractError):
            self.validate(invalid)

        invalid = copy.deepcopy(self.dataset)
        invalid["sources"][0]["sha256"] = "0" * 64
        with self.assertRaises(ContractError):
            self.validate(invalid)

        invalid_inventory = copy.deepcopy(self.inventory)
        invalid_inventory["sources"][0]["machineChecks"][
            "pdfHeaderPresentInFirst1024Bytes"
        ] = False
        with self.assertRaisesRegex(ContractError, "found no PDF header"):
            self.validate(inventory=invalid_inventory)

        source = self.root / "neet_exam" / "sample.pdf"
        source.write_bytes(b"%PDF-1.7\nmodified after indexing")
        with self.assertRaisesRegex(ContractError, "changed since inventory"):
            self.validate()

    def test_duplicate_ids_and_duplicate_options_are_rejected(self):
        invalid = copy.deepcopy(self.dataset)
        invalid["questions"].append(copy.deepcopy(invalid["questions"][0]))
        with self.assertRaisesRegex(ContractError, "duplicate record IDs"):
            self.validate(invalid)

        invalid = copy.deepcopy(self.dataset)
        invalid["questions"][0]["options"][1]["id"] = "a"
        with self.assertRaisesRegex(ContractError, "duplicate option IDs"):
            self.validate(invalid)

    def test_new_pdfs_are_picked_up_on_inventory_refresh(self):
        (self.root / "jee_exams").mkdir()
        (self.root / "jeeadv_exams").mkdir()
        new_paper = self.root / "jee_exams" / "new-paper.pdf"
        new_paper.write_bytes(b"%PDF-1.7\nnew source")

        inventory = inventory_sources(self.root)

        self.assertEqual(inventory["sourceCount"], 2)
        discovered = next(
            source for source in inventory["sources"]
            if source["path"] == "jee_exams/new-paper.pdf"
        )
        self.assertTrue(discovered["machineChecks"]["pdfHeaderPresentInFirst1024Bytes"])
        self.assertEqual(discovered["rightsStatus"], "not_recorded")

    def test_real_corpus_inventory_is_machine_checked_without_editing_sources(self):
        root = Path(__file__).resolve().parents[1]
        inventory = inventory_sources(root)
        self.assertEqual(inventory["sourceCount"], 202)
        invalid = [
            item["path"] for item in inventory["sources"]
            if not item["machineChecks"]["pdfHeaderPresentInFirst1024Bytes"]
        ]
        self.assertEqual(invalid, [])
        self.assertTrue(all(item["rightsStatus"] == "not_recorded"
                            for item in inventory["sources"]))
        by_path = {item["path"]: item for item in inventory["sources"]}
        self.assertEqual(
            by_path["jeeadv_exams/2007_1.pdf"]["indexing"]["year"],
            2007,
        )
        self.assertEqual(
            by_path["jeeadv_exams/2007_1.pdf"]["indexing"]["session"],
            "paper_1",
        )
        self.assertIsNone(
            by_path["jee_exams/que_1733310643.pdf"]["indexing"]["year"]
        )
        self.assertEqual(
            by_path["neet_exam/NEET 2021 - Physics_With_Solution_P2_H.pdf"][
                "indexing"
            ]["subject"],
            "physics",
        )
        self.assertEqual(
            by_path["neet_exam/NEET 2021 - Physics_With_Solution_P2_H.pdf"][
                "indexing"
            ]["status"],
            "filename_hints_only",
        )


if __name__ == "__main__":
    unittest.main()
