# Manual Section 27: Nomenclature and Conventions

## 27.0 Preamble

This document establishes the authoritative nomenclature, symbols, and naming conventions for the entire framework, encompassing the Modal Symmetry Paradigm (MSP), Monistic Box Calculus (MBC), Modal Structural Cohesion (MSC), and the overarching IGSOA system. Its purpose is to ensure consistency, clarity, and precision across all documentation, code, and conceptual discussions.

This nomenclature governs:
*   MSP, MBC, MSC, and the IGSOA library (tiers, schemas, operators, modules, tests).
*   Textbook, encyclopedia volumes, and narrative materials.

## 27.1 Naming Conventions

### 27.1.1 General Conventions

*   **Tiers:** `lower_snake_case` (e.g., `tier_01_metadata.json5`)
*   **Files:** `lower_snake_case`
*   **Operators:** `UPPER_SNAKE_CASE` (e.g., `OP_DELTA_GRADIENT`)
*   **Invariants:** `UPPER_SNAKE_CASE` (e.g., `INVAR_TRIUNITY_CONSISTENCY`)
*   **Concepts:** `Title Case` (e.g., Causal Sphere of Influence)
*   **Abbreviations:** `UPPER_SNAKE_CASE` (e.g., CSI, MSP, MBC)

### 27.1.2 Operator Naming Pattern

`OP_<TIER>_<FAMILY>_<NAME>`
*   `TIER`: `T00`–`T12` with leading zero.
*   `FAMILY`: Latinized name of the associated core symbol (e.g., DELTA, PHI, PI, MU).
*   `NAME`: Descriptive function in `UPPER_SNAKE_CASE`.

### 27.1.3 Axiom Naming Pattern

`AX_T<TIER>_<SHORT_NAME>`

### 27.1.4 Invariant Naming Pattern

`INVAR_<SCOPE>_<SHORT_NAME>`
*   `SCOPE`: `GLOBAL`, `T00`–`T12`.

## 27.2 Allowed Characters and Symbol Uniqueness

*   **Identifiers:** Alphanumeric characters (A–Z, a–z, 0–9) and underscore (_).
*   **Symbols:** Unicode mathematical and Greek symbols are allowed only where explicitly registered.
*   **Symbol Uniqueness:** Each symbol (`δ`, `Φ`, `Π`, `μ`, `λ`, `ψ`, `Σ`, `Θ`, `χ`, `Ω`, `ρ`, `Ξ`, etc.) has one primary meaning within the core framework. Overloading is not permitted for core symbols; qualified forms (e.g., `δ_01`, `δ_07`) must be explicitly documented.

## 27.3 Canonical Symbol Registry

This registry provides the authoritative definitions for the core mathematical and conceptual symbols used throughout the framework.

### `δ` (Delta)

*   **Family:** Deviation Geometry
*   **Primary Tier:** 01
*   **Category:** Core Operator Family
*   **Definition:** Represents deviation from static symmetry; the first difference that breaks uniformity in the substrate.
*   **Related Terms:** Deviation, First Difference

### `Φ` (Phi)

*   **Family:** Semantic Projection
*   **Primary Tier:** 02
*   **Category:** Core Operator Family
*   **Definition:** Projects deviation-structured states into semantic space, creating meaning-bearing configurations.
*   **Related Terms:** Projection, Semantic Field

### `Π` (Pi)

*   **Family:** Evaluation / Causality
*   **Primary Tier:** 03
*   **Category:** Core Operator Family
*   **Definition:** Evaluates, selects, or weights semantic configurations, imposing truth, value, or causal status.
*   **Related Terms:** Evaluation, Truth, Causality

### `μ` (Mu)

*   **Family:** Local Weight / Micro-Adjacency
*   **Primary Tier:** 04
*   **Category:** Core Operator Family
*   **Definition:** Represents local weighting, adjacency, and micro-scale coupling between semantic units.
*   **Related Terms:** Weight, Adjacency

### `λ` (Lambda)

*   **Family:** Curvature / Deformation
*   **Primary Tier:** 05
*   **Category:** Core Operator Family
*   **Definition:** Encodes curvature and deformation of semantic manifolds, bending paths of semantic transport.
*   **Related Terms:** Curvature, Deformation

### `ψ` (Psi)

*   **Family:** Wave / Oscillation / Mode Interaction
*   **Primary Tier:** 06
*   **Category:** Core Operator Family
*   **Definition:** Represents oscillatory and wave-like interactions between modes of meaning.
*   **Related Terms:** Wave, Oscillation, Mode Interaction

### `Σ` (Sigma)

*   **Family:** Summation / Contraction
*   **Primary Tier:** 07
*   **Category:** Core Operator Family
*   **Definition:** Aggregates or contracts semantic structures along specified axes or indices.
*   **Related Terms:** Aggregation, Contraction

### `Θ` (Theta)

*   **Family:** Polarity / Logic Routing
*   **Primary Tier:** 08
*   **Category:** Core Operator Family
*   **Definition:** Routes semantic flow according to logical polarity, thresholds, or switching conditions.
*   **Related Terms:** Polarity, Logic Gate, Routing

### `χ` (Chi)

*   **Family:** Semantic Evolution / Time
*   **Primary Tier:** 09
*   **Category:** Core Operator Family
*   **Definition:** Implements semantic evolution through time or ordered stages of transformation.
*   **Related Terms:** Semantic Time, Evolution

### `Ω` (Omega)

*   **Family:** Global Constraints / Normalization
*   **Primary Tier:** 10
*   **Category:** Core Operator Family
*   **Definition:** Applies global constraints, normalization, and boundary conditions across the semantic system.
*   **Related Terms:** Constraints, Normalization

### `ρ` (Rho)

*   **Family:** Layer / Meta-Structure / Federation
*   **Primary Tier:** 11
*   **Category:** Core Operator Family
*   **Definition:** Governs layer structure, federation, and meta-layer relationships among semantic subsystems.
*   **Related Terms:** Layering, Federation, Meta-Structure

### `Ξ` (Xi)

*   **Family:** Top-Meta / Universal Schema
*   **Primary Tier:** 12
   **Category:** Core Operator Family
*   **Definition:** Encodes universal schema operations, cross-tier normalization, and high-level meta-coordination.
*   **Related Terms:** Universal Schema, Meta-Coordinator