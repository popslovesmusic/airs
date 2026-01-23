#!/usr/bin/env python3
"""
json_gate.py - JSONL validator and minifier for AIRS ingress

Features:
  - Parse one or more JSON objects from stdin/file
  - Minify to canonical JSONL (one object per line, no trailing newline)
  - Validate the entire JSONL stream (line-level diagnostics)

Usage examples:
  python json_gate.py --mode validate -i cmd.jsonl
  python json_gate.py --mode minify   -i pretty.json -o cmd.jsonl
  Get-Content cmd.json | python json_gate.py --mode gate > clean.jsonl

Exit codes:
  0 on success; 1 on validation error; 2 on IO errors
"""

import argparse
import json
import sys
from typing import Any, List, Optional, Tuple


def read_text(path: str) -> str:
    try:
        if path == "-" or path is None:
            return sys.stdin.read()
        with open(path, "r", encoding="utf-8") as f:
            return f.read()
    except OSError as e:
        sys.stderr.write(f"IO error: {e}\n")
        sys.exit(2)


def write_text(path: str | None, data: str) -> None:
    try:
        if path is None or path == "-":
            sys.stdout.write(data)
        else:
            with open(path, "w", encoding="utf-8") as f:
                f.write(data)
    except OSError as e:
        sys.stderr.write(f"IO error: {e}\n")
        sys.exit(2)


def format_error(error_class: str,
                 line: int,
                 column: int,
                 message: str,
                 hint: str,
                 stage: str = "json_ingress_validation") -> dict:
    return {
        "status": "rejected",
        "stage": stage,
        "error_class": error_class,
        "line": line,
        "column": column,
        "message": message,
        "hint": hint,
    }


def parse_objects(raw: str, allow_blank_lines: bool = False) -> Tuple[Optional[List[Any]], Optional[dict]]:
    """
    Parse one or more JSON objects from arbitrary whitespace-separated input.
    Returns (objects, error_dict).
    """
    decoder = json.JSONDecoder()
    idx = 0
    n = len(raw)

    objects: List[Any] = []

    while True:
        # Skip whitespace between objects, but optionally detect blank lines
        whitespace_start = idx
        newline_count = 0
        while idx < n and raw[idx].isspace():
            if raw[idx] == "\n":
                newline_count += 1
            idx += 1
        if idx >= n:
            break

        if objects and newline_count > 1 and not allow_blank_lines:
            line_no = raw.count("\n", 0, whitespace_start) + 1
            return None, format_error(
                "EMPTY_LINE",
                line_no,
                1,
                "Blank line detected between JSON objects",
                "Remove empty lines; JSONL requires exactly one JSON object per line.",
                stage="json_ingress_minify",
            )

        try:
            obj, next_idx = decoder.raw_decode(raw, idx)
        except json.JSONDecodeError as e:
            return None, format_error(
                "INVALID_JSON_SYNTAX",
                e.lineno or 1,
                e.colno or 1,
                f"JSON parse error: {e.msg}",
                "Fix the JSON syntax at the reported line/column.",
                stage="json_ingress_minify",
            )

        objects.append(obj)
        idx = next_idx

    if not objects:
        return None, format_error(
            "EMPTY_INPUT",
            1,
            1,
            "No JSON objects found",
            "Provide at least one JSON object.",
            stage="json_ingress_minify",
        )

    return objects, None


def to_canonical_jsonl(objs: List[Any]) -> str:
    """Serialize a list of JSON-serializable objects to canonical JSONL (no trailing newline)."""
    return "\n".join(json.dumps(obj, separators=(",", ":"), ensure_ascii=True) for obj in objs)


def validate_jsonl_stream(canonical: str) -> Optional[dict]:
    """Validate canonical JSONL string; return error dict if invalid."""
    lines = canonical.splitlines()
    if not lines:
        return format_error(
            "EMPTY_INPUT",
            1,
            1,
            "No JSON objects found",
            "Provide at least one JSON object."
        )

    for i, line in enumerate(lines, 1):
        if line.strip() == "":
            return format_error(
                "EMPTY_LINE",
                i,
                1,
                "Blank line in JSONL stream",
                "Remove empty lines; JSONL requires exactly one JSON object per line."
            )
        try:
            obj = json.loads(line)
        except json.JSONDecodeError as e:
            return format_error(
                "INVALID_JSON_SYNTAX",
                i,
                e.colno or 1,
                f"Line {i} JSON parse error: {e.msg}",
                "Fix the JSON syntax on this line."
            )

        if not isinstance(obj, dict):
            return format_error(
                "INVALID_ROOT_TYPE",
                i,
                1,
                "Top-level value must be a JSON object",
                "Make each line an object with a 'command' field."
            )
        if "command" not in obj:
            return format_error(
                "MISSING_REQUIRED_FIELD",
                i,
                1,
                "Missing 'command' field",
                "Add a non-empty string 'command' field."
            )
        if not isinstance(obj["command"], str) or not obj["command"].strip():
            return format_error(
                "INVALID_COMMAND_FIELD",
                i,
                1,
                "'command' must be a non-empty string",
                "Ensure 'command' is a non-empty string."
            )

    return None


def main() -> None:
    parser = argparse.ArgumentParser(description="Validate/minify JSON ingress for AIRS CLI")
    parser.add_argument("--mode", choices=["validate", "minify", "gate"], default="gate",
                        help="validate: check only; minify: emit canonical JSONL; gate: minify then validate, emit JSONL")
    parser.add_argument("-i", "--input", default="-", help="input file (default: stdin)")
    parser.add_argument("-o", "--output", default=None, help="output file (default: stdout)")
    args = parser.parse_args()

    raw = read_text(args.input)
    if raw == "":
        sys.stdout.write(json.dumps(format_error(
            "EMPTY_INPUT",
            1,
            1,
            "Empty input",
            "Provide at least one JSON object.",
            stage="json_ingress_minify"
        ), ensure_ascii=True) + "\n")
        sys.exit(1)

    # Step A: parse and minify to canonical JSONL
    allow_blank = args.mode == "minify"
    objs, parse_err = parse_objects(raw, allow_blank_lines=allow_blank)
    if parse_err:
        sys.stdout.write(json.dumps(parse_err, ensure_ascii=True) + "\n")
        sys.exit(1)

    canonical = to_canonical_jsonl(objs)

    if args.mode == "minify":
        write_text(args.output, canonical + ("\n" if not canonical.endswith("\n") else ""))
        sys.exit(0)

    # Step B: validate canonical JSONL
    validation_err = validate_jsonl_stream(canonical)
    if validation_err:
        sys.stdout.write(json.dumps(validation_err, ensure_ascii=True) + "\n")
        sys.exit(1)

    if args.mode == "validate":
        sys.stderr.write("OK: valid JSONL stream\n")
        sys.exit(0)

    # gate mode: emit canonical JSONL
    write_text(args.output, canonical + ("\n" if not canonical.endswith("\n") else ""))


if __name__ == "__main__":
    main()
