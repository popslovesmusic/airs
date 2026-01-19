from __future__ import annotations

import copy
import logging
from dataclasses import dataclass
from typing import Dict, List, Optional, Set, Tuple

from sid_crf import authorize_rewrite
from sid_parser import parse_expression
from sid_ast import Atom, Expr, OpExpr

# Logger
logger = logging.getLogger(__name__)

# Configuration constants (Minor Issue #9: limit rewrite iterations)
MAX_REWRITE_ITERATIONS = 1000  # Prevent infinite loops


@dataclass(frozen=True)
class RewriteResult:
    """Immutable result of a rewrite operation."""
    applied: bool
    diagram: dict  # Deep copied to prevent mutation
    messages: tuple  # Immutable tuple instead of list


@dataclass(frozen=True)
class SidePattern:
    op: Optional[str]
    var: str


@dataclass(frozen=True)
class EdgePattern:
    left: SidePattern
    edge_label: str
    right: SidePattern


@dataclass(frozen=True)
class RulePattern:
    edges: List[EdgePattern]


def parse_side(text: str) -> SidePattern:
    text = text.strip()
    if "(" in text and text.endswith(")"):
        op, rest = text.split("(", 1)
        op = op.strip()
        var = rest[:-1].strip()
        if not var:
            raise ValueError(f"Invalid side pattern: {text!r}")
        return SidePattern(op=op, var=var)
    return SidePattern(op=None, var=text)


def parse_rule_pattern(text: str) -> RulePattern:
    if "--" not in text or "-->" not in text:
        raise ValueError(f"Invalid rule pattern: {text!r}")
    segments = [seg.strip() for seg in text.split(",") if seg.strip()]
    edges: List[EdgePattern] = []
    for segment in segments:
        if "--" not in segment or "-->" not in segment:
            raise ValueError(f"Invalid rule segment: {segment!r}")
        left_part, rest = segment.split("--", 1)
        edge_label, right_part = rest.split("-->", 1)
        left = parse_side(left_part)
        right = parse_side(right_part)
        edges.append(
            EdgePattern(left=left, edge_label=edge_label.strip(), right=right)
        )
    return RulePattern(edges=edges)


def match_side(pattern: SidePattern, node: dict) -> Optional[Dict[str, str]]:
    if pattern.op and node.get("op") != pattern.op:
        return None
    node_id = node.get("id")
    if not node_id:
        return None
    return {pattern.var: node_id}


def merge_bindings(
    a: Optional[Dict[str, str]], b: Optional[Dict[str, str]]
) -> Optional[Dict[str, str]]:
    if a is None or b is None:
        return None
    merged = dict(a)
    for key, value in b.items():
        if key in merged and merged[key] != value:
            return None
        merged[key] = value
    return merged


def match_edge_pattern(
    pattern: EdgePattern, edge: dict, nodes: Dict[str, dict], bindings: Dict[str, str]
) -> Optional[Dict[str, str]]:
    if edge.get("label") != pattern.edge_label:
        return None
    from_node = nodes.get(edge.get("from"))
    to_node = nodes.get(edge.get("to"))
    if not from_node or not to_node:
        return None
    bind_left = match_side(pattern.left, from_node)
    bind_right = match_side(pattern.right, to_node)
    merged = merge_bindings(bindings, merge_bindings(bind_left, bind_right))
    return merged


def find_first_match(
    diagram: dict, pattern: RulePattern, forbidden_edges: Optional[Set[str]] = None
) -> Optional[Dict[str, str]]:
    nodes = {n.get("id"): n for n in diagram.get("nodes", []) if n.get("id")}
    edges = list(diagram.get("edges", []))
    used: Set[str] = set()
    forbidden = forbidden_edges or set()

    def backtrack(
        idx: int, bindings: Dict[str, str], used_edges: Set[str]
    ) -> Optional[Dict[str, str]]:
        if idx >= len(pattern.edges):
            bindings["_edge_ids"] = ",".join(sorted(used_edges))
            return bindings
        edge_pattern = pattern.edges[idx]
        for edge in edges:
            edge_id = edge.get("id")
            if not edge_id or edge_id in used_edges or edge_id in forbidden:
                continue
            new_bindings = match_edge_pattern(edge_pattern, edge, nodes, bindings)
            if new_bindings is None:
                continue
            next_used = set(used_edges)
            next_used.add(edge_id)
            result = backtrack(idx + 1, new_bindings, next_used)
            if result is not None:
                return result
        return None

    for start_edge in edges:
        start_id = start_edge.get("id")
        if not start_id or start_id in forbidden:
            continue
        initial = match_edge_pattern(pattern.edges[0], start_edge, nodes, {})
        if initial is None:
            continue
        result = backtrack(1, initial, {start_id})
        if result is not None:
            return result
    return None


def find_all_matches(diagram: dict, pattern: RulePattern, max_matches: int = 1000) -> List[Dict[str, str]]:
    """
    Find all matches of a pattern in a diagram.

    Args:
        diagram: The diagram to search
        pattern: The pattern to match
        max_matches: Maximum number of matches to prevent exponential blowup (default 1000)

    Returns:
        List of variable bindings for each match
    """
    matches: List[Dict[str, str]] = []
    forbidden: Set[str] = set()

    while len(matches) < max_matches:
        match = find_first_match(diagram, pattern, forbidden_edges=forbidden)
        if not match:
            break
        edge_ids_raw = match.get("_edge_ids", "")
        if edge_ids_raw:
            forbidden.update(edge_ids_raw.split(","))
        matches.append(match)

    return matches


def _next_id(diagram: dict, prefix: str, id_type: str = "node") -> str:
    """
    Generate next unique ID for diagram element.
    Unified implementation for nodes and edges (Minor Issue #8).

    Args:
        diagram: The diagram to check for existing IDs
        prefix: ID prefix (e.g., "n", "e", "rule_n")
        id_type: "node" or "edge" - which collection to check
    """
    if id_type == "node":
        existing = {n.get("id") for n in diagram.get("nodes", []) if n.get("id")}
    else:  # edge
        existing = {e.get("id") for e in diagram.get("edges", []) if e.get("id")}

    idx = 1
    while True:
        candidate = f"{prefix}{idx}"
        if candidate not in existing:
            return candidate
        idx += 1


def next_node_id(diagram: dict, prefix: str) -> str:
    """Generate next unique node ID. Wrapper for backward compatibility."""
    return _next_id(diagram, prefix, "node")


def apply_replacement(
    diagram: dict, replacement: RulePattern, bindings: Dict[str, str], rule_id: str
) -> dict:
    nodes = list(diagram.get("nodes", []))
    edges = list(diagram.get("edges", []))

    nodes_by_id = {n.get("id"): n for n in nodes if n.get("id")}
    wrapper_cache: Dict[Tuple[str, str], str] = {}

    def ensure_node(side: SidePattern) -> str:
        node_id = bindings.get(side.var)
        if node_id:
            if side.op is None:
                return node_id
            cache_key = (side.op, node_id)
            cached = wrapper_cache.get(cache_key)
            if cached:
                return cached
            new_id = next_node_id({"nodes": nodes}, f"{rule_id}_n")
            new_node = {"id": new_id, "op": side.op, "inputs": [node_id]}
            nodes.append(new_node)
            nodes_by_id[new_id] = new_node
            wrapper_cache[cache_key] = new_id
            return new_id
        if side.op is None:
            raise ValueError(f"Unbound variable {side.var} requires op in replacement")
        new_id = next_node_id({"nodes": nodes}, f"{rule_id}_n")
        new_node = {"id": new_id, "op": side.op, "inputs": []}
        nodes.append(new_node)
        nodes_by_id[new_id] = new_node
        bindings[side.var] = new_id
        return new_id

    used_edge_ids = set()
    edge_ids_raw = bindings.get("_edge_ids", "")
    if edge_ids_raw:
        used_edge_ids.update(edge_ids_raw.split(","))
    edges = [e for e in edges if e.get("id") not in used_edge_ids]

    for edge_pattern in replacement.edges:
        new_from_id = ensure_node(edge_pattern.left)
        new_to_id = ensure_node(edge_pattern.right)
        new_edge_id = next_node_id(
            {"nodes": [{"id": e.get("id")} for e in edges]}, f"{rule_id}_e"
        )
        edges.append(
            {
                "id": new_edge_id,
                "from": new_from_id,
                "to": new_to_id,
                "label": edge_pattern.edge_label,
            }
        )

    new_diagram = dict(diagram)
    new_diagram["nodes"] = nodes
    new_diagram["edges"] = edges
    return new_diagram


def next_edge_id(diagram: dict, prefix: str) -> str:
    """Generate next unique edge ID. Wrapper for backward compatibility."""
    return _next_id(diagram, prefix, "edge")


def is_variable(atom: str) -> bool:
    """Check if an atom name represents a variable (for pattern matching)."""
    if atom.startswith("$"):
        return True
    return len(atom) == 1 and atom.islower()


def _has_cycle(diagram: dict) -> bool:
    """Check if diagram contains cycles using DFS."""
    nodes = {n.get("id"): n for n in diagram.get("nodes", []) if n.get("id")}
    edges_by_from = {}
    for edge in diagram.get("edges", []):
        from_id = edge.get("from")
        if from_id not in edges_by_from:
            edges_by_from[from_id] = []
        edges_by_from[from_id].append(edge.get("to"))

    visited = set()
    rec_stack = set()

    def has_cycle_dfs(node_id: str) -> bool:
        visited.add(node_id)
        rec_stack.add(node_id)
        for neighbor in edges_by_from.get(node_id, []):
            if neighbor not in visited:
                if has_cycle_dfs(neighbor):
                    return True
            elif neighbor in rec_stack:
                return True
        rec_stack.remove(node_id)
        return False

    for node_id in nodes:
        if node_id not in visited:
            if has_cycle_dfs(node_id):
                return True
    return False


def match_expr(
    expr: Expr,
    node_id: str,
    nodes: Dict[str, dict],
    bindings: Dict[str, str],
    matched: Set[str],
    bound_nodes: Set[str],
) -> Optional[Dict[str, str]]:
    node = nodes.get(node_id)
    if not node:
        return None
    if isinstance(expr, Atom):
        if is_variable(expr.name):
            var_name = expr.name.lstrip("$")
            existing = bindings.get(var_name)
            if existing and existing != node_id:
                return None
            bindings = dict(bindings)
            bindings[var_name] = node_id
            bound_nodes.add(node_id)
            return bindings
        if node.get("op") == "P" and expr.name in node.get("dof_refs", []):
            return bindings
        meta_args = node.get("meta", {}).get("atom_args", [])
        if expr.name in meta_args:
            return bindings
        return None

    if isinstance(expr, OpExpr):
        if node.get("op") != expr.op:
            return None
        input_ids = node.get("inputs", [])
        matched.add(node_id)
        if input_ids:
            if len(input_ids) < len(expr.args):
                return None
            for arg_expr, input_id in zip(expr.args, input_ids):
                result = match_expr(
                    arg_expr, input_id, nodes, bindings, matched, bound_nodes
                )
                if result is None:
                    return None
                bindings = result
        else:
            if expr.op == "P" and expr.args:
                arg = expr.args[0]
                if isinstance(arg, Atom):
                    if is_variable(arg.name):
                        var_name = arg.name.lstrip("$")
                        existing = bindings.get(var_name)
                        if existing and existing != node_id:
                            return None
                        bindings = dict(bindings)
                        bindings[var_name] = node_id
                        bound_nodes.add(node_id)
                    else:
                        if arg.name not in node.get("dof_refs", []):
                            return None
                else:
                    return None
            elif expr.args:
                return None
        return bindings
    return None


def find_expr_match(
    diagram: dict, expr: Expr
) -> Optional[Tuple[str, Dict[str, str], Set[str], Set[str]]]:
    nodes = {n.get("id"): n for n in diagram.get("nodes", []) if n.get("id")}
    for node_id in nodes:
        matched: Set[str] = set()
        bound_nodes: Set[str] = set()
        bindings: Dict[str, str] = {}
        result = match_expr(expr, node_id, nodes, bindings, matched, bound_nodes)
        if result is not None:
            return node_id, result, matched, bound_nodes
    return None


def build_expr(
    expr: Expr,
    nodes: List[dict],
    edges: List[dict],
    bindings: Dict[str, str],
    rule_id: str,
) -> str:
    def new_node_id() -> str:
        return next_node_id({"nodes": nodes}, f"{rule_id}_n")

    def new_edge_id() -> str:
        return next_edge_id({"edges": edges}, f"{rule_id}_e")

    if isinstance(expr, Atom):
        if is_variable(expr.name):
            var_name = expr.name.lstrip("$")
            node_id = bindings.get(var_name)
            if not node_id:
                raise ValueError(f"Unbound variable {expr.name}")
            return node_id
        node_id = new_node_id()
        nodes.append({"id": node_id, "op": "P", "dof_refs": [expr.name]})
        return node_id

    if isinstance(expr, OpExpr):
        input_ids = [
            build_expr(a, nodes, edges, bindings, rule_id) for a in expr.args
        ]
        node_id = new_node_id()
        node: dict = {"id": node_id, "op": expr.op}
        if input_ids:
            node["inputs"] = input_ids
        if expr.op == "P" and expr.args and isinstance(expr.args[0], Atom):
            atom = expr.args[0]
            if not is_variable(atom.name):
                node["dof_refs"] = [atom.name]
        if expr.op == "O":
            node["irreversible"] = True
        nodes.append(node)
        for input_id in input_ids:
            edges.append(
                {"id": new_edge_id(), "from": input_id, "to": node_id, "label": "arg"}
            )
        return node_id
    raise ValueError("Unknown expression type")


def apply_rewrite_stub(
    constraints: List[dict],
    state: dict,
    diagram: dict,
    csi: dict,
    rewrite_rule: dict,
    apply_all: bool = False,
) -> RewriteResult:
    from sid_crf import assign_inu_labels

    # Assign I/N/U labels before authorization
    if "inu_labels" not in state:
        state["inu_labels"] = assign_inu_labels(diagram, constraints, state, csi)

    authorized, errors, warnings = authorize_rewrite(
        constraints, state, diagram, csi, rewrite_rule
    )
    messages = []
    for warn in warnings:
        messages.append(f"WARNING: {warn}")
    if not authorized:
        for err in errors:
            messages.append(f"ERROR: {err}")
        return RewriteResult(applied=False, diagram=copy.deepcopy(diagram), messages=tuple(messages))

    pattern_expr_text = rewrite_rule.get("pattern_expr")
    replacement_expr_text = rewrite_rule.get("replacement_expr")
    if pattern_expr_text and replacement_expr_text:
        try:
            pattern_expr = parse_expression(pattern_expr_text)
            replacement_expr = parse_expression(replacement_expr_text)
        except ValueError as exc:
            messages.append(f"ERROR: {exc}")
            return RewriteResult(applied=False, diagram=copy.deepcopy(diagram), messages=tuple(messages))

        match = find_expr_match(diagram, pattern_expr)
        if not match:
            messages.append(f"Rewrite {rewrite_rule.get('id')} not applicable")
            return RewriteResult(applied=False, diagram=copy.deepcopy(diagram), messages=tuple(messages))
        root_id, bindings, matched_nodes, bound_nodes = match

        nodes = list(diagram.get("nodes", []))
        edges = list(diagram.get("edges", []))
        new_root = build_expr(
            replacement_expr,
            nodes,
            edges,
            bindings,
            rewrite_rule.get("id", "rw"),
        )

        remove_nodes = matched_nodes - bound_nodes
        new_edges = []
        for edge in edges:
            from_id = edge.get("from")
            to_id = edge.get("to")
            if to_id in remove_nodes:
                edge = dict(edge)
                edge["to"] = new_root
                new_edges.append(edge)
                continue
            if from_id in remove_nodes or to_id in remove_nodes:
                continue
            new_edges.append(edge)

        new_nodes = [n for n in nodes if n.get("id") not in remove_nodes]
        new_diagram = dict(diagram)
        new_diagram["nodes"] = new_nodes
        new_diagram["edges"] = new_edges

        # Validate no cycles introduced
        if _has_cycle(new_diagram):
            messages.append(f"ERROR: Rewrite {rewrite_rule.get('id')} would introduce cycle")
            return RewriteResult(applied=False, diagram=copy.deepcopy(diagram), messages=tuple(messages))

        messages.append(f"Rewrite {rewrite_rule.get('id')} applied")
        return RewriteResult(applied=True, diagram=copy.deepcopy(new_diagram), messages=tuple(messages))

    try:
        pattern = parse_rule_pattern(rewrite_rule.get("pattern", ""))
        replacement = parse_rule_pattern(rewrite_rule.get("replacement", ""))
    except ValueError as exc:
        messages.append(f"ERROR: {exc}")
        return RewriteResult(applied=False, diagram=copy.deepcopy(diagram), messages=tuple(messages))

    bindings_list = (
        find_all_matches(diagram, pattern) if apply_all else [find_first_match(diagram, pattern)]
    )
    bindings_list = [b for b in bindings_list if b]
    if not bindings_list:
        messages.append(f"Rewrite {rewrite_rule.get('id')} not applicable")
        return RewriteResult(applied=False, diagram=copy.deepcopy(diagram), messages=tuple(messages))

    current = diagram
    applied_any = False
    for bindings in bindings_list:
        try:
            current = apply_replacement(
                current, replacement, bindings, rewrite_rule.get("id", "rw")
            )
        except ValueError as exc:
            messages.append(f"ERROR: {exc}")
            return RewriteResult(applied=applied_any, diagram=copy.deepcopy(current), messages=tuple(messages))
        applied_any = True
    messages.append(f"Rewrite {rewrite_rule.get('id')} applied")
    return RewriteResult(applied=applied_any, diagram=copy.deepcopy(current), messages=tuple(messages))


def apply_rewrites_stub(
    pkg: dict,
    state_id: str,
    diagram_id: str,
    max_iterations: int = MAX_REWRITE_ITERATIONS,
) -> RewriteResult:
    """
    Apply all rewrite rules to a diagram iteratively.

    Args:
        pkg: SID package
        state_id: State ID to use
        diagram_id: Diagram ID to rewrite
        max_iterations: Maximum iterations to prevent infinite loops (Minor Issue #9)

    Returns:
        RewriteResult with final diagram
    """
    diagrams = {d["id"]: d for d in pkg.get("diagrams", []) if "id" in d}
    states = {s["id"]: s for s in pkg.get("states", []) if "id" in s}
    csis = {c["id"]: c for c in pkg.get("csis", []) if "id" in c}
    constraints = list(pkg.get("constraints", []))
    rules = list(pkg.get("rewrite_rules", []))

    state = states.get(state_id)
    diagram = diagrams.get(diagram_id)
    csi = csis.get(state.get("csi_id")) if state else None

    if not state or not diagram or not csi:
        return RewriteResult(
            applied=False,
            diagram=copy.deepcopy(diagram or {}),
            messages=("ERROR: missing state, diagram, or CSI",),
        )

    messages: List[str] = []
    current = diagram
    applied_any = False
    iteration = 0

    for rule in rules:
        if iteration >= max_iterations:
            logger.warning(f"Rewrite iteration limit ({max_iterations}) reached")
            messages.append(f"WARNING: Iteration limit ({max_iterations}) reached")
            break

        result = apply_rewrite_stub(
            constraints, state, current, csi, rule, apply_all=True
        )
        messages.extend(result.messages)
        current = result.diagram
        applied_any = applied_any or result.applied
        iteration += 1

    return RewriteResult(applied=applied_any, diagram=copy.deepcopy(current), messages=tuple(messages))


def apply_rewrites_to_package(pkg: dict, state_id: str, diagram_id: str) -> RewriteResult:
    result = apply_rewrites_stub(pkg, state_id, diagram_id)
    if not result.applied:
        return result
    diagrams = pkg.get("diagrams", [])
    for idx, diagram in enumerate(diagrams):
        if diagram.get("id") == diagram_id:
            diagrams[idx] = result.diagram
            break
    pkg["diagrams"] = diagrams
    states = pkg.get("states", [])
    constraints = list(pkg.get("constraints", []))
    csis = {c["id"]: c for c in pkg.get("csis", []) if "id" in c}
    from sid_crf import assign_inu_labels
    for state in states:
        if state.get("diagram_id") != diagram_id:
            continue
        csi = csis.get(state.get("csi_id"))
        if not csi:
            continue
        state["inu_labels"] = assign_inu_labels(result.diagram, constraints, state, csi)
    pkg["states"] = states
    return result


def rule_applicable(diagram: dict, rule: dict) -> bool:
    pattern_expr_text = rule.get("pattern_expr")
    replacement_expr_text = rule.get("replacement_expr")
    if pattern_expr_text and replacement_expr_text:
        try:
            pattern_expr = parse_expression(pattern_expr_text)
        except ValueError:
            return False
        return find_expr_match(diagram, pattern_expr) is not None
    pattern_text = rule.get("pattern")
    replacement_text = rule.get("replacement")
    if not pattern_text or not replacement_text:
        return False
    try:
        pattern = parse_rule_pattern(pattern_text)
    except ValueError:
        return False
    return find_first_match(diagram, pattern) is not None
