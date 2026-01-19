# Manual Section 05: Worked Example - The "Inter-Departmental Approval" Scenario

## 5.0 Purpose

This section demonstrates the application of the complete FRP/CSI framework to a canonical problem. It will show how the **Two-Step Resolution Mandate** moves from mapping the causal terrain to a final, rigorous diagnosis.

**Scenario:**
- The Finance Dept says: "We will approve the budget if the Operations Dept confirms the strategy is sound."
- The Operations Dept says: "We will confirm the strategy is sound only if the Finance Dept approves the budget."

## 5.1 Step 1: CSI Mapping

Before any evaluation can occur, we map the Causal Sphere of Influence.

1.  **Identify Entities:** We identify two core propositions:
    - `B`: "The budget is approved by Finance."
    - `S`: "The strategy is confirmed as sound by Operations."

2.  **Define Causal Dependencies:** We translate the narrative into causal relationships:
    - The resolution of `B` is dependent on the state of `S`. ( `S → B` )
    - The resolution of `S` is dependent on the state of `B`. ( `B → S` )

3.  **Map the Sphere:** The CSI for this system is a closed sphere containing only propositions `B` and `S`. The causal pathways are entirely internal to this sphere, forming a closed, two-node loop.

4.  **Identify External Anchors:** We scan for any causal pathways that originate from *outside* this `{B, S}` sphere. In this scenario, none exist. There is no third-party review, no executive override, and no pre-existing axiom that can provide an initial `IS` or `NOT` state to either `B` or `S`.

**CSI Mapping Conclusion:** The system is a closed, ungrounded, two-node loop.

## 5.2 Step 2: FRP Evaluation

With the CSI map established, we apply the Formal Resolution Path to the pathways *within* this sphere.

1.  **Structural Audit:** The FRP's audit immediately detects the cyclic dependency graph (`B → S → B`) identified during CSI mapping.

2.  **Taxonomic Classification:** The FRP consults the *Taxonomy of the Undetermined* to classify this structure.
    - It does not match `SRL` or `STC` because the propositions are not intrinsically self-contradictory.
    - It does not match `VAGUE` as there is no underlying continuum.
    - It perfectly matches the structural signature of **Category II: Ungrounded Circularity**.

3.  **Prescribed System Action:** The taxonomy mandates the action for an `UNGROUNDED` state: **Suspension**. The system must not enter an infinite "wait" state. Instead, it must cease evaluation.

4.  **Issue Terminal State:** The FRP issues its final, conclusive diagnosis for the system as a whole.

**FRP Evaluation Conclusion:** `UNDETERMINED(UNGROUNDED)`

## 5.3 Final Resolution Summary

The framework does not attempt to resolve whether the budget is approved. Instead, it provides a complete and rigorous diagnosis of the system's structure. The final output is the terminal state `UNDETERMINED(UNGROUNDED)`, which asserts that a binary resolution is structurally impossible without an external causal anchor. The system has reached a stable, correct, and final conclusion: non-resolution.
