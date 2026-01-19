# Manual Section 10: Formal Constraint Model

## 10.0 Preamble

For the framework to operate safely, particularly under conditions of MSP-compliant learning, not all constraints can be treated equally. The system mandates a formal partitioning of the global constraint set `C` into three distinct, architecturally defined classes. This partitioning ensures that the system's core integrity can be maintained while still allowing for dynamic adaptation.

## 10.1 The Partitioned Constraint Set

Any constraint `c` within the framework must belong to one of the following three classes:

-   **`C-core` (Core Constraints):** This is the set of sealed, non-updatable, and immutable invariants of the system. It includes the MSP Invariants (`manual/07_MSP_Invariants.md`) and the foundational axioms of the framework. `C-core` can only be changed via a formal **Revision Event**.

-   **`C-learn` (Learnable Constraints):** This is the dynamic set of constraints that the MSP-compliant learning model is permitted to evolve. These constraints are typically parametric and do not define the fundamental logic of the system. A change to this set is mediated by the `Δ` operator and governed by the L-Rules (see `manual/09_MSP_Compliant_Learning.md`).

-   **`C-heur` (Heuristic Constraints):** This is a special class of learnable constraints that can never, under any circumstances, force a binary `IS` or `NOT` resolution. They represent non-eliminative information, such as statistical tendencies, preferences, or confidence weightings. `C-heur` constraints may be used by the system to rank or select from among multiple, symmetry-equivalent possibilities, but they are forbidden by `M4 - The Invariant of Modal Non-Promotion` from collapsing a protected `UNDETERMINED` state.

## 10.2 The Constraint Schema

To enforce this partitioning, every learnable constraint `c` in `C-learn` or `C-heur` must conform to a formal schema. This allows the system to understand the constraint's role and to control what aspects of it may evolve.

A constraint is a tuple: `c = ⟨type, domain, modal, predicate, θ, κ⟩`

-   **type:** The constraint's classification (e.g., `Calibration`, `Boundary`, `Preference`). This determines its function.
-   **domain:** The set of propositions or states to which the constraint applies.
-   **modal:** The constraint's modal class (e.g., `necessity`, `probability`), which is checked for compatibility by MSC.
-   **predicate:** The frozen, non-learnable logical or mathematical form of the constraint.
-   **θ (theta):** The set of learnable parameters for the predicate (e.g., numerical thresholds, weights).
-   **κ (kappa):** A non-authoritative metadata field representing the confidence, strength, or source of the constraint.

This schema ensures that "learning" is a highly disciplined process. The fundamental logic (`predicate`) of a constraint cannot be changed by the learning model; only its parameters (`θ`) can be tuned.
