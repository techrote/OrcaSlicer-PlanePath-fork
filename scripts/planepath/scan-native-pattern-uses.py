#!/usr/bin/env python3
"""Inventory every tracked source reference and switch involving InfillPattern native enums.

PP-003 uses this as a whole-checkout audit aid. It deliberately reports rather than rewrites
contextual switches: factory dispatch, geometry heuristics and effective-pattern substitutions
remain valid native special cases.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
from pathlib import Path


SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".ipp", ".inl", ".m", ".mm"}


def tracked_source_files(root: Path) -> list[Path]:
    raw = subprocess.check_output(["git", "-C", str(root), "ls-files", "-z"])
    files = []
    for entry in raw.decode("utf-8").split("\0"):
        if not entry:
            continue
        path = root / entry
        if path.suffix.lower() in SOURCE_SUFFIXES:
            files.append(path)
    return files


def native_patterns(root: Path) -> list[str]:
    text = (root / "src/libslic3r/PrintConfig.hpp").read_text(encoding="utf-8")
    match = re.search(r"enum\s+InfillPattern\s*:\s*int\s*\{(?P<body>.*?)\bipCount\b", text, re.S)
    if match is None:
        raise RuntimeError("could not locate InfillPattern enum in PrintConfig.hpp")
    names = re.findall(r"\bip[A-Za-z0-9_]+\b", match.group("body"))
    names = list(dict.fromkeys(names))
    if len(names) != 32 or names[-1] != "ipScriptedPlanePath":
        raise RuntimeError("Expected 31 unchanged native values plus the appended scripted sentinel")
    return names[:-1]


def switch_blocks(lines: list[str], token_re: re.Pattern[str]) -> list[dict[str, object]]:
    blocks: list[dict[str, object]] = []
    i = 0
    while i < len(lines):
        if "switch" not in lines[i]:
            i += 1
            continue

        start = i
        depth = 0
        saw_open = False
        j = i
        while j < len(lines):
            line = lines[j]
            if "{" in line:
                saw_open = True
            if saw_open:
                depth += line.count("{")
                depth -= line.count("}")
            if saw_open and depth <= 0:
                break
            j += 1

        if saw_open:
            body = "\n".join(lines[start : min(j + 1, len(lines))])
            cases = sorted(set(re.findall(r"\bcase\s+(?:(?:Slic3r::)?InfillPattern::)?(ip[A-Za-z0-9_]+)\b", body)))
            if cases:
                blocks.append(
                    {
                        "line": start + 1,
                        "end_line": min(j + 1, len(lines)),
                        "cases": cases,
                        "has_default": bool(re.search(r"\bdefault\s*:", body)),
                        "native_tokens": sorted(set(token_re.findall(body))),
                    }
                )
            i = max(j + 1, i + 1)
        else:
            i += 1
    return blocks


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", default=".", help="repository checkout root")
    parser.add_argument("--out", help="write JSON inventory to this path")
    args = parser.parse_args()

    root = Path(args.repo).resolve()
    patterns = native_patterns(root)
    token_re = re.compile(r"\b(?:" + "|".join(map(re.escape, patterns + ["ipScriptedPlanePath", "ipCount"])) + r")\b")

    references: list[dict[str, object]] = []
    switches: list[dict[str, object]] = []
    multi_pattern_expressions: list[dict[str, object]] = []
    literal_virtual_capabilities: list[dict[str, object]] = []
    virtual_literal_re = re.compile(
        r"\b(?P<method>no_sort|is_self_crossing|use_bridge_flow)\s*\([^)]*\)[^{;]*\{[^}]*?return\s+(?P<value>true|false)\s*;"
    )
    scanned = 0

    for path in tracked_source_files(root):
        try:
            text = path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        scanned += 1
        rel = path.relative_to(root).as_posix()
        lines = text.splitlines()

        for line_no, line in enumerate(lines, 1):
            tokens = sorted(set(token_re.findall(line)))
            if not tokens:
                continue
            references.append({"path": rel, "line": line_no, "tokens": tokens, "text": line.strip()})
            if len([token for token in tokens if token != "ipCount"]) >= 2:
                multi_pattern_expressions.append(
                    {"path": rel, "line": line_no, "tokens": tokens, "text": line.strip()}
                )

        for block in switch_blocks(lines, token_re):
            block["path"] = rel
            switches.append(block)

        for match in virtual_literal_re.finditer(text):
            line_no = text.count("\n", 0, match.start()) + 1
            line = lines[line_no - 1].strip() if line_no <= len(lines) else ""
            literal_virtual_capabilities.append(
                {
                    "path": rel,
                    "line": line_no,
                    "method": match.group("method"),
                    "value": match.group("value"),
                    "text": line,
                }
            )

    head = subprocess.check_output(["git", "-C", str(root), "rev-parse", "HEAD"], text=True).strip()
    inventory = {
        "head": head,
        "native_pattern_count": len(patterns),
        "compiled_scripted_sentinels": ["ipScriptedPlanePath"],
        "compiled_pattern_count": len(patterns) + 1,
        "native_patterns": patterns,
        "source_files_scanned": scanned,
        "reference_count": len(references),
        "references": references,
        "switch_count": len(switches),
        "switches": switches,
        "multi_pattern_expression_count": len(multi_pattern_expressions),
        "multi_pattern_expressions": multi_pattern_expressions,
        "literal_virtual_capability_count": len(literal_virtual_capabilities),
        "literal_virtual_capabilities": literal_virtual_capabilities,
    }

    encoded = json.dumps(inventory, indent=2, sort_keys=True)
    if args.out:
        Path(args.out).write_text(encoded + "\n", encoding="utf-8")
    else:
        print(encoded)

    print(
        "PP-003 source audit: "
        f"{len(patterns)} native patterns + 1 scripted sentinel; {scanned} source files; "
        f"{len(references)} references; {len(switches)} enum switches; "
        f"{len(multi_pattern_expressions)} multi-pattern expression lines; "
        f"{len(literal_virtual_capabilities)} literal virtual capability returns."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
