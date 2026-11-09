#!/usr/bin/env python3
"""Build a text-free, heuristic topic index from the supplied exam PDFs."""

import argparse
import hashlib
import json
import logging
import re
import warnings
from pathlib import Path, PurePosixPath

from pypdf import PdfReader


ROOT = Path(__file__).resolve().parents[1]
INVENTORY_PATH = ROOT / "dynamic-site" / "data" / "paper_inventory.json"
OUTPUT_PATH = ROOT / "data" / "exam-corpus.json"
MIN_TOPIC_KEYWORD_MATCHES = 2
MAX_SOURCE_BYTES = 100 * 1024 * 1024
INDEX_METHOD_VERSION = 1

TOPIC_KEYWORDS = {
    "physics": {
        "Mechanics": ("kinematics", "velocity", "acceleration", "projectile", "newton", "friction"),
        "Work, energy & power": ("work energy theorem", "kinetic energy", "potential energy", "power"),
        "Rotational motion": ("torque", "angular momentum", "moment of inertia", "rolling motion"),
        "Gravitation": ("gravitational field", "gravitational potential", "escape velocity", "kepler"),
        "Properties of matter": ("surface tension", "viscosity", "bernoulli", "elasticity", "young modulus"),
        "Thermal physics": ("thermodynamics", "heat capacity", "specific heat", "carnot", "ideal gas"),
        "Oscillations & waves": ("simple harmonic", "oscillation", "standing wave", "doppler", "wavelength"),
        "Electrostatics": ("electric field", "electric potential", "gauss law", "capacitor", "coulomb"),
        "Current electricity": ("ohm law", "kirchhoff", "resistance", "resistivity", "electric current"),
        "Magnetism & induction": ("magnetic field", "lorentz force", "faraday law", "inductance", "magnetic flux"),
        "Optics": ("lens", "mirror", "interference", "diffraction", "polarisation", "refractive index"),
        "Modern physics": ("photoelectric", "de broglie", "radioactive", "nuclear", "semiconductor", "binding energy"),
    },
    "chemistry": {
        "Atomic structure": ("atomic structure", "quantum number", "electronic configuration", "bohr model"),
        "Chemical bonding": ("chemical bonding", "hybridisation", "molecular orbital", "bond angle", "vsepr"),
        "Chemical thermodynamics": ("enthalpy", "entropy", "gibbs energy", "thermodynamic", "heat of formation"),
        "Equilibrium": ("chemical equilibrium", "ionic equilibrium", "buffer solution", "solubility product", "ph"),
        "Electrochemistry": ("electrochemical", "nernst equation", "electrode potential", "electrolysis", "galvanic cell"),
        "Chemical kinetics": ("chemical kinetics", "rate constant", "half life", "activation energy", "reaction rate"),
        "Solutions": ("colligative", "raoult law", "osmotic pressure", "molarity", "molality"),
        "Coordination compounds": ("coordination compound", "ligand", "coordination number", "crystal field"),
        "Periodic properties": ("periodic table", "ionization enthalpy", "electron affinity", "electronegativity"),
        "Organic reactions": ("nucleophilic", "electrophilic", "substitution reaction", "elimination reaction", "oxidation"),
        "Hydrocarbons": ("alkane", "alkene", "alkyne", "aromatic", "benzene"),
        "Biomolecules & polymers": ("amino acid", "carbohydrate", "protein", "polymer", "nucleic acid"),
    },
    "mathematics": {
        "Algebra": ("quadratic equation", "inequality", "binomial", "complex number", "polynomial"),
        "Sequences & series": ("arithmetic progression", "geometric progression", "sequence and series", "sum of n terms"),
        "Matrices & determinants": ("matrix", "determinant", "inverse matrix", "eigenvalue"),
        "Calculus": ("derivative", "differentiation", "integration", "definite integral", "continuity"),
        "Differential equations": ("differential equation", "order and degree", "particular solution"),
        "Coordinate geometry": ("straight line", "circle", "parabola", "ellipse", "hyperbola"),
        "Vectors & 3D geometry": ("scalar product", "vector product", "direction cosines", "three dimensional", "skew lines"),
        "Trigonometry": ("trigonometric", "sine rule", "cosine rule", "inverse trigonometric"),
        "Probability": ("probability", "conditional probability", "bayes theorem", "random variable"),
        "Statistics & counting": ("standard deviation", "mean deviation", "permutation", "combination"),
    },
    "biology": {
        "Cell biology": ("cell cycle", "cell organelle", "plasma membrane", "mitosis", "meiosis"),
        "Plant physiology": ("photosynthesis", "transpiration", "mineral nutrition", "plant growth"),
        "Human physiology": ("cardiac cycle", "nephron", "respiration", "nervous system", "hormone"),
        "Reproduction": ("reproduction", "menstrual cycle", "fertilisation", "pollination", "embryo"),
        "Genetics & evolution": ("mendel", "inheritance", "gene expression", "natural selection", "evolution"),
        "Biotechnology": ("recombinant dna", "polymerase chain reaction", "pcr", "biotechnology", "vector"),
        "Ecology": ("ecosystem", "biodiversity", "ecological", "food chain", "population interaction"),
        "Diversity & classification": ("classification", "taxonomy", "biodiversity", "kingdom", "phylogeny"),
        "Health & disease": ("pathogen", "immunity", "antibody", "vaccine", "communicable disease"),
    },
}

YEAR_PATTERNS = {
    "neet": (
        re.compile(r"\b(?:NEET|AIPMT)\b.{0,100}?\b((?:19|20)\d{2})\b", re.IGNORECASE),
    ),
    "jee_main": (
        re.compile(r"\bJEE\s*Main\b.{0,100}?\b((?:19|20)\d{2})\b", re.IGNORECASE),
        re.compile(r"\bAIEEE\b.{0,100}?\b((?:19|20)\d{2})\b", re.IGNORECASE),
    ),
    "jee_advanced": (
        re.compile(r"\bJEE\s*(?:\(\s*)?(?:ADVANCED|ADV)\s*\)?\s*[-–:]?\s*((?:19|20)\d{2})\b", re.IGNORECASE),
        re.compile(r"\bJEE\s*[-–]\s*((?:19|20)\d{2})\b", re.IGNORECASE),
    ),
}


def detect_year(filename_year, exam_series, text):
    if isinstance(filename_year, int) and 1900 <= filename_year <= 2100:
        return filename_year, "filename_hint"
    first_page_text = text[:12000]
    matches = {
        int(match.group(1))
        for pattern in YEAR_PATTERNS[exam_series]
        for match in pattern.finditer(first_page_text)
    }
    if len(matches) == 1:
        return matches.pop(), "title_text_candidate"
    return None, "needs_mapping"


def detect_topics(text):
    normalized = re.sub(r"\s+", " ", text.casefold())
    signals = []
    for subject, topics in TOPIC_KEYWORDS.items():
        for label, keywords in topics.items():
            matches = [
                keyword for keyword in keywords
                if re.search(rf"(?<!\w){re.escape(keyword)}(?!\w)", normalized)
            ]
            if len(matches) >= MIN_TOPIC_KEYWORD_MATCHES:
                signals.append({
                    "subject": subject,
                    "label": label,
                    "keywordMatches": len(matches),
                })
    return sorted(signals, key=lambda item: (item["subject"], item["label"]))


def file_sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as source_file:
        for chunk in iter(lambda: source_file.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def build_corpus(root=ROOT, inventory=None, existing_corpus=None):
    root = Path(root).resolve()
    if inventory is None:
        inventory = json.loads(INVENTORY_PATH.read_text(encoding="utf-8"))
    if not isinstance(inventory.get("sources"), list):
        raise ValueError("Paper inventory does not contain a source list.")

    cached_papers = {}
    if (
        isinstance(existing_corpus, dict)
        and existing_corpus.get("corpusVersion") == 1
        and isinstance(existing_corpus.get("papers"), list)
        and existing_corpus.get("indexMethodVersion", INDEX_METHOD_VERSION)
        == INDEX_METHOD_VERSION
    ):
        cached_papers = {
            paper["id"]: paper
            for paper in existing_corpus["papers"]
            if isinstance(paper, dict) and isinstance(paper.get("id"), str)
        }

    logging.getLogger("pypdf").setLevel(logging.ERROR)
    papers = []
    for source in inventory["sources"]:
        relative = PurePosixPath(source["path"])
        if relative.is_absolute() or ".." in relative.parts:
            raise ValueError(f"Inventory source path escapes repository: {source['path']}")
        path = (root / Path(*relative.parts)).resolve()
        if root not in path.parents or not path.is_file():
            raise FileNotFoundError(f"Inventory source is missing: {source['path']}")
        if path.stat().st_size > MAX_SOURCE_BYTES:
            raise ValueError(f"Exam PDF exceeds the 100 MiB processing limit: {source['path']}")
        digest = file_sha256(path)
        if digest != source["sha256"]:
            raise ValueError(f"Source changed since inventory generation: {source['path']}")

        cached = cached_papers.get(source["id"])
        filename_year = source.get("indexing", {}).get("year")
        has_filename_year = isinstance(filename_year, int) and 1900 <= filename_year <= 2100
        if (
            cached
            and cached.get("sha256") == digest
            and cached.get("examSeries") == source["examSeries"]
            and cached.get("sourceName") == source["originalName"]
            and (cached.get("yearSource") == "filename_hint") == has_filename_year
            and (not has_filename_year or cached.get("year") == filename_year)
        ):
            cached_record = dict(cached)
            cached_record["ownerVerificationStatus"] = (
                source.get("ownerVerification", {}).get("status", "not_reviewed")
            )
            papers.append(cached_record)
            continue

        pages = 0
        text_parts = []
        extraction_error = False
        try:
            with warnings.catch_warnings():
                warnings.simplefilter("ignore")
                reader = PdfReader(str(path), strict=False)
                pages = len(reader.pages)
                for page_number, page in enumerate(reader.pages, start=1):
                    try:
                        text_parts.append(page.extract_text() or "")
                    except Exception as error:
                        extraction_error = True
                        logging.warning(
                            "Could not extract page %d from %s: %s",
                            page_number,
                            source["path"],
                            error,
                        )
        except Exception as error:
            extraction_error = True
            logging.warning("Could not read %s: %s", source["path"], error)
        text = "\n".join(text_parts)
        year, year_source = detect_year(source.get("indexing", {}).get("year"), source["examSeries"], text)
        searchable_chars = len(re.sub(r"\s+", "", text))
        topics = detect_topics(text) if searchable_chars >= 200 else []
        if extraction_error:
            extraction_status = "partial"
        elif searchable_chars < 200:
            extraction_status = "low_text"
        else:
            extraction_status = "text_extracted"
        papers.append({
            "id": source["id"],
            "examSeries": source["examSeries"],
            "sourceName": source["originalName"],
            "sha256": digest,
            "ownerVerificationStatus": source.get("ownerVerification", {}).get("status", "not_reviewed"),
            "year": year,
            "yearSource": year_source,
            "pages": pages,
            "searchableCharacters": searchable_chars,
            "extractionStatus": extraction_status,
            "topicReviewStatus": "machine_candidates_unreviewed",
            "topicSignals": topics,
        })

    summary = {}
    for series in ("neet", "jee_main", "jee_advanced"):
        records = [paper for paper in papers if paper["examSeries"] == series]
        summary[series] = {
            "paperFiles": len(records),
            "textExtracted": sum(paper["extractionStatus"] == "text_extracted" for paper in records),
            "lowTextOrScanned": sum(paper["extractionStatus"] == "low_text" for paper in records),
            "yearMappedCandidates": sum(paper["year"] is not None for paper in records),
        }
    return {
        "corpusVersion": 1,
        "indexMethodVersion": INDEX_METHOD_VERSION,
        "inventoryVersion": inventory.get("inventoryVersion"),
        "sourceCount": len(papers),
        "topicMethod": {
            "status": "heuristic_unreviewed",
            "minimumDistinctKeywordMatchesPerTopic": MIN_TOPIC_KEYWORD_MATCHES,
            "storesExtractedQuestionText": False,
            "description": (
                "Deterministic keyword matches over local PDF text extraction. "
                "Year and topic matches are candidates, not verified labels; "
                "scanned pages and layout errors reduce coverage."
            ),
        },
        "summary": summary,
        "papers": papers,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--inventory", type=Path, default=INVENTORY_PATH)
    parser.add_argument("--output", type=Path, default=OUTPUT_PATH)
    args = parser.parse_args()
    inventory = json.loads(args.inventory.read_text(encoding="utf-8"))
    existing_corpus = (
        json.loads(args.output.read_text(encoding="utf-8")) if args.output.is_file() else None
    )
    corpus = build_corpus(ROOT, inventory, existing_corpus)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(corpus, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"Indexed {corpus['sourceCount']} PDFs to {args.output}")
    for series, summary in corpus["summary"].items():
        print(
            f"{series}: {summary['paperFiles']} files, "
            f"{summary['textExtracted']} text extracted, "
            f"{summary['lowTextOrScanned']} low-text/scanned, "
            f"{summary['yearMappedCandidates']} year candidates"
        )
    print("No extracted question text is included; years/topics remain unreviewed candidates.")


if __name__ == "__main__":
    main()
