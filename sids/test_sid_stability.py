"""
Tests for SID structural stability checks.
"""
from __future__ import annotations

from sid_stability import (
    check_loop_convergence,
    check_no_admissible_rewrites,
    check_only_identity_rewrites,
    compute_stability_metrics,
    is_structurally_stable,
)


def test_check_only_identity_rewrites():
    """Test identity rewrite checking."""
    # Only identity rewrites
    rules1 = [
        {"id": "r1", "pattern": "P(x)", "replacement": "P(x)"},
        {"id": "r2", "pattern_expr": "O(y)", "replacement_expr": "O(y)"}
    ]
    ok, msg = check_only_identity_rewrites(rules1, {})
    assert ok is True
    print("PASS: test_check_only_identity_rewrites (identity) passed")

    # Has non-identity rewrite
    rules2 = [
        {"id": "r1", "pattern": "P(x)", "replacement": "P(x)"},
        {"id": "r2", "pattern": "P(x)", "replacement": "O(x)"}
    ]
    ok, msg = check_only_identity_rewrites(rules2, {})
    assert ok is False
    assert "r2" in msg
    print("PASS: test_check_only_identity_rewrites (non-identity) passed")


def test_check_loop_convergence():
    """Test loop convergence checking."""
    # Insufficient history
    state1 = {"loop_history": []}
    ok, msg = check_loop_convergence(state1)
    assert ok is False
    print("PASS: test_check_loop_convergence (insufficient history) passed")

    # Fully converged
    state2 = {
        "loop_history": [
            {"inu_labels": {"n1": "I", "n2": "I"}},
            {"inu_labels": {"n1": "I", "n2": "I"}}
        ]
    }
    ok, msg = check_loop_convergence(state2)
    assert ok is True
    assert "fully converged" in msg
    print("PASS: test_check_loop_convergence (converged) passed")

    # Not converged
    state3 = {
        "loop_history": [
            {"inu_labels": {"n1": "I", "n2": "I", "n3": "I"}},
            {"inu_labels": {"n1": "I", "n2": "N", "n3": "U"}}
        ]
    }
    ok, msg = check_loop_convergence(state3, tolerance=0.1)
    assert ok is False
    print("PASS: test_check_loop_convergence (not converged) passed")

    # Converged within tolerance
    state4 = {
        "loop_history": [
            {"inu_labels": {"n1": "I", "n2": "I", "n3": "I", "n4": "I", "n5": "I"}},
            {"inu_labels": {"n1": "I", "n2": "I", "n3": "I", "n4": "I", "n5": "U"}}
        ]
    }
    ok, msg = check_loop_convergence(state4, tolerance=0.3)
    assert ok is True  # 1/5 = 0.2 < 0.3
    print("PASS: test_check_loop_convergence (within tolerance) passed")


def test_compute_stability_metrics():
    """Test stability metrics computation."""
    pkg = {
        "states": [
            {
                "id": "s1",
                "inu_labels": {
                    "n1": "I",
                    "n2": "I",
                    "n3": "U",
                    "e1": "I"
                },
                "loop_history": [
                    {"inu_labels": {"n1": "I", "n2": "I", "n3": "U", "e1": "I"}},
                    {"inu_labels": {"n1": "I", "n2": "I", "n3": "I", "e1": "I"}}
                ]
            }
        ],
        "diagrams": [
            {
                "id": "d1",
                "nodes": [
                    {"id": "n1", "op": "P"},
                    {"id": "n2", "op": "O"},
                    {"id": "n3", "op": "C"}
                ],
                "edges": [
                    {"id": "e1", "from": "n1", "to": "n2"}
                ]
            }
        ]
    }

    metrics = compute_stability_metrics(pkg, "s1", "d1")

    assert "admissible_volume" in metrics
    assert "collapse_ratio" in metrics
    assert "gradient_coherence" in metrics
    assert "transport_fidelity" in metrics
    assert "loop_gain" in metrics

    # Check specific values
    assert metrics["collapse_count"] == 1  # One O node
    assert metrics["coupling_count"] == 1  # One C node
    assert metrics["collapse_ratio"] == 1/3  # 1 out of 3 nodes
    assert metrics["gradient_coherence"] == 1/3  # 1 coupling out of 3 nodes

    print("PASS: test_compute_stability_metrics passed")


def test_is_structurally_stable():
    """Test overall stability checking."""
    # Create a minimal stable package
    pkg = {
        "states": [
            {
                "id": "s1",
                "diagram_id": "d1",
                "csi_id": "csi1",
                "inu_labels": {"n1": "I"},
                "loop_history": [
                    {"inu_labels": {"n1": "I"}},
                    {"inu_labels": {"n1": "I"}}
                ]
            }
        ],
        "diagrams": [
            {
                "id": "d1",
                "nodes": [{"id": "n1", "op": "P"}],
                "edges": []
            }
        ],
        "csis": [
            {
                "id": "csi1",
                "allowed_dofs": ["Freedom"],
                "allowed_pairs": []
            }
        ],
        "constraints": [],
        "rewrite_rules": []
    }

    is_stable, satisfied, message = is_structurally_stable(pkg, "s1", "d1")

    # Should be stable due to multiple conditions
    assert is_stable is True
    assert len(satisfied) > 0
    assert "STABLE" in message

    print("PASS: test_is_structurally_stable passed")


def test_metrics_collapse_ratio():
    """Test collapse ratio calculation."""
    pkg = {
        "states": [{"id": "s1", "inu_labels": {}}],
        "diagrams": [
            {
                "id": "d1",
                "nodes": [
                    {"id": "n1", "op": "P"},
                    {"id": "n2", "op": "O"},
                    {"id": "n3", "op": "O"},
                    {"id": "n4", "op": "C"}
                ],
                "edges": []
            }
        ]
    }

    metrics = compute_stability_metrics(pkg, "s1", "d1")

    assert metrics["collapse_count"] == 2
    assert metrics["collapse_ratio"] == 0.5  # 2 out of 4
    print("PASS: test_metrics_collapse_ratio passed")


def test_metrics_transport_fidelity():
    """Test transport fidelity calculation."""
    pkg = {
        "states": [{"id": "s1", "inu_labels": {}}],
        "diagrams": [
            {
                "id": "d1",
                "nodes": [
                    {"id": "n1", "op": "T", "meta": {"target_compartment": "c2"}},
                    {"id": "n2", "op": "T", "meta": {"target_compartment": "c3"}},
                    {"id": "n3", "op": "T", "meta": {}},  # Invalid - no target
                    {"id": "n4", "op": "P"}
                ],
                "edges": []
            }
        ]
    }

    metrics = compute_stability_metrics(pkg, "s1", "d1")

    assert metrics["transport_count"] == 3
    assert metrics["transport_fidelity"] == 2/3  # 2 valid out of 3
    print("PASS: test_metrics_transport_fidelity passed")


def run_all_tests():
    """Run all stability tests."""
    print("Running stability tests...\n")

    test_check_only_identity_rewrites()
    test_check_loop_convergence()
    test_compute_stability_metrics()
    test_is_structurally_stable()
    test_metrics_collapse_ratio()
    test_metrics_transport_fidelity()

    print("\nPASS: All stability tests passed!")


if __name__ == "__main__":
    run_all_tests()
