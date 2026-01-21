# Chapter 26: Termination Principles

The ultimate expression of discipline is knowing when to stop. In the Constraint Resolution Framework, we use **Termination Principles** to manage the end-of-life for claims, processes, and even the system itself.

### When Resolution Must Stop
Resolution is not an open-ended conversation. It is a process that must conclude when:
1.  **The Budget is Exhausted:** If a claim cannot be resolved within its assigned resource limit, the process is terminated. We do not chase diminishing returns.
2.  **A Conflict is Irresolvable:** If the claim reaches a state of `SRL` or `STC` (from the Undecidability Taxonomy), we stop trying to "fix" it. We accept the undecidability and terminate the resolution.
3.  **The Information is Redundant:** If a resolution would not change the state of the SSOT, it is terminated. We do not perform redundant work.

### When Silence is Correct
The most difficult discipline to master is **Epistemic Silence**. This is the refusal to offer an opinion or a model when the constraints are insufficient. 
Silence is not ignorance; it is a calculated structural decision. It prevents the system from being polluted with low-confidence noise. In CRF, "I don't know" (correctly taxonomized as `UNDETERMINED`) is always superior to "It might be X." Silence is the bouncer of the SSOT.

### When Tools are Retired
Tools (Operator Tiers, AIRS modules) are retired when they are no longer "Fit for Purpose." 
*   They are replaced by a more precise constraint.
*   They are found to have a systemic flaw.
*   They are bypassed by a new, more efficient resolution pipeline.

When a tool is retired, its history is preserved, but its influence on the current model is purged. We do not allow legacy code to dictate current resolution.

### When Questions are Rejected
Finally, CRF provides the power to **Reject the Question**. Many intellectual debates are ill-posed; they violate Modal Closure or rely on undefined deviations. 
Instead of trying to answer these questions, CRF terminates them at the entry point. We refuse to engage with questions that cannot, by their very structure, lead to a resolution. This is the ultimate "Pre-Filter" described in Chapter 1. We protect the system by refusing to allow nonsense to enter the gauntlet.
