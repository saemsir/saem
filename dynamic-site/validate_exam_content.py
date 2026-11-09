#!/usr/bin/env python3
"""Validate a versioned exam content file against its schema and source corpus."""

import argparse
import hashlib
import json
import re
from fractions import Fraction
from pathlib import Path, PurePosixPath

from jsonschema import Draft202012Validator, FormatChecker

from inventory_exam_papers import ROOT


SCHEMA_PATH = Path(__file__).resolve().parent / "data" / "exam-content.schema.json"
MAX_RATIONAL_COMPONENT = (1 << 63) - 1
RATIONAL_TEXT = re.compile(r"-?(?:0|[1-9][0-9]*)(?:/[1-9][0-9]*)?\Z")


class ContractError(ValueError):
    pass


def load_json(path):
    with Path(path).open("r", encoding="utf-8") as file:
        return json.load(file)


def validate_schema(instance, schema_path=SCHEMA_PATH):
    schema = load_json(schema_path)
    Draft202012Validator.check_schema(schema)
    validator = Draft202012Validator(schema, format_checker=FormatChecker())
    errors = sorted(validator.iter_errors(instance), key=lambda error: list(map(str, error.absolute_path)))
    if errors:
        details = []
        for error in errors:
            location = "/" + "/".join(str(part) for part in error.absolute_path)
            details.append(f"{location}: {error.message}")
        raise ContractError("\n".join(details))


def validate_references(dataset):
    errors = []
    sources = {source["id"]: source for source in dataset["sources"]}
    profiles = {profile["id"]: profile for profile in dataset["ruleProfiles"]}
    papers = {paper["id"]: paper for paper in dataset["papers"]}
    topics = {topic["id"]: topic for topic in dataset["topics"]}

    for collection_name, records in (
        ("sources", dataset["sources"]),
        ("ruleProfiles", dataset["ruleProfiles"]),
        ("papers", dataset["papers"]),
        ("topics", dataset["topics"]),
        ("questions", dataset["questions"]),
    ):
        ids = [record["id"] for record in records]
        if len(ids) != len(set(ids)):
            errors.append(f"{collection_name}: duplicate record IDs")

    for profile in dataset["ruleProfiles"]:
        for source_id in profile["sourceIds"]:
            source = sources.get(source_id)
            if source is None:
                errors.append(f"ruleProfiles/{profile['id']}: unknown source {source_id}")
            elif source["examSeries"] != profile["examSeries"]:
                errors.append(f"ruleProfiles/{profile['id']}: source belongs to a different exam series")
        if set(profile["questionTypes"]) != set(profile["scoringByType"]):
            errors.append(f"ruleProfiles/{profile['id']}: scoring types must exactly match questionTypes")
        for question_type, marks in profile["scoringByType"].items():
            for mark_name, mark_text in marks.items():
                if not is_kernel_rational(mark_text):
                    errors.append(
                        f"ruleProfiles/{profile['id']}/{question_type}/{mark_name}: "
                        "mark is not a reduced rational supported by CosRat"
                    )

    for paper in dataset["papers"]:
        for source_id in paper["sourceIds"]:
            source = sources.get(source_id)
            if source is None:
                errors.append(f"papers/{paper['id']}: unknown source {source_id}")
            elif source["examSeries"] != paper["examSeries"]:
                errors.append(f"papers/{paper['id']}: source belongs to a different exam series")
        profile = profiles.get(paper["ruleProfileId"])
        if profile is None:
            errors.append(f"papers/{paper['id']}: unknown rule profile {paper['ruleProfileId']}")
        elif (
            profile["examSeries"] != paper["examSeries"]
            or profile["year"] != paper["year"]
            or profile["session"] != paper["session"]
        ):
            errors.append(f"papers/{paper['id']}: rule profile does not match exam/year/session")

    for question in dataset["questions"]:
        paper = papers.get(question["paperId"])
        if paper is None:
            errors.append(f"questions/{question['id']}: unknown paper {question['paperId']}")
        elif question["sourceId"] not in paper["sourceIds"]:
            errors.append(
                f"questions/{question['id']}: source is not attached to paper {question['paperId']}"
            )
        if question["sourceId"] not in sources:
            errors.append(f"questions/{question['id']}: unknown source {question['sourceId']}")
        for topic_id in question["topicIds"]:
            if topic_id not in topics:
                errors.append(f"questions/{question['id']}: unknown topic {topic_id}")
        if paper is not None:
            profile = profiles.get(paper["ruleProfileId"])
            if profile and question["type"] not in profile["questionTypes"]:
                errors.append(
                    f"questions/{question['id']}: question type is not supported by paper rule profile"
                )
            if any(
                topics.get(topic_id, {}).get("subject") != paper["subject"]
                for topic_id in question["topicIds"]
            ):
                errors.append(
                    f"questions/{question['id']}: topic subject does not match the paper subject"
                )
        options = question.get("options", [])
        option_ids = [option["id"] for option in options]
        if len(option_ids) != len(set(option_ids)):
            errors.append(f"questions/{question['id']}: duplicate option IDs")
        answer_key = question["answerKey"]
        answer_ids = []
        if "choiceId" in answer_key:
            answer_ids.append(answer_key["choiceId"])
        if "choiceIds" in answer_key:
            answer_ids.extend(answer_key["choiceIds"])
        for pair in answer_key.get("pairs", []):
            answer_ids.extend((pair["leftId"], pair["rightId"]))
        for answer_id in answer_ids:
            if answer_id not in option_ids:
                errors.append(f"questions/{question['id']}: answer references unknown option {answer_id}")
        if question["type"] == "numeric":
            value = answer_key.get("value")
            if not is_kernel_rational(value):
                errors.append(
                    f"questions/{question['id']}: numeric answer is not a reduced CosRat value"
                )

    for topic in dataset["topics"]:
        parent_id = topic.get("parentId")
        if parent_id is not None and parent_id not in topics:
            errors.append(f"topics/{topic['id']}: unknown parent topic {parent_id}")
        visited = set()
        current_id = topic["id"]
        while current_id in topics:
            if current_id in visited:
                errors.append(f"topics/{topic['id']}: parent topic cycle")
                break
            visited.add(current_id)
            current_id = topics[current_id].get("parentId")
            if current_id is None:
                break

    if errors:
        raise ContractError("\n".join(errors))


def is_kernel_rational(value):
    if not isinstance(value, str) or not RATIONAL_TEXT.fullmatch(value):
        return False
    try:
        fraction = Fraction(value)
    except (ValueError, ZeroDivisionError):
        return False
    return (
        str(fraction) == value
        and abs(fraction.numerator) <= MAX_RATIONAL_COMPONENT
        and fraction.denominator <= MAX_RATIONAL_COMPONENT
    )


def validate_sources_against_inventory(dataset, inventory, repository_root=ROOT):
    manifest_sources = {source["id"]: source for source in inventory["sources"]}
    errors = []
    root = Path(repository_root).resolve()
    for source in dataset["sources"]:
        manifest_source = manifest_sources.get(source["id"])
        if manifest_source is None:
            errors.append(f"sources/{source['id']}: source is not in the supplied inventory")
            continue
        if source["path"] != manifest_source["path"]:
            errors.append(f"sources/{source['id']}: path does not match the inventory")
            continue
        if source["sha256"] != manifest_source["sha256"]:
            errors.append(f"sources/{source['id']}: SHA-256 does not match the inventory")
            continue
        if not manifest_source["machineChecks"]["pdfHeaderPresentInFirst1024Bytes"]:
            errors.append(f"sources/{source['id']}: inventory found no PDF header")
            continue
        path = PurePosixPath(source["path"])
        if path.is_absolute() or ".." in path.parts:
            errors.append(f"sources/{source['id']}: path must stay inside the repository")
            continue
        resolved = (root / Path(*path.parts)).resolve()
        if root not in resolved.parents or not resolved.is_file():
            errors.append(f"sources/{source['id']}: source file is missing or escapes the repository")
            continue
        digest = hashlib.sha256()
        with resolved.open("rb") as source_file:
            for chunk in iter(lambda: source_file.read(1024 * 1024), b""):
                digest.update(chunk)
        if digest.hexdigest() != source["sha256"]:
            errors.append(f"sources/{source['id']}: source file changed since inventory generation")
            continue
        if source["rightsStatus"] == "permission_confirmed" and not manifest_source.get(
            "rightsStatus"
        ) == "permission_confirmed":
            errors.append(
                f"sources/{source['id']}: dataset cannot promote unrecorded inventory permissions"
            )
    if errors:
        raise ContractError("\n".join(errors))


def validate_dataset(dataset, inventory, repository_root=ROOT, schema_path=SCHEMA_PATH):
    validate_schema(dataset, schema_path)
    validate_references(dataset)
    validate_sources_against_inventory(dataset, inventory, repository_root)
    return True


def question_is_mock_eligible(question, dataset):
    papers = {paper["id"]: paper for paper in dataset["papers"]}
    sources = {source["id"]: source for source in dataset["sources"]}
    profiles = {profile["id"]: profile for profile in dataset["ruleProfiles"]}
    paper = papers.get(question.get("paperId"))
    if paper is None or paper["review"]["status"] != "owner_verified":
        return False
    profile = profiles.get(paper["ruleProfileId"])
    if profile is None or profile["review"]["status"] != "owner_verified":
        return False
    if question.get("review", {}).get("status") != "owner_verified":
        return False
    if question.get("solution", {}).get("review", {}).get("status") != "owner_verified":
        return False
    source = sources.get(question.get("sourceId"))
    if source is None or source["rightsStatus"] != "permission_confirmed":
        return False
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dataset", type=Path)
    parser.add_argument(
        "--inventory",
        type=Path,
        default=ROOT / "dynamic-site" / "data" / "paper_inventory.json",
    )
    args = parser.parse_args()
    try:
        dataset = load_json(args.dataset)
        inventory = load_json(args.inventory)
        validate_dataset(dataset, inventory)
    except (OSError, json.JSONDecodeError, KeyError, TypeError, ContractError) as error:
        parser.exit(1, f"Exam content validation failed: {error}\n")
    print(
        f"Valid exam-content v{dataset['schemaVersion']}: "
        f"{len(dataset['papers'])} papers, {len(dataset['questions'])} questions."
    )
    print("No record is mock-eligible unless paper, question, solution, and source permissions pass.")


if __name__ == "__main__":
    main()
