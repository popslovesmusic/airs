# SID Code Documentation

## Overview
This repository now includes a minimal implementation stack to model, validate, parse, and rewrite Semantic Interaction Diagrams (SIDs).

Key components:
- Spec: `sid_technical_specifications.md`
- Package schema: `sid_package.schema.json`
- Example package: `sid_package.example.json`
- Parser/AST: `sid_parser.py`, `sid_ast.py`
- AST to diagram: `sid_ast_to_diagram.py`
- CRF + validation: `sid_crf.py`, `sid_validator.py`
- Rewrite engine: `sid_rewrite.py`
- CLI tools: `sid_ast_cli.py`, `sid_validate_cli.py`, `sid_rewrite_cli.py`, `sid_pipeline_cli.py`

## Data Model
- `sid_schema.json`: minimal, informal schema reference for code boundaries.
- `sid_package.schema.json`: JSON Schema (2020-12) for validation.

## CLI Tools

### Parse Expression to AST or Diagram
```
python sid_ast_cli.py "C(P(x), O(y))" --mode ast
python sid_ast_cli.py "P(Freedom)" --mode diagram
```

### Validate Package
```
python sid_validate_cli.py sid_package.example.json
```

### Apply Rewrites
```
python sid_rewrite_cli.py sid_package.example.json s0 d0 -o sid_package.example.rewritten.json
```

### Validate + Rewrite Pipeline
```
python sid_pipeline_cli.py sid_package.example.json s0 d0 -o sid_package.example.rewritten.json
```

## Rewrite Rule Forms
Each rewrite must specify one of:
- Edge-form:
  - `pattern`: e.g., `P(x) --C--> y`
  - `replacement`: e.g., `P(x) --C--> O(y)`
- Expression-form:
  - `pattern_expr`: e.g., `C(P(x), y)`
  - `replacement_expr`: e.g., `C(P(x), O(y))`

Expression rules support variable binding (use `$x` or single-letter lowercase like `x`) and literals (e.g., `Freedom`, `vertical`). For `P(<literal>)`, the literal must match a node’s `dof_refs`.

## CRF Predicates
`sid_crf.py` provides a predicate registry. Current default:
- `no_cross_csi_interaction`: validates edge DOF pairs against CSI `allowed_pairs`.

Preconditions supported in rewrite rules:
- `admissible`: requires no `N` labels in `state.inu_labels`.
- `no_hard_conflict`: enforced by constraint evaluation.

## Validation Rules
`sid_validator.py` checks:
- Diagram integrity (unique IDs, valid edge/node references, DOF existence).
- CSI boundaries (DOF and pair checks).
- Collapse irreversibility (`O` nodes must set `irreversible=true`).
- Rewrite rule form validity and expression parsing.

## Example Package
`sid_package.example.json` encodes the canonical pre-collapse and post-collapse diagrams and is compatible with the schema and validators.
