# Chapter 18: CSI — Bounded Causality

The final component of operational resolution is the **Causal Sphere of Influence (CSI)**. In an interconnected system, a single change can trigger an infinite cascade of effects. If one Box is updated, every Box connected to it may need re-evaluation. Without a boundary, the system would spend all its resources on endless ripple effects. CSI is the mechanism that bounds causality.

### The Problem of Unlimited Influence
In unbounded knowledge systems, we often see "ripple laundering." This happens when a weak assumption is used to support a theory, which then supports another theory, and so on, until the original weakness is buried under ten layers of "established" knowledge. 

Unlimited causality allows errors to propagate and hide. It also makes resolution computationally impossible; you cannot resolve a single claim if its resolution requires re-calculating the entire universe of connected claims.

### CSI as a Containment Field
CSI defines the **admissibility boundary** for a specific resolution transaction. It dictates which Boxes are allowed to influence the current evaluation. 
*   **Active CSI:** The immediate neighbors and ancestors of the claim being resolved.
*   **Inactive CSI:** Boxes that are connected but beyond the current "Causal Depth" limit.

By bounding the causal sphere, we ensure that resolution remains a local and finite process. We prevent distant, unrelated conflicts from polluting the current resolution. 

### Preventing Ripple Laundering
CSI prevents ripple laundering by enforcing a "Causal Decay" rule. As an influence moves through a chain of connectors, its weight (μ) must be strictly tracked. If the influence becomes too diluted or too complex, it is cut off. 

This forces claims to be **locally solvent**. A claim must survive based on the information within its immediate CSI. It cannot rely on a "magic" result from a distant part of the system that it hasn't formally accounted for. This is the implementation of "Modal Structural Cohesion" at the operational level.

### Causality as Admissibility, Not Force
In the Constraint Resolution Framework, causality is treated as a rule of **admissibility**, not a physical force. It is the system asking: "Is this piece of information allowed to be considered in this resolution?"

By framing causality this way, we maintain total control over the resolution pipeline. We decide where the causal ripples stop. This allows the system to remain stable even when parts of it are in a state of high turbulence. CSI is the firebreak that prevents a single failed resolution from burning down the entire model.
