import json
import os
import subprocess
import sys

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SID_PYTHON_PATH = os.path.join(REPO_ROOT, "sids")
sys.path.insert(0, SID_PYTHON_PATH)

from sid_parser import parse_expression
from sid_ast_to_diagram import expr_to_diagram


def run_dump_expr(dump_path, expr):
    proc = subprocess.run(
        [dump_path, expr],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if proc.returncode != 0:
        raise RuntimeError(f"dump failed: {proc.stderr.decode('utf-8', errors='ignore')}")
    try:
        return json.loads(proc.stdout.decode("utf-8"))
    except json.JSONDecodeError as exc:
        raise RuntimeError(f"dump output not JSON: {proc.stdout!r}") from exc


def normalize_diagram(diagram):
    nodes = diagram.get("nodes", [])
    edges = diagram.get("edges", [])

    id_to_op = {}
    id_to_dof = {}
    for node in nodes:
        node_id = node.get("id")
        op = node.get("op")
        if not node_id or not op:
            continue
        id_to_op[node_id] = op
        dof_refs = node.get("dof_refs", [])
        if isinstance(dof_refs, list):
            id_to_dof[node_id] = tuple(sorted(dof_refs))
        else:
            id_to_dof[node_id] = tuple()

    indegree = {node_id: 0 for node_id in id_to_op}
    edge_info = []
    for edge in edges:
        from_id = edge.get("from")
        to_id = edge.get("to")
        if not from_id or not to_id:
            continue
        if id_to_op.get(from_id) == "Atom" or id_to_op.get(to_id) == "Atom":
            continue
        if to_id in indegree:
            indegree[to_id] += 1
        label = edge.get("label", "arg")
        edge_info.append(label)

    node_info = []
    for node_id, op in id_to_op.items():
        if op == "Atom":
            continue
        node_info.append((op, indegree.get(node_id, 0), id_to_dof.get(node_id, tuple())))

    return {
        "node_info": sorted(node_info),
        "edge_info": sorted(edge_info),
        "node_count": len(node_info),
        "edge_count": len(edge_info),
    }


def validate_expression(dump_path, expr):
    python_expr = parse_expression(expr)
    py_diagram = expr_to_diagram(python_expr, "d_expr")
    cpp_diagram = run_dump_expr(dump_path, expr)

    return normalize_diagram(py_diagram), normalize_diagram(cpp_diagram)


def main():
    repo_root = REPO_ROOT
    dump_path = os.path.join(repo_root, "build", "Release", "sid_dump_expr.exe")
    if not os.path.exists(dump_path):
        raise SystemExit(f"Missing helper at {dump_path}")

    expressions = [
        "P(Freedom)",
        "C(P(Freedom), O(P(Decision)))",
        "O(P(Beta))",
        "C(P(Alpha), P(Beta))",
        "S+(P(Alpha), P(Beta), P(Delta))",
    ]

    for expr in expressions:
        py_norm, cpp_norm = validate_expression(dump_path, expr)
        if py_norm != cpp_norm:
            print(f"FAIL: mismatch for {expr}")
            print("python:", py_norm)
            print("cpp:", cpp_norm)
            raise SystemExit(1)

    print("PASS: all validation checks matched")


if __name__ == "__main__":
    main()
