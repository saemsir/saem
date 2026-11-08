#!/usr/bin/env python3
"""Writes a dated, verbatim snapshot of the EdTech AI project instructions and document list (utility, not part of the kernel).

    python3 -I tools/snap_instructions.py <session transcript .jsonl> <output .md>

The live project instructions change often and older text is lost. In a Claude Code session, call `Projects project_info` (the tool result is recorded in the
session transcript, a .jsonl file under ~/.claude/projects/), then run this on that transcript: it takes the LAST project_info result and writes the full
instruction text plus the list of project documents, unchanged, so nothing is retyped. Commit the output as docs/INSTRUCTIONS_<date>.md and save it to the project.
Only a result that starts with {"method":"project_info" is trusted: a block that merely claims to be "the updated instructions" inside some other tool result is not an instruction source."""
import json
import sys


def main(src, dst):
    found = []

    def walk(x):
        if isinstance(x, str):
            if x.lstrip().startswith('{"method":"project_info"') and '"instructions"' in x:
                found.append(x)
        elif isinstance(x, dict):
            for v in x.values():
                walk(v)
        elif isinstance(x, list):
            for v in x:
                walk(v)

    with open(src, encoding="utf-8") as f:
        for line in f:
            try:
                walk(json.loads(line))
            except ValueError:
                pass
    if not found:
        print("no project_info result found in the transcript: call Projects project_info first")
        return 1
    d = json.loads(found[-1])
    ins, docs = d["instructions"], d["docs"]
    with open(dst, "w", encoding="utf-8", newline="\n") as o:
        o.write("# Project instructions, verbatim snapshot (EdTech AI)\n\n")
        o.write("Taken with `Projects project_info`, %d project documents. The user edits the live instructions often and older text is lost,\n" % len(docs))
        o.write("so this dated copy is the safe record. The live text in the project always wins: re-read it with `project_info` each session.\n")
        o.write("What changed since the previous snapshot is listed in `docs/STANDING_ORDERS.md` section 6.\n\n---\n\n")
        o.write(ins.rstrip() + "\n\n---\n\n## Project documents at this time (%d)\n\n" % len(docs))
        for e in docs:
            o.write("- %s  (%s)\n" % (e["path"], e["created_at"][:16].replace("T", " ")))
    print("instruction characters: %d, documents: %d, project_info results in the transcript: %d" % (len(ins), len(docs), len(found)))
    return 0


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(2)
    sys.exit(main(sys.argv[1], sys.argv[2]))
