from __future__ import annotations

import argparse
import json
import sys

from sid_rewrite import apply_rewrites_to_package
from sid_validator import validate_package


def load_json(path: str) -> dict:
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def write_json(path: str, data: dict) -> None:
    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, sort_keys=True)
        f.write("\n")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description="Validate a SID package and apply rewrites."
    )
    parser.add_argument("input", help="Path to sid_package.json")
    parser.add_argument("state_id", help="State id to use for authorization")
    parser.add_argument("diagram_id", help="Diagram id to rewrite")
    parser.add_argument(
        "-o",
        "--output",
        help="Path to write updated package (defaults to in-place)",
    )
    args = parser.parse_args(argv)

    pkg = load_json(args.input)
    errors, warnings = validate_package(pkg)
    for warn in warnings:
        print(f"WARNING: {warn}")
    if errors:
        for err in errors:
            print(f"ERROR: {err}")
        return 1

    result = apply_rewrites_to_package(pkg, args.state_id, args.diagram_id)
    for msg in result.messages:
        print(msg)
    if not result.applied:
        print("No rewrites applied.")

    post_errors, post_warnings = validate_package(pkg)
    for warn in post_warnings:
        print(f"WARNING: post-rewrite {warn}")
    if post_errors:
        for err in post_errors:
            print(f"ERROR: post-rewrite {err}")
        return 1

    output_path = args.output or args.input
    write_json(output_path, pkg)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
