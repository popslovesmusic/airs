# SID Semantic Processor - Architecture Overview

## System Hierarchy

```
AIRS (AI Research Suite)
  └─ Governance / Ethics / Meta-research
  └─ DASE (Dynamic Autonomous Simulation Environment)
      └─ Scenario management / Parameter sweeps / CLI (JSON-driven)
      └─ DSE (Dynamic Simulation Engine)
          └─ Orchestration / Scheduling / Persistence
          └─ SID (Semantic Interaction Diagrams)
              ├─ SSP(I) - Admitted Processor
              ├─ SSP(N) - Excluded Processor
              ├─ SSP(U) - Undecided Processor
              └─ Mixer - Semantic Arbiter
```

## Key Architectural Decisions

### SID is NOT the home of AIRS or DASE
- SID lives *inside* DSE
- DSE lives inside DASE
- DASE lives inside AIRS

### Current Goal
Prototype a **SID ternary simulation** using **three coupled engines** (one per processor):
- **SSP(I)** - admitted / stabilized
- **SSP(N)** - excluded / forbidden
- **SSP(U)** - undecided / reservoir

## Core Concepts

### Ternary State Model
- **0 (∅)** - Potential/Null/Undecided
- **+1 (S+)** - Positive Assertion/Affirmed
- **-1 (S-)** - Negative Assertion/Rejected

### Conservation Law
```
I(t) + N(t) + U(t) = C    ∀t
```
Where C is total semantic capacity (constant).

### Semantic State Processor (SSP)
**Not** a "drive" (storage media) - it is a **ternary semantic processing unit** implemented on digital hardware.

A role-locked processing unit that:
- Hosts exactly one engine
- Represents exactly one ternary role (I, N, or U)
- Exposes fields + metrics
- Accepts writes only from its local engine
- Accepts semantic intervention only via the SID Mixer

### SID Mixer Responsibilities
- Read engine buffers + metrics
- Enforce ternary invariants
- Apply admissibility filters
- Perform irreversible collapse
- Emit SID metrics:
  - `loop_gain`
  - `admissible_volume`
  - `collapse_ratio`
  - `conservation_error`
- Signal **Transport (T)** when stability predicate is met

**Mixer is semantic coupling, not numerical blending.**

## Architectural Invariants (Hard Rules)

### Role Lock
```
∀t: SSP(x).role = constant
```
Processors cannot change roles.

### Single Writer
```
Only local engine mutates SSP field
```

### No Cross-Visibility
```
SSP(x) cannot observe SSP(y) where x ≠ y
```

### Irreversibility
```
U → I (allowed)
U → N (allowed)
I → U (forbidden)
I ↔ N (forbidden)
N → U (forbidden)
```

### Conservation
```
I(t) + N(t) + U(t) = C
```

## Implementation Direction

- Use **3 C++ engines = 3 SSPs**
- Prefer **2D** engines initially
- Treat engines as black boxes
- Mixer sits above `*_capi.*` interfaces
- No engine-to-engine coupling without mixer mediation

## Ternary Model

**Soft ternary** (mass-conserving fields) preferred:
- I + N + U = constant
- Collapse = irreversible transfer from U

Hard ternary can be explored later.

## Engine Creation

Engines are **created by DASE CLI** (JSON-driven), not by SID directly:
- SID only attaches to existing engines
- SID never calls engine constructors
- DASE issues handles and shared buffers

## Transport Operator

Controlled phase transitions triggered when stability predicates are satisfied:

```
Transport_Ready ⟺ (
    loop_gain_converged ∧
    admissible_volume_threshold_met ∧
    collapse_ratio_monotonic ∧
    no_invariant_violations
)
```

Only when all conditions hold does the system emit a `T` signal.

## One-Line Contract

**SID is the irreversible semantic coupler that converts undecided dynamics into admissible or excluded structure under conserved ternary law.**
