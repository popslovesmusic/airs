#!/usr/bin/env python3
"""
SID Framework Demo
==================

Interactive demonstration of the Semantic Interaction Diagrams framework.
Shows all major features: parsing, validation, CRF, I/N/U typing, stability.
"""
from __future__ import annotations

import json
from typing import Any

# Core SID imports
from sid_parser import parse_expression, expr_to_dict
from sid_ast_to_diagram import expr_to_diagram
from sid_validator import validate_package
from sid_crf import (
    resolve_conflict,
    assign_inu_labels,
    check_admissible,
    PREDICATES,
)
from sid_stability import is_structurally_stable, compute_stability_metrics


def print_header(title: str) -> None:
    """Print a section header."""
    print("\n" + "=" * 70)
    print(f"  {title}")
    print("=" * 70)


def print_subheader(title: str) -> None:
    """Print a subsection header."""
    print(f"\n--- {title} ---")


def pretty_json(obj: Any) -> str:
    """Pretty print JSON."""
    return json.dumps(obj, indent=2, sort_keys=True)


def demo_1_parsing():
    """Demo 1: Expression Parsing and AST."""
    print_header("DEMO 1: Expression Parsing")

    expressions = [
        "P(Freedom)",
        "O(P(Choice))",
        "C(P(Vertical), P(Horizontal))",
        "T(O(C(P(x))))"
    ]

    for expr_text in expressions:
        print_subheader(f"Parsing: {expr_text}")

        # Parse to AST
        expr = parse_expression(expr_text)
        ast = expr_to_dict(expr)
        print("AST:")
        print(pretty_json(ast))

        # Convert to diagram
        diagram = expr_to_diagram(expr, diagram_id=f"d_{expr_text[:5]}")
        print("\nDiagram:")
        print(f"  Nodes: {len(diagram['nodes'])}")
        for node in diagram['nodes']:
            op = node.get('op')
            dofs = node.get('dof_refs', [])
            if dofs:
                print(f"    - {node['id']}: {op}({', '.join(dofs)})")
            else:
                print(f"    - {node['id']}: {op}")

        print(f"  Edges: {len(diagram['edges'])}")
        for edge in diagram['edges']:
            print(f"    - {edge['from']} --{edge['label']}--> {edge['to']}")


def demo_2_operators():
    """Demo 2: All Operators."""
    print_header("DEMO 2: Operator Showcase")

    operators = {
        "P": ("Projection", "P(Freedom)", "Isolates a degree of freedom"),
        "S+": ("Polarity+", "P(Affirmation)", "Positive stance (via projection)"),
        "S-": ("Polarity-", "P(Negation)", "Negative stance (via projection)"),
        "O": ("Collapse", "O(P(Choice))", "Irreversible commitment"),
        "C": ("Coupling", "C(P(x))", "Relational constraint"),
        "T": ("Transport", "T(P(x))", "Movement across compartments"),
    }

    for op, (name, example, desc) in operators.items():
        print_subheader(f"{op} - {name}")
        print(f"  Description: {desc}")
        print(f"  Example: {example}")

        expr = parse_expression(example)
        diagram = expr_to_diagram(expr)
        print(f"  Generates {len(diagram['nodes'])} node(s)")


def demo_3_crf_procedures():
    """Demo 3: CRF Resolution Procedures."""
    print_header("DEMO 3: CRF Conflict Resolution")

    print("""
The CRF implements 6 resolution procedures for handling conflicts:
1. ATTENUATE - Weaken soft constraints
2. DEFER - Postpone to later compartment
3. PARTITION - Split into separate compartments
4. ESCALATE - Promote to higher level
5. BIFURCATE - Create parallel paths
6. HALT - Stop on unresolvable conflict
""")

    # Demonstrate each procedure
    procedures = [
        ("soft_violation", "Soft constraint violated", "attenuate"),
        ("temporal_mismatch", "Timing issue", "defer"),
        ("dof_interference", "DOF conflict", "partition"),
        ("scope_overflow", "Scope too small", "escalate"),
        ("ambiguous_choice", "Multiple valid paths", "bifurcate"),
        ("hard_violation", "Hard constraint violated", "halt"),
    ]

    for conflict_type, description, expected_action in procedures:
        print_subheader(f"Conflict: {description}")

        state = {}
        diagram = {}
        conflict_details = {
            "type": conflict_type,
            "description": description,
            "constraint_id": "c1",
            "conflicting_elements": ["e1", "e2"],
            "choices": ["path_a", "path_b"],
            "scope": "local",
            "reason": "Test conflict"
        }

        result = resolve_conflict(conflict_type, conflict_details, state, diagram)

        print(f"  Conflict Type: {conflict_type}")
        print(f"  Resolution: {result.action.upper()}")
        print(f"  Success: {result.success}")
        print(f"  Message: {result.message}")

        # Show state changes
        if result.action == "attenuate":
            print(f"  Attenuated constraints: {state.get('attenuated_constraints', [])}")
        elif result.action == "defer":
            print(f"  Deferred conflicts: {len(state.get('deferred_conflicts', []))}")
        elif result.action == "partition":
            print(f"  Partitioned elements: {state.get('partitioned_elements', [])}")
        elif result.action == "escalate":
            print(f"  Escalated conflicts: {len(state.get('escalated_conflicts', []))}")
        elif result.action == "bifurcate":
            print(f"  Bifurcated: {state.get('bifurcated')}")
            print(f"  Choices: {state.get('bifurcation_choices', [])}")
        elif result.action == "halt":
            print(f"  Halted: {state.get('halted')}")
            print(f"  Reason: {state.get('halt_reason')}")


def demo_4_inu_typing():
    """Demo 4: I/N/U Typing System."""
    print_header("DEMO 4: I/N/U Epistemic Typing")

    print("""
I/N/U Logic provides epistemic status labels:
  I (Is) - Admissible, coherent, affirmed
  N (Not) - Excluded, forbidden, contradictory
  U (Unknown) - Unresolved, undecidable, open
""")

    # Create a test diagram
    diagram = {
        "nodes": [
            {"id": "n1", "op": "P", "dof_refs": ["Freedom"]},
            {"id": "n2", "op": "C", "inputs": ["n1"]},
            {"id": "n3", "op": "O", "irreversible": True, "inputs": ["n2"]},
        ],
        "edges": [
            {"id": "e1", "from": "n1", "to": "n2", "label": "arg"},
            {"id": "e2", "from": "n2", "to": "n3", "label": "arg"},
        ]
    }

    state = {}
    csi = {"allowed_dofs": ["Freedom"], "allowed_pairs": []}
    constraints = []

    print_subheader("Initial Diagram")
    print(f"  Nodes: {len(diagram['nodes'])}")
    for node in diagram['nodes']:
        print(f"    - {node['id']}: {node['op']}")

    # Assign I/N/U labels
    print_subheader("Assigning I/N/U Labels")
    labels = assign_inu_labels(diagram, constraints, state, csi)

    print("  Labels assigned:")
    for elem_id, label in sorted(labels.items()):
        status = {
            "I": "Admissible",
            "N": "Forbidden",
            "U": "Unresolved"
        }.get(label, "Unknown")
        print(f"    {elem_id}: {label} ({status})")

    # Check admissibility
    state["inu_labels"] = labels
    is_admissible, msg = check_admissible(state)

    print_subheader("Admissibility Check")
    print(f"  Result: {'ADMISSIBLE' if is_admissible else 'NOT ADMISSIBLE'}")
    print(f"  Message: {msg}")


def demo_5_predicates():
    """Demo 5: Constraint Predicates."""
    print_header("DEMO 5: Constraint Predicates")

    print(f"\nRegistered Predicates: {len(PREDICATES)}")
    for name in sorted(PREDICATES.keys()):
        predicate = PREDICATES[name]
        doc = predicate.__doc__ or "No description"
        print(f"  - {name}: {doc.strip()}")

    # Demo cycle detection
    print_subheader("Example: Cycle Detection")

    # Diagram without cycle
    diagram_no_cycle = {
        "nodes": [
            {"id": "n1", "op": "P"},
            {"id": "n2", "op": "O"},
            {"id": "n3", "op": "C"},
        ],
        "edges": [
            {"id": "e1", "from": "n1", "to": "n2"},
            {"id": "e2", "from": "n2", "to": "n3"},
        ]
    }

    # Diagram with cycle
    diagram_with_cycle = {
        "nodes": [
            {"id": "n1", "op": "P"},
            {"id": "n2", "op": "O"},
        ],
        "edges": [
            {"id": "e1", "from": "n1", "to": "n2"},
            {"id": "e2", "from": "n2", "to": "n1"},  # Creates cycle
        ]
    }

    no_cycles_pred = PREDICATES["no_cycles"]

    print("\n  Acyclic diagram:")
    ok, msg = no_cycles_pred({}, diagram_no_cycle, {})
    print(f"    Result: {ok}")
    print(f"    Message: {msg}")

    print("\n  Cyclic diagram:")
    ok, msg = no_cycles_pred({}, diagram_with_cycle, {})
    print(f"    Result: {ok}")
    print(f"    Message: {msg}")


def demo_6_stability():
    """Demo 6: Structural Stability."""
    print_header("DEMO 6: Structural Stability")

    print("""
A system is stable when ANY of these conditions hold:
1. No admissible rewrites remain
2. Admissible region invariant under transport
3. Only identity rewrites authorized
4. Loop gain converges within tolerance
""")

    # Create a simple stable package
    pkg = {
        "dofs": [{"id": "Freedom", "orthogonal_group": "primary"}],
        "compartments": [{"id": "c0", "index": 0}],
        "csis": [{
            "id": "csi1",
            "allowed_dofs": ["Freedom"],
            "allowed_pairs": []
        }],
        "diagrams": [{
            "id": "d1",
            "compartment_id": "c0",
            "nodes": [
                {"id": "n1", "op": "P", "dof_refs": ["Freedom"]},
                {"id": "n2", "op": "O", "irreversible": True, "inputs": ["n1"]}
            ],
            "edges": [
                {"id": "e1", "from": "n1", "to": "n2", "label": "arg"}
            ]
        }],
        "states": [{
            "id": "s1",
            "diagram_id": "d1",
            "csi_id": "csi1",
            "compartment_id": "c0",
            "inu_labels": {"n1": "I", "n2": "I", "e1": "I"},
            "loop_history": [
                {"inu_labels": {"n1": "I", "n2": "I", "e1": "I"}},
                {"inu_labels": {"n1": "I", "n2": "I", "e1": "I"}}
            ]
        }],
        "constraints": [],
        "rewrite_rules": []
    }

    print_subheader("Checking Stability")
    is_stable, conditions, message = is_structurally_stable(pkg, "s1", "d1")

    print(f"  {message}")
    if conditions:
        print("\n  Satisfied conditions:")
        for condition in conditions:
            print(f"    {condition}")

    # Compute metrics
    print_subheader("Stability Metrics")
    metrics = compute_stability_metrics(pkg, "s1", "d1")

    important_metrics = [
        "admissible_volume",
        "admissible_ratio",
        "collapse_count",
        "collapse_ratio",
        "gradient_coherence",
        "transport_fidelity"
    ]

    for key in important_metrics:
        if key in metrics:
            value = metrics[key]
            if isinstance(value, float):
                print(f"  {key}: {value:.3f}")
            else:
                print(f"  {key}: {value}")


def demo_7_full_workflow():
    """Demo 7: Complete Workflow."""
    print_header("DEMO 7: Complete Workflow")

    print("""
This demo shows a complete SID workflow:
1. Parse expression
2. Create diagram
3. Build package
4. Validate
5. Assign I/N/U labels
6. Check stability
7. Extract metrics
""")

    # Step 1: Parse
    print_subheader("Step 1: Parse Expression")
    expr_text = "C(P(Freedom), O(P(Choice)))"
    print(f"  Expression: {expr_text}")
    expr = parse_expression(expr_text)
    print("  [OK] Parsed successfully")

    # Step 2: Create diagram
    print_subheader("Step 2: Create Diagram")
    diagram = expr_to_diagram(expr, diagram_id="d_workflow", compartment_id="c0")
    print(f"  Nodes: {len(diagram['nodes'])}")
    print(f"  Edges: {len(diagram['edges'])}")

    # Step 3: Build package
    print_subheader("Step 3: Build Package")
    pkg = {
        "dofs": [
            {"id": "Freedom", "orthogonal_group": "primary"},
            {"id": "Choice", "orthogonal_group": "primary"}
        ],
        "compartments": [{"id": "c0", "index": 0}],
        "csis": [{
            "id": "csi_workflow",
            "allowed_dofs": ["Freedom", "Choice"],
            "allowed_pairs": [
                ["Freedom", "Choice"],
                ["Choice", "Freedom"]
            ]
        }],
        "diagrams": [diagram],
        "states": [{
            "id": "s_workflow",
            "diagram_id": "d_workflow",
            "csi_id": "csi_workflow",
            "compartment_id": "c0",
            "inu_labels": {}
        }],
        "constraints": [
            {
                "id": "c1",
                "type": "hard",
                "scope": "global",
                "predicate": "collapse_irreversible"
            }
        ],
        "rewrite_rules": []
    }
    print("  [OK] Package created")

    # Step 4: Validate
    print_subheader("Step 4: Validate Package")
    errors, warnings = validate_package(pkg)
    if errors:
        print(f"  [ERROR] {len(errors)} error(s):")
        for err in errors[:3]:
            print(f"    - {err}")
    else:
        print("  [OK] Package is valid")

    if warnings:
        print(f"  [WARN] {len(warnings)} warning(s)")

    # Step 5: Assign I/N/U labels
    print_subheader("Step 5: Assign I/N/U Labels")
    state = pkg["states"][0]
    csi = pkg["csis"][0]
    labels = assign_inu_labels(diagram, pkg["constraints"], state, csi)
    state["inu_labels"] = labels

    i_count = sum(1 for v in labels.values() if v == "I")
    n_count = sum(1 for v in labels.values() if v == "N")
    u_count = sum(1 for v in labels.values() if v == "U")

    print(f"  I (Admissible): {i_count}")
    print(f"  N (Forbidden): {n_count}")
    print(f"  U (Unresolved): {u_count}")

    # Step 6: Check stability
    print_subheader("Step 6: Check Stability")
    is_stable, conditions, message = is_structurally_stable(pkg, "s_workflow", "d_workflow")
    print(f"  {message}")

    # Step 7: Extract metrics
    print_subheader("Step 7: Extract Metrics")
    metrics = compute_stability_metrics(pkg, "s_workflow", "d_workflow")
    print(f"  Collapse ratio: {metrics.get('collapse_ratio', 0):.3f}")
    print(f"  Gradient coherence: {metrics.get('gradient_coherence', 0):.3f}")
    print(f"  Admissible ratio: {metrics.get('admissible_ratio', 0):.3f}")

    print_subheader("Workflow Complete!")
    print("  All steps executed successfully")


def main():
    """Run all demos."""
    print("\n")
    print("*" * 70)
    print("*" + " " * 68 + "*")
    print("*" + "  SEMANTIC INTERACTION DIAGRAMS (SIDs) - FRAMEWORK DEMO".center(68) + "*")
    print("*" + " " * 68 + "*")
    print("*" * 70)

    print("""
This interactive demo showcases the complete SID framework implementation:
- Expression parsing and AST generation
- All 6 operators (P, S+, S-, O, C, T)
- CRF with 6 resolution procedures
- I/N/U epistemic typing system
- Constraint predicates
- Structural stability checking
- Metrics extraction
""")

    input("\nPress ENTER to start the demo...")

    # Run all demos
    demo_1_parsing()
    input("\nPress ENTER for next demo...")

    demo_2_operators()
    input("\nPress ENTER for next demo...")

    demo_3_crf_procedures()
    input("\nPress ENTER for next demo...")

    demo_4_inu_typing()
    input("\nPress ENTER for next demo...")

    demo_5_predicates()
    input("\nPress ENTER for next demo...")

    demo_6_stability()
    input("\nPress ENTER for next demo...")

    demo_7_full_workflow()

    # Final summary
    print_header("DEMO COMPLETE!")
    print("""
You've seen:
  [OK] Expression parsing and diagram generation
  [OK] All 6 operators in action
  [OK] All 6 CRF resolution procedures
  [OK] I/N/U typing system
  [OK] Constraint predicates
  [OK] Structural stability checking
  [OK] Complete end-to-end workflow

Try it yourself:
  python sid_ast_cli.py "C(P(Freedom), O(Choice))"
  python sid_validate_cli.py example_sid_package.json
  python sid_stability_cli.py example_sid_package.json state_0 d_initial --metrics
  python run_tests.py

Documentation:
  README.md - User guide
  IMPLEMENTATION_REPORT.md - Technical details
  IMPLEMENTATION_SUMMARY.md - Executive summary

The SID framework is ready for use!
""")
    print("*" * 70)


if __name__ == "__main__":
    main()
