# Chapter 19: Mathematical Falsification Before Execution

In the Constraint Resolution Framework, computation is a scarce resource. We do not run code to "see what happens" unless we have first verified that the question is well-posed. This is the principle of **Mathematical Falsification Before Execution**.

### Invariants as Kill Switches
Every simulation or computational model is bounded by a set of invariants—mathematical truths that must hold for the model to be valid. 
*   **Conservation Laws:** Energy, mass, or information cannot be created or destroyed.
*   **Dimensional Consistency:** You cannot add meters to seconds.
*   **Logical Non-Contradiction:** A variable cannot be both `A` and `NOT A` simultaneously.

In CRF, these invariants are encoded as **Kill Switches**. Before a single line of simulation code is executed, the model is subjected to a static analysis that checks these invariants. If a violation is found, the execution is killed immediately. We do not simulate physically impossible scenarios.

### Limit Behavior
We also test the **Limit Behavior** of the model. What happens as variables approach zero or infinity? 
*   Does the model crash?
*   Does it produce `NaN` (Not a Number)?
*   Does it violate a boundary condition?

If a model behaves chaotically or irrationally at its limits, it is likely structurally unsound even in its "normal" range. CRF demands that models remain well-behaved at the edges.

### Projection Collapse
This pre-execution phase is where we test for **Projection Collapse**. We verify that the projection (Φ) being used is isomorphic to the substrate rules. If the map distorts the territory beyond an acceptable tolerance, we reject the map.

### Why Math is Adversarial
In this phase, mathematics is not a tool for construction; it is a weapon of destruction. We use math to try and break the model. We search for the one case, the one parameter setting, that causes the logic to unravel. 
This adversarial approach saves immense resources. It is far cheaper to prove a model is mathematically impossible in one second than to run a million-core simulation for a week only to discover the results are garbage. In CRF, math is the bouncer at the door of computation.
