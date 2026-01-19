# SID Mixer - Specification v0.1

## Foundational Invariants (Must Always Hold)

### 1. Ternary Conservation (Soft Ternary)
```
I(t) + N(t) + U(t) = C   for all t
```
- C is fixed per scenario
- Engines may internally violate this; Mixer corrects globally
  - If `I+N+U > C`, Mixer routes U->I/N then collapses U (dual-mask law); if still over, scale I/N down to hit C
  - If `I+N+U < C`, Mixer injects mass into U proportional to the current U field (uniform only if U is zero)

### 2. Role Separation
Engines never:
- Read each other
- Transfer mass/state directly
- Apply admissibility logic

**Mixer is the only coupling locus.**

### 3. Irreversibility
**Transfers:**
- U → I (admission)
- U → N (exclusion)

**Forbidden:**
- I → U
- N → U
- I ↔ N

Collapse is one-way.

### 4. Semantic, Not Numeric
- Mixer does not average, blend, or smooth fields
- Decisions are predicate-based, not arithmetic

## Stability & Transport Predicate

Transport `T` is signaled when all are true:
- `loop_gain` converges (Δ < ε for K steps)
- `admissible_volume / C ≥ θ`
- `collapse_ratio` monotonic for K steps
- No invariant violation for K steps

Transport is **event emission**, not state mutation.

## Observable SID Metrics (Emitted Each Tick)

### loop_gain
Feedback amplification across I↔U boundary

### admissible_volume
Total mass in I: `admissible_volume = I`

### excluded_volume
Total mass in N: `excluded_volume = N`

### undecided_volume
Total mass in U: `undecided_volume = U`

### collapse_ratio
```
collapse_ratio = (I + N) / C
```
Proportion of total semantic capacity that has been committed.

Alternative formulation (irreversible depletion):
```
collapse_ratio = (U₀ - U) / U₀
```

### conservation_error
```
conservation_error = |(I + N + U) - C|
```

### transport_ready
Boolean flag: `1` if stability conditions met for K steps, `0` otherwise.

## Mixer Operations

### Observation
Read the semantic fields of I, N, and U (read-only, no mutation).

### Invariant Checking
Verify that conservation holds: `I + N + U = C`
Detect violations and signal error conditions.

### Admissibility Evaluation
Determine which content in U is:
- **Admissible** (can transfer to I)
- **Inadmissible** (must transfer to N)

This is where rewrite rules, CSI filters, and semantic policies are applied.

### Collapse Directive
Issue a collapse command to SSP(U), specifying:
- Which regions of the semantic field should collapse
- In which direction (to I or to N)
- With what magnitude

### Metric Emission
Calculate and expose system-level metrics (listed above).

## What the Mixer Explicitly Does Not Do

The mixer:
- **Does not write to I or N** (only U is mutable via collapse)
- **Does not perform numerical averaging**
- **Does not contain physics** (dynamics live in processors)
- **Does not make arbitrary choices** (all decisions follow formal rewrite rules)

## Real Invariant Implementation

### Loop Gain (Proxy)
```c
deltaI = I - prev_I
deltaU = prev_U - U  // positive if U decreased
inst_gain = deltaI / max(deltaU, 1e-12)

// Smooth gain
loop_gain = 0.9 * loop_gain + 0.1 * inst_gain
```

### Transport Predicate (Minimal, Real)
System is stable if:
- `conservation_error ≤ eps_conservation`
- `|deltaI| ≤ eps_delta`
- `|deltaU| ≤ eps_delta`

If stable for K consecutive steps, set `transport_ready = 1`.

## Collapse Execution

### Current Stub Behavior
```c
// Mask: M_I = all ones (policy-free), M_N = all zeros
// Alpha = small fraction per request
sid_collapse_mask_t mask = { M_I, M_N, len };
sid_ssp_route_from_ssp(ssp_I, ssp_U, M_I, alpha);
sid_ssp_route_from_ssp(ssp_N, ssp_U, M_N, alpha);
sid_ssp_apply_collapse_mask(ssp_U, &mask, alpha);
```

### Future: Admissibility-Shaped Collapse
Mixer computes two disjoint masks:
- `M_I(x)` - admissible → included
- `M_N(x)` - inadmissible → excluded

See `03_COLLAPSE_MASKS.md` for full formalization.

## Mixer Interface (C-API)

```c
typedef struct {
    double loop_gain;
    double admissible_volume;
    double excluded_volume;
    double undecided_volume;
    double collapse_ratio;
    double conservation_error;
    int    transport_ready;
} sid_mixer_metrics_t;

typedef struct sid_mixer sid_mixer_t;

/* Lifecycle */
sid_mixer_t* sid_mixer_create(double total_mass_C);
void         sid_mixer_destroy(sid_mixer_t*);

/* Step */
void sid_mixer_step(
    sid_mixer_t* mixer,
    const sid_ssp_t* ssp_I,
    const sid_ssp_t* ssp_N,
    sid_ssp_t* ssp_U
);

/* Collapse mediation */
void sid_mixer_request_collapse(
    sid_mixer_t* mixer,
    sid_ssp_t* ssp_I,
    sid_ssp_t* ssp_N,
    sid_ssp_t* ssp_U
);

/* Observation */
sid_mixer_metrics_t sid_mixer_metrics(const sid_mixer_t*);
```

## One-Line Contract

**The SID Mixer is a semantic arbiter that enforces ternary conservation and irreversible collapse through predicate-based adjudication, not numerical blending.**
