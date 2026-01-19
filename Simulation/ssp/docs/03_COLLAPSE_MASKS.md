# Collapse Masks - Formal Specification v1.0

## 1. Formal Definition

Let the undecided semantic field be:
```
U: Ω → ℝ≥0
```
where Ω is the discrete index set (cells, modes, bins).

A **collapse mask** is a function:
```
M: Ω → [0,1]
```

with the interpretation:
- `M(x)` is the admissible fraction of undecided semantic mass at location x eligible for irreversible collapse at this step.

**This is not probability. It is semantic eligibility density.**

## 2. Admissibility Structure (Mixer-Side)

The mixer computes two disjoint masks:
```
M_I(x), M_N(x)
```

subject to:
```
∀x ∈ Ω: 0 ≤ M_I(x) + M_N(x) ≤ 1
```

### Interpretation:
- `M_I`: admissible → included (transfer to I)
- `M_N`: inadmissible → excluded (transfer to N)
- **Remainder stays in U**

### Define Residual:
```
M_U(x) = 1 - (M_I(x) + M_N(x))
```

## 3. Collapse Law (Irreversibility)

For a collapse step with scalar intensity `α ≥ 0`:

```
ΔU(x) = α · (M_I(x) + M_N(x)) · U(x)
```

and the update:
```
U'(x) = U(x) - ΔU(x)
```

with hard constraint:
```
U'(x) ≥ 0
```

**This is one-way by construction. No inverse exists.**

## 4. Ternary Routing Semantics

Collapsed mass is not returned to U. Instead:

```
ΔI(x) = α · M_I(x) · U(x)
ΔN(x) = α · M_N(x) · U(x)
```

### Interpretation:
- I gains structured, stabilized mass
- N gains excluded, forbidden mass
- U loses undecidable mass irreversibly

**Collapse is semantic adjudication, not redistribution.**

## 5. Transport-Safe Constraints

To preserve SID invariants:

### Local Bounds
```
0 ≤ α · (M_I(x) + M_N(x)) ≤ 1
```

### Global Conservation (Soft Ternary)
```
Σ_x (I + N + U) = C    (up to numerical error)
```

### Monotonicity
```
Σ_x U_{t+1} ≤ Σ_x U_t
```

### Non-Creation
```
ΔI(x), ΔN(x) ≥ 0
```

## 6. Canonical Collapse Mask Families

### (A) Threshold Mask (Stability-Based)
```
M_I(x) = {
    1  if s(x) ≥ θ_I
    0  otherwise
}
```
where `s(x)` is a local stability indicator.

### (B) Soft Admissibility (Sigmoid)
```
M_I(x) = σ(k(s(x) - θ))
```
Smooth, differentiable, tunable.

### (C) Exclusion by Divergence
```
M_N(x) = {
    1  if d(x) ≥ θ_N
    0  otherwise
}
```
where `d(x)` is divergence metric.

### (D) Conservative Mixed Mask
```
M_I(x) + M_N(x) ≤ η < 1
```
Preserves undecided reservoir.

## 7. C Interface (Minimal, Correct)

### Mask Structure
```c
typedef struct {
    double* M_I;   /* admissible mask */
    double* M_N;   /* exclusion mask */
    uint64_t len;
} sid_collapse_mask_t;
```

### Mixer → SSP(U)
```c
void sid_ssp_apply_collapse_mask(
    sid_ssp_t* ssp_U,
    const sid_collapse_mask_t* mask,
    double alpha
);
```

### SSP(U) Execution Rule
```c
U[i] -= alpha * (M_I[i] + M_N[i]) * U[i];
// with clamping to ensure U[i] >= 0
```

**Important:** SSP does not know why the mask exists. It only executes the collapse directive.

## 8. Formal Spec Stub (Archive-Ready JSON5)

```json5
{
  id: "SID_CollapseMask_v1",
  domain: "Undecided semantic field",
  mask_definition: "M: Omega -> [0,1]",
  components: {
    M_I: "admissible inclusion mask",
    M_N: "inadmissible exclusion mask",
    M_U: "implicit residual"
  },
  constraints: [
    "0 <= M_I(x) + M_N(x) <= 1",
    "collapse is irreversible",
    "U is monotonically non-increasing"
  ],
  collapse_law: {
    delta_U: "alpha * (M_I + M_N) * U",
    routing: {
      I: "alpha * M_I * U",
      N: "alpha * M_N * U"
    }
  },
  semantics: {
    I: "admitted structure",
    N: "excluded structure",
    U: "undecided reservoir"
  }
}
```

## One-Line Formal Statement

**A collapse mask is a bounded semantic eligibility field that irreversibly transfers undecided mass into admissible or excluded structure under ternary conservation.**

## What's Next (Logically)

1. Implement mask generators (stability/divergence driven)
2. Route collapsed mass into I/N SSPs (optional)
3. Make Transport a function of mask exhaustion
4. Connect masks to CSI / R-Map later
