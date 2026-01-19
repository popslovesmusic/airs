# 34_Primitive_Structures.md

## Tier 00: Primitive Structures

Tier 00 defines the foundational "Primitive Structures" within the Monistic Box Calculus (MBC) framework. These are the most basic building blocks, their fundamental properties, interactions, and validation mechanisms, all operating under the overarching principles of the Modal Symmetry Paradigm (MSP). This tier establishes the core schema for all Boxes and the root deviation operator from which all other deviations are expressed.

As per the `msp_tier_index_binder.json5`, Tier 00, "Primitive Structures," operates at `tier: 0` and has a fundamental dependency on `MSP_SYSTEM`.

---

### 1. Axioms of Primitive Structures (`tier_00_axiom_box.json5`)

The following axioms establish the foundational logical principles governing Primitive Structures:

*   **A00_box_schema:** Every Box must declare: `box_id`, `box_type`, `name`, `description`, `ports`, `operators`, `states`, `relations`, `invariants`. (Role: `schema`)
    *   This axiom mandates a canonical structure for all Boxes, ensuring consistency across the framework.
*   **A01_monistic_context:** All Boxes are expressions of a single underlying monistic context; Boxes do not represent independent substances. (Role: `ontology`)
    *   This axiom underscores the interconnectedness of all semantic structures, deriving from a unified potential as defined in the Semantic Substrate.
*   **A02_single_D_operator:** There is a single root deviation operator `D`; all `δ_n` are expressions of `D` performing the same action at different levels. (Role: `operator`)
    *   This establishes the fundamental nature of deviation within the system, linking all variations back to a single causal origin.
*   **A03_invariant_respect:** No operator may modify a Box in a way that violates its own declared invariants. (Role: `safety`)
    *   A critical safety axiom ensuring the structural integrity and stability of Boxes under operational transformations.

---

### 2. Interaction Rules for Primitive Structures (`tier_00_interaction_table.json5`)

These rules define the fundamental interactions that can occur involving Primitive Structures:

*   **I00_box_create:** `from: "BoxCreate"`, `to: "Box"`, `effect: "instantiate"`
    *   **Description:** Creates a new Box with a valid schema but no domain-specific content. This is the basic mechanism for bringing new semantic containers into existence.
*   **I01_box_link:** `from: "BoxLink"`, `to: "BoxPair"`, `effect: "reference"`
    *   **Description:** Establishes a non-destructive reference relation between two Boxes. This allows for the formation of networks and dependencies without altering the internal state of linked Boxes.
*   **I02_invariant_check:** `from: "InvariantCheck"`, `to: "Box"`, `effect: "validate"`
    *   **Description:** Evaluates whether a Box satisfies its invariants; returns pass/fail. Essential for maintaining the integrity of Boxes according to `A03_invariant_respect`.
*   **I03_D_expression:** `from: "D"`, `to: "delta_n"`, `effect: "expression_of_D"`
    *   **Description:** Interprets `δ_n` as an expression of the single root deviation operator `D`. Reinforces `A02_single_D_operator` by defining how specific deviations are understood in relation to the universal deviation operator.

---

### 3. Metadata for Primitive Structures (`tier_00_metadata.json5`)

This metadata provides descriptive and organizational context for Tier 00:

*   **`tier_id`:** "T00"
*   **`name`:** "MetaStructure_BoxCalculus_Core"
*   **`version`:** "0.1.0"
*   **`description`:** Defines the core Box schema (box_id, box_type, ports, operators, states, relations, invariants) and the meta-level monistic context for all other tiers.
*   **`scope`:** `["box_schema_definition", "meta_box_types", "root_operator_D", "meta_invariants"]`
*   **`dependencies`:** `[]` (implicitly depends on MSP_SYSTEM as per msp_tier_index_binder)
*   **`provides`:** `["BoxSchema", "MetaBoxTypes", "RootOperator_D", "CoreMetaAxioms"]`
    *   This section confirms the outputs and foundational elements provided by Tier 00 to the higher tiers.

---

### 4. Module Packaging for Primitive Structures (`tier_00_module_pack.json5`)

The module pack organizes the components of Tier 00 into logical units:

*   **M00_core_schema:**
    *   **Description:** Bundles the core Box schema and meta-operators.
    *   **Includes:** `tier_00_metadata.json5`, `tier_00_operator_pack.json5`, `tier_00_interaction_table.json5`, `tier_00_axiom_box.json5`, `tier_00_rewrite_system.json5`.
        *   This module demonstrates that the various JSON5 files defining Tier 00 constitute a cohesive "core schema" package.
*   **M01_base_boxes:**
    *   **Description:** References to primitive Boxes `Bx0–Bx7`.
    *   **Box References:** `Bx0`, `Bx1`, `Bx2`, `Bx3`, `Bx4`, `Bx5`, `Bx6`, `Bx7`.
        *   These represent the elementary, pre-defined Boxes that serve as the most atomic semantic units within the framework.

---

### 5. Operators for Primitive Structures (`tier_00_operator_pack.json5`)

These are the fundamental operators that can be directly applied at Tier 00:

*   **`D` (Root Deviation Operator):**
    *   **Kind:** `root_deviation`
    *   **Description:** Single fundamental deviation operator. All `δ_n` are expressions of `D` performing the same action at different levels.
    *   **Expressions:** `delta0`, `delta1`, `delta2`, `delta3`, `delta4`.
*   **`BoxCreate`:**
    *   **Kind:** `meta`
    *   **Description:** Creates a new Box instance conforming to the core Box schema.
*   **`BoxLink`:**
    *   **Kind:** `meta`
    *   **Description:** Defines a link or reference between two Box instances without modifying their internal invariants.
*   **`InvariantCheck`:**
    *   **Kind:** `meta`
    *   **Description:** Checks whether a Box satisfies all of its own declared invariants.

---

### 6. Rewrite System for Primitive Structures (`tier_00_rewrite_system.json5`)

The rewrite system defines rules for normalizing and maintaining consistency within Primitive Structures:

*   **R00_canonical_box_fields:**
    *   **Pattern:** "Box without required fields"
    *   **Rewrite To:** "Box with all required schema fields present (even if empty arrays)."
    *   **Description:** Ensures every Box matches the canonical schema, enforcing structural integrity.
*   **R01_empty_null_normalization:**
    *   **Pattern:** "Missing optional sections (ports/operators/states/relations/invariants)"
    *   **Rewrite To:** "Explicit empty arrays for missing sections."
    *   **Description:** Normalizes Box structure for easier agent parsing and consistent representation.
*   **R02_D_unification:**
    *   **Pattern:** "Multiple independent deviation operators"
    *   **Rewrite To:** "Single D with multiple `δ_n` expressions."
    *   **Description:** Enforces the single-operator view of deviation as per axiom `A02`.
*   **Normal Form Description:** A Box is in normal form when it has all required schema fields, explicit arrays for optional sections, and all deviation operators factored through `D`.

---

### 7. Test Suite for Primitive Structures (`tier_00_test_suite.json5`)

The test suite validates the correctness and adherence of Primitive Structures to their defined rules and schemas:

*   **T00_box_schema_fields:**
    *   **Description:** Verify that every known Box (`Bx0–Bx7`) has all required schema fields.
    *   **Inputs:** `Bx0`, `Bx1`, `Bx2`, `Bx3`, `Bx4`, `Bx5`, `Bx6`, `Bx7`.
    *   **Expected:** `pass_if_all_boxes_match_schema`.
*   **T01_D_single_operator:**
    *   **Description:** Verify that all deviation operators `δ0–δ4` unify under root operator `D`.
    *   **Inputs:** `tier_00_operator_pack.json5`.
    *   **Expected:** `pass_if_D_has_expressions_[delta0..delta4]`.
*   **T02_invariant_respect:**
    *   **Description:** Check that no Tier 00 rule or operator explicitly violates a Box's own invariants.
    *   **Inputs:** `tier_00_axiom_box.json5`, `tier_00_rewrite_system.json5`.
    *   **Expected:** `pass_if_no_conflicting_rules_found`.

---

**Relationship to MSP_SYSTEM and Relational Substrate:**

Tier 00, "Primitive Structures," serves as the immediate emergent layer from the undifferentiated Semantic Substrate, becoming the first structured elements available upon activation by the Tri-Unity. Its fundamental dependency on `MSP_SYSTEM` implies that the very definition and behavior of these primitive Boxes and operators are inherently governed by the modal symmetries and invariants specified by MSP. This ensures that even at the most basic level, semantic structures adhere to the overarching principles of the framework.