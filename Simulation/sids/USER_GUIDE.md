# SID Framework - User Guide for Beginners
**Getting Started with Semantic Interaction Diagrams**

---

## Welcome!

This guide will help you start using the SID (Semantic Interaction Diagrams) framework **without** needing to understand all the theory first. Think of it as learning to drive before studying automotive engineering.

**What You'll Learn:**
1. What files you need and how to create them
2. How to run the tools (copy-paste commands)
3. What the output means
4. Common tasks and examples

**What You DON'T Need:**
- Advanced mathematics
- Deep theoretical knowledge
- Programming experience (beyond running commands)

Let's get started!

---

## Table of Contents

1. [Quick Start (5 Minutes)](#quick-start-5-minutes)
2. [Understanding the Basics](#understanding-the-basics)
3. [Input Formats](#input-formats)
4. [Your First SID Package](#your-first-sid-package)
5. [Common Tasks](#common-tasks)
6. [Walkthrough Examples](#walkthrough-examples)
7. [Troubleshooting](#troubleshooting)
8. [Quick Reference](#quick-reference)

---

## Quick Start (5 Minutes)

### What You Need

1. **Python 3.9+** installed
2. **This SID framework** (the files you have)
3. **A text editor** (Notepad, VSCode, etc.)
4. **A terminal/command prompt**

### Your First Command

Let's parse a simple expression:

```bash
# Navigate to the SID directory
cd D:\diagrams

# Parse an expression
python sid_ast_cli.py "P(Freedom)"
```

**You should see:**
```json
{
  "edges": [],
  "id": "d_expr",
  "nodes": [
    {
      "dof_refs": ["Freedom"],
      "id": "n1",
      "op": "P"
    }
  ]
}
```

🎉 **Congratulations!** You just created your first SID diagram!

---

## Understanding the Basics

### What is SID?

Think of SID as a way to **draw diagrams about decisions and possibilities** using simple building blocks.

**Analogy:** Like LEGO blocks, SID has basic pieces you combine to build complex structures.

### The 5 Basic Operators

You only need to know 5 operators to get started:

| Operator | Name | What It Does | Example |
|----------|------|--------------|---------|
| **P** | Projection | "I'm thinking about X" | `P(Freedom)` |
| **S+** | Positive | "I like X" / "Yes to X" | `S+(Choice)` |
| **S-** | Negative | "I don't like X" / "No to X" | `S-(Constraint)` |
| **O** | Collapse | "I've decided on X (can't undo)" | `O(P(Choice))` |
| **C** | Coupling | "X and Y go together" | `C(P(A), P(B))` |

**That's it!** These 5 operators are all you need to start.

---

## Input Formats

### 1. Expressions (Text Format)

**What:** Simple text that describes a diagram
**Where:** Type directly in commands
**Format:** `Operator(arguments)`

**Examples:**
```
P(Freedom)                    # Simple: thinking about Freedom
S+(Choice)                    # Positive stance on Choice
O(P(Decision))                # Collapsed projection of Decision
C(P(X), P(Y))                # Coupling two projections
```

**Rules:**
- Operators are CAPITAL letters: P, S+, S-, O, C, T
- Arguments go in parentheses: `P(something)`
- Multiple arguments separated by commas: `C(arg1, arg2)`
- Names can be letters, numbers, underscores: `P(MyChoice_1)`

---

### 2. JSON Package Files

**What:** Complete description of a SID system
**Where:** Saved as `.json` files
**Why:** Stores diagrams, rules, and constraints together

**Basic Structure:**
```json
{
  "dofs": [...],          // Degrees of Freedom (possibilities)
  "compartments": [...],  // Stages or levels
  "csis": [...],          // Boundaries of what can interact
  "diagrams": [...],      // The actual diagrams
  "states": [...],        // Current status
  "constraints": [...],   // Rules that must be followed
  "rewrite_rules": [...]  // How diagrams can transform
}
```

**Don't worry!** We'll build this step by step.

---

## Your First SID Package

Let's create a simple package from scratch. We'll model a **decision about what to eat**.

### Step 1: Create a New File

Create a file called `my_first_sid.json` in the SID directory.

### Step 2: Copy This Template

```json
{
  "dofs": [
    {
      "id": "Food",
      "orthogonal_group": "choices"
    }
  ],
  "compartments": [
    {
      "id": "c0",
      "index": 0
    }
  ],
  "csis": [
    {
      "id": "csi_main",
      "allowed_dofs": ["Food"],
      "allowed_pairs": []
    }
  ],
  "diagrams": [
    {
      "id": "d1",
      "compartment_id": "c0",
      "nodes": [
        {
          "id": "n1",
          "op": "P",
          "dof_refs": ["Food"]
        }
      ],
      "edges": []
    }
  ],
  "states": [
    {
      "id": "s1",
      "diagram_id": "d1",
      "csi_id": "csi_main",
      "compartment_id": "c0",
      "inu_labels": {}
    }
  ],
  "constraints": [],
  "rewrite_rules": []
}
```

### Step 3: What Each Part Means

**DOFs (Degrees of Freedom):**
- These are the "things you can think about"
- In our example: `Food` is something we can consider
- Think of it as "a variable" or "a topic"

**Compartments:**
- These are "stages" or "moments in time"
- `c0` is the first stage (index 0)
- Like "before deciding" vs "after deciding"

**CSIs (Causal Sphere of Influence):**
- These define "what can interact with what"
- Our `csi_main` says: "You can think about Food"
- It's like setting boundaries

**Diagrams:**
- The actual visual structure
- Our diagram has one node: `P(Food)` - "thinking about Food"
- Nodes are connected by edges (we have none yet)

**States:**
- The current situation
- Links a diagram to a CSI
- Tracks status with `inu_labels` (we'll explain later)

**Constraints & Rewrite Rules:**
- Rules about what's allowed
- How diagrams can change
- Empty for now (we'll add later)

### Step 4: Validate Your Package

```bash
python sid_validate_cli.py my_first_sid.json
```

**You should see:**
```
OK: package passes minimal SID checks
```

🎉 **Success!** You've created a valid SID package!

---

## Common Tasks

### Task 1: Parse an Expression

**What:** Convert text to diagram format

**Command:**
```bash
python sid_ast_cli.py "P(MyIdea)"
```

**When to use:**
- Testing expressions
- Creating diagram snippets
- Learning the syntax

---

### Task 2: Validate a Package

**What:** Check if your JSON file is correct

**Command:**
```bash
python sid_validate_cli.py your_file.json
```

**What it checks:**
- File is valid JSON
- All required fields present
- All references point to real things
- No duplicate IDs

**Output:**
- `OK: package passes minimal SID checks` ✅ Good!
- `ERROR: ...` ❌ Something's wrong (see error message)

---

### Task 3: Check Stability

**What:** See if your diagram is "settled" (no more changes needed)

**Command:**
```bash
python sid_stability_cli.py your_file.json state_id diagram_id --metrics
```

**Example:**
```bash
python sid_stability_cli.py example_sid_package.json state_0 d_initial --metrics
```

**Output:**
```
Stability: True
Conditions met: ['no_rewrites', 'identity_only']
Metrics:
  admissible_volume: 5
  collapse_ratio: 0.20
  loop_gain: 0.0001
```

**What this means:**
- `Stability: True` = System is stable (good!)
- `Conditions met` = Why it's stable
- `Metrics` = Numbers about the diagram

---

### Task 4: Apply Rewrites

**What:** Transform your diagram according to rules

**Command:**
```bash
python sid_rewrite_cli.py input.json state_id diagram_id -o output.json
```

**Example:**
```bash
python sid_rewrite_cli.py example_sid_package.json state_0 d_initial -o result.json
```

**What it does:**
- Reads your package
- Applies all rewrite rules
- Saves the result

---

## Walkthrough Examples

### Example 1: Simple Decision (No Theory Required)

**Scenario:** You're deciding whether to accept a job offer.

#### Step 1: Create the DOF (what you're thinking about)

```json
"dofs": [
  {
    "id": "JobOffer",
    "orthogonal_group": "decision"
  }
]
```

**Translation:** "I'm thinking about a JobOffer"

#### Step 2: Create a Compartment (when)

```json
"compartments": [
  {
    "id": "before_decision",
    "index": 0
  }
]
```

**Translation:** "This is before I decide"

#### Step 3: Set Boundaries (CSI)

```json
"csis": [
  {
    "id": "csi_decision",
    "allowed_dofs": ["JobOffer"],
    "allowed_pairs": []
  }
]
```

**Translation:** "I'm only thinking about JobOffer right now"

#### Step 4: Create the Diagram

```json
"diagrams": [
  {
    "id": "considering",
    "compartment_id": "before_decision",
    "nodes": [
      {
        "id": "n1",
        "op": "P",
        "dof_refs": ["JobOffer"]
      }
    ],
    "edges": []
  }
]
```

**Translation:** "I'm projecting (thinking about) the JobOffer"

**Visual:**
```
┌──────────────┐
│  P(JobOffer) │
└──────────────┘
```

#### Step 5: Add a Decision (Collapse)

Let's add a second diagram where you've decided:

```json
{
  "id": "decided",
  "compartment_id": "before_decision",
  "nodes": [
    {
      "id": "n1",
      "op": "P",
      "dof_refs": ["JobOffer"]
    },
    {
      "id": "n2",
      "op": "O",
      "inputs": ["n1"],
      "irreversible": true
    }
  ],
  "edges": [
    {
      "id": "e1",
      "from": "n1",
      "to": "n2",
      "label": "arg"
    }
  ]
}
```

**Translation:** "I've collapsed (decided on) the JobOffer - can't undo!"

**Visual:**
```
┌──────────────┐      ┌────────────────────┐
│  P(JobOffer) │─────▶│  O(P(JobOffer))    │
└──────────────┘      └────────────────────┘
                           (DECIDED!)
```

---

### Example 2: Two Choices (Coupling)

**Scenario:** Choosing between Pizza and Salad, and they're related.

#### Expression

```bash
python sid_ast_cli.py "C(P(Pizza), P(Salad))"
```

**Meaning:** "Pizza and Salad are coupled - choosing one affects the other"

#### Output

```json
{
  "nodes": [
    {
      "id": "n1",
      "op": "P",
      "dof_refs": ["Pizza"]
    },
    {
      "id": "n2",
      "op": "P",
      "dof_refs": ["Salad"]
    },
    {
      "id": "n3",
      "op": "C",
      "inputs": ["n1", "n2"]
    }
  ],
  "edges": [
    {
      "id": "e1",
      "from": "n1",
      "to": "n3",
      "label": "arg"
    },
    {
      "id": "e2",
      "from": "n2",
      "to": "n3",
      "label": "arg"
    }
  ]
}
```

**Visual:**
```
┌──────────┐
│ P(Pizza) │──┐
└──────────┘  │
              ▼
         ┌────────────┐
         │  C(P,P)    │
         └────────────┘
              ▲
┌──────────┐  │
│ P(Salad) │──┘
└──────────┘
```

---

### Example 3: Full Workflow (End-to-End)

Let's model: **"I'm considering learning Python or JavaScript, then I'll decide on one."**

#### Step 1: Create the Package

**File: `learning_decision.json`**

```json
{
  "dofs": [
    {
      "id": "Python",
      "orthogonal_group": "languages"
    },
    {
      "id": "JavaScript",
      "orthogonal_group": "languages"
    }
  ],
  "compartments": [
    {
      "id": "c0_considering",
      "index": 0
    },
    {
      "id": "c1_decided",
      "index": 1
    }
  ],
  "csis": [
    {
      "id": "csi_lang",
      "allowed_dofs": ["Python", "JavaScript"],
      "allowed_pairs": [
        ["Python", "JavaScript"]
      ]
    }
  ],
  "diagrams": [
    {
      "id": "d_considering",
      "compartment_id": "c0_considering",
      "nodes": [
        {
          "id": "n1",
          "op": "P",
          "dof_refs": ["Python"]
        },
        {
          "id": "n2",
          "op": "P",
          "dof_refs": ["JavaScript"]
        },
        {
          "id": "n3",
          "op": "C",
          "inputs": ["n1", "n2"]
        }
      ],
      "edges": [
        {
          "id": "e1",
          "from": "n1",
          "to": "n3",
          "label": "arg"
        },
        {
          "id": "e2",
          "from": "n2",
          "to": "n3",
          "label": "arg"
        }
      ]
    }
  ],
  "states": [
    {
      "id": "s0",
      "diagram_id": "d_considering",
      "csi_id": "csi_lang",
      "compartment_id": "c0_considering",
      "inu_labels": {}
    }
  ],
  "constraints": [],
  "rewrite_rules": [
    {
      "id": "decide_python",
      "pattern_expr": "C(P(Python), P(JavaScript))",
      "replacement_expr": "O(P(Python))",
      "preconditions": ["admissible"]
    }
  ]
}
```

#### Step 2: Validate

```bash
python sid_validate_cli.py learning_decision.json
```

Expected: `OK: package passes minimal SID checks`

#### Step 3: Check Initial State

```bash
python sid_stability_cli.py learning_decision.json s0 d_considering
```

Expected: You'll see if the system is stable

#### Step 4: Apply Rewrite (Make a Decision)

```bash
python sid_rewrite_cli.py learning_decision.json s0 d_considering -o decided.json
```

**What happens:**
- The rewrite rule matches `C(P(Python), P(JavaScript))`
- Replaces it with `O(P(Python))` - "Decided on Python!"
- Saves result to `decided.json`

#### Step 5: Check the Result

Look at `decided.json` - the diagram has changed from "considering both" to "decided on Python"!

---

## Troubleshooting

### Problem: "ERROR: JSON parse error"

**Cause:** Invalid JSON syntax

**Fix:**
1. Check for missing commas: `{...}, {...}`
2. Check for missing quotes: `"id": "value"`
3. Check for trailing commas: `[...,]` ← Remove last comma
4. Use a JSON validator: https://jsonlint.com/

**Example Error:**
```json
{
  "id": "test"    // ❌ Missing comma
  "value": 123
}
```

**Fixed:**
```json
{
  "id": "test",   // ✅ Added comma
  "value": 123
}
```

---

### Problem: "ERROR: Diagram X references missing DOF Y"

**Cause:** You used a DOF name that isn't defined

**Fix:**
1. Check the `dofs` section
2. Add the missing DOF
3. Or fix the typo in the diagram

**Example:**
```json
// Diagram uses "Freedm" (typo)
"dof_refs": ["Freedm"]

// But DOF is defined as "Freedom"
"dofs": [{"id": "Freedom", ...}]
```

**Fix:** Change `"Freedm"` to `"Freedom"`

---

### Problem: "ERROR: Collapse node X must set irreversible=true"

**Cause:** Collapse operators (O) need `irreversible: true`

**Fix:** Add the field to your O nodes

**Before:**
```json
{
  "id": "n2",
  "op": "O",
  "inputs": ["n1"]
}
```

**After:**
```json
{
  "id": "n2",
  "op": "O",
  "inputs": ["n1"],
  "irreversible": true  // ✅ Added this
}
```

---

### Problem: "ParseError: Expected 2 arguments, got 1"

**Cause:** Operator arity violation (wrong number of arguments)

**Operators and their argument counts:**
- P: exactly 1 argument: `P(X)`
- S+: 1 or more: `S+(X)` or `S+(X, Y)`
- S-: 1 or more: `S-(X)` or `S-(X, Y)`
- O: exactly 1: `O(X)`
- C: exactly 2: `C(X, Y)`
- T: exactly 1: `T(X)`

**Example Error:**
```bash
python sid_ast_cli.py "C(P(X))"  # ❌ C needs 2 arguments
```

**Fixed:**
```bash
python sid_ast_cli.py "C(P(X), P(Y))"  # ✅ Now has 2
```

---

### Problem: "Rewrite X not applicable"

**Cause:** The pattern doesn't match your diagram

**Fix:**
1. Check that your diagram has the pattern
2. Verify the pattern syntax
3. Use exact names (case-sensitive)

**Example:**
```json
// Rule looking for:
"pattern_expr": "P(Freedom)"

// But diagram has:
"dof_refs": ["freedom"]  // ❌ lowercase
```

**Fix:** Match the case exactly

---

## Quick Reference

### Command Cheat Sheet

```bash
# Parse expression
python sid_ast_cli.py "P(Something)"

# Validate package
python sid_validate_cli.py file.json

# Check stability
python sid_stability_cli.py file.json state_id diagram_id --metrics

# Apply rewrites
python sid_rewrite_cli.py file.json state_id diagram_id -o output.json

# Full pipeline (validate + rewrite + validate)
python sid_pipeline_cli.py file.json state_id diagram_id -o output.json

# Run all tests
python run_tests.py
```

---

### Expression Syntax Quick Reference

```
P(X)              # Projection: thinking about X
S+(X)             # Positive polarity
S-(X)             # Negative polarity
O(X)              # Collapse: irreversible decision
C(X, Y)           # Coupling: X and Y related
T(X)              # Transport: move to next compartment

# Examples
P(Freedom)
S+(Choice)
O(P(Decision))
C(P(A), P(B))
C(P(X), O(P(Y)))
```

---

### Package Structure Template

```json
{
  "dofs": [
    {"id": "SomeThing", "orthogonal_group": "group1"}
  ],
  "compartments": [
    {"id": "c0", "index": 0}
  ],
  "csis": [
    {
      "id": "csi1",
      "allowed_dofs": ["SomeThing"],
      "allowed_pairs": []
    }
  ],
  "diagrams": [
    {
      "id": "d1",
      "compartment_id": "c0",
      "nodes": [
        {"id": "n1", "op": "P", "dof_refs": ["SomeThing"]}
      ],
      "edges": []
    }
  ],
  "states": [
    {
      "id": "s1",
      "diagram_id": "d1",
      "csi_id": "csi1",
      "compartment_id": "c0",
      "inu_labels": {}
    }
  ],
  "constraints": [],
  "rewrite_rules": []
}
```

---

### Operator Argument Rules

| Operator | Min Args | Max Args | Example |
|----------|----------|----------|---------|
| P | 1 | 1 | `P(X)` |
| S+ | 1 | No limit | `S+(X, Y, Z)` |
| S- | 1 | No limit | `S-(X, Y)` |
| O | 1 | 1 | `O(X)` |
| C | 2 | 2 | `C(X, Y)` |
| T | 1 | 1 | `T(X)` |

---

### Common Field Names

| Field | What It Is | Example Value |
|-------|------------|---------------|
| `id` | Unique identifier | `"n1"`, `"my_diagram"` |
| `op` | Operator type | `"P"`, `"O"`, `"C"` |
| `dof_refs` | DOF names | `["Freedom"]` |
| `inputs` | Node IDs this connects to | `["n1", "n2"]` |
| `from` | Edge start node | `"n1"` |
| `to` | Edge end node | `"n2"` |
| `label` | Edge type | `"arg"` |
| `irreversible` | Required for O operator | `true` |

---

## Next Steps

### You've Learned:
✅ How to parse expressions
✅ How to create SID packages
✅ How to validate your work
✅ How to apply rewrites
✅ How to troubleshoot common errors

### What's Next?

**Level 2 (Intermediate):**
1. Add constraints to your packages
2. Create complex rewrite rules
3. Use multiple compartments
4. Explore I/N/U labels

**Level 3 (Advanced):**
1. Read `sid_technical_specifications.md` for theory
2. Study `IMPLEMENTATION_REPORT.md` for internals
3. Explore the `dev/manual/` theoretical extensions

**Resources:**
- `README.md` - Technical reference
- `example_sid_package.json` - Working example
- `sid_technical_specifications.md` - Full theory

---

## Getting Help

### Built-in Help

```bash
# Get help for any tool
python sid_ast_cli.py --help
python sid_validate_cli.py --help
```

### Example Files

Check out `example_sid_package.json` - it's a complete, working example you can study and modify.

### Run the Demo

```bash
python demo_auto.py
```

This runs through a complete workflow automatically so you can see it in action.

---

## Summary

**Remember:**
1. **Start simple** - One DOF, one diagram
2. **Validate often** - Check your JSON frequently
3. **Read error messages** - They tell you exactly what's wrong
4. **Use examples** - Copy and modify working examples
5. **Experiment** - You can't break anything!

**The 5 Operators:**
- **P** = Thinking about something
- **S+** = Positive stance
- **S-** = Negative stance
- **O** = Final decision (irreversible)
- **C** = Things connected together

**The 3 Main Commands:**
```bash
python sid_validate_cli.py file.json              # Check if valid
python sid_stability_cli.py file.json s1 d1       # Check if stable
python sid_rewrite_cli.py file.json s1 d1 -o out  # Apply changes
```

---

**Welcome to SID!** You're now ready to create and manipulate semantic interaction diagrams. Start with simple examples and build up complexity as you get comfortable.

Happy diagramming! 🎉

---

*User Guide v1.0 - Created 2026-01-17*
