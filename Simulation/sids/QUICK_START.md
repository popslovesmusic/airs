# SID Quick Start - 2-Minute Guide
**Get running in 2 minutes or less**

---

## Installation

```bash
# No installation needed! Just Python 3.9+
cd D:\diagrams
```

---

## The 5-Second Test

```bash
python sid_ast_cli.py "P(Freedom)"
```

If you see JSON output, **you're ready!** ✅

---

## The 5 Operators (All You Need)

```
P(X)      "I'm thinking about X"
S+(X)     "I like X"
S-(X)     "I don't like X"
O(X)      "I've decided on X (can't undo)"
C(X,Y)    "X and Y go together"
```

---

## The 3 Essential Commands

### 1. Parse (Test Expressions)
```bash
python sid_ast_cli.py "P(MyIdea)"
```

### 2. Validate (Check Files)
```bash
python sid_validate_cli.py example_sid_package.json
```

### 3. Apply Changes (Transform)
```bash
python sid_rewrite_cli.py example_sid_package.json state_0 d_initial
```

---

## Your First Package (Copy-Paste Ready)

**File: `test.json`**

```json
{
  "dofs": [{"id": "Choice", "orthogonal_group": "main"}],
  "compartments": [{"id": "c0", "index": 0}],
  "csis": [{
    "id": "csi1",
    "allowed_dofs": ["Choice"],
    "allowed_pairs": []
  }],
  "diagrams": [{
    "id": "d1",
    "compartment_id": "c0",
    "nodes": [{"id": "n1", "op": "P", "dof_refs": ["Choice"]}],
    "edges": []
  }],
  "states": [{
    "id": "s1",
    "diagram_id": "d1",
    "csi_id": "csi1",
    "compartment_id": "c0",
    "inu_labels": {}
  }],
  "constraints": [],
  "rewrite_rules": []
}
```

**Test it:**
```bash
python sid_validate_cli.py test.json
# Should say: OK: package passes minimal SID checks
```

---

## Common Patterns

### Pattern: Simple Decision
```bash
python sid_ast_cli.py "O(P(MyDecision))"
```
Means: "I've decided on MyDecision (irreversible)"

### Pattern: Two Choices
```bash
python sid_ast_cli.py "C(P(OptionA), P(OptionB))"
```
Means: "OptionA and OptionB are related"

### Pattern: Positive Stance
```bash
python sid_ast_cli.py "S+(P(GoodIdea))"
```
Means: "I'm positive about this GoodIdea"

---

## File Structure Cheat Sheet

```json
{
  "dofs": [                   // What you can think about
    {"id": "Thing", "orthogonal_group": "group"}
  ],
  "compartments": [           // When/where (stages)
    {"id": "c0", "index": 0}
  ],
  "csis": [                   // Boundaries
    {"id": "csi1", "allowed_dofs": ["Thing"], "allowed_pairs": []}
  ],
  "diagrams": [               // The actual diagrams
    {
      "id": "d1",
      "compartment_id": "c0",
      "nodes": [{"id": "n1", "op": "P", "dof_refs": ["Thing"]}],
      "edges": []
    }
  ],
  "states": [                 // Current status
    {"id": "s1", "diagram_id": "d1", "csi_id": "csi1",
     "compartment_id": "c0", "inu_labels": {}}
  ],
  "constraints": [],          // Rules (optional)
  "rewrite_rules": []         // Transformations (optional)
}
```

---

## Operator Rules (Arguments)

| Operator | Arguments | Example |
|----------|-----------|---------|
| P | 1 | `P(X)` |
| S+ | 1+ | `S+(X)` or `S+(X,Y)` |
| S- | 1+ | `S-(X)` or `S-(X,Y)` |
| O | 1 | `O(X)` |
| C | 2 | `C(X,Y)` |
| T | 1 | `T(X)` |

---

## Troubleshooting (Top 5 Errors)

### 1. "JSON parse error"
**Fix:** Check commas, quotes, brackets in your JSON

### 2. "Collapse node must set irreversible=true"
**Fix:** Add `"irreversible": true` to O nodes

### 3. "references missing DOF"
**Fix:** Add the DOF to the `dofs` array or fix the typo

### 4. "Expected 2 arguments, got 1"
**Fix:** Check operator argument counts (C needs 2, P needs 1, etc.)

### 5. "Rewrite not applicable"
**Fix:** Pattern doesn't match - check your pattern syntax and names

---

## Get Help

```bash
# Built-in help
python sid_ast_cli.py --help

# See a working example
cat example_sid_package.json

# Run automated demo
python demo_auto.py

# Run tests
python run_tests.py
```

---

## Next Steps

1. ✅ Read `USER_GUIDE.md` for detailed walkthrough
2. ✅ Study `example_sid_package.json`
3. ✅ Try the examples in USER_GUIDE.md
4. ✅ Read `README.md` for technical details
5. ✅ Explore `sid_technical_specifications.md` for theory

---

## Visual Summary

```
Input               Parse              Validate           Transform
─────               ─────              ────────           ─────────

"P(X)"     →     JSON Diagram    →    Check Rules   →   Apply Rewrites
                                             ↓
                                          ✅ Valid
                                             ↓
                                         Use It!
```

---

**That's it!** You're ready to use SID. Start with simple examples and build up.

**Key Resources:**
- `USER_GUIDE.md` - Full beginner guide
- `example_sid_package.json` - Working example
- `README.md` - Technical reference

---

*Quick Start v1.0 - 2026-01-17*
