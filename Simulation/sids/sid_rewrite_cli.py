from __future__ import annotations

import argparse
import json
import sys

from sid_rewrite import apply_rewrites_to_package


def load_json(path: str) -> dict:
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def write_json(path: str, data: dict) -> None:
    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, sort_keys=True)
        f.write("\n")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description="Apply SID rewrite rules to a diagram in a package."
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
    result = apply_rewrites_to_package(pkg, args.state_id, args.diagram_id)
    for msg in result.messages:
        print(msg)
    if not result.applied:
        print("No rewrites applied.")
    output_path = args.output or args.input
    write_json(output_path, pkg)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
