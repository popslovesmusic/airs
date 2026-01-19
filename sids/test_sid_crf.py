"""
Tests for SID CRF resolution procedures and I/N/U typing.
"""
from __future__ import annotations

from sid_crf import (
    attenuate,
    bifurcate,
    defer,
    escalate,
    halt,
    partition,
    resolve_conflict,
    assign_inu_labels,
    check_admissible,
    PREDICATES,
)


def test_attenuate():
    """Test soft constraint attenuation."""
    state = {}
    diagram = {}
    conflict_details = {"constraint_id": "c1"}

    result = attenuate(conflict_details, state, diagram)

    assert result.action == "attenuate"
    assert result.success is True
    # Check new_state instead of mutated state
    assert result.new_state is not None
    assert "c1" in result.new_state.get("attenuated_constraints", [])
    # Ensure original state unchanged
    assert "attenuated_constraints" not in state
    print("PASS: test_attenuate")


def test_defer():
    """Test conflict deferral."""
    state = {}
    diagram = {}
    conflict_details = {"type": "temporal", "id": "conflict1"}

    result = defer(conflict_details, state, diagram)

    assert result.action == "defer"
    assert result.success is True
    assert result.new_state is not None
    assert len(result.new_state.get("deferred_conflicts", [])) == 1
    assert "deferred_conflicts" not in state
    print("PASS: test_defer passed")


def test_partition():
    """Test element partitioning."""
    state = {}
    diagram = {}
    conflict_details = {"conflicting_elements": ["e1", "e2", "e3"]}

    result = partition(conflict_details, state, diagram)

    assert result.action == "partition"
    assert result.success is True
    assert result.new_state is not None
    assert len(result.new_state.get("partitioned_elements", [])) == 3
    assert "partitioned_elements" not in state
    print("PASS: test_partition passed")


def test_escalate():
    """Test conflict escalation."""
    state = {}
    diagram = {}
    conflict_details = {"scope": "local", "reason": "insufficient context"}

    result = escalate(conflict_details, state, diagram)

    assert result.action == "escalate"
    assert result.success is True
    assert result.new_state is not None
    assert len(result.new_state.get("escalated_conflicts", [])) == 1
    assert "escalated_conflicts" not in state
    print("PASS: test_escalate passed")


def test_bifurcate():
    """Test state bifurcation."""
    state = {}
    diagram = {}
    conflict_details = {"choices": ["path_a", "path_b"]}

    result = bifurcate(conflict_details, state, diagram)

    assert result.action == "bifurcate"
    assert result.success is True
    assert result.new_state is not None
    assert result.new_state.get("bifurcated") is True
    assert len(result.new_state.get("bifurcation_choices", [])) == 2
    assert "bifurcated" not in state
    print("PASS: test_bifurcate passed")


def test_halt():
    """Test execution halt."""
    state = {}
    diagram = {}
    conflict_details = {"reason": "Hard constraint violation"}

    result = halt(conflict_details, state, diagram)

    assert result.action == "halt"
    assert result.success is False
    assert result.new_state is not None
    assert result.new_state.get("halted") is True
    assert "halted" not in state
    print("PASS: test_halt passed")


def test_resolve_conflict_dispatcher():
    """Test conflict resolution dispatcher."""
    state = {}
    diagram = {}

    # Test soft violation -> attenuate
    result = resolve_conflict("soft_violation", {"constraint_id": "c1"}, state, diagram)
    assert result.action == "attenuate"

    # Test temporal mismatch -> defer
    state2 = {}
    result = resolve_conflict("temporal_mismatch", {"type": "temp"}, state2, diagram)
    assert result.action == "defer"

    # Test DOF interference -> partition
    state3 = {}
    result = resolve_conflict("dof_interference", {"conflicting_elements": []}, state3, diagram)
    assert result.action == "partition"

    # Test scope overflow -> escalate
    state4 = {}
    result = resolve_conflict("scope_overflow", {"scope": "local"}, state4, diagram)
    assert result.action == "escalate"

    # Test ambiguous choice -> bifurcate
    state5 = {}
    result = resolve_conflict("ambiguous_choice", {"choices": []}, state5, diagram)
    assert result.action == "bifurcate"

    # Test hard violation -> halt
    state6 = {}
    result = resolve_conflict("hard_violation", {"reason": "test"}, state6, diagram)
    assert result.action == "halt"

    # Test unknown -> halt
    state7 = {}
    result = resolve_conflict("unknown_type", {}, state7, diagram)
    assert result.action == "halt"

    print("PASS: test_resolve_conflict_dispatcher passed")


def test_assign_inu_labels():
    """Test I/N/U label assignment."""
    diagram = {
        "nodes": [
            {"id": "n1", "op": "P", "dof_refs": ["Freedom"]},
            {"id": "n2", "op": "O", "irreversible": True},
        ],
        "edges": [
            {"id": "e1", "from": "n1", "to": "n2", "label": "arg"}
        ]
    }

    state = {}
    csi = {"allowed_dofs": ["Freedom"], "allowed_pairs": []}
    constraints = []

    labels = assign_inu_labels(diagram, constraints, state, csi)

    # With no constraints, all should be "I"
    assert labels.get("n1") == "I"
    assert labels.get("n2") == "I"
    assert labels.get("e1") == "I"

    print("PASS: test_assign_inu_labels passed")


def test_check_admissible():
    """Test admissibility checking."""
    # All I (admissible)
    state1 = {"inu_labels": {"n1": "I", "n2": "I", "e1": "I"}}
    ok, msg = check_admissible(state1)
    assert ok is True
    assert "All elements admissible" in msg

    # Contains N (forbidden)
    state2 = {"inu_labels": {"n1": "I", "n2": "N", "e1": "I"}}
    ok, msg = check_admissible(state2)
    assert ok is False
    assert "forbidden" in msg

    # Contains U (unresolved)
    state3 = {"inu_labels": {"n1": "I", "n2": "U", "e1": "I"}}
    ok, msg = check_admissible(state3)
    assert ok is True
    assert "unresolved" in msg

    print("PASS: test_check_admissible passed")


def test_predicates_registered():
    """Test that predicates are properly registered."""
    assert "no_cross_csi_interaction" in PREDICATES
    assert "collapse_irreversible" in PREDICATES
    assert "no_cycles" in PREDICATES
    assert "valid_compartment_transitions" in PREDICATES
    print("PASS: test_predicates_registered passed")


def test_no_cycles_predicate():
    """Test cycle detection predicate."""
    predicate = PREDICATES["no_cycles"]

    # No cycle
    diagram1 = {
        "nodes": [
            {"id": "n1", "op": "P"},
            {"id": "n2", "op": "O"},
        ],
        "edges": [
            {"id": "e1", "from": "n1", "to": "n2"}
        ]
    }
    ok, msg = predicate({}, diagram1, {})
    assert ok is True

    # Has cycle
    diagram2 = {
        "nodes": [
            {"id": "n1", "op": "P"},
            {"id": "n2", "op": "O"},
        ],
        "edges": [
            {"id": "e1", "from": "n1", "to": "n2"},
            {"id": "e2", "from": "n2", "to": "n1"}
        ]
    }
    ok, msg = predicate({}, diagram2, {})
    assert ok is False
    assert "Cycle detected" in msg

    print("PASS: test_no_cycles_predicate passed")


def run_all_tests():
    """Run all CRF tests."""
    print("Running CRF tests...\n")

    test_attenuate()
    test_defer()
    test_partition()
    test_escalate()
    test_bifurcate()
    test_halt()
    test_resolve_conflict_dispatcher()
    test_assign_inu_labels()
    test_check_admissible()
    test_predicates_registered()
    test_no_cycles_predicate()

    print("\nPASS: All CRF tests passed!")


if __name__ == "__main__":
    run_all_tests()
