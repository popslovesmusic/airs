#!/usr/bin/env python3
"""
json_gate.py - Single-command JSON ingress gate for AIRS

Governance (fixed):
  - Exactly ONE JSON command per invocation
  - No JSONL streams, no multi-command piping
  - Sequencing is the agent's responsibility
  - Gate = Minifier (1 object) -> Validator (same object) -> stdout
"""

import argparse
import json
import sys
from typing import Any, Tuple, Optional


# ---------- I/O helpers ----------

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


# ---------- Error formatting ----------

def error_payload(error_class: str,
                  message: str,
                  hint: str,
                  stage: str,
                  line: int = 1,
                  column: int = 1) -> dict:
    return {
        "status": "rejected",
        "stage": stage,
        "error_class": error_class,
        "line": line,
        "column": column,
        "message": message,
        "hint": hint,
    }


# ---------- Core logic ----------

def parse_single_object(raw: str) -> Tuple[Optional[Any], Optional[dict]]:
    """Parse exactly one JSON value from raw text, rejecting extras."""
    decoder = json.JSONDecoder()
    try:
        obj, idx = decoder.raw_decode(raw)
    except json.JSONDecodeError as e:
        return None, error_payload(
            "INVALID_JSON_SYNTAX",
            f"JSON parse error: {e.msg}",
            "Fix the JSON syntax at the reported position.",
            stage="json_ingress_minify",
            line=e.lineno or 1,
            column=e.colno or 1,
        )

    # Reject trailing non-whitespace content (second object or garbage)
    remainder = raw[idx:].strip()
    if remainder:
        # Crude line/col for remainder start
        line = raw.count("\n", 0, idx) + 1
        col = idx - raw.rfind("\n", 0, idx) if "\n" in raw[:idx] else idx + 1
        return None, error_payload(
            "EXTRA_CONTENT",
            "Extra content found after the first JSON object",
            "Provide exactly one JSON object per invocation.",
            stage="json_ingress_validation",
            line=line,
            column=col,
        )

    return obj, None


def validate_object(obj: Any) -> Optional[dict]:
    if obj is None:
        return error_payload(
            "EMPTY_INPUT",
            "No JSON object provided",
            "Provide exactly one JSON object.",
            stage="json_ingress_validation",
        )
    if not isinstance(obj, dict):
        return error_payload(
            "INVALID_ROOT_TYPE",
            "Top-level value must be a JSON object",
            "Wrap the command fields in a JSON object.",
            stage="json_ingress_validation",
        )
    if "command" not in obj:
        return error_payload(
            "MISSING_REQUIRED_FIELD",
            "Missing required field: command",
            "Add a top-level \"command\" field (non-empty string).",
            stage="json_ingress_validation",
        )
    if not isinstance(obj["command"], str) or not obj["command"].strip():
        return error_payload(
            "INVALID_COMMAND_FIELD",
            "\"command\" must be a non-empty string",
            "Set \"command\" to a non-empty string.",
            stage="json_ingress_validation",
        )
    return None


def minify_object(obj: Any) -> str:
    return json.dumps(obj, separators=(",", ":"), ensure_ascii=True)


def main() -> None:
    parser = argparse.ArgumentParser(description="Validate/minify single JSON command for AIRS CLI")
    parser.add_argument("--mode", choices=["validate", "minify", "gate"], default="gate",
                        help="validate: check only; minify: emit single-line JSON; gate: validate then minify")
    parser.add_argument("-i", "--input", default="-", help="input file (default: stdin)")
    parser.add_argument("-o", "--output", default=None, help="output file (default: stdout)")
    args = parser.parse_args()

    raw = read_text(args.input)
    if raw.strip() == "":
        write_text(args.output if args.mode == "minify" else None,
                   json.dumps(error_payload(
                       "EMPTY_INPUT",
                       "Empty input",
                       "Provide exactly one JSON object.",
                       stage="json_ingress_minify"
                   ), ensure_ascii=True) + "\n")
        sys.exit(1)

    obj, parse_err = parse_single_object(raw)
    if parse_err:
        write_text(args.output if args.mode == "minify" else None,
                   json.dumps(parse_err, ensure_ascii=True) + "\n")
        sys.exit(1)

    validation_err = validate_object(obj)
    if validation_err:
        write_text(args.output if args.mode == "minify" else None,
                   json.dumps(validation_err, ensure_ascii=True) + "\n")
        sys.exit(1)

    # Successful parse + validation
    if args.mode == "validate":
        sys.stderr.write("OK: valid single JSON command\n")
        sys.exit(0)

    minified = minify_object(obj)
    write_text(args.output, minified + ("\n" if not minified.endswith("\n") else ""))


if __name__ == "__main__":
    main()
