# Manual Section 02: Taxonomy of the Undetermined

## 2.0 Preamble

The Formal Resolution Path (FRP) identifies that not all failures in logic are equal. Where a binary system might yield a simple `false` or an error, the FRP classifies unresolvable states based on their underlying structural topology. An `UNDETERMINED` outcome is not a statement of ignorance, but a precise diagnosis of a structural flaw. This taxonomy provides the formal definitions for these diagnostic categories.

---

## 2.1 Category I: Self-Referential Loop (SRL)

- **Formal Definition:** A state where the evaluative resolution of a proposition or rule is dependent upon its own output. The proposition requires its own truth value as an input to its truth function.

- **Structural Signature:** A proposition `P` whose truth is a function of its own negation. The canonical form is a direct contradiction: `P ↔ ¬P`.

- **System Action: Refusal.** The system must identify the immediate self-reference as a non-evaluable procedural loop. It must not attempt to execute the proposition, as this would lead to infinite oscillation. The structure is treated as syntactically invalid for the purposes of resolution.

- **Terminal State Label:** `UNDETERMINED(SRL)`

- **Canonical Example:** The "Security Protocol" Scenario.
    - *Narrative:* A security AI is programmed with the rule: "If this rule is determined to be invalid, the system must immediately validate it."
    - *Analysis:* The rule's execution depends on its own state of invalidity, creating a perfect `P ↔ ¬P` loop. The system must refuse to evaluate the rule and assign the `UNDETERMINED(SRL)` state.

---

## 2.2 Category II: Ungrounded Circularity (Loop)

- **Formal Definition:** A state where two or more propositions have a mutual, circular dependency for their resolution, but no proposition in the loop has an external, independent anchor or "seed" to initiate the truth-finding cascade.

- **Structural Signature:** A dependency graph containing a cycle where no node in the cycle is a grounded axiom (e.g., `P → Q`, `Q → R`, `R → P`).

- **System Action: Suspension.** The system identifies the dependency loop and, finding no external anchor, suspends evaluation. It must not "wait" for a resolution that can never come from within the loop.

- **Terminal State Label:** `UNDETERMINED(UNGROUNDED)`

- **Canonical Example:** The "Inter-Departmental Approval" Scenario.
    - *Narrative:* Finance approves if Operations confirms; Operations confirms if Finance approves.
    - *Analysis:* A simple `P → Q` and `Q → P` loop. The system is stable but un-initialized. It must suspend and report the `UNDETERMINED(UNGROUNDED)` state, awaiting an external anchor.

---

## 2.3 Category III: Vagueness Threshold (Sorites)

- **Formal Definition:** A state where a predicate's applicability is not sharply decidable due to its reliance on an underlying continuum. As a continuous variable changes incrementally, there is no discrete point where the predicate's truth value flips from `IS` to `NOT`.

- **Structural Signature:** A predicate `P(x)` where `x` is a continuous or finely-grained discrete variable, and the rules imply that for a small change `Δ`, `P(x) → P(x-Δ)`.

- **System Action: Caution.** The system must not force a single, arbitrary threshold. Instead, it must define two provability bounds: a lower bound below which the predicate is provably `NOT`, and an upper bound above which it is provably `IS`. The region between these bounds is the penumbra of vagueness.

- **Terminal State Label:** `UNDETERMINED(VAGUE)`

- **Canonical Example:** The "Autonomous Ship Navigation" Scenario.
    - *Narrative:* A ship's hull integrity (SIU) degrades slowly from 100. "Significant damage" is not a precise point.
    - *Analysis:* The system defines a "provably healthy" bound (e.g., >95 SIU) and a "provably damaged" bound (e.g., <70 SIU). Any value between these results in a terminal state of `UNDETERMINED(VAGUE)`, triggering a specific "caution" protocol.

---

## 2.4 Category IV: Set-Theoretic Contradiction (STC)

- **Formal Definition:** A state where the very definition of an object, set, or category is inherently self-contradictory according to the rules of membership or property assignment.

- **Structural Signature:** A definition that implies a logical contradiction of the form `x ∈ S ↔ x ∉ S`. This is a special, more fundamental case of SRL that applies to definitions rather than procedures.

- **System Action: Nullification.** The system must identify the definitional contradiction and refuse to instantiate the object. The definition itself is flagged as "non-realizable." This prevents the contradiction from "exploding" and poisoning the logical integrity of the entire system.

- **Terminal State Label:** `UNDETERMINED(STC)`

- **Canonical Example:** The "Universal Registry" Scenario.
    - *Narrative:* A registry of "all lists that do not contain themselves."
    - *Analysis:* The registry's own membership criteria leads to the contradiction `Registry ∈ Registry ↔ Registry ∉ Registry`. The system must nullify the definition of the Registry itself, labeling it `UNDETERMINED(STC)`.
