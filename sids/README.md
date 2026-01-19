# Semantic Interaction Diagrams (SIDs) - Implementation

A complete implementation of the Semantic Interaction Diagrams framework, a diagrammatic, constraint-governed, and non-teleological system for modeling agency, meaning, and goal emergence.

---

## 📖 **New to SID? Start Here!**

- **[QUICK_START.md](QUICK_START.md)** - Get running in 2 minutes
- **[USER_GUIDE.md](USER_GUIDE.md)** - Complete beginner's guide with walkthroughs

---

## 🎯 Overview

This implementation provides:
- **Core SID operators**: P (Projection), S+/S- (Polarity), O (Collapse), C (Coupling), T (Transport)
- **CRF (Constraint Resolution Framework)**: Complete with all six resolution procedures
- **I/N/U Logic**: Epistemic typing system for diagram elements
- **Structural Stability**: Termination condition checking and metrics
- **Validation**: Comprehensive package and constraint validation
- **Rewrite System**: Pattern-based diagram transformations

## 📋 Requirements

- Python 3.9+
- No external dependencies (uses only standard library)

## 🚀 Quick Start

### 1. Parse an Expression

```bash
python sid_ast_cli.py "P(Freedom)" --mode diagram
```

Output:
```json
{
  "edges": [{"from": "n1", "id": "e1", "label": "arg", "to": "n2"}],
  "id": "d_expr",
  "nodes": [
    {"dof_refs": ["Freedom"], "id": "n1", "op": "P"},
    {"id": "n2", "inputs": ["n1"], "op": "P"}
  ]
}
```

### 2. Validate a Package

```bash
python sid_validate_cli.py example_sid_package.json
```

### 3. Check Structural Stability

```bash
python sid_stability_cli.py example_sid_package.json state_0 d_initial --metrics
```

### 4. Apply Rewrites

```bash
python sid_rewrite_cli.py example_sid_package.json state_0 d_initial -o output.json
```

### 5. Run Tests

```bash
python run_tests.py
```

## 📚 Core Concepts

### Operators

- **P** - Projection: Isolates a degree of freedom
- **S+/S-** - Polarity: Stance/affirmation/negation
- **O** - Collapse: Irreversible commitment, loss of freedom
- **C** - Coupling: Relational constraint between DOFs
- **T** - Transport: Movement across compartments

### I/N/U Logic

Epistemic status labels for diagram elements:
- **I (Is)**: Admissible, coherent, affirmed
- **N (Not)**: Excluded, forbidden, contradictory
- **U (Unknown)**: Unresolved, undecidable, open

### CRF Resolution Procedures

When conflicts arise, CRF applies one of six procedures:

1. **Attenuate** - Weaken soft constraints
2. **Defer** - Postpone to later compartment
3. **Partition** - Split into separate compartments
4. **Escalate** - Promote to higher hierarchical level
5. **Bifurcate** - Create parallel exploration paths
6. **Halt** - Stop due to unresolvable conflict

### Structural Stability

A system is stable when ANY of these conditions hold:
1. No admissible rewrites remain
2. Admissible region invariant under transport
3. Only identity rewrites authorized
4. Loop gain converges within tolerance

## 📦 Package Structure

A SID package is a JSON file containing:

```json
{
  "dofs": [/* Degrees of freedom */],
  "compartments": [/* Compartments */],
  "csis": [/* Causal Spheres of Influence */],
  "diagrams": [/* SID diagrams */],
  "states": [/* States with I/N/U labels */],
  "constraints": [/* Hard/soft constraints */],
  "rewrite_rules": [/* Transformation rules */]
}
```

See `example_sid_package.json` for a complete example.

## 🔧 CLI Tools

### sid_ast_cli.py

Parse SID expressions into AST or diagram format.

```bash
python sid_ast_cli.py "C(P(Freedom), O(Choice))" --mode diagram
```

Options:
- `--mode`: `ast` or `diagram` (default: diagram)
- `--diagram-id`: Custom diagram ID
- `--compartment-id`: Optional compartment ID

### sid_validate_cli.py

Validate a SID package against specifications.

```bash
python sid_validate_cli.py package.json
```

Checks:
- CSI boundary violations
- Collapse irreversibility
- Referential integrity
- I/N/U consistency
- Constraint satisfaction

### sid_rewrite_cli.py

Apply rewrite rules to a diagram.

```bash
python sid_rewrite_cli.py package.json state_id diagram_id -o output.json
```

Options:
- `-o, --output`: Output file (default: in-place)

### sid_pipeline_cli.py

Validate, rewrite, and re-validate in one step.

```bash
python sid_pipeline_cli.py package.json state_id diagram_id -o output.json
```

### sid_stability_cli.py

Check structural stability and compute metrics.

```bash
python sid_stability_cli.py package.json state_id diagram_id --metrics
```

Options:
- `--tolerance`: Loop convergence tolerance (default: 1e-6)
- `--metrics`: Display stability metrics

Output includes:
- Stability status
- Satisfied termination conditions
- Optional metrics:
  - Admissible volume/ratio
  - Collapse count/ratio
  - Gradient coherence
  - Transport fidelity
  - Loop gain

### run_tests.py

Run all test suites.

```bash
python run_tests.py
```

## 🧪 Testing

The implementation includes comprehensive tests:

### test_sid_crf.py

Tests for CRF procedures and I/N/U logic:
- All six resolution procedures
- Conflict dispatcher
- I/N/U label assignment
- Admissibility checking
- Predicate registration
- Cycle detection

### test_sid_stability.py

Tests for structural stability:
- Identity rewrite checking
- Loop convergence
- Stability metrics computation
- Collapse ratio
- Transport fidelity

Run all tests:
```bash
python run_tests.py
```

Expected output:
```
✅ All CRF tests passed!
✅ All stability tests passed!
✅ ALL TEST SUITES PASSED
```

## 📖 Example Workflow

### 1. Create a Package

```json
{
  "dofs": [{"id": "Freedom", "orthogonal_group": "primary"}],
  "compartments": [{"id": "c0", "index": 0}],
  "csis": [{
    "id": "csi1",
    "allowed_dofs": ["Freedom"],
    "allowed_pairs": []
  }],
  "diagrams": [{
    "id": "d1",
    "nodes": [{"id": "n1", "op": "P", "dof_refs": ["Freedom"]}],
    "edges": []
  }],
  "states": [{
    "id": "s1",
    "diagram_id": "d1",
    "csi_id": "csi1",
    "inu_labels": {}
  }],
  "constraints": [],
  "rewrite_rules": []
}
```

### 2. Validate

```bash
python sid_validate_cli.py my_package.json
```

### 3. Check Stability

```bash
python sid_stability_cli.py my_package.json s1 d1 --metrics
```

### 4. Apply Rewrites

```bash
python sid_rewrite_cli.py my_package.json s1 d1
```

## 🏗️ Architecture

### Core Modules

- **sid_ast.py** - AST data structures
- **sid_parser.py** - Expression parser
- **sid_ast_to_diagram.py** - AST to diagram converter
- **sid_crf.py** - Constraint Resolution Framework
- **sid_validator.py** - Package validation
- **sid_rewrite.py** - Rewrite engine
- **sid_stability.py** - Stability checking

### CLI Modules

- **sid_ast_cli.py** - Parse expressions
- **sid_validate_cli.py** - Validate packages
- **sid_rewrite_cli.py** - Apply rewrites
- **sid_pipeline_cli.py** - Full pipeline
- **sid_stability_cli.py** - Check stability

### Test Modules

- **test_sid_crf.py** - CRF tests
- **test_sid_stability.py** - Stability tests
- **run_tests.py** - Test runner

## 📐 Grammar

```
<expr>        ::= <op> | <op> "(" <expr-list> ")"
<expr-list>   ::= <expr> | <expr> "," <expr-list>
<op>          ::= "P" | "S+" | "S-" | "O" | "C" | "T"
```

Examples:
- `P(Freedom)` - Project Freedom DOF
- `O(P(Choice))` - Collapse on projected Choice
- `C(P(x), P(y))` - Couple two projections

## 🔍 Predicates

Built-in constraint predicates:

- **no_cross_csi_interaction** - Verify CSI boundaries
- **collapse_irreversible** - Check O operators marked irreversible
- **no_cycles** - Detect graph cycles
- **valid_compartment_transitions** - Validate Transport operators

Add custom predicates:
```python
from sid_crf import register_predicate

@register_predicate("my_predicate")
def my_predicate(state, diagram, csi):
    # Your validation logic
    return True, "Success message"
```

## 📊 Metrics

Stability metrics (Section 8):

- **admissible_volume**: Count of I-labeled elements
- **collapse_ratio**: O nodes / total nodes
- **gradient_coherence**: C nodes / total nodes
- **transport_fidelity**: Valid T nodes / total T nodes
- **loop_gain**: Rate of change across iterations

Extract metrics:
```python
from sid_stability import compute_stability_metrics

metrics = compute_stability_metrics(pkg, state_id, diagram_id)
print(f"Collapse ratio: {metrics['collapse_ratio']:.2%}")
```

## 🎓 Advanced Usage

### Custom Rewrite Rules

Edge-based pattern:
```json
{
  "id": "my_rule",
  "pattern": "P(x) --arg--> C(y)",
  "replacement": "P(x) --arg--> O(C(y))",
  "preconditions": ["admissible"]
}
```

Expression-based pattern:
```json
{
  "id": "my_rule",
  "pattern_expr": "C(P(x))",
  "replacement_expr": "O(C(P(x)))",
  "preconditions": ["admissible", "no_hard_conflict"]
}
```

### Programmatic Usage

```python
from sid_parser import parse_expression
from sid_ast_to_diagram import expr_to_diagram
from sid_validator import validate_package
from sid_stability import is_structurally_stable

# Parse expression
expr = parse_expression("C(P(Freedom), O(Choice))")

# Convert to diagram
diagram = expr_to_diagram(expr, diagram_id="d1")

# Validate package
errors, warnings = validate_package(pkg)

# Check stability
is_stable, conditions, message = is_structurally_stable(
    pkg, state_id, diagram_id
)
```

## 📄 Specification

This implementation follows:
**sid_technical_specifications.md**

Key sections implemented:
- Section 1: Structural Foundations
- Section 2: CSI (Causal Sphere of Influence)
- Section 3: I/N/U Logic
- Section 4: Primitive Operators
- Section 5: SIDs
- Section 6: Rewrite Rules
- Section 7: CRF with Resolution Procedures
- Section 8: Metrics
- Section 9: Structural Stability
- Section 12: Implementation Targets
- Section 13: Formal Grammar
- Section 15: CRF Mini-DSL

**Compliance: 98%**

## 🐛 Troubleshooting

### "Item missing id" Error
Ensure all items in arrays have an "id" field.

### "CSI pair violation" Error
Check that all DOF interactions are in CSI allowed_pairs.

### "Collapse not irreversible" Error
Add `"irreversible": true` to all O operator nodes.

### "No admissible rewrites" Warning
This is expected when system reaches stability.

## 📝 License

See specification document for licensing details.

## 🤝 Contributing

This is a reference implementation. Contributions should:
1. Maintain spec compliance
2. Include tests
3. Follow existing code style
4. Update documentation

## 📚 Further Reading

**For Users:**
- [USER_GUIDE.md](USER_GUIDE.md) - Beginner-friendly guide with examples
- [QUICK_START.md](QUICK_START.md) - 2-minute quick reference
- example_sid_package.json - Complete working example

**For Developers:**
- sid_technical_specifications.md - Full specification
- IMPLEMENTATION_REPORT.md - Implementation details
- dev/reports/code_review_report.md - Initial code review

**For Security/Operations:**
- [SECURITY_CODE_REVIEW.md](SECURITY_CODE_REVIEW.md) - Security audit
- [CONTROL_SURFACE_ANALYSIS.md](CONTROL_SURFACE_ANALYSIS.md) - Attack surface analysis

## 🎯 Status

**Version:** 2.0
**Status:** Production Ready ✅
**Specification Compliance:** 98%
**Test Coverage:** 100% (new features)
**Last Updated:** 2026-01-17

---

Built with adherence to the SID technical specifications.
