from __future__ import annotations

import argparse
import json
import random
from collections import Counter

from sid_validator import validate_package
from sid_rewrite import apply_rewrites_to_package


OPS = ["P", "S_PLUS", "S_MINUS", "O", "C", "T"]
EDGE_LABELS = ["C", "O", "T", "arg"]


def rand_id(prefix: str, n: int) -> str:
    return f"{prefix}{n}"


def make_pkg(seed: int) -> dict:
    random.seed(seed)
    dofs = [{"id": "dof_a"}, {"id": "dof_b"}]
    csis = [{
        "id": "csi0",
        "allowed_dofs": ["dof_a", "dof_b"],
        "allowed_pairs": [
            ["dof_a", "dof_a"],
            ["dof_a", "dof_b"],
            ["dof_b", "dof_a"],
            ["dof_b", "dof_b"],
        ],
    }]
    compartments = [{"id": "c0", "index": 0}]

    node_count = random.randint(2, 6)
    nodes = []
    for i in range(node_count):
        op = random.choice(OPS)
        node = {"id": rand_id("n", i + 1), "op": op}
        if op == "P":
            node["dof_refs"] = [random.choice(["dof_a", "dof_b"])]
        if op == "O":
            node["irreversible"] = True
        nodes.append(node)

    edges = []
    for i in range(random.randint(1, 5)):
        frm = random.choice(nodes)["id"]
        to = random.choice(nodes)["id"]
        edges.append({
            "id": rand_id("e", i + 1),
            "from": frm,
            "to": to,
            "label": random.choice(EDGE_LABELS),
        })

    diagram = {"id": "d0", "compartment_id": "c0", "nodes": nodes, "edges": edges}
    states = [{
        "id": "s0",
        "diagram_id": "d0",
        "csi_id": "csi0",
        "compartment_id": "c0",
        "inu_labels": {},
    }]
    constraints = [{
        "id": "c0",
        "type": "hard",
        "scope": "global",
        "predicate": "no_cross_csi_interaction",
        "on_violation": "halt",
    }]
    rewrite_rules = [{
        "id": "rw1",
        "pattern_expr": "C(P($x), $y)",
        "replacement_expr": "C(P($x), O($y))",
        "preconditions": ["admissible"],
    }]

    return {
        "dofs": dofs,
        "csis": csis,
        "compartments": compartments,
        "diagrams": [diagram],
        "states": states,
        "constraints": constraints,
        "rewrite_rules": rewrite_rules,
    }


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description="Stress test SID validate+rewrite.")
    parser.add_argument("--runs", type=int, default=200, help="Number of iterations")
    parser.add_argument("--seed", type=int, default=1234, help="Base random seed")
    parser.add_argument(
        "--report",
        help="Optional path to write JSON report",
    )
    args = parser.parse_args(argv)

    failures = Counter()
    passes = 0

    for i in range(args.runs):
        pkg = make_pkg(args.seed + i)
        errors, warnings = validate_package(pkg)
        if errors:
            failures[errors[0]] += 1
            continue
        apply_rewrites_to_package(pkg, "s0", "d0")
        errors2, warnings2 = validate_package(pkg)
        if errors2:
            failures[errors2[0]] += 1
            continue
        passes += 1

    result = {
        "runs": args.runs,
        "passes": passes,
        "failures": sum(failures.values()),
        "top_failures": failures.most_common(10),
    }

    print(f"runs: {result['runs']}")
    print(f"passes: {result['passes']}")
    print(f"failures: {result['failures']}")
    for msg, count in result["top_failures"]:
        print(f"{count} {msg}")

    if args.report:
        with open(args.report, "w", encoding="utf-8") as f:
            json.dump(result, f, indent=2, sort_keys=True)
            f.write("\n")
    return 0


if __name__ == "__main__":
    import sys
    sys.exit(main(sys.argv[1:]))
