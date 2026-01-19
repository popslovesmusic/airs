# Manual Section 09: MSP-Compliant Learning

## 9.0 Preamble: Learning as Governed Evolution

For the framework to adapt, its constraint set must be able to evolve based on new evidence or observations. However, this "learning" process is dangerous if left ungoverned, as it can silently corrupt the framework's logical integrity. MSP-compliant learning is a doctrine that allows constraints to evolve without violating the core MSP Invariants. It treats learning not as an accumulation of knowledge, but as a disciplined evolution of the system's structure.

## 9.1 The Partitioned Constraint Set

To enable safe learning, the active constraint set `C` is formally partitioned into three distinct classes. This partitioning is a mandatory architectural requirement.

- **C-core (Core Constraints):** The sealed, non-updatable invariants of the system, including the MSP Invariants themselves. This set is immutable.
- **C-learn (Learnable Constraints):** A dynamic set of constraints that may be updated by the learning process. These are typically constraints related to calibration, boundaries, or other parametric values.
- **C-heur (Heuristic Constraints):** A set of non-forcing suggestions, such as preferences or statistical tendencies. Heuristics may be used to rank or guide choices among symmetry-equivalent outcomes, but they are forbidden by MSP from ever forcing a binary `IS` or `NOT` resolution.

## 9.2 The Learning Update Operator (Δ)

All learning is mediated by an update operator, `Δ`, which takes the current learnable constraint set and a new piece of evidence to produce a new learnable set: `Δ(C-learn, Evidence) → C-learn'`.

The central rule of MSP-compliant learning is: **An update is valid if and only if the resulting total constraint set (`C-core ∪ C-learn'`) does not violate any MSP Invariants.** If an update would produce a state that breaches an invariant (e.g., by forcing a symmetry collapse), the update must be rejected or quarantined.

## 9.3 MSP-Compliant Learning Rules (L-Rules)

These are the primary rules governing the `Δ` operator.

- **L1 — Non-Collapse Guarantee:** Learning cannot create a constraint that would force an `IS` or `NOT` decision where symmetry previously existed. It can only break symmetry by adding legitimate, eliminative constraints.
- **L2 — Modal Non-Promotion Guarantee:** Statistical evidence (e.g., frequency, probability) can only update parameters within `C-learn` or `C-heur`. It cannot be promoted into a `C-core` constraint or be given the power of a hard, eliminative rule.
- **L3 — Modal Closure of Learning:** An update must not introduce a constraint that would require the mixing of incompatible modal classes during evaluation.
- **L4 — Monotonic Safety:** An update cannot retroactively invalidate a previously valid resolution without issuing a formal **Revision Event**. This prevents the system's history from being silently rewritten.
- **L5 — Reversibility Requirement:** Every update should be representable as a deterministic and invertible (or at least removable) delta, allowing for system rollbacks and auditable logs.

By adhering to these rules, the framework can adapt and refine its precision without sacrificing the logical rigor that underpins its integrity.
