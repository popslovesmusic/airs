"""
Performance tests for SID framework (M17).
Tests with large diagrams and complex patterns.
"""
from __future__ import annotations

import time
from sid_parser import parse_expression
from sid_ast_to_diagram import expr_to_diagram
from sid_crf import assign_inu_labels, no_cycles
from sid_rewrite import find_all_matches, RulePattern, EdgePattern, SidePattern


def test_large_diagram_inu_labeling():
    """Test I/N/U labeling performance with 100+ nodes."""
    # Create diagram with 100 nodes
    nodes = [{"id": f"n{i}", "op": "P", "dof_refs": ["Freedom"]} for i in range(100)]
    edges = []

    diagram = {"nodes": nodes, "edges": edges}
    state = {}
    csi = {"allowed_dofs": ["Freedom"], "allowed_pairs": []}
    constraints = []

    start = time.time()
    labels = assign_inu_labels(diagram, constraints, state, csi)
    elapsed = time.time() - start

    # Should complete in under 100ms (optimized version)
    assert elapsed < 0.1, f"Took {elapsed:.3f}s, expected < 0.1s"
    assert len(labels) == 100
    print(f"PASS: test_large_diagram_inu_labeling ({elapsed*1000:.1f}ms for 100 nodes)")


def test_cycle_detection_performance():
    """Test cycle detection with large graph."""
    # Create chain of 100 nodes (no cycle)
    nodes = [{"id": f"n{i}", "op": "P"} for i in range(100)]
    edges = [{"id": f"e{i}", "from": f"n{i}", "to": f"n{i+1}"} for i in range(99)]

    diagram = {"nodes": nodes, "edges": edges}

    start = time.time()
    has_cycle, msg = no_cycles({}, diagram, {})
    elapsed = time.time() - start

    # Should complete quickly
    assert elapsed < 0.1, f"Took {elapsed:.3f}s"
    assert has_cycle is True  # No cycle detected
    assert "No cycles" in msg
    print(f"PASS: test_cycle_detection_performance ({elapsed*1000:.1f}ms for 100 nodes)")


def test_deep_nesting_parse():
    """Test parser with deeply nested expressions."""
    # O(O(O(O(P(Freedom)))))
    expr_text = "P(Freedom)"
    for _ in range(10):
        expr_text = f"O({expr_text})"

    start = time.time()
    expr = parse_expression(expr_text)
    elapsed = time.time() - start

    # Should handle deep nesting
    assert elapsed < 0.1
    assert expr is not None
    print(f"PASS: test_deep_nesting_parse ({elapsed*1000:.1f}ms for depth 10)")


def test_complex_pattern_matching():
    """Test pattern matching doesn't blow up with many edges."""
    # Diagram with 50 nodes and 100 edges
    nodes = [{"id": f"n{i}", "op": "P"} for i in range(50)]
    edges = [{"id": f"e{i}", "from": f"n{i%50}", "to": f"n{(i+1)%50}", "label": "arg"} for i in range(100)]

    diagram = {"nodes": nodes, "edges": edges}

    # Simple pattern that won't match everything
    pattern = RulePattern(edges=[
        EdgePattern(
            left=SidePattern(op="P", var="x"),
            edge_label="arg",
            right=SidePattern(op="P", var="y")
        )
    ])

    start = time.time()
    matches = find_all_matches(diagram, pattern, max_matches=100)
    elapsed = time.time() - start

    # Should complete in reasonable time
    assert elapsed < 1.0, f"Took {elapsed:.3f}s"
    assert len(matches) <= 100  # Respects max_matches limit
    print(f"PASS: test_complex_pattern_matching ({elapsed*1000:.1f}ms, {len(matches)} matches)")


def test_diagram_construction_performance():
    """Test diagram construction from complex expression."""
    # C(P(A), O(P(B)))
    expr = parse_expression("C(P(A), O(P(B)))")

    start = time.time()
    diagram = expr_to_diagram(expr, "d_perf")
    elapsed = time.time() - start

    # Should be fast
    assert elapsed < 0.01
    assert len(diagram["nodes"]) >= 3  # At least P, O, C nodes
    print(f"PASS: test_diagram_construction_performance ({elapsed*1000:.1f}ms)")


def run_all_tests():
    """Run all performance tests."""
    print("Running performance tests (M17)...\n")

    test_large_diagram_inu_labeling()
    test_cycle_detection_performance()
    test_deep_nesting_parse()
    test_complex_pattern_matching()
    test_diagram_construction_performance()

    print("\nPASS: All performance tests passed!")


if __name__ == "__main__":
    run_all_tests()
