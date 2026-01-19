from __future__ import annotations

import argparse
import json
import sys

from sid_validator import validate_package


def load_json(path: str) -> dict:
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description="Validate a SID package.")
    parser.add_argument("input", help="Path to sid_package.json")
    args = parser.parse_args(argv)

    pkg = load_json(args.input)
    errors, warnings = validate_package(pkg)
    for warn in warnings:
        print(f"WARNING: {warn}")
    if errors:
        for err in errors:
            print(f"ERROR: {err}")
        return 1
    print("OK: package passes minimal SID checks")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
