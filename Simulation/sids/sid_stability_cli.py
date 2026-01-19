from __future__ import annotations

import argparse
import json
import sys

from sid_stability import is_structurally_stable, compute_stability_metrics


def load_json(path: str) -> dict:
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description="Check structural stability of a SID state/diagram."
    )
    parser.add_argument("input", help="Path to sid_package.json")
    parser.add_argument("state_id", help="State id to check")
    parser.add_argument("diagram_id", help="Diagram id to check")
    parser.add_argument(
        "--tolerance",
        type=float,
        default=1e-6,
        help="Loop convergence tolerance (default: 1e-6)"
    )
    parser.add_argument(
        "--metrics",
        action="store_true",
        help="Compute and display stability metrics"
    )
    args = parser.parse_args(argv)

    pkg = load_json(args.input)

    # Check stability
    is_stable, satisfied, message = is_structurally_stable(
        pkg, args.state_id, args.diagram_id, args.tolerance
    )

    print(message)
    if satisfied:
        print("\nSatisfied termination conditions:")
        for condition in satisfied:
            print(f"  {condition}")

    # Compute metrics if requested
    if args.metrics:
        print("\nStability Metrics:")
        metrics = compute_stability_metrics(pkg, args.state_id, args.diagram_id)
        for key, value in sorted(metrics.items()):
            if value is not None:
                if isinstance(value, float):
                    print(f"  {key}: {value:.6f}")
                else:
                    print(f"  {key}: {value}")
            else:
                print(f"  {key}: N/A")

    return 0 if is_stable else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
