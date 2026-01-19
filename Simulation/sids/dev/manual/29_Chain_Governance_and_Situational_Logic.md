# Manual Section 29: Chain Governance and Situational Logic

## 29.0 Preamble

This section defines the **Chain Governance Layer**, the meta-system that governs the lawful composition of the framework's core operators (`CSI`, `MSP`, `MSC`, `MBC`, `CRF`). It moves the framework from a single, fixed pipeline to a dynamic system where operator chains are constructed based on situational intent.

The core principle is that the chain order is not arbitrary. It is selected from a finite set of formally defined **Lawful Chains**, each corresponding to a unique **Situation Type**. This prevents logical corruption, eliminates redundancy, and forbids the unconstrained optionality that leads to a "zero-state" of meaning.

## 29.1 The Shift from a Static Pipeline to Dynamic Selection

The framework is not a single, static pipeline but a set of operators that can be composed into "chains."

-   **Operators:** `CSI`, `MSP`, `MSC`, `MBC`, `CRF` are the fundamental components of analysis.
-   **Chain:** A sequence of these operators, e.g., `CSI → MSP → MSC`.
-   **Situation:** A structural intent for an analysis (e.g., "mapping reachability" vs. "producing a verdict"). Critically, a situation is defined by the structural operation being performed, not by the specific content or "topic" of the analysis.

The system does not ask, "Is this chain correct?" It asks, "Given the situation, what is the one lawful chain?"

## 29.2 Chain Governance Rules

The construction of any lawful chain is governed by a minimal, necessary set of rules that preserve the framework's integrity.

1.  **Rule 1 — Non-Creation of Reachability:** No operator may introduce new causal reachability. Only `CSI` can define the initial boundaries of influence. Other operators may restrict this field but never expand it.
2.  **Rule 2 — Modal Non-Inversion:** No operator may invalidate a symmetry decision made by an earlier `MSP` application within the same chain.
3.  **Rule 3 — Admissibility Precedes Resolution:** `CRF` may only be applied after `MSC` has been successfully applied to the same structural view. This prevents illicit "shortcut" evaluations.
4.  **Rule 4 — Boundary Operations Are Non-Decisive:** `MBC` shapes and carves constraint space but is not a terminal operator. Any chain ending in `MBC` is incomplete.
5.  **Rule 5 — Resolution Is Terminal:** Once `CRF` produces a verdict (`IS`, `NOT`, `UNDETERMINED`), no upstream operator (`MSP`, `MSC`) may be reapplied to that resolved instance without initiating a formal revision, which constitutes a new situation.
6.  **Rule 6 — Anti-Drift (No Repetition):** Operators may not repeat within a single, unbroken evaluation chain. This prevents ungrounded loops, ensures finite termination, and reinforces the "one placement, one chain" principle by disallowing the unconstrained multiplicity that leads to a "zero-state."

## 29.2.1 Why Chains Become Unique and Finite (One Placement, One Chain)

Under these governance rules, especially the anti-drift principle (Rule 6) and the explicit precedence constraints inherent in the framework's architecture, the number of lawful chains is not merely finite but collapses to a single, canonical form for each specific situational intent. This uniqueness is enforced by:

-   **No-Repeat Rule:** Operators cannot repeat within a single chain, preventing infinite permutations and ensuring a finite length.
-   **Strict Precedence Constraints:**
    -   Reachability must be established before any modal or admissibility action (`CSI` always precedes `MSP`, `MSC`, `MBC`, `CRF` in a chain for a given structural view).
    -   Symmetry law must be fixed before admissibility screening (`MSP` before `MSC`).
    -   Admissibility must precede execution (`MSC` before `CRF`).
    -   Boundary-shaping (`MBC`) may occur only after admissibility and before resolution (between `MSC` and `CRF`).
-   **Terminal Rule:** `CRF` is the terminal operator for a resolution instance, meaning no upstream operator may follow it within the same chain.

These constraints ensure that for any given task or "structural intent," there is exactly one lawful, normal-form chain. Any apparent "dynamic ordering" is thus reinterpreted as dynamic selection among these finite, canonical chains, not arbitrary permutation.


## 29.3 The Canonical Situation Set

These rules give rise to a minimal, finite set of lawful situations. The estimated set is 5 core types, with 2 optional extensions for post-resolution analysis. Any analytical task must be classified into one of these situations, which in turn determines the single lawful chain to be used.

### The 5 Core Situation Types

-   **S1 — Reachability Mapping**
    -   **Intent:** Determine structural accessibility and influence boundaries.
    -   **Chain:** `CSI`
    -   **Output:** CSI map / heatmap.
    -   **Forbidden:** Any output regarding modality, admissibility, or truth.

-   **S2 — Modal Symmetry Determination**
    -   **Intent:** Determine allowed distinctions and symmetry classes.
    -   **Chain:** `CSI → MSP`
    -   **Output:** Symmetry classes, equivalence sets, non-forcing constraints.
    -   **Forbidden:** Admissibility verdicts or final resolutions.

-   **S3 — Admissibility Screening**
    -   **Intent:** Determine if a question or evaluation is lawful to attempt.
    -   **Chain:** `CSI → MSP → MSC`
    -   **Output:** An `admissible` / `inadmissible` classification.
    -   **Forbidden:** Final `I/N/U` resolution.

-   **S4 — Boundary / Constraint Shaping (Non-terminal)**
    -   **Intent:** Prepare, shape, and bound the constraint space for a potential resolution.
    -   **Chain:** `CSI → MSP → MSC → MBC`
    -   **Output:** A bounded, shaped constraint configuration ready for resolution.
    -   **Forbidden:** A final `I/N/U` verdict. *(Note: This situation is only valid if non-terminal shaping work is an authorized workflow.)*

-   **S5 — Terminal Resolution**
    -   **Intent:** Produce a stable, final evaluative verdict for a given instance.
    -   **Chain:** `CSI → MSP → MSC → MBC → CRF`
    -   **Output:** `IS`, `NOT`, or a typed `UNDETERMINED` state.
    -   **Forbidden:** Any post-hoc revision of symmetry or admissibility within the same instance.

### Optional Extensions

-   **S6 — Post-Resolution Impact Analysis**
    -   **Intent:** Analyze the consequences of a settled `CRF` verdict.
    -   **Chain:** `CRF → CSI`
    -   **Note:** This is an analytical, not a re-evaluative, chain.

-   **S7 — Revision / Re-entry Under New Scope**
    -   **Intent:** Formally re-evaluate a topic under an explicitly altered scope.
    -   **Chain:** `CSI → MSP → MSC → ...`
    -   **Note:** This is governed as a new, distinct instance, not a continuation of a previous chain.

## 29.4 Conclusion: Governance Over Ambiguity

By formalizing a finite set of lawful situations and their corresponding operator chains, the framework prevents the combinatorial explosion of arbitrary permutations. This Chain Governance Layer ensures that while the system is dynamic in its selection of tools, it is rigorously deterministic and non-redundant in their application. It is the architectural guarantee that "dynamic" does not mean "unconstrained."
