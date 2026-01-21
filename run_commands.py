"""
Unified JSONL orchestrator for dase_cli/sid_cli.

Usage:
  python run_commands.py --input commands.jsonl --engine dase_cli --out responses.jsonl

Notes:
  - One JSON object per line in the input file.
  - Default engine: dase_cli (Simulation/bin/dase_cli.exe).
  - Use --engine sid_cli to target the SID-only CLI.
  - Produces a JSONL of responses; optional summary printed to stdout.
"""

import argparse
import json
import subprocess
from pathlib import Path


def run(engine_path: Path, commands):
    payload = "\n".join(json.dumps(c) for c in commands) + "\n"
    proc = subprocess.run([str(engine_path)], input=payload, text=True, capture_output=True)
    responses = [json.loads(line) for line in proc.stdout.splitlines() if line.strip()]
    return responses, proc


def main():
    parser = argparse.ArgumentParser(description="Unified JSONL orchestrator for dase_cli/sid_cli")
    parser.add_argument("--input", required=True, help="Path to JSONL file (one JSON object per line)")
    parser.add_argument("--engine", choices=["dase_cli", "sid_cli"], default="dase_cli", help="Target engine CLI")
    parser.add_argument("--out", default="responses.jsonl", help="Path to write responses JSONL")
    parser.add_argument("--summary", action="store_true", help="Print summary to stdout")
    args = parser.parse_args()

    base = Path(__file__).parent
    engine_path = base / "Simulation" / "bin" / (args.engine + ".exe")
    if not engine_path.exists():
        raise SystemExit(f"Engine binary not found: {engine_path}")

    cmds = []
    with Path(args.input).open() as f:
        for idx, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue
            try:
                cmds.append(json.loads(line))
            except Exception as e:
                raise SystemExit(f"Parse error on line {idx}: {e}")

    responses, proc = run(engine_path, cmds)
    Path(args.out).write_text("\n".join(json.dumps(r) for r in responses))

    if args.summary:
        ok = sum(1 for r in responses if r.get("status") == "success")
        err = len(responses) - ok
        print(f"Engine: {args.engine} returncode={proc.returncode}")
        print(f"Total responses: {len(responses)}, success={ok}, error={err}")


if __name__ == "__main__":
    main()
