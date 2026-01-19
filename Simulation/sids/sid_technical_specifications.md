# Semantic Interaction Diagrams (SIDs) - Technical Specifications

## 0. Purpose and Scope
The Semantic Interaction Diagrams (SIDs) framework is a diagrammatic, constraint-governed, and non-teleological system designed to model agency, meaning, and goal emergence. It assumes no pre-defined goals, optimization, time, psychology, or symbolic semantics. Agency and goals arise as structural consequences, not as inputs. The framework is intended to be:
*   Structurally minimal
*   Procedurally governed
*   Diagrammatically computable
*   Compatible with mathematics without being defined by it

## 1. Structural Foundations (Pre-Math)

### 1.1 Degrees of Freedom (DOF)
*   **Assumption:** Distinctions exist, constraints exist, change is possible.
*   **Definition:** Degrees of Freedom (DOF) represent the dimensions of possibility within which an entity can operate.
*   **Types:** DOFs can be independent (orthogonal) or coupled.

### 1.2 Orthogonality of Causation
*   **Definition:** Two fundamental, orthogonal causal domains are distinguished:
    *   **Horizontal Causation:** Diachronic, temporal succession (sequence of events).
    *   **Vertical Causation:** Synchronic, hierarchical dependence (structure at a given moment).
*   **Constraint:** Mixing these domains produces category errors. Neither is primary.

### 1.3 Possibility Space
*   **Definition:** Orthogonal DOFs span a possibility space.
*   **Characteristics:** This space is not metric, probabilistic, or goal-oriented. It represents the domain of what is admissible.

## 2. CSI -- Causal Sphere of Influence

### 2.1 Definition
*   **CSI:** Defines the boundary of admissible interaction at a given scale or context.
*   **Rules:**
    *   Inside CSI -> interaction is possible.
    *   Outside CSI -> interaction is forbidden or irrelevant.
*   **Properties:** CSI is not temporal, not spatial, and not dynamic by default.

### 2.2 Role
*   CSI functions as a hard gate:
    *   Constrains which operators may interact.
    *   Limits which diagrams may be drawn.
    *   Bounds resolution scope.
*   **Precedence:** CSI precedes diagrams and CRF.

## 3. I / N / U Logic

### 3.1 Definition
*   **Type:** A classification logic, not an inferential one.
*   **States:**
    *   **I (Is):** Admissible, coherent, affirmed.
    *   **N (Not):** Excluded, forbidden, contradictory.
    *   **U (Unknown):** Unresolved, undecidable, open.

### 3.2 Role
*   I/N/U labels the epistemic status of constraints, relations, or diagram elements at the current stage.
*   **Limitations:** It does not infer truth, resolve conflict, or choose outcomes.

### 3.3 Placement
*   I/N/U sits between diagrams and CRF as a typing layer.

## 4. Primitive Operators

*   **Function:** Operators act on semantic space but do not encode meaning themselves.
*   **Composition:** Operators are composable and non-teleological.

### 4.1 Minimal Operator Set
*   **Projection (P):** Isolate a DOF.
*   **Polarity (S+ / S-):** Stance / affirmation / negation.
*   **Collapse (O):** Irreversible commitment / loss of freedom.
*   **Coupling (C):** Relational constraint between DOFs.
*   **Transport (T):** Movement across compartments.

## 5. Semantic Interaction Diagrams (SIDs)

### 5.1 Definition
*   **SID:** A composed expression of operators representing local interactions in semantic space.
*   **Characteristics:** Diagrams are calculational, not illustrative; local, not global; and rewriteable.

### 5.2 Core Loop
*   The fundamental SID pattern is:
    `Freedom -> Choice (Collapse) -> Gradient -> Freedom`
*   This loop underlies agency, deliberation, and stabilization.

### 5.3 Compartments
*   **Operation:** Diagrams operate within logarithmic compartments.
*   **Properties:**
    *   Each compartment compresses history.
    *   Transitions are irreversible.
    *   Memory is structural, not stored.
*   **Result:** Yields non-Markovian behavior without memory variables.

## 6. Rewrite and Equivalence Rules

### 6.1 Rewrite Rules
*   **Specification:** Rewrite rules specify how diagrams may be transformed without changing structural effect.
*   **Authorization:** All rewrites must be authorized by CRF and must respect CSI boundaries.

### 6.2 Equivalence
*   **Condition:** Two diagrams are equivalent if and only if:
    *   They preserve admissibility.
    *   They preserve collapse irreversibility.
    *   They yield the same post-compartment constraints.
*   **Scope:** Equivalence is conditional, not universal.

## 7. CRF -- Constraint Resolution Framework

### 7.1 Nature
*   **Definition:** CRF is a procedural language, not an operator, not a metric, and not part of semantic space.

### 7.2 Role
*   CRF governs:
    *   Admissibility of diagrams.
    *   Authorization of rewrites.
    *   Timing and legality of collapse handling of unresolved conflict.
*   **Limitation:** CRF never chooses outcomes.

### 7.3 Mechanics
*   **Operation:** CRF operates via:
    *   Constraint typing (hard/soft, local/global).
    *   Admissibility checks.
    *   Ordered resolution procedures (attenuate, defer, partition, escalate, bifurcate, halt).
    *   Explicit failure semantics.
*   **Utility:** CRF enables human discourse using familiar terms without semantic smuggling.

## 8. Metrics (Earned, Not Assumed)

*   **Timing:** Metrics may be extracted only after structure stabilizes.
*   **Examples:** Admissible volume, collapse ratio, gradient coherence, transport fidelity, loop gain.
*   **Status:** Metrics are downstream and optional.

## 9. Structural Stability / Termination

*   **Definition:** A semantic process is structurally stable when:
    *   No admissible rewrites remain, OR
    *   Admissible region is invariant under transport, OR
    *   CRF authorizes only identity rewrites, OR
    *   Loop gain converges within tolerance.
*   **Outcome:** At stability, a goal has emerged as a fixed attractor without being defined.

## 10. Minimal Core (Irreducible)

*   The system cannot function without:
    *   Orthogonal DOFs
    *   CSI admissibility boundary
    *   I/N/U epistemic typing
    *   Operators {P, S, O, C, T}
    *   SIDs with rewrite rules
    *   CRF procedural governance

## 11. Interpretation (Last)

*   **Timing:** Only after stability may the system be interpreted as: agency, goals, cognition, policy.
*   **Authority:** Interpretation has no upstream authority.

## 12. Implementation Targets (For Code Readiness)

This section makes the spec implementation-ready by introducing explicit data boundaries, structure, and the minimum code-facing vocabulary. It does not change the semantics above.

### 12.1 Core Data Types (Minimal)
*   **DOF:** Identifier, orthogonal group, optional metadata.
*   **CSI:** Set of admissible DOF pairs or scopes.
*   **INU:** Enum {I, N, U} for typing constraints/elements.
*   **Operator:** Enum {P, S_PLUS, S_MINUS, O, C, T}.
*   **Compartment:** Integer index or opaque id.
*   **Diagram:** Graph or expression tree of operator nodes and edges.
*   **Constraint:** {id, type, scope, predicate, action}.
*   **RewriteRule:** {id, pattern, replacement, preconditions}.
*   **Metric:** {id, compute_fn, dependencies}.

### 12.2 Required Checks (Minimum)
*   **CSI boundary check:** all operator interactions must be inside CSI.
*   **Typing check:** assign I/N/U to nodes and edges.
*   **Rewrite authorization:** CRF gate on any rewrite.
*   **Collapse irreversibility:** any O operation must be marked irreversible and survives compartment transitions.

## 13. Formal Diagram Grammar (Proposed Minimal Syntax)

Note: A formal grammar is not provided in source documents. This grammar is a minimal proposal for coding and can be revised without changing semantics.

```
<expr>        ::= <op> | <op> "(" <expr-list> ")"
<expr-list>   ::= <expr> | <expr> "," <expr-list>
<op>          ::= "P" | "S+" | "S-" | "O" | "C" | "T"
```

## 14. Canonical Execution Trace (Demonstrative)

This is a single, minimal execution trace. It is not a benchmark, not empirical, and not domain-specific.

**Purpose:** Show exactly how SIDs, CSI, I/N/U, and CRF interact end-to-end.

**Initial Conditions**
*   **Semantic Space:** Two orthogonal DOFs: Vertical (hierarchical dependence) and Horizontal (temporal succession).
*   **CSI:** Active CSI includes both DOFs and their intersection ("now"). No cross-CSI interaction allowed.

**Step 0 -- Initial Diagram**
```
[P(Freedom)] --C--> [Potential Gradient]
```
*   P = Projection operator (isolates a DOF)
*   C = Coupling operator
*   No collapse yet
*   Diagram is fully admissible under CSI

**Step 1 -- I/N/U Classification**
*   Projection: I
*   Coupling: I
*   Gradient: U

**Interpretation:** Structure is admissible, outcome unresolved, no contradiction.

**Step 2 -- CRF Admissibility Check**
*   All elements inside CSI: ok
*   No hard-hard conflict: ok
*   No forbidden interaction: ok
*   Result: rewrites are authorized.

**Step 3 -- Authorized Rewrite**
CRF authorizes Collapse (O) but does not require it.
```
[P(Freedom)] --C--> [O(Choice)] --T--> [Gradient]
```
*   O = Collapse operator (irreversible commitment)
*   T = Transport (into next compartment)

**Step 4 -- Compartment Transition**
*   Transport compresses history.
*   Prior freedom is no longer available.
*   Collapse is irreversible.
*   System becomes non-Markovian structurally.

**Step 5 -- Stability Check**
*   No admissible rewrites remain: ok
*   Admissible region invariant: ok
*   Only identity rewrites allowed: ok
*   System is structurally stable.

**Result:** A goal has emerged as a fixed attractor without being defined.

**Failure Variant (for completeness)**
*   If collapse had violated a hard constraint, CRF would forbid rewrite.
*   Authorized actions: defer, partition, bifurcate, or halt.
*   Failure is explicit and first-class.

## 15. CRF Mini-DSL (Procedural Rules in Code Form)

*   **Nature:** CRF functions as a procedural language governing admissibility and execution.
*   **Rule Components and Mechanics (Conceptual Code Form):**
    *   **Constraint Definition:**
        ```
        DEFINE_CONSTRAINT(
          constraint_id,
          type: 'hard' | 'soft',
          scope: 'local' | 'global',
          conditions: [...],
          action: ...
        )
        ```
    *   **Admissibility Checks:**
        ```
        FUNCTION check_admissibility(current_sid_state, proposed_operation):
          // Evaluate hard/soft, local/global constraints
          // Check CSI boundaries and I/N/U typing
          // Check against rewrite rules for equivalence preservation
          RETURN TRUE | FALSE
        ```
    *   **Resolution Procedures:**
        ```
        PROCEDURE resolve_conflict(conflict_details, current_state):
          IF (conflict_type == 'type_A'):
            attenuate(details)
          ELSE IF (conflict_type == 'type_B'):
            defer(details)
          ELSE IF (conflict_type == 'type_C'):
            partition(details)
          ELSE IF (conflict_type == 'type_D'):
            escalate(details)
          ELSE IF (conflict_type == 'type_E'):
            bifurcate(details)
          ELSE IF (conflict_type == 'type_F'):
            halt(details)
        ```
*   **Note:** The source documents describe CRF semantics but do not provide explicit DSL syntax. The above is conceptual and should be refined during implementation.

## 16. Rewrite-Law Appendix (Like Algebraic Identities)

*   **Definition:** Rewrite rules specify transformations that can be applied to SIDs without altering their fundamental structural effect or equivalence.
*   **Purpose:** Define how diagrams can be manipulated, simplified, or transformed while preserving key properties.
*   **Equivalence Criteria:** See Section 6.2.
*   **TBD:** Specific rewrite laws are not defined in source materials and must be authored for implementation.

## 17. Minimal Implementation Vocabulary (Glossary)

This is a semantic checksum for engineers.

**Diagram**
A data structure representing composed operator interactions. Diagrams do not store meaning.

**Operator**
A structural transformation acting on diagrams. Operators have no semantics or preferences.

**Projection (P)**
Isolates a degree of freedom.

**Coupling (C)**
Introduces a relational constraint between DOFs.

**Collapse (O)**
Irreversible loss of freedom / commitment. Structural, not psychological.

**Transport (T)**
Moves structure between compartments while compressing history.

**Compartment**
A logarithmic structural unit that enforces non-Markovian behavior without memory variables.

**CSI (Causal Sphere of Influence)**
Hard admissibility boundary defining what may interact.

**I / N / U**
Epistemic status labels: I = admissible, N = forbidden, U = unresolved. Labels only; no inference.

**CRF (Constraint Resolution Framework)**
Procedural rule system that authorizes or forbids rewrites. CRF never chooses outcomes.

**Rewrite**
A structure-preserving transformation of a diagram.

**Stability / Termination**
A fixed point where no admissible rewrites remain.

**Metric**
An observable extracted after stability. Metrics never control behavior.
