# Chapter 20: Simulation as Hostile Environment

If a model survives mathematical falsification, it earns the right to enter **Simulation**. However, in the Constraint Resolution Framework, simulation is not a "victory lap." It is a hostile environment designed to expose hidden weaknesses.

### Simulation is Not Evidence
A common fallacy is to treat simulation results as empirical evidence. "The model says X happens, therefore X is true." CRF rejects this. A simulation is merely a **consequence engine**. It tells you what *would* happen *if* your assumptions were correct. It tests the internal consistency of your logic, not its external truth.

### Stability ≠ Correctness
A simulation can be perfectly stable and yet completely wrong. A model of a flat earth can be internally consistent if you tweak the physics enough. Therefore, we do not look for "success" in simulation; we look for **failure modes**.
*   Under what conditions does the model break?
*   How much stress can it take before it violates an invariant?
*   Does it produce "emergent" behavior that contradicts basic laws?

### Emergence ≠ Meaning
Unbounded systems often celebrate "emergence"—unexpected patterns that arise from simple rules. CRF views emergence with suspicion. Emergence is often a sign of "leaky constraints"—variables interacting in unauthorized ways. 
When we see emergence, we do not applaud; we audit. We trace the causal path to ensure the new pattern is a legitimate consequence of the substrate, not an artifact of the simulation engine (e.g., floating-point errors or rounding artifacts).

### Declared Failure Criteria
Before running any simulation, the researcher must declare **Failure Criteria**. You must state, in advance, what outcome would prove your model wrong. 
*   "If the system energy exceeds X, the model fails."
*   "If the agent population drops to zero, the model fails."

Running a simulation without declared failure criteria is "exploratory play," not rigorous resolution. CRF demands that every run has a clear pass/fail condition.
