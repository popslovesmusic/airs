# 58. Tier 01 Core Deviation Operator Pack

This document provides the detailed specification for the Tier 01 Core Deviation Operator Pack within the Monistic Box Calculus (MBC) framework. This pack defines fundamental operators crucial for establishing semantic difference, mode composition, and the embedding of mode-level semantics into Box-level configurations. It represents a foundational set of operations at one of the earliest tiers, directly contributing to the framework's Deviation Geometry.

---

## 58.1. Metadata

*   **Artifact Type:** `operator_pack`
*   **Artifact ID:** `Tier_01_Core_Deviation_Operator_Pack`
*   **Version:** `0.1.0`
*   **Tier:** `1`
*   **Status:** `DRAFT_TEST_ARTIFACT`
*   **Description:** Minimal Tier-01 operator pack capturing a core deviation operator and simple compositional structure for audit testing.
*   **Author:** `JH+LLM`
*   **Created At:** `2025-12-10`
*   **Registry Reference:** `MBC_REGISTRY::Tier_01::Operator_Packs::Core_Deviation`

---

## 58.2. Context

This operator pack operates within a foundational context, assuming the existence of basic primitives and contributing specific operators to the MBC system.

*   **Assumes Primitives:**
    *   `PRIM::SUBSTRATE`
    *   `PRIM::MODE`
    *   `PRIM::DIFFERENCE`
*   **Exports Operators:**
    *   `OP::DELTA`
    *   `OP::MERGE`
    *   `OP::BOX_EMBED`
*   **External References:**
    *   `BOX::Generic_Mode_Box`
    *   `INV::Deviation_Measure`

---

## 58.3. Operators

This pack defines three core operators that enable fundamental semantic and structural manipulations at Tier 01.

### 58.3.1. OP::DELTA (Deviation Operator)

*   **Operator ID:** `OP::DELTA`
*   **Name:** `Deviation Operator`
*   **Symbol:** `δ`
*   **Arity:** `2`
*   **Domain:** `[MODE, MODE]`
*   **Codomain:** `DIFFERENCE`
*   **Description:** Given two modes `m1` and `m2` on the same substrate, `δ(m1, m2)` returns the semantic difference relation capturing how `m2` deviates from `m1`. This is the canonical candidate for a Tier-01 deviation operator connecting MSP primitives to MBC Box-level constructions.
*   **Laws:**
    *   **Antisymmetry:** `δ(m1, m2) = -δ(m2, m1)`
    *   **Identity Element:** For identity mode `m*`, `δ(m*, m*) = 0_DIFFERENCE` (null deviation).
    *   **Triangle-like:** For modes `m1`, `m2`, `m3`: `δ(m1, m3) = δ(m1, m2) + δ(m2, m3)` when all three share a coherent substrate chart.
*   **Constraints:**
    *   **Substrate Compatibility:** `m1` and `m2` must be defined on the same substrate region; otherwise `δ` is undefined.
    *   **Domain Totality:** `δ` is only total over pairs of modes whose semantic encoding is compatible (e.g., same mode family).
    *   **Invariants Preserved:** `INV::Deviation_Measure`

### 58.3.2. OP::MERGE (Mode Merge Operator)

*   **Operator ID:** `OP::MERGE`
*   **Name:** `Mode Merge Operator`
*   **Symbol:** `⊕`
*   **Arity:** `2`
*   **Domain:** `[MODE, MODE]`
*   **Codomain:** `MODE`
*   **Description:** Given two compatible modes `m1` and `m2`, `⊕` produces a composite mode `m3 = m1 ⊕ m2` that encodes both patterns on the same substrate region. This is the basic compositional glue for building complex semantic fields from simpler modes.
*   **Laws:**
    *   **Commutative:** `m1 ⊕ m2 = m2 ⊕ m1` when modes are compatible.
    *   **Associative:** `(m1 ⊕ m2) ⊕ m3 = m1 ⊕ (m2 ⊕ m3)` within a single semantic frame.
    *   **Neutral Element:** There exists a neutral mode `m0` such that `m ⊕ m0 = m` for all `m`.
*   **Constraints:**
    *   **Compatibility:** `⊕` is only defined when `m1` and `m2` are semantically compatible (e.g., same carrier type / frame).
    *   **Non-Destructive:** Merging does not erase either constituent mode; information may overlap but must not be annihilated without explicit cancellation semantics.

### 58.3.3. OP::BOX_EMBED (Box Embedding Operator)

*   **Operator ID:** `OP::BOX_EMBED`
*   **Name:** `Box Embedding Operator`
*   **Symbol:** `□↦`
*   **Arity:** `2`
*   **Domain:** `[BOX, MODE]`
*   **Codomain:** `BOX`
*   **Description:** Given a Box `B` and a mode `m` compatible with `B`’s interface, `BOX_EMBED(B, m)` returns a new Box `B'` whose internal state includes `m` as part of its configuration. This operator is the bridge from mode-level semantics into Box-level system configuration, and is a primary target for structural and semantic audits.
*   **Laws:**
    *   **Idempotent on Same Mode:** Embedding the same mode `m` into `B` multiple times yields a configuration equivalent up to internal Box equivalence.
    *   **Functorial Hint:** Composition of embeddings corresponds to composition of Box morphisms when defined.
*   **Constraints:**
    *   **Interface Match:** `m` must satisfy `B`’s input schema/interface; otherwise embedding is undefined.
    *   **No Global Side Effects:** `BOX_EMBED` must not implicitly modify external Boxes; its action is local to `B`.

---

## 58.4. Rewrites

This pack defines specific rewrite rules that govern transformations involving its operators, ensuring consistent behavior within the MBC framework.

### 58.4.1. RW::Delta_Triangle_Expansion

*   **Rule ID:** `RW::Delta_Triangle_Expansion`
*   **Description:** Express `δ(m1, m3)` in terms of `δ(m1, m2)` and `δ(m2, m3)` when a coherent intermediate mode `m2` exists.
*   **Pattern:** `δ(m1, m3)`
*   **Rewrite To:** `δ(m1, m2) + δ(m2, m3)`
*   **Side Conditions:**
    *   `m1`, `m2`, `m3` share a compatible substrate chart.
    *   The semantic frame is stable across the triple (no frame jump).

### 58.4.2. RW::Merge_Mode_Associativity

*   **Rule ID:** `RW::Merge_Mode_Associativity`
*   **Description:** Normalize nested merges into a right-associated normal form.
*   **Pattern:** `(m1 ⊕ m2) ⊕ m3`
*   **Rewrite To:** `m1 ⊕ (m2 ⊕ m3)`
*   **Side Conditions:**
    *   All modes are mutually compatible and in the same frame.

---

## 58.5. Invariants

This pack defines specific invariants that must be preserved during the operation of its operators, guaranteeing fundamental properties within the MBC system.

### 58.5.1. INV::Deviation_Measure

*   **Invariant ID:** `INV::Deviation_Measure`
*   **Description:** There exists a deviation measure `|δ|` such that `|δ(m1, m2)| ≥ 0` and `|δ(m1, m2)| = 0` iff `m1` and `m2` are semantically identical in the given frame.
*   **Scope:** `OP::DELTA`

### 58.5.2. INV::Merge_Information_Monotonicity

*   **Invariant ID:** `INV::Merge_Information_Monotonicity`
*   **Description:** The informational content of `m1 ⊕ m2` is never strictly less than that of either constituent mode alone.
*   **Scope:** `OP::MERGE`
