# Chapter 16: CRF Proper: Resolution Mechanics

We now reach the operational core of the framework: the **Resolution Pipeline**. This is the specific sequence of events that takes a raw claim and either stabilizes it into a Box or terminates it. Resolution is not an abstract concept; it is a finite computational process with a beginning, a middle, and an end.

### Constraint Stacking
The primary mechanic of resolution is **Constraint Stacking**. A claim is subjected to a sequence of filters (the Operator Tiers). 
1.  **First Stack:** Mathematical and Logical invariants. If a claim is self-contradictory or mathematically impossible, it is terminated instantly.
2.  **Second Stack:** Structural and Modal constraints. Does the claim fit within the MSP geometry? Does it violate Modal Closure?
3.  **Third Stack:** Causal and Empirical constraints (if applicable). Does it conflict with existing stabilized knowledge?

A claim only advances if it survives the current stack. We do not evaluate everything at once; we evaluate in stages, from least expensive to most expensive. This is "Epistemic Efficiency."

### Escalation and Termination
If a claim encounters a conflict that it cannot resolve, the system initiates an **Escalation**. The conflict is pushed to a higher operator tier for resolution. If even the global Ω tier cannot resolve the conflict, the claim is **Terminated**.

Termination is a first-class operation. It is not a failure of the system; it is the desired outcome for any claim that cannot pay its structural way. A terminated claim is purged from the active workspace, and its resources are reclaimed. This prevents the "Claim Hoarding" that plagues unbounded systems.

### No Infinite Deliberation
A critical rule of CRF resolution is the **prohibition of infinite deliberation**. Every resolution process is assigned a **Resolution Budget**—a finite limit on the number of computational cycles and causal steps it is allowed to consume. 

If a claim cannot be resolved within its budget, it is not left in an "Undetermined" state indefinitely. It is **Timed Out**. A time-out is treated as a temporary termination; the claim is archived but removed from the active model. We do not allow a single difficult claim to clog the entire pipeline. If you cannot resolve it quickly, you do not understand it well enough to keep it.

### Resolution States
The result of a resolution pipeline is always one of three states:
*   **IS:** The claim is structurally valid and consistent with all applied constraints. It is stabilized into a Box.
*   **NOT:** The claim is structurally invalid or contradicts a higher-level constraint. it is terminated.
*   **UNDETERMINED:** The claim is structurally valid but lacks sufficient information to reach an `IS` or `NOT` state. It is routed to the **Undecidability Taxonomy** for classification and quarantine.
