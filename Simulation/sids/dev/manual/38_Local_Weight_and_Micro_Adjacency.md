# 38_Local_Weight_and_Micro_Adjacency.md

## Tier 04: Local Weight and Micro-Adjacency

Tier 04, "Local Weight and Micro-Adjacency," is primarily concerned with the formal definition and application of the `Φ` (Phi) projection operator family within the Monistic Box Calculus (MBC) framework. This tier details how semantic patterns are mapped, interpreted, and transported across different modal expressions and structures without altering their fundamental substance. It emphasizes the crucial role of `Φ` operators in changing interpretive frames while strictly preserving underlying invariants and ensuring consistent interaction with deviation (`δ`) operators.

As per the `msp_tier_index_binder.json5`, Tier 04 operates at `tier: 4` and depends on `MSP_SYSTEM` and `TIER_00_PRIMITIVES`. Due to its use of `δ` operators and interaction with semantic projection, it implicitly builds upon `TIER_01_DELTA_GEOMETRY`, `TIER_02_PHI_PROJECTION`, and `TIER_03_PI_EVALUATION`.

---

### 1. Axioms of Local Weight and Micro-Adjacency (`tier_04_axiom_box.json5`)

The following axioms establish the foundational principles governing `Φ` projection and its interaction within the framework:

*   **A40_semantic_non_destructive:** `Φ` mappings must not destroy or create substances; they change interpretation only. (Role: `safety`)
    *   This axiom ensures that `Φ` operators are purely transformational on the level of interpretation or frame, preserving the ontological integrity of the underlying semantic "substances" as defined by the monistic context.
*   **A41_projection_commutes_with_D:** `Φ` must commute with deviation: `Φ ∘ δn = δn ∘ Φ`. (Role: `operator_consistency`)
    *   This is a critical consistency axiom, guaranteeing that the order of applying a semantic projection and a deviation operator does not affect the final outcome. This ensures predictable behavior across different levels of abstraction and transformation.
*   **A42_phi_identity:** `Φ0` acts as the identity mapping: `Φ0(X) = X`. (Role: `identity`)
    *   `Φ0` serves as a baseline or neutral projection, leaving the Box unchanged. It is a necessary component for normalization and a reference point for more complex projections.
*   **A43_phi_preserves_invariants:** Semantic projection must preserve invariants declared by the target Box. (Role: `invariant_preservation`)
    *   This axiom safeguards the inherent properties and constraints of Boxes, ensuring that `Φ` transformations respect their fundamental structural integrity.

---

### 2. Interaction Rules for Local Weight and Micro-Adjacency (`tier_04_interaction_table.json5`)

These rules define the specific interactions and effects of `Φ` operators in semantic projection:

*   **I40_phi0_identity:** `operator: "phi0"`, `effect: "return_same_box"`
    *   **Description:** `Φ0` returns the Box unchanged; used as a normalization base case. It is the fundamental "do nothing" projection.
*   **I41_phi_structure:** `operator: "phi_structure"`, `effect: "map_pattern"`
    *   **Description:** Maps structural patterns but leaves modal identities unaltered. This operator allows for the re-interpretation or re-contextualization of a Box's internal structure without changing its fundamental IGS/OA nature.
*   **I42_phi_projection_map:** `operator: "phi_projection_map"`, `effect: "semantic_translation"`
    *   **Description:** Translates a Box into a new semantic frame while preserving invariants. This is the general mechanism for adapting a semantic structure to different interpretive contexts.

---

### 3. Metadata for Local Weight and Micro-Adjacency (`tier_04_metadata.json5`)

This metadata provides descriptive and organizational context for Tier 04:

*   **`tier_id`:** "T04"
*   **`name`:** "PhiProjection"
*   **`version`:** "0.1.0"
*   **`description`:** Defines the `Φ` projection operator family for semantic mapping, transporting patterns across modalities, deviations, and structures. `Φ` does not alter underlying substances but changes interpretive frames.
*   **`scope`:** `["phi_mapping", "semantic_projection", "cross_modal_translation", "structure_preservation"]`
*   **`dependencies`:** `["T00", "T01", "T02", "T03"]`
    *   Explicitly states the dependencies on Primitive Structures, Deviation Geometry, Semantic Projection, and Truth and Causality Evaluation, highlighting that `Φ` operations are built upon and interact with these prior foundational tiers.
*   **`provides`:** `["phi0", "phi_structure", "phi_projection_map"]`
    *   Confirms the specific `Φ` operators made available by Tier 04.

---

### 4. Module Packaging for Local Weight and Micro-Adjacency (`tier_04_module_pack.json5`)

The module pack organizes the components of Tier 04 into a logical unit:

*   **M40_phi_projection_core:**
    *   **Description:** Core semantic projection machinery.
    *   **Includes:** `tier_04_metadata.json5`, `tier_04_operator_pack.json5`, `tier_04_interaction_table.json5`, `tier_04_axiom_box.json5`, `tier_04_rewrite_system.json5`.
        *   This module consolidates all the core definitions for Local Weight and Micro-Adjacency, emphasizing their collective function in defining `Φ` projection.

---

### 5. Operators for Local Weight and Micro-Adjacency (`tier_04_operator_pack.json5`)

These are the specific `Φ` projection operators central to Tier 04:

*   **`Phi`:**
    *   **Kind:** `projection_root`
    *   **Description:** The root projection operator enabling semantic mapping between Box structures.
*   **`phi0`:**
    *   **Kind:** `projection`, `level: 0`
    *   **Description:** Identity-like baseline mapping preserving exact structure: `Φ0(X) = X`.
*   **`phi_structure`:**
    *   **Kind:** `projection`, `level: 1`
    *   **Description:** Maps structural patterns without transporting modal identity.
*   **`phi_projection_map`:**
    *   **Kind:** `projection`, `level: 2`
    *   **Description:** General semantic mapping between Boxes; alters descriptive frame but not underlying substance.

---

### 6. Rewrite System for Local Weight and Micro-Adjacency (`tier_04_rewrite_system.json5`)

The rewrite system defines rules for normalizing and maintaining consistency in `Φ` projections:

*   **R40_phi_chain_normalization:**
    *   **Pattern:** "`phi_projection_map(phi_projection_map(X))`"
    *   **Rewrite To:** "`phi_projection_map(X)`"
    *   **Description:** Collapses chained projection mappings into a single projection, optimizing the representation.
*   **R41_phi_delta_commutation:**
    *   **Pattern:** "`phi_projection_map(delta_n(X))`"
    *   **Rewrite To:** "`delta_n(phi_projection_map(X))`"
    *   **Description:** Enforces the commuting axiom between `Φ` and `δn`, ensuring consistent order of operations.
*   **R42_phi_identity_elimination:**
    *   **Pattern:** "`phi0(X)`"
    *   **Rewrite To:** "`X`"
    *   **Description:** Eliminates unnecessary `Φ0` calls, simplifying expressions where the identity mapping is applied.
*   **Normal Form Description:** A Box is in `Φ-normal` form when projection has at most one `Φ-operator` applied, `Φ0` is eliminated, and `Φ` commutes with deviation operators.

---

### 7. Test Suite for Local Weight and Micro-Adjacency (`tier_04_test_suite.json5`)

The test suite validates the correctness and adherence of `Φ` projections to their defined rules and properties:

*   **T40_phi_identity_test:**
    *   **Description:** Verify `Φ0(X) = X`.
    *   **Inputs:** `phi0`, `Bx4`.
    *   **Expected:** `pass_if_output_equals_input`.
*   **T41_phi_delta_commute_test:**
    *   **Description:** Verify `Φ` commutes with all `δn`.
    *   **Inputs:** `phi_projection_map`, `delta0`, `delta1`, `delta2`, `delta3`, `delta4`, `delta5`.
    *   **Expected:** `pass_if_phi_delta_commute`.
*   **T42_phi_invariant_preservation:**
    *   **Description:** Ensure `Φ` mappings do not violate Box invariants.
    *   **Inputs:** `phi_projection_map`, `Bx3`.
    *   **Expected:** `pass_if_invariants_preserved`.

---

**Relationship to MSP_SYSTEM, Relational Substrate, Primitive Structures, Deviation Geometry, and Semantic Projection:**

Tier 04, "Local Weight and Micro-Adjacency," marks a pivotal point where the framework gains the ability to interpret and translate semantic patterns across various contexts and structures. Building upon the foundational `MSP_SYSTEM` invariants, the underlying Semantic Substrate, Primitive Structures (Tier 00), Deviation Geometry (Tier 01), and Semantic Projection (Tier 02), this tier introduces the crucial `Φ` operator family. These operators allow for flexible manipulation of semantic interpretations without compromising the monistic nature of the substrate or the integrity of Box invariants. It enables the system to handle the nuanced weighting and adjacency of semantic elements in a context-sensitive manner, essential for higher-level cognitive and evaluative processes.