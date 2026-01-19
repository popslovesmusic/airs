from __future__ import annotations

import json
import logging
import sys
from dataclasses import dataclass
from typing import Dict, List, Set, Tuple

from sid_crf import authorize_rewrite, evaluate_constraints
from sid_parser import parse_expression

# Logger
logger = logging.getLogger(__name__)


@dataclass(frozen=True)
class ValidationError:
    """Structured validation error (Minor Issue #13)."""
    category: str  # e.g., "duplicate_id", "missing_id", "missing_reference"
    severity: str  # "error" or "warning"
    message: str
    context: dict  # Additional context (item_type, id, etc.)

    def __str__(self) -> str:
        return self.message


def load_package(path: str) -> dict:
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def index_by_id(items: List[dict], item_type: str = "item") -> Tuple[Dict[str, dict], List[ValidationError]]:
    """
    Index items by id, returning index and any errors found.

    Returns structured ValidationError objects (Minor Issue #13).
    """
    index = {}
    errors = []
    for item in items:
        item_id = item.get("id")
        if not item_id:
            errors.append(ValidationError(
                category="missing_id",
                severity="error",
                message=f"{item_type} missing id",
                context={"item_type": item_type, "item": item}
            ))
            continue
        if item_id in index:
            errors.append(ValidationError(
                category="duplicate_id",
                severity="error",
                message=f"Duplicate {item_type} id: {item_id}",
                context={"item_type": item_type, "id": item_id}
            ))
            continue
        index[item_id] = item
    return index, errors


def node_dofs(node: dict) -> List[str]:
    return list(node.get("dof_refs", []))


def validate_collapse_irreversibility(diagrams: Dict[str, dict]) -> List[ValidationError]:
    """Validate all Collapse (O) operators are marked irreversible."""
    errors = []
    for diagram in diagrams.values():
        for node in diagram.get("nodes", []):
            if node.get("op") == "O" and not node.get("irreversible"):
                errors.append(ValidationError(
                    category="collapse_not_irreversible",
                    severity="error",
                    message=f"Collapse node {node.get('id')} in diagram {diagram.get('id')} "
                            "must set irreversible=true",
                    context={
                        "diagram_id": diagram.get("id"),
                        "node_id": node.get("id"),
                        "op": "O"
                    }
                ))
    return errors


def pair_key(a: str, b: str) -> Tuple[str, str]:
    return (a, b)


def validate_csi_boundaries(
    diagrams: Dict[str, dict], states: Dict[str, dict], csis: Dict[str, dict], dofs: Dict[str, dict]
) -> List[ValidationError]:
    """
    Validate CSI boundaries and DOF usage.

    Includes validation that allowed_pairs reference valid DOFs (Minor Issue #14).
    """
    errors = []
    for state in states.values():
        diagram_id = state.get("diagram_id")
        csi_id = state.get("csi_id")
        if diagram_id not in diagrams or csi_id not in csis:
            errors.append(ValidationError(
                category="missing_reference",
                severity="error",
                message=f"State {state.get('id')} references missing diagram or CSI",
                context={
                    "state_id": state.get("id"),
                    "diagram_id": diagram_id,
                    "csi_id": csi_id
                }
            ))
            continue
        diagram = diagrams[diagram_id]
        csi = csis[csi_id]
        allowed_dofs: Set[str] = set(csi.get("allowed_dofs", []))
        allowed_pairs_raw = csi.get("allowed_pairs", [])

        # Minor Issue #14: Validate that allowed_pairs reference valid DOFs
        for pair in allowed_pairs_raw:
            if len(pair) != 2:
                errors.append(ValidationError(
                    category="invalid_csi_pair",
                    severity="error",
                    message=f"CSI {csi_id} allowed_pair must have exactly 2 elements, got {len(pair)}",
                    context={"csi_id": csi_id, "pair": pair}
                ))
                continue
            dof_a, dof_b = pair[0], pair[1]
            if dof_a not in dofs:
                errors.append(ValidationError(
                    category="invalid_csi_pair_dof",
                    severity="error",
                    message=f"CSI {csi_id} allowed_pair references unknown DOF: {dof_a}",
                    context={"csi_id": csi_id, "pair": pair, "unknown_dof": dof_a}
                ))
            if dof_b not in dofs:
                errors.append(ValidationError(
                    category="invalid_csi_pair_dof",
                    severity="error",
                    message=f"CSI {csi_id} allowed_pair references unknown DOF: {dof_b}",
                    context={"csi_id": csi_id, "pair": pair, "unknown_dof": dof_b}
                ))

        allowed_pairs: Set[Tuple[str, str]] = set(
            pair_key(p[0], p[1]) for p in allowed_pairs_raw if len(p) == 2
        )
        nodes = {n["id"]: n for n in diagram.get("nodes", []) if "id" in n}

        for node in diagram.get("nodes", []):
            for dof in node_dofs(node):
                if dof not in allowed_dofs:
                    errors.append(ValidationError(
                        category="dof_outside_csi",
                        severity="error",
                        message=f"Diagram {diagram_id} node {node.get('id')} "
                                f"uses DOF {dof} outside CSI {csi_id}",
                        context={
                            "diagram_id": diagram_id,
                            "node_id": node.get("id"),
                            "dof": dof,
                            "csi_id": csi_id
                        }
                    ))

        if allowed_pairs:
            for edge in diagram.get("edges", []):
                from_node = nodes.get(edge.get("from"))
                to_node = nodes.get(edge.get("to"))
                if not from_node or not to_node:
                    continue
                from_dofs = node_dofs(from_node)
                to_dofs = node_dofs(to_node)
                for fd in from_dofs:
                    for td in to_dofs:
                        if pair_key(fd, td) not in allowed_pairs:
                            errors.append(ValidationError(
                                category="csi_pair_violation",
                                severity="error",
                                message=f"Diagram {diagram_id} edge {edge.get('id')} "
                                        f"violates CSI {csi_id} pair ({fd}, {td})",
                                context={
                                    "diagram_id": diagram_id,
                                    "edge_id": edge.get("id"),
                                    "csi_id": csi_id,
                                    "pair": (fd, td)
                                }
                            ))
    return errors


def validate_diagram_integrity(
    diagrams: Dict[str, dict],
    dofs: Dict[str, dict],
    compartments: Dict[str, dict],
) -> List[ValidationError]:
    """Validate diagram structure and references."""
    errors = []
    for diagram in diagrams.values():
        diagram_id = diagram.get("id")
        node_ids = set()
        edge_ids = set()
        nodes = {}
        for node in diagram.get("nodes", []):
            node_id = node.get("id")
            if not node_id:
                errors.append(ValidationError(
                    category="missing_id",
                    severity="error",
                    message=f"Diagram {diagram_id} has node missing id",
                    context={"diagram_id": diagram_id, "node": node}
                ))
                continue
            if node_id in node_ids:
                errors.append(ValidationError(
                    category="duplicate_id",
                    severity="error",
                    message=f"Diagram {diagram_id} has duplicate node id {node_id}",
                    context={"diagram_id": diagram_id, "node_id": node_id}
                ))
            node_ids.add(node_id)
            nodes[node_id] = node
            for dof in node_dofs(node):
                if dof not in dofs:
                    errors.append(ValidationError(
                        category="missing_reference",
                        severity="error",
                        message=f"Diagram {diagram_id} node {node_id} references unknown DOF {dof}",
                        context={"diagram_id": diagram_id, "node_id": node_id, "dof": dof}
                    ))

        for edge in diagram.get("edges", []):
            edge_id = edge.get("id")
            if not edge_id:
                errors.append(ValidationError(
                    category="missing_id",
                    severity="error",
                    message=f"Diagram {diagram_id} has edge missing id",
                    context={"diagram_id": diagram_id, "edge": edge}
                ))
                continue
            if edge_id in edge_ids:
                errors.append(ValidationError(
                    category="duplicate_id",
                    severity="error",
                    message=f"Diagram {diagram_id} has duplicate edge id {edge_id}",
                    context={"diagram_id": diagram_id, "edge_id": edge_id}
                ))
            edge_ids.add(edge_id)
            from_id = edge.get("from")
            to_id = edge.get("to")
            if from_id not in nodes or to_id not in nodes:
                errors.append(ValidationError(
                    category="missing_reference",
                    severity="error",
                    message=f"Diagram {diagram_id} edge {edge_id} references missing node",
                    context={
                        "diagram_id": diagram_id,
                        "edge_id": edge_id,
                        "from": from_id,
                        "to": to_id
                    }
                ))

        for node in diagram.get("nodes", []):
            for input_id in node.get("inputs", []):
                if input_id not in nodes:
                    errors.append(ValidationError(
                        category="missing_reference",
                        severity="error",
                        message=f"Diagram {diagram_id} node {node.get('id')} "
                                f"references missing input {input_id}",
                        context={
                            "diagram_id": diagram_id,
                            "node_id": node.get("id"),
                            "input_id": input_id
                        }
                    ))

        compartment_id = diagram.get("compartment_id")
        if compartment_id and compartment_id not in compartments:
            errors.append(ValidationError(
                category="missing_reference",
                severity="error",
                message=f"Diagram {diagram_id} references missing compartment {compartment_id}",
                context={"diagram_id": diagram_id, "compartment_id": compartment_id}
            ))
    return errors


def validate_state_integrity(
    states: Dict[str, dict],
    diagrams: Dict[str, dict],
    csis: Dict[str, dict],
    compartments: Dict[str, dict],
) -> List[ValidationError]:
    """Validate state references."""
    errors = []
    for state in states.values():
        state_id = state.get("id")
        diagram_id = state.get("diagram_id")
        csi_id = state.get("csi_id")
        if diagram_id not in diagrams:
            errors.append(ValidationError(
                category="missing_reference",
                severity="error",
                message=f"State {state_id} references missing diagram {diagram_id}",
                context={"state_id": state_id, "diagram_id": diagram_id}
            ))
        if csi_id not in csis:
            errors.append(ValidationError(
                category="missing_reference",
                severity="error",
                message=f"State {state_id} references missing CSI {csi_id}",
                context={"state_id": state_id, "csi_id": csi_id}
            ))
        compartment_id = state.get("compartment_id")
        if compartment_id and compartment_id not in compartments:
            errors.append(ValidationError(
                category="missing_reference",
                severity="error",
                message=f"State {state_id} references missing compartment {compartment_id}",
                context={"state_id": state_id, "compartment_id": compartment_id}
            ))
    return errors


def validate_package(pkg: dict) -> Tuple[List[str], List[str]]:
    """
    Validate package and return (errors, warnings).

    Returns string messages for backward compatibility.
    Internal validation uses structured ValidationError objects (Minor Issue #13).

    Minor Issue #15: JSON Schema validation is not implemented.
    Rationale: JSON Schema adds external dependency and complexity.
    Current validation provides equivalent coverage via custom predicates.
    To add: Use jsonschema library with schema file.
    """
    validation_errors: List[ValidationError] = []
    warnings: List[str] = []

    diagrams, diag_errors = index_by_id(pkg.get("diagrams", []), "diagram")
    validation_errors.extend(diag_errors)
    csis, csi_errors = index_by_id(pkg.get("csis", []), "CSI")
    validation_errors.extend(csi_errors)
    dofs, dof_errors = index_by_id(pkg.get("dofs", []), "DOF")
    validation_errors.extend(dof_errors)
    compartments, comp_errors = index_by_id(pkg.get("compartments", []), "compartment")
    validation_errors.extend(comp_errors)
    states, state_errors = index_by_id(pkg.get("states", []), "state")
    validation_errors.extend(state_errors)
    constraints = list(pkg.get("constraints", []))
    rewrite_rules = list(pkg.get("rewrite_rules", []))

    validation_errors.extend(validate_diagram_integrity(diagrams, dofs, compartments))
    validation_errors.extend(validate_state_integrity(states, diagrams, csis, compartments))
    validation_errors.extend(validate_collapse_irreversibility(diagrams))
    validation_errors.extend(validate_csi_boundaries(diagrams, states, csis, dofs))  # Added dofs param

    for rule in rewrite_rules:
        has_edge_form = bool(rule.get("pattern") and rule.get("replacement"))
        has_expr_form = bool(rule.get("pattern_expr") and rule.get("replacement_expr"))
        if not has_edge_form and not has_expr_form:
            validation_errors.append(ValidationError(
                category="invalid_rewrite_rule",
                severity="error",
                message=f"Rewrite {rule.get('id')} must define "
                        "pattern+replacement or pattern_expr+replacement_expr",
                context={"rule_id": rule.get("id")}
            ))
        if has_expr_form:
            try:
                parse_expression(rule.get("pattern_expr", ""))
                parse_expression(rule.get("replacement_expr", ""))
            except ValueError as exc:
                validation_errors.append(ValidationError(
                    category="invalid_rewrite_expr",
                    severity="error",
                    message=f"Rewrite {rule.get('id')} has invalid expr: {exc}",
                    context={"rule_id": rule.get("id"), "error": str(exc)}
                ))

    # Initialize inu_labels for all states before validation
    from sid_crf import assign_inu_labels
    for state in states.values():
        diagram = diagrams.get(state.get("diagram_id"))
        csi = csis.get(state.get("csi_id"))
        if diagram and csi and "inu_labels" not in state:
            state["inu_labels"] = assign_inu_labels(diagram, constraints, state, csi)

    for state in states.values():
        diagram = diagrams.get(state.get("diagram_id"))
        csi = csis.get(state.get("csi_id"))
        if not diagram or not csi:
            continue

        # Evaluate constraints
        c_errors, c_warnings = evaluate_constraints(
            constraints, state, diagram, csi
        )
        warnings.extend(c_warnings)
        # Convert string errors to ValidationError for consistency
        for err_msg in c_errors:
            validation_errors.append(ValidationError(
                category="constraint_violation",
                severity="error",
                message=err_msg,
                context={"state_id": state.get("id")}
            ))

        # Removed S×R authorization loop - only validate rule well-formedness
        # Rule applicability is checked during execution, not validation

    # Convert ValidationError objects to strings for backward compatibility
    error_messages = [str(err) for err in validation_errors]
    logger.info(f"Validation complete: {len(error_messages)} errors, {len(warnings)} warnings")
    return error_messages, warnings


def main(argv: List[str]) -> int:
    if len(argv) != 2:
        print("Usage: python sid_validator.py <sid_package.json>")
        return 2
    pkg = load_package(argv[1])
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
    sys.exit(main(sys.argv))
