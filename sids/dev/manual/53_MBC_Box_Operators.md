# 53. MBC Box Operators

This document describes a set of fundamental Box Operators (`Bx_` prefix) within the Monistic Box Calculus (MBC) framework. These operators are specialized functional units designed to perform specific manipulations and transformations on semantic or modal fields within Boxes, often deriving from the "Proto-Operators" (PO) defined in `Section 48: MBC Proto-Operators`.

---

## 53.1. Bx_Amplify

*   **Operator ID:** `Bx_Amplify`
*   **Type:** `functional_operator`
*   **Proto Origin:** `PO_Amplifier`
*   **Description:** Increases semantic, modal, or structural intensity. Produces stronger gradients or distinctions.
*   **I/O Signature:**
    *   **Input:** `SemanticField`
    *   **Output:** `EnhancedField`
*   **Functional Role:**
    *   **Performs:** Scalar amplification of semantic gradients.
    *   **Supports:** `δ-deviation`, `Θ-polarity enhancement`.
    *   **Forbids:** Creation of new classes, binary evaluation.
*   **Operator Invariants:**
    *   **Monotonicity:** Amplification must preserve original ordering.
    *   **MagnitudeBound:** Amplification must remain finite.
    *   **No-Polarity-Creation:** Cannot induce new signed structures.
*   **Computational Examples:**
    ```
    Amplify(Δsmall) → Δlarge deviation seed for δ-tier processing.
    ```

---

## 53.2. Bx_Filter

*   **Operator ID:** `Bx_Filter`
*   **Type:** `functional_operator`
*   **Proto Origin:** `PO_Filter`
*   **Description:** Functionally attenuates selected semantic components according to criteria. Prepares structures for `Φ-projection`.
*   **I/O Signature:**
    *   **Input:** `SemanticField`, `FilterMask`
    *   **Output:** `AttenuatedField`
*   **Functional Role:**
    *   **Performs:** Selective attenuation of semantic content.
    *   **Supports:** `Φ-projection`, semantic isolation.
    *   **Forbids:** Class reassignment, modal deletion.
*   **Operator Invariants:**
    *   **Identity-Safe:** Filtering cannot delete semantic identity.
    *   **Attenuation-Monotonicity:** Must not amplify.
    *   **Projection-Neutrality:** Cannot perform `Φ-projective` action.
*   **Computational Examples:**
    ```
    Filter(field, mask_A) → isolated structure suitable for Φ classification.
    ```

---

## 53.3. Bx_Integrate

*   **Operator ID:** `Bx_Integrate`
*   **Type:** `functional_operator`
*   **Proto Origin:** `PO_Integrator`
*   **Description:** Performs semantic integration within a Box. Accumulates structural or modal content into smooth, continuous forms.
*   **I/O Signature:**
    *   **Input:** `SemanticField`, `ModalSeries`
    *   **Output:** `IntegratedField`
*   **Functional Role:**
    *   **Performs:** Continuous semantic accumulation.
    *   **Supports:** `δ-curvature continuity`, `Σ-contraction preparation`.
    *   **Forbids:** Truth evaluation, polarity routing.
*   **Operator Invariants:**
    *   **Continuity-Preservation:** Result must not introduce discontinuities.
    *   **Order-Invariance:** Integration must be commutative over unordered inputs.
    *   **Non-Synthetic:** May not introduce new semantic classes.
*   **Computational Examples:**
    ```
    Integrate({a,b,c}) → smooth field enabling δ curvature extraction later.
    ```

---

## 53.4. Bx_Modulate

*   **Operator ID:** `Bx_Modulate`
*   **Type:** `functional_operator`
*   **Proto Origin:** `PO_Modulator`
*   **Description:** Applies functional frequency, mode, or phase modulation to semantic fields. Enables `ψ-oscillation` effects.
*   **I/O Signature:**
    *   **Input:** `SemanticField`, `ModulationProfile`
    *   **Output:** `ModulatedField`
*   **Functional Role:**
    *   **Performs:** Frequency shaping, phase variation.
    *   **Supports:** `ψ-wave operators`, semantic resonance.
    *   **Forbids:** Truth evaluation, polarity extraction.
*   **Operator Invariants:**
    *   **Mode-Purity:** Modulated modes must derive from MSP roots.
    *   **Phase-Stability:** Must not create singularities.
    *   **Non-Evaluative:** Cannot collapse meaning.
*   **Computational Examples:**
    ```
    Modulate(field, ω) → oscillatory semantic field used by ψ-tier.
    ```

---

## 53.5. Bx_Route

*   **Operator ID:** `Bx_Route`
*   **Type:** `functional_operator`
*   **Proto Origin:** `PO_Separator`, `PO_Divider`
*   **Description:** Routes semantic streams into separate channels based on structure, modality, or invariant class. Performs controlled splitting.
*   **I/O Signature:**
    *   **Input:** `SemanticField`
    *   **Output:** `SemanticField[]` (Array of Semantic Fields)
*   **Functional Role:**
    *   **Performs:** Semantic path routing.
    *   **Supports:** `Θ-polarity paths`, `Π-evaluation pipelines`.
    *   **Forbids:** Semantic destruction, forced binarity.
*   **Operator Invariants:**
    *   **Non-Destructive:** Routing must preserve channel identity.
    *   **Partition-Coherence:** Routed segments must be internally uniform.
    *   **No-Evaluation:** May not assign truth or meaning.
*   **Computational Examples:**
    ```
    Route(field) → [left_branch, right_branch] for Π evaluation.
    ```

---

## 53.6. Bx_SSum (Semantic Summation)

*   **Operator ID:** `Bx_SSum`
*   **Type:** `functional_operator`
*   **Proto Origin:** `PO_Summer`
*   **Description:** Combines semantic or modal channels while preserving identity. Functional version of proto-superposition.
*   **I/O Signature:**
    *   **Input:** `SemanticField[]` (Array of Semantic Fields)
    *   **Output:** `CompositeField`
*   **Functional Role:**
    *   **Performs:** Identity-preserving summation.
    *   **Supports:** `ψ-envelope formation`, `Σ-aggregation`.
    *   **Forbids:** Destructive interference, semantic collapse.
*   **Operator Invariants:**
    *   **Identity-Preservation:** Inputs must remain individually recoverable.
    *   **Superposition-Purity:** No loss of semantic structure.
    *   **No-Collapse:** Cannot reduce to a single evaluative state.
*   **Computational Examples:**
    ```
    Sum([a,b,c]) → composite superposed field enabling ψ-wave formation.
    ```

---

## 53.7. Bx_Transform

*   **Operator ID:** `Bx_Transform`
*   **Type:** `functional_operator`
*   **Proto Origin:** `PO_Integrator`, `PO_Amplifier`, `PO_Summer`, `PO_Modulator`, `PO_Filter`, `PO_Separator`, `PO_Divider` (Multiple Proto-Operators)
*   **Description:** General-purpose structural transformer that performs controlled reconfiguration of semantic or modal fields according to rewrite or Box rules.
*   **I/O Signature:**
    *   **Input:** `SemanticField`, `TransformRule`
    *   **Output:** `SemanticField`
*   **Functional Role:**
    *   **Performs:** Modal reshaping, structural reconfiguration, dimensional adjustment, semantic remapping.
    *   **Supports:** Rewrite systems, cross-tier normalization, semantic evolution.
    *   **Forbids:** Arbitrary destruction, cross-tier violation.
*   **Operator Invariants:**
    *   **Rewrite-Safety:** Transform must obey tier rewrite constraints.
    *   **Semantic-Conservation:** Cannot remove semantic identity.
    *   **Dimensional-Stability:** May not increase dimension unless allowed by Tier.
*   **Computational Examples:**
    ```
    Transform(field, rule_δ→Φ) → semantic projection-ready field.
    ```