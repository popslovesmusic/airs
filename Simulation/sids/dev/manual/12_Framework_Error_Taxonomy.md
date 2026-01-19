# Manual Section 12: Framework Error Taxonomy

## 12.0 Preamble

A crucial feature of the framework is its ability to distinguish between a valid logical outcome (including `UNDETERMINED`) and a failure in the evaluation process itself. These failures are not logical resolutions; they are structural faults that occur when a layer in the stack attempts to violate the principles of a higher layer. These are surfaced as first-class, typed errors.

An error from this taxonomy indicates that the reasoning process itself is invalid and must be aborted. It is a higher-order failure than a paradoxical but classifiable `UNDETERMINED` state. Importantly, these errors (especially `E-MSC` errors) represent the framework *correctly refusing* to evaluate a malformed or out-of-scope query, not a contradiction within the framework itself.

## 12.1 MSP Error Family (E-MSP)

MSP errors are the most severe, indicating a violation of the framework's universal symmetry laws. They are triggered when the MSC or CRF attempts to perform an operation that is fundamentally disallowed by the Modal Symmetry Paradigm.

-   **`E-M1: Modal Mixing Error`**
    -   **Trigger:** An evaluation attempts to combine constraints from incompatible modal classes (e.g., probability and necessity) without a valid, MSP-defined morphism.
    -   **Effect:** The evaluation is immediately aborted.

-   **`E-M2: Symmetry Collapse Error`**
    -   **Trigger:** The CRF attempts to return a binary `IS` or `NOT` while valid, symmetry-equivalent alternatives still exist.
    -   **Effect:** The resolution is invalidated and forced to the `UNDETERMINED` state, with the error logged as a violation. This is a corrective action where the framework overrules an invalid CRF resolution.

-   **`E-M3: Negative Inference Error`**
    -   **Trigger:** The CRF returns a `NOT` state based on the absence of a supporting constraint for `IS`, rather than the presence of a provably excluding constraint.
    -   **Effect:** The evaluation is invalidated. This enforces the `Non-Forcing` invariant.

-   **`E-M4: Modal Promotion Error`**
    -   **Trigger:** A constraint from a lower modal class (e.g., a `C-heur` statistical tendency) is used to force a binary `IS` or `NOT` resolution.
    -   **Effect:** The evaluation is invalidated and downgraded to `UNDETERMINED`.

-   **`E-M5: Frame Bias Error`**
    -   **Trigger:** The framework produces a different resolution for a problem after a symmetry-preserving transformation is applied to its inputs.
    -   **Effect:** The evaluation is invalidated, and the CRF evaluator itself is flagged as biased and non-compliant.

## 12.2 MSC Error Family (E-MSC)

MSC errors are triggered when a proposed evaluation is structurally malformed, even if the underlying modal distinctions are valid under MSP. The MSC layer blocks these queries before they can engage the CRF.

-   **`E-MSC-1: Non-Comparable Outcomes`**
    -   **Trigger:** An evaluation attempts to compare or decide between outcomes that are not structurally equivalent (an "apples to oranges" comparison).
    -   **Effect:** The evaluation is rejected by MSC.

-   **`E-MSC-2: Non-Eliminative Constraint Misuse`**
    -   **Trigger:** A constraint that can only rank or apply preference (like a `C-heur` constraint) is used in a context that requires a hard, eliminative rule.
    -   **Effect:** The evaluation is rejected by MSC.

-   **`E-MSC-3: Incomplete Structural Domain`**
    -   **Trigger:** The set of constraints provided for an evaluation is structurally incomplete, preventing the CRF from being able to form a coherent dependency graph.
    -   **Effect:** The evaluation is rejected by MSC.

-   **`E-MSC-4: Mono-Structural Cohesion Violation (MSCoh)`**
    -   **Trigger:** An evaluation attempts to combine constraints from different, incompatible structural interpretation domains in a single atomic query, leading to fundamental contradictions across domains.
    -   **Effect:** The evaluation is rejected by MSC as structurally incoherent.

-   **`E-MSC-5: Contradiction Fault`**
    -   **Trigger:** Direct logical contradictions are detected within a single structural interpretation domain (e.g., `P AND NOT P`, or conflicting terms like "always" vs "never" for the same entity) during MSC's structural integrity checks.
    -   **Effect:** The evaluation is rejected by MSC as fundamentally inconsistent.

-   **`E-MSC-6: Ambiguity Fault`**
    -   **Trigger:** The level of detected ambiguity (e.g., excessive use of modal qualifiers like "might," "possibly," or vague terms like "roughly") within the problem set exceeds a predefined threshold, preventing rigorous logical evaluation.
    -   **Effect:** The evaluation is rejected by MSC due to insufficient precision for a definitive logical process.