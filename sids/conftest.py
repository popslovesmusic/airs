"""
Shared test fixtures for SID framework tests (Minor Issue #16).

Reduces code duplication across test files.
"""
from __future__ import annotations

import pytest


@pytest.fixture
def minimal_dofs():
    """Minimal DOF set for testing."""
    return [
        {"id": "Freedom", "description": "Core freedom DOF"},
        {"id": "Energy", "description": "Energy DOF"},
        {"id": "Position", "description": "Position DOF"},
        {"id": "Momentum", "description": "Momentum DOF"},
    ]


@pytest.fixture
def minimal_compartments():
    """Minimal compartment set for testing."""
    return [
        {"id": "c1", "name": "Compartment 1"},
        {"id": "c2", "name": "Compartment 2"},
    ]


@pytest.fixture
def minimal_csi(minimal_dofs):
    """Minimal CSI for testing."""
    dof_ids = [d["id"] for d in minimal_dofs]
    return {
        "id": "csi1",
        "allowed_dofs": dof_ids,
        "allowed_pairs": [
            [dof_ids[0], dof_ids[1]],
            [dof_ids[1], dof_ids[0]],
            [dof_ids[2], dof_ids[3]],
        ],
    }


@pytest.fixture
def minimal_diagram():
    """Minimal valid diagram for testing."""
    return {
        "id": "diag1",
        "compartment_id": "c1",
        "nodes": [
            {"id": "n1", "op": "P", "dof_refs": ["Freedom"]},
            {"id": "n2", "op": "P", "dof_refs": ["Energy"]},
        ],
        "edges": [
            {"id": "e1", "from": "n1", "to": "n2", "label": "arg"}
        ],
    }


@pytest.fixture
def minimal_state(minimal_diagram, minimal_csi):
    """Minimal valid state for testing."""
    return {
        "id": "s1",
        "diagram_id": minimal_diagram["id"],
        "csi_id": minimal_csi["id"],
        "compartment_id": "c1",
        "inu_labels": {
            "n1": "I",
            "n2": "I",
            "e1": "I",
        },
    }


@pytest.fixture
def minimal_constraints():
    """Minimal constraint set for testing."""
    return [
        {
            "id": "c1",
            "type": "hard",
            "predicate": "no_cycles",
            "description": "Graph must be acyclic",
        },
        {
            "id": "c2",
            "type": "hard",
            "predicate": "collapse_irreversible",
            "description": "Collapse operators must be irreversible",
        },
    ]


@pytest.fixture
def minimal_rewrite_rules():
    """Minimal rewrite rule set for testing."""
    return [
        {
            "id": "r1",
            "pattern_expr": "P(Freedom)",
            "replacement_expr": "S+(Freedom)",
            "preconditions": ["admissible"],
        }
    ]


@pytest.fixture
def minimal_package(
    minimal_dofs,
    minimal_compartments,
    minimal_csi,
    minimal_diagram,
    minimal_state,
    minimal_constraints,
    minimal_rewrite_rules,
):
    """Minimal valid SID package for testing."""
    return {
        "dofs": minimal_dofs,
        "compartments": minimal_compartments,
        "csis": [minimal_csi],
        "diagrams": [minimal_diagram],
        "states": [minimal_state],
        "constraints": minimal_constraints,
        "rewrite_rules": minimal_rewrite_rules,
    }


@pytest.fixture
def large_diagram():
    """Large diagram for performance testing (100+ nodes)."""
    nodes = []
    edges = []
    for i in range(100):
        nodes.append({
            "id": f"n{i}",
            "op": "P",
            "dof_refs": ["Freedom"],
        })
        if i > 0:
            edges.append({
                "id": f"e{i}",
                "from": f"n{i-1}",
                "to": f"n{i}",
                "label": "arg",
            })
    return {
        "id": "large_diag",
        "compartment_id": "c1",
        "nodes": nodes,
        "edges": edges,
    }


@pytest.fixture
def deeply_nested_expression():
    """Deeply nested expression for parsing tests."""
    expr = "P(Freedom)"
    for _ in range(10):
        expr = f"O({expr})"
    return expr


@pytest.fixture
def complex_csi_with_many_pairs(minimal_dofs):
    """CSI with many allowed pairs for validation testing."""
    dof_ids = [d["id"] for d in minimal_dofs]
    pairs = []
    for a in dof_ids:
        for b in dof_ids:
            pairs.append([a, b])
    return {
        "id": "complex_csi",
        "allowed_dofs": dof_ids,
        "allowed_pairs": pairs,
    }
