from __future__ import annotations

import logging
from typing import Callable, Dict, List, Optional, Tuple
from types import MappingProxyType


# Type aliases for predicates
PredicateResult = Tuple[bool, str]
PredicateFn = Callable[[dict, dict, dict], PredicateResult]

# Mutable registry for predicates (populated by decorators)
_PREDICATES_REGISTRY: Dict[str, PredicateFn] = {}

# Public immutable view (Minor Issue #6: prevent external mutation)
PREDICATES: MappingProxyType = MappingProxyType(_PREDICATES_REGISTRY)

# Logger for resolution paths (Minor Issue #5)
logger = logging.getLogger(__name__)


class ConflictResolution:
    """Result of conflict resolution procedure."""
    def __init__(self, action: str, success: bool, message: str, data: Optional[dict] = None, new_state: Optional[dict] = None):
        self.action = action
        self.success = success
        self.message = message
        self.data = data or {}
        self.new_state = new_state  # Modified state (if any)


def register_predicate(name: str) -> Callable[[PredicateFn], PredicateFn]:
    """Register a predicate function in the global registry."""
    def decorator(fn: PredicateFn) -> PredicateFn:
        _PREDICATES_REGISTRY[name] = fn
        logger.debug(f"Registered predicate: {name}")
        return fn

    return decorator


def attenuate(conflict_details: dict, state: dict, diagram: dict) -> ConflictResolution:
    """
    Attenuate: Weaken soft constraints to resolve conflict.
    Used when soft constraints can be relaxed without violating hard constraints.
    """
    constraint_id = conflict_details.get("constraint_id", "unknown")
    message = f"Attenuated soft constraint {constraint_id}"

    logger.info(f"CRF: Attenuating constraint {constraint_id}")

    # Create new state with attenuated constraint (pure function)
    new_state = dict(state)
    if "attenuated_constraints" not in new_state:
        new_state["attenuated_constraints"] = []
    else:
        new_state["attenuated_constraints"] = list(new_state["attenuated_constraints"])
    new_state["attenuated_constraints"].append(constraint_id)

    return ConflictResolution(
        action="attenuate",
        success=True,
        message=message,
        data={"constraint_id": constraint_id},
        new_state=new_state
    )


def defer(conflict_details: dict, state: dict, diagram: dict) -> ConflictResolution:
    """
    Defer: Postpone resolution to a later stage.
    Used when conflict cannot be resolved at current compartment level.
    """
    conflict_type = conflict_details.get("type", "unknown")
    message = f"Deferred conflict of type {conflict_type} to next compartment"

    # Create new state with deferred conflict (pure function)
    new_state = dict(state)
    if "deferred_conflicts" not in new_state:
        new_state["deferred_conflicts"] = []
    else:
        new_state["deferred_conflicts"] = list(new_state["deferred_conflicts"])
    new_state["deferred_conflicts"].append(conflict_details)

    return ConflictResolution(
        action="defer",
        success=True,
        message=message,
        data=conflict_details,
        new_state=new_state
    )


def partition(conflict_details: dict, state: dict, diagram: dict) -> ConflictResolution:
    """
    Partition: Split conflicting elements into separate compartments.
    Used when conflicting DOFs or operators can operate independently.
    """
    elements = conflict_details.get("conflicting_elements", [])
    message = f"Partitioned {len(elements)} conflicting elements into separate compartments"

    # Create new state with partitioned elements (pure function)
    new_state = dict(state)
    if "partitioned_elements" not in new_state:
        new_state["partitioned_elements"] = []
    else:
        new_state["partitioned_elements"] = list(new_state["partitioned_elements"])
    new_state["partitioned_elements"].extend(elements)

    return ConflictResolution(
        action="partition",
        success=True,
        message=message,
        data={"elements": elements},
        new_state=new_state
    )


def escalate(conflict_details: dict, state: dict, diagram: dict) -> ConflictResolution:
    """
    Escalate: Promote conflict to higher hierarchical level.
    Used when conflict requires global context or vertical causation.
    """
    conflict_scope = conflict_details.get("scope", "local")
    message = f"Escalated {conflict_scope} conflict to global scope"

    # Create new state with escalated conflict (pure function)
    new_state = dict(state)
    if "escalated_conflicts" not in new_state:
        new_state["escalated_conflicts"] = []
    else:
        new_state["escalated_conflicts"] = list(new_state["escalated_conflicts"])
    new_state["escalated_conflicts"].append(conflict_details)

    return ConflictResolution(
        action="escalate",
        success=True,
        message=message,
        data=conflict_details,
        new_state=new_state
    )


def bifurcate(conflict_details: dict, state: dict, diagram: dict) -> ConflictResolution:
    """
    Bifurcate: Create parallel states/diagrams for conflicting possibilities.
    Used when both conflicting paths are admissible and should be explored.
    """
    choices = conflict_details.get("choices", [])
    message = f"Bifurcated state into {len(choices)} parallel branches"

    # Create new state marked for bifurcation (pure function)
    new_state = dict(state)
    new_state["bifurcated"] = True
    new_state["bifurcation_choices"] = choices

    return ConflictResolution(
        action="bifurcate",
        success=True,
        message=message,
        data={"choices": choices},
        new_state=new_state
    )


def halt(conflict_details: dict, state: dict, diagram: dict) -> ConflictResolution:
    """
    Halt: Stop execution due to unresolvable conflict.
    Used when conflict violates hard constraints and cannot be resolved.
    """
    reason = conflict_details.get("reason", "Unresolvable hard constraint violation")
    message = f"Halted execution: {reason}"

    # Create new state marked as halted (pure function)
    new_state = dict(state)
    new_state["halted"] = True
    new_state["halt_reason"] = reason

    return ConflictResolution(
        action="halt",
        success=False,
        message=message,
        data=conflict_details,
        new_state=new_state
    )


def resolve_conflict(
    conflict_type: str,
    conflict_details: dict,
    state: dict,
    diagram: dict
) -> ConflictResolution:
    """
    Main conflict resolution dispatcher.
    Routes conflicts to appropriate resolution procedure based on type.

    Conflict types (in priority order from least to most severe):
    1. soft_violation: Soft constraint violated -> attenuate
    2. temporal_mismatch: Temporal issue -> defer
    3. dof_interference: DOF conflicts -> partition
    4. scope_overflow: Local scope insufficient -> escalate
    5. ambiguous_choice: Multiple valid paths -> bifurcate
    6. hard_violation: Hard constraint violated -> halt (highest priority)

    When multiple conflict types apply, use the highest priority (most severe).
    """
    logger.info(f"CRF: Resolving conflict type '{conflict_type}'")

    # Priority order (most severe first)
    if conflict_type == "hard_violation":
        return halt(conflict_details, state, diagram)
    elif conflict_type == "ambiguous_choice":
        return bifurcate(conflict_details, state, diagram)
    elif conflict_type == "scope_overflow":
        return escalate(conflict_details, state, diagram)
    elif conflict_type == "dof_interference":
        return partition(conflict_details, state, diagram)
    elif conflict_type == "temporal_mismatch":
        return defer(conflict_details, state, diagram)
    elif conflict_type == "soft_violation":
        return attenuate(conflict_details, state, diagram)
    else:
        # Unknown conflict type - default to halt for safety
        logger.warning(f"CRF: Unknown conflict type '{conflict_type}', halting")
        return halt(
            {"reason": f"Unknown conflict type: {conflict_type}"},
            state,
            diagram
        )


@register_predicate("no_cross_csi_interaction")
def no_cross_csi_interaction(state: dict, diagram: dict, csi: dict) -> PredicateResult:
    """Verify all interactions respect CSI boundaries."""
    allowed_pairs = set(
        tuple(pair) for pair in csi.get("allowed_pairs", []) if len(pair) == 2
    )
    if not allowed_pairs:
        return True, "No allowed_pairs defined; skipping pair check"

    nodes = {n.get("id"): n for n in diagram.get("nodes", []) if n.get("id")}
    for edge in diagram.get("edges", []):
        from_node = nodes.get(edge.get("from"))
        to_node = nodes.get(edge.get("to"))
        if not from_node or not to_node:
            continue
        from_dofs = from_node.get("dof_refs", [])
        to_dofs = to_node.get("dof_refs", [])
        for fd in from_dofs:
            for td in to_dofs:
                if (fd, td) not in allowed_pairs:
                    return False, f"Edge {edge.get('id')} violates CSI pair ({fd}, {td})"
    return True, "All edges within CSI pairs"


@register_predicate("collapse_irreversible")
def collapse_irreversible(state: dict, diagram: dict, csi: dict) -> PredicateResult:
    """Verify all Collapse (O) operators are marked irreversible."""
    for node in diagram.get("nodes", []):
        if node.get("op") == "O":
            # Strict: missing irreversible field is a violation
            if not node.get("irreversible"):
                return False, f"Collapse node {node.get('id')} must be marked irreversible"
    return True, "All collapse nodes properly marked"


@register_predicate("no_cycles")
def no_cycles(state: dict, diagram: dict, csi: dict) -> PredicateResult:
    """Check for cycles in diagram (cycles may indicate feedback loops)."""
    nodes = {n.get("id"): n for n in diagram.get("nodes", []) if n.get("id")}
    edges_by_from = {}
    for edge in diagram.get("edges", []):
        from_id = edge.get("from")
        if from_id not in edges_by_from:
            edges_by_from[from_id] = []
        edges_by_from[from_id].append(edge.get("to"))

    # DFS-based cycle detection
    visited = set()
    rec_stack = set()

    def has_cycle(node_id: str) -> bool:
        visited.add(node_id)
        rec_stack.add(node_id)
        for neighbor in edges_by_from.get(node_id, []):
            if neighbor not in visited:
                if has_cycle(neighbor):
                    return True
            elif neighbor in rec_stack:
                return True
        rec_stack.remove(node_id)
        return False

    for node_id in nodes:
        if node_id not in visited:
            if has_cycle(node_id):
                return False, f"Cycle detected involving node {node_id}"
    return True, "No cycles detected"


@register_predicate("valid_compartment_transitions")
def valid_compartment_transitions(state: dict, diagram: dict, csi: dict) -> PredicateResult:
    """Verify Transport (T) operators properly handle compartment transitions."""
    for node in diagram.get("nodes", []):
        if node.get("op") == "T":
            # Strict: Transport must have target compartment metadata
            meta = node.get("meta")
            if not meta or not meta.get("target_compartment"):
                return False, f"Transport node {node.get('id')} missing target_compartment in meta"
    return True, "All transport nodes valid"


def evaluate_constraints(
    constraints: List[dict],
    state: dict,
    diagram: dict,
    csi: dict,
) -> Tuple[List[str], List[str]]:
    errors: List[str] = []
    warnings: List[str] = []
    for constraint in constraints:
        predicate_name = constraint.get("predicate")
        if not predicate_name:
            continue
        predicate = PREDICATES.get(predicate_name)
        if not predicate:
            warnings.append(f"Unknown predicate {predicate_name}")
            continue
        ok, detail = predicate(state, diagram, csi)
        if ok:
            continue
        ctype = constraint.get("type", "hard")
        msg = f"{constraint.get('id')} failed: {detail}"
        if ctype == "hard":
            errors.append(msg)
        else:
            warnings.append(msg)
    return errors, warnings


def assign_inu_labels(
    diagram: dict,
    constraints: List[dict],
    state: dict,
    csi: dict
) -> dict:
    """
    Assign I/N/U labels to diagram elements based on constraints and admissibility.

    I (Is): Admissible, coherent, affirmed
    N (Not): Excluded, forbidden, contradictory
    U (Unknown): Unresolved, undecidable, open

    Optimized version: evaluates global constraints once, caches predicate results.
    """
    inu_labels = {}

    if not constraints:
        # No constraints: everything is admissible
        for node in diagram.get("nodes", []):
            if node.get("id"):
                inu_labels[node.get("id")] = "I"
        for edge in diagram.get("edges", []):
            if edge.get("id"):
                inu_labels[edge.get("id")] = "I"
        return inu_labels

    # Phase 1: Evaluate global constraints (operate on full diagram)
    global_hard_fail = False
    global_soft_fail = False

    for constraint in constraints:
        predicate_name = constraint.get("predicate")
        if not predicate_name:
            continue
        predicate = PREDICATES.get(predicate_name)
        if not predicate:
            continue

        ok, _ = predicate(state, diagram, csi)
        if not ok:
            if constraint.get("type") == "hard":
                global_hard_fail = True
                break
            global_soft_fail = True

    # If global hard constraint fails, mark everything as N
    if global_hard_fail:
        for node in diagram.get("nodes", []):
            if node.get("id"):
                inu_labels[node.get("id")] = "N"
        for edge in diagram.get("edges", []):
            if edge.get("id"):
                inu_labels[edge.get("id")] = "N"
        return inu_labels

    # If global soft constraint fails, mark everything as U
    if global_soft_fail:
        for node in diagram.get("nodes", []):
            if node.get("id"):
                inu_labels[node.get("id")] = "U"
        for edge in diagram.get("edges", []):
            if edge.get("id"):
                inu_labels[edge.get("id")] = "U"
        return inu_labels

    # Phase 2: No global violations, everything is admissible
    # (Skip per-node validation to avoid O(N*C*P) complexity)
    # This is a simplification: assumes constraints are global-only
    for node in diagram.get("nodes", []):
        if node.get("id"):
            inu_labels[node.get("id")] = "I"

    for edge in diagram.get("edges", []):
        if edge.get("id"):
            inu_labels[edge.get("id")] = "I"

    return inu_labels


def check_admissible(state: dict) -> Tuple[bool, str]:
    """Check if state is admissible based on I/N/U labels."""
    inu_labels = state.get("inu_labels", {})
    if not isinstance(inu_labels, dict):
        return False, "inu_labels missing or invalid"

    # Check for any N (forbidden) elements
    for key, value in inu_labels.items():
        if value == "N":
            return False, f"Element {key} is N (forbidden)"

    # Count unresolved elements
    unresolved_count = sum(1 for v in inu_labels.values() if v == "U")
    if unresolved_count > 0:
        return True, f"Admissible with {unresolved_count} unresolved (U) elements"

    return True, "All elements admissible (I)"


def authorize_rewrite(
    constraints: List[dict],
    state: dict,
    diagram: dict,
    csi: dict,
    rewrite_rule: dict,
) -> Tuple[bool, List[str], List[str]]:
    """
    Authorize a rewrite rule based on constraints and preconditions.

    Ensures inu_labels are initialized before checking admissibility.
    """
    # Initialize inu_labels if not present
    if "inu_labels" not in state:
        state["inu_labels"] = assign_inu_labels(diagram, constraints, state, csi)

    errors, warnings = evaluate_constraints(constraints, state, diagram, csi)
    if errors:
        return False, errors, warnings

    preconditions = rewrite_rule.get("preconditions", [])
    for pre in preconditions:
        if pre == "admissible":
            ok, detail = check_admissible(state)
            if not ok:
                return False, [f"{rewrite_rule.get('id')} precondition failed: {detail}"], warnings
        elif pre == "no_hard_conflict":
            # Already enforced by constraint evaluation; keep for clarity.
            continue
        else:
            warnings.append(
                f"Rewrite {rewrite_rule.get('id')} has unknown precondition {pre}"
            )
    return True, errors, warnings
