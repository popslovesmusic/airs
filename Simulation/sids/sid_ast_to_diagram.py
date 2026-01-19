from __future__ import annotations

from dataclasses import dataclass
from typing import Dict, List, Optional, Tuple

from sid_ast import Atom, Expr, OpExpr


@dataclass
class DiagramBuild:
    nodes: List[dict]
    edges: List[dict]
    counter: int = 0

    def next_id(self, prefix: str) -> str:
        self.counter += 1
        return f"{prefix}{self.counter}"


def expr_to_diagram(
    expr: Expr,
    diagram_id: str = "d_expr",
    compartment_id: Optional[str] = None,
) -> dict:
    """
    Convert an AST expression to a diagram representation.

    Args:
        expr: The expression AST to convert
        diagram_id: Unique identifier for the diagram
        compartment_id: Optional compartment this diagram belongs to.
                       Compartments represent spatial/temporal regions.
                       Validated later by sid_validator.py against package compartments.

    Returns:
        Dictionary with 'id', 'nodes', 'edges', and optionally 'compartment_id'
    """
    build = DiagramBuild(nodes=[], edges=[])

    def build_expr(node_expr: Expr) -> Tuple[Optional[str], Dict[str, List[str]]]:
        if isinstance(node_expr, Atom):
            return None, {"atoms": [node_expr.name]}

        if not isinstance(node_expr, OpExpr):
            raise ValueError("Unknown expression type")

        atom_args: List[str] = []
        input_ids: List[str] = []
        for arg in node_expr.args:
            child_id, info = build_expr(arg)
            atom_args.extend(info.get("atoms", []))
            if child_id:
                input_ids.append(child_id)

        node_id = build.next_id("n")
        node: dict = {"id": node_id, "op": node_expr.op}
        if input_ids:
            node["inputs"] = input_ids

        # Handle atom arguments based on operator semantics
        if node_expr.op == "P" and len(atom_args) == 1 and not input_ids:
            # P operator with single atom argument -> set dof_refs
            node["dof_refs"] = [atom_args[0]]
        elif node_expr.op in ("S+", "S-") and atom_args and not input_ids:
            # Superposition operators with atom arguments -> also use dof_refs
            node["dof_refs"] = atom_args
        elif atom_args:
            # Other operators: store atoms in metadata for reference
            # Note: This is for tracking purposes; actual semantics use input edges
            node["meta"] = {"atom_args": atom_args}

        build.nodes.append(node)

        for input_id in input_ids:
            edge_id = build.next_id("e")
            build.edges.append(
                {
                    "id": edge_id,
                    "from": input_id,
                    "to": node_id,
                    "label": "arg",  # Standardized: all edges represent argument relationships
                }
            )

        return node_id, {"atoms": []}

    node_id, atom_info = build_expr(expr)
    if node_id is None and atom_info.get("atoms"):
        node_id = build.next_id("n")
        build.nodes.append(
            {
                "id": node_id,
                "op": "P",
                "dof_refs": [atom_info["atoms"][0]],
                "meta": {"atom_only": True},
            }
        )

    diagram = {"id": diagram_id, "nodes": build.nodes, "edges": build.edges}
    if compartment_id:
        diagram["compartment_id"] = compartment_id

    # Validate diagram structure before returning
    _validate_diagram_structure(diagram)
    return diagram


def _validate_diagram_structure(diagram: dict) -> None:
    """Validate diagram has valid structure (no orphaned nodes, valid edge references)."""
    node_ids = {n.get("id") for n in diagram.get("nodes", []) if n.get("id")}

    # Check all edges reference valid nodes
    for edge in diagram.get("edges", []):
        from_id = edge.get("from")
        to_id = edge.get("to")

        if from_id not in node_ids:
            raise ValueError(
                f"Edge {edge.get('id')} references non-existent 'from' node: {from_id}"
            )
        if to_id not in node_ids:
            raise ValueError(
                f"Edge {edge.get('id')} references non-existent 'to' node: {to_id}"
            )

    # Check all node inputs reference valid nodes
    for node in diagram.get("nodes", []):
        for input_id in node.get("inputs", []):
            if input_id not in node_ids:
                raise ValueError(
                    f"Node {node.get('id')} references non-existent input node: {input_id}"
                )
