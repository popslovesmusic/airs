# Manual Section 03: The Causal Sphere of Influence (CSI)

## 3.0 Preamble

The Constraint Resolution Framework (CRF) does not operate in a vacuum. Before it can evaluate the structural validity of a proposition, it must first understand the boundaries of what is possible. It must have a map of the causal terrain. The Causal Sphere of Influence (CSI) provides this map, operationalizing the **Axiom of the Causal Boundary**.

## 3.1 Formal Definition of CSI

The Causal Sphere of Influence is the complete set of all entities, states, or operations that can exert a causal effect upon, or be affected by, a given subject entity. This influence is defined by **structural accessibility**, not by temporal or spatial proximity.

If an entity `B` is within the CSI of entity `A`, it means a pathway for influence exists. This does not specify the nature, speed, or timing of the influence, only its structural possibility. Conversely, if `B` is outside the CSI of `A`, no causal interaction is possible, regardless of other factors.

## 3.2 Structural vs. Temporal Boundaries: A Fundamental Distinction

It is critical to distinguish the CSI from concepts that define boundaries based on temporal propagation.

-   **Temporal Boundaries** define limits of causation based on a sequence of events over time. They answer the question, "What can be affected, given the flow of time and the rate at which influence propagates?"

-   The **CSI** defines the boundaries of causation based on **structural accessibility**. It answers the question, "What can be affected, given the architecture of the system?" It is a concept rooted in logical and systemic topology.

For example, a variable inside a computationally sealed-off function in a computer program is outside the CSI of a variable in another function, even if they are processed nanoseconds apart. Their separation is structural, not temporal.

## 3.3 The Two-Step Resolution Mandate

All logical evaluations within this framework must follow a strict, two-step sequence. This is a non-negotiable architectural rule.

1.  **Step 1: CSI Mapping.** Before evaluating a proposition `P`, the system must first map its CSI. This involves identifying all other propositions, constraints, and domains that have structural accessibility to `P`. This step asks: "What is the complete universe of relevant influences?"

2.  **Step 2: CRF Evaluation.** Only after the CSI has been fully mapped and its boundaries established can the Constraint Resolution Framework (CRF) be applied. The CRF then audits the logical pathways *within* this defined sphere for structural flaws (SRL, Ungrounded Loops, etc.). This step asks: "Within this bounded universe, is resolution structurally lawful?"

A proposition cannot be evaluated until its causal neighborhood has been defined. To do otherwise is to risk contamination from irrelevant contexts or to miss dependencies that render the proposition unresolvable.

## 3.4 CSI as a Diagnostic Prerequisite for CRF

The CSI provides the essential context for the CRF to correctly diagnose `UNDETERMINED` states. The nature of a logical flaw is often revealed by the shape of the causal fields involved.

-   **UNDETERMINED(UNGROUNDED):** CSI mapping reveals a closed loop of dependencies with no causal anchor from outside the loop's own sphere.

-   **UNDETERMINED(SRL) / (STC):** CSI mapping reveals that an entity's causal sphere problematically intersects with itself, such that its state is a function of its own state. It is a collision within a single causal field.

-   **UNDETERMINED(VAGUE):** CSI mapping reveals that the entity's causal sphere does not have a crisp boundary, but instead overlaps with the sphere of its negation in a gradual penumbra.

In essence, the CSI provides the map of the territory, and the CRF is the set of rules for navigating that map. Without the map, navigation is impossible.

---
*For a more detailed, formal treatment of the mechanics of influence propagation, including the concepts of a semantic speed limit, meaning cones, and semantic horizons, see `manual/20_Semantic_Causality_and_Horizons.md`.*