from __future__ import annotations

import logging
from typing import Dict, List, Optional, Tuple
from collections import deque

# Logger
logger = logging.getLogger(__name__)

# Configuration (Minor Issue #12: prevent unbounded loop history)
MAX_LOOP_HISTORY = 100  # Keep last 100 iterations only


def check_no_admissible_rewrites(
    pkg: dict,
    state_id: str,
    diagram_id: str
) -> Tuple[bool, str]:
    """
    Check if no admissible rewrites remain for the given state/diagram.
    This is one of the termination conditions from Section 9.
    """
    from sid_crf import authorize_rewrite

    states = {s["id"]: s for s in pkg.get("states", []) if "id" in s}
    diagrams = {d["id"]: d for d in pkg.get("diagrams", []) if "id" in d}
    csis = {c["id"]: c for c in pkg.get("csis", []) if "id" in c}
    constraints = list(pkg.get("constraints", []))
    rewrite_rules = list(pkg.get("rewrite_rules", []))

    state = states.get(state_id)
    diagram = diagrams.get(diagram_id)
    csi = csis.get(state.get("csi_id")) if state else None

    if not state or not diagram or not csi:
        return False, "Missing state, diagram, or CSI"

    from sid_rewrite import rule_applicable

    # Check if any rewrite is authorized and applicable
    for rule in rewrite_rules:
        authorized, errors, warnings = authorize_rewrite(
            constraints, state, diagram, csi, rule
        )
        if authorized and rule_applicable(diagram, rule):
            return False, f"Rewrite {rule.get('id')} is still admissible"

    return True, "No admissible rewrites remain"


def check_invariant_under_transport(
    diagram: dict,
    state: dict,
    csi: dict,
    constraints: List[dict]
) -> Tuple[bool, str]:
    """
    Check if admissible region is invariant under Transport operations.
    This means Transport (T) operators don't change what's admissible.

    Fixed: Only checks that existing admissible nodes remain admissible,
    allows new nodes to be added.
    """
    from sid_crf import assign_inu_labels

    current_labels = state.get("inu_labels", {})
    computed_labels = assign_inu_labels(diagram, constraints, state, csi)

    # Find all Transport nodes
    transport_nodes = [n for n in diagram.get("nodes", []) if n.get("op") == "T"]

    if not transport_nodes:
        return True, "No transport operations present"

    # Check if any transport node could change admissibility
    admissible_ids = {k for k, v in computed_labels.items() if v == "I"}

    for t_node in transport_nodes:
        node_id = t_node.get("id")
        # If transport node itself is admissible, the region is preserved
        if node_id not in admissible_ids:
            return False, f"Transport node {node_id} is not in admissible region"

    # Only check that existing admissible elements remain admissible
    # (Allow new elements to be added without violating invariance)
    for key in current_labels:
        if current_labels.get(key) == "I" and computed_labels.get(key) != "I":
            return False, f"Previously admissible element {key} is no longer admissible"

    return True, "Admissible region invariant under transport"


def check_only_identity_rewrites(
    rewrite_rules: List[dict],
    diagram: dict
) -> Tuple[bool, str]:
    """
    Check if only identity rewrites are authorized.
    Identity rewrite: pattern == replacement (no actual change).
    """
    for rule in rewrite_rules:
        pattern = rule.get("pattern") or rule.get("pattern_expr")
        replacement = rule.get("replacement") or rule.get("replacement_expr")

        if pattern != replacement:
            return False, f"Non-identity rewrite {rule.get('id')} present"

    return True, "Only identity rewrites authorized"


def check_loop_convergence(
    state: dict,
    tolerance: float = 1e-6
) -> Tuple[bool, str]:
    """
    Check if loop gain has converged within tolerance.
    Loop gain represents the amount of change in successive iterations.
    """
    loop_history = state.get("loop_history", [])

    if len(loop_history) < 2:
        return False, "Insufficient loop history for convergence check"

    # Calculate change between last two iterations
    if len(loop_history) >= 2:
        prev = loop_history[-2]
        curr = loop_history[-1]

        # Simple metric: count of changed elements
        prev_state = prev.get("inu_labels", {})
        curr_state = curr.get("inu_labels", {})

        changes = 0
        all_keys = set(prev_state.keys()) | set(curr_state.keys())
        for key in all_keys:
            if prev_state.get(key) != curr_state.get(key):
                changes += 1

        if changes == 0:
            return True, "Loop has fully converged (no changes)"

        change_ratio = changes / len(all_keys) if all_keys else 0
        if change_ratio < tolerance:
            return True, f"Loop gain converged (change ratio: {change_ratio:.6f})"

        return False, f"Loop not converged (change ratio: {change_ratio:.6f})"

    return False, "Cannot determine convergence"


def is_structurally_stable(
    pkg: dict,
    state_id: str,
    diagram_id: str,
    tolerance: float = 1e-6,
    require_all: bool = False
) -> Tuple[bool, List[str], str]:
    """
    Determine if a semantic process is structurally stable.

    From Section 9, a process is stable when:
    1. No admissible rewrites remain, OR
    2. Admissible region is invariant under transport, OR
    3. CRF authorizes only identity rewrites, OR
    4. Loop gain converges within tolerance

    Args:
        pkg: SID package
        state_id: State ID to check
        diagram_id: Diagram ID to check
        tolerance: Convergence tolerance for loop gain
        require_all: If True, require ALL conditions (AND semantics).
                    If False, require ANY condition (OR semantics).
                    Default False per spec (Minor Issue #10).

    Returns: (is_stable, satisfied_conditions, overall_message)
    """
    states = {s["id"]: s for s in pkg.get("states", []) if "id" in s}
    diagrams = {d["id"]: d for d in pkg.get("diagrams", []) if "id" in d}
    csis = {c["id"]: c for c in pkg.get("csis", []) if "id" in c}
    rewrite_rules = list(pkg.get("rewrite_rules", []))

    state = states.get(state_id)
    diagram = diagrams.get(diagram_id)
    csi = csis.get(state.get("csi_id")) if state else None

    if not state or not diagram or not csi:
        return False, [], "Missing state, diagram, or CSI"

    satisfied = []

    # Check condition 1: No admissible rewrites
    no_rewrites, msg1 = check_no_admissible_rewrites(pkg, state_id, diagram_id)
    if no_rewrites:
        satisfied.append(f"[OK] {msg1}")

    # Check condition 2: Invariant under transport
    invariant, msg2 = check_invariant_under_transport(
        diagram, state, csi, list(pkg.get("constraints", []))
    )
    if invariant:
        satisfied.append(f"[OK] {msg2}")

    # Check condition 3: Only identity rewrites
    identity_only, msg3 = check_only_identity_rewrites(rewrite_rules, diagram)
    if identity_only:
        satisfied.append(f"[OK] {msg3}")

    # Check condition 4: Loop convergence
    converged, msg4 = check_loop_convergence(state, tolerance)
    if converged:
        satisfied.append(f"[OK] {msg4}")

    # Determine stability based on require_all parameter
    if require_all:
        # AND semantics: all conditions must be met
        num_total = 4
        is_stable = len(satisfied) == num_total
        if is_stable:
            message = f"System is STABLE (all {num_total} conditions met)"
        else:
            message = f"System is NOT STABLE ({len(satisfied)}/{num_total} conditions met, need all)"
    else:
        # OR semantics: any condition met (default per spec)
        is_stable = len(satisfied) > 0
        if is_stable:
            message = f"System is STABLE ({len(satisfied)} condition(s) met)"
        else:
            message = "System is NOT STABLE (no termination conditions met)"

    logger.info(f"Stability check: {message}")
    return is_stable, satisfied, message


def compute_stability_metrics(pkg: dict, state_id: str, diagram_id: str) -> dict:
    """
    Compute optional metrics after stability is achieved.
    From Section 8: Metrics are earned, not assumed.

    Returns dictionary of metrics:
    - admissible_volume: Count of admissible (I) elements
    - collapse_ratio: Ratio of Collapse nodes to total nodes
    - gradient_coherence: Measure of coupling consistency
    - transport_fidelity: Measure of compartment transition quality
    - loop_gain: Rate of change in recent iterations
    """
    states = {s["id"]: s for s in pkg.get("states", []) if "id" in s}
    diagrams = {d["id"]: d for d in pkg.get("diagrams", []) if "id" in d}

    state = states.get(state_id)
    diagram = diagrams.get(diagram_id)

    if not state or not diagram:
        return {}

    metrics = {}

    # Admissible volume
    inu_labels = state.get("inu_labels", {})
    admissible_count = sum(1 for v in inu_labels.values() if v == "I")
    total_count = len(inu_labels)
    metrics["admissible_volume"] = admissible_count
    metrics["admissible_ratio"] = admissible_count / total_count if total_count > 0 else 0

    # Collapse ratio
    nodes = diagram.get("nodes", [])
    collapse_nodes = [n for n in nodes if n.get("op") == "O"]
    metrics["collapse_count"] = len(collapse_nodes)
    metrics["collapse_ratio"] = len(collapse_nodes) / len(nodes) if nodes else 0

    # Gradient coherence (measure of coupling)
    coupling_nodes = [n for n in nodes if n.get("op") == "C"]
    metrics["coupling_count"] = len(coupling_nodes)
    metrics["gradient_coherence"] = len(coupling_nodes) / len(nodes) if nodes else 0

    # Transport fidelity
    transport_nodes = [n for n in nodes if n.get("op") == "T"]
    valid_transports = sum(
        1 for n in transport_nodes
        if n.get("meta", {}).get("target_compartment")
    )
    metrics["transport_count"] = len(transport_nodes)
    # Fixed: Return None (undefined) when no transport nodes, not 1.0 (perfect)
    metrics["transport_fidelity"] = valid_transports / len(transport_nodes) if transport_nodes else None

    # Loop gain (from history)
    loop_history = state.get("loop_history", [])
    if len(loop_history) >= 2:
        prev = loop_history[-2].get("inu_labels", {})
        curr = loop_history[-1].get("inu_labels", {})
        all_keys = set(prev.keys()) | set(curr.keys())
        changes = sum(1 for k in all_keys if prev.get(k) != curr.get(k))
        metrics["loop_gain"] = changes / len(all_keys) if all_keys else 0
    else:
        metrics["loop_gain"] = None

    return metrics
