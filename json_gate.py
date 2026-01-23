#!/usr/bin/env python3
"""
json_gate.py - Single-entry JSONL validator and minifier for AIRS ingress

Features:
  - Validate: ensure exactly one JSON object, parseable, has "command"
  - Minify: emit single-line JSON (no whitespace/newlines)
  - Gate (default): validate then minify; rejects extras/trailing junk

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
from typing import Any


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


def parse_single_json_object(raw: str) -> Any:
    decoder = json.JSONDecoder()
    try:
        obj, idx = decoder.raw_decode(raw)
    except json.JSONDecodeError as e:
        raise ValueError(f"JSON parse error: {e.msg} at pos {e.pos}") from e

    # Reject trailing non-whitespace
    remaining = raw[idx:].strip()
    if remaining:
        raise ValueError("Extra content found after first JSON value")

    if not isinstance(obj, dict):
        raise ValueError("Top-level JSON must be an object")
    if "command" not in obj:
        raise ValueError("Missing required 'command' field")
    if not isinstance(obj["command"], str) or not obj["command"].strip():
        raise ValueError("'command' must be a non-empty string")
    return obj


def enforce_single_line(raw: str) -> None:
    lines = raw.splitlines()
    # Allow a trailing empty line, but no second non-empty line
    non_empty = [ln for ln in lines if ln.strip() != ""]
    if len(non_empty) > 1:
        raise ValueError("Input must be a single JSON object on one line (JSONL)")


def minify(obj: Any) -> str:
    return json.dumps(obj, separators=(",", ":"), ensure_ascii=True)


def main() -> None:
    parser = argparse.ArgumentParser(description="Validate/minify JSON ingress for AIRS CLI")
    parser.add_argument("--mode", choices=["validate", "minify", "gate"], default="gate",
                        help="validate: check only; minify: emit single line; gate: validate then minify")
    parser.add_argument("-i", "--input", default="-", help="input file (default: stdin)")
    parser.add_argument("-o", "--output", default=None, help="output file (default: stdout)")
    args = parser.parse_args()

    raw = read_text(args.input)
    if raw == "":
        sys.stderr.write("Validation error: empty input\n")
        sys.exit(1)

    try:
        # Gate and validate enforce single-line framing on input; minify accepts multi-line.
        if args.mode in {"validate", "gate"}:
            enforce_single_line(raw)

        obj = parse_single_json_object(raw)

        if args.mode == "validate":
            sys.stderr.write("OK: valid single-line JSON command\n")
            sys.exit(0)

        output = minify(obj)
        if args.mode == "minify":
            # Accept pretty JSON but still verify single object semantics.
            pass
        write_text(args.output, output + ("\n" if not output.endswith("\n") else ""))
    except ValueError as e:
        sys.stderr.write(f"Validation error: {e}\n")
        sys.exit(1)


if __name__ == "__main__":
    main()
