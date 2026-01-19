# SID Semantic State Processor (SSP) - Specification v0.1

## Definition

A **SID Semantic Processor (SSP)** is a role-locked ternary processing unit that hosts one dynamical engine and exposes its evolving state as a semantic field, suitable for irreversible ternary adjudication by the SID Mixer.

### Key Correction from Early Naming

**Not** a "drive" (implies storage/media).
**Is** a semantic processing abstraction that appears persistent because semantics require continuity.

## SSP Invariants (Hard Constraints)

### Role Lock
```
ssp.role ∈ {I, N, U} and immutable after creation
```

### Single Engine Residency
```
Exactly one engine instance per SSP
```

### Semantic Isolation
```
ssp_X cannot observe ssp_Y
```

### Write Discipline
```
Only local engine mutates semantic field
```

### Capacity (Semantic, not physical)
```
∫ semantic_field ≤ capacity
```
Capacity represents semantic load, not memory usage.

## SSP ↔ Engine Contract

### Engine Responsibilities
**Produce:**
- Semantic field
- Internal stability metrics

**Respect:**
- Write bounds
- Timestep discipline

### SSP Responsibilities
**Provide:**
- Stable semantic field
- Temporal continuity

**Prevent:**
- External mutation
- Cross-SSP reads

### Relationship
- **Engine** = dynamics
- **SSP** = semantic carrier

## SSP ↔ SID Mixer Contract

### Mixer May:
**Read:**
- Semantic fields (I, N, U)
- Stability metrics

**Decide:**
- Admissibility
- Exclusion
- Collapse amount

### Mixer May Not:
- Write fields directly
- Access engine internals
- Bypass SSP mediation

### Collapse Rule
Collapse is executed by **SSP(U) only**:
- Mixer issues a collapse directive
- SSP applies it atomically

This preserves irreversibility and locality.

## SSP Metrics

Each SSP computes per-field metrics on each commit:

### Stability
```
stability = 1 - clamp(sum(field)/capacity)
```
Semantic load headroom (1 = plenty of headroom, 0 = saturated)

### Coherence
```
coherence = 1 / (1 + variance)
```
Field uniformity (higher when uniform)

### Divergence
```
divergence = mean |field[i] - field[i-1]|
```
Roughness / flux proxy

## JSON5 Archive Stub

```json5
{
  id: "SID_SSP_SPEC_v0.1",
  name: "Semantic State Processor",
  role: "ternary_semantic_processing_unit",
  ternary_role: ["I","N","U"],
  invariants: [
    "role_locked",
    "single_engine_residency",
    "semantic_isolation",
    "engine_only_writes",
    "bounded_semantic_capacity"
  ],
  responsibilities: {
    host_engine: true,
    maintain_semantic_field: true,
    apply_irreversible_collapse: "U_only"
  },
  forbidden: [
    "cross_processor_visibility",
    "admissibility_decisions",
    "direct_field_writes_by_mixer",
    "transport_signaling"
  ],
  interfaces: {
    engine: ["field_mutation","metric_emission"],
    mixer: ["field_observation","collapse_directive"]
  }
}
```

## One-Line Definition

**A SID Semantic Processor is a bounded, role-locked state substrate that preserves engine dynamics while enforcing semantic isolation.**
