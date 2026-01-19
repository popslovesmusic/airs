# Manual Section 18: Computational Foundations & Monistic Box Calculus

## 18.0 Preamble

This section details the computational underpinnings of the logic framework. It begins by connecting the framework to established principles in computer science via the Curry-Howard Isomorphism, and then provides the foundational specification for the **Monistic Box Calculus (MBC)**, the primary computational system of the framework.

---

## 18.1 The Curry-Howard Isomorphism for Box Calculus (CH-BC)

A foundational theorem of the logic system is a variant of the Curry-Howard isomorphism, here denoted **Theorem (CH-BC)**. This theorem establishes a direct, formal equivalence between four distinct domains.

**Theorem (CH-BC):** There is an isomorphism between four domains:
`Types ≃ Propositions ≃ Boxes ≃ Programs`

**Implications for the Logic System:**
This isomorphism is not merely an analogy; it is a structural identity with profound operational consequences:

-   **A Proposition is a Type:** Every logical proposition can be treated as a type. An object has that type if and only if it constitutes a proof of that proposition.
-   **A Proof is a Program:** A formal proof of a proposition is structurally identical to a computer program that computes a result of that proposition's corresponding type.
-   **A Program is a Box:** A program, in this context, is an object within **Box Calculus**.
-   **A Box is a Proposition:** Every well-formed "Box" represents a proposition that can be evaluated.

This theorem provides the bridge between the abstract logic of the framework and its computational execution, ensuring that any logically provable statement is also computationally constructible.

---

## 18.2 Monistic Box Calculus (MBC) Foundational Specification

MBC is the operational calculus of meaning within the framework, providing the system for computation. It is constrained by the modal geometry of MSP and the semantic physics of MSC, making it a physically and structurally grounded calculus.

### 18.2.1 MBC Structural Components

MBC consists of six major structural components:

1.  **Boxes (`Bxₙ`):** The primary semantic units of computation. A Box is a stabilized, bounded semantic identity—a structured portion of the semantic manifold with defined ports, states, operators, and invariants.
2.  **Connectors (`Cₙ`):** The fundamental relational edges that define structure and link Boxes. Connectors have intrinsic properties like Directionality, Tension, Polarity, and Conductivity, and they transfer semantic flux.
3.  **Operators (`Opₙ`):** Functions that act on Boxes to transform, combine, rewrite, or otherwise process them. Each operator belongs to a specific computational Tier.
4.  **Rewrite Systems (`Rₙ`):** Sets of symbolic and structural transformation rules that handle normalization, reduction to canonical forms, error correction, and structural repair.
5.  **Module Packs (`Modₙ`):** Functional subsystems that group operators, rewrites, and invariants into specialized circuits (e.g., filters, amplifiers, normalizers).
6.  **Axiom Boxes & Interaction Tables:** Axiom Boxes define structural ground truths, while Interaction Tables define the set of legal compositions and interactions between operators, ensuring a rule-bound computational environment.

### 18.2.2 The 13 Tiers of MBC Operations

MBC is organized into 13 hierarchical Tiers of computation, each handling a distinct class of operations.

| Tier | Name | Symbol | Function |
| :--- | :--- | :--- | :--- |
| 01 | Deviation | δ | Detect difference |
| 02 | Projection | Φ | Format into modal space |
| 03 | Evaluation | Π | Check consistency / truth |
| 04 | Adjacency | κ | Form connectors and substructures |
| 05 | Curvature | λ | Apply context/bias warping |
| 06 | Waveform | ψ | Oscillation & mode interaction |
| 07 | Summation | Σ | Aggregation into narrative bundles |
| 08 | Polarity | ± | Routing under sign constraints |
| 09 | Semantic Time | χ | Ordering & evolution of meaning |
| 10 | Global Constraints | Ω | Invariants & normalization |
| 11 | Meta-Structure | ρ | Multi-level structural cohesion |
| 12 | Universal Schema | Ξ | Schema-level integration |
| 13 | Rho-Coherence | T or Ω | Total structural self-consistency |

### 18.2.3 MBC Computational Cycle

The canonical MBC cycle processes a semantic entity through the 13 Tiers in sequence, representing a complete act of semantic computation from perception to coherence. The cycle is:
`δ → Φ → Π → κ → λ → ψ → Σ → ± → χ → Ω → ρ → Ξ → Ω`

### 18.2.4 MBC Invariants

MBC's stability is guaranteed because it must obey several core invariants, including:
*   **Structural Invariance:** Boxes must remain valid structures after operations.
*   **Connector Integrity:** No illegal relations can be formed.
*   **Cohesion Conservation:** All transformations are governed by the laws of semantic physics from MSC.
*   **Tier Legality:** Operators must follow the correct hierarchy.
*   **Rewrite Confluence:** Normal forms must be unique and reachable.
*   **Semantic Propagation Limits (`χₛ`):** The system must respect the universal speed limit of meaning.
