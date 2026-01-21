# Chapter 17: Undecidability Taxonomy

In the Constraint Resolution Framework, `UNDETERMINED` is not a trash bin for confusing ideas. it is a **legitimate terminal state** that requires precise classification. A claim that cannot be resolved is just as informative as one that can, provided we know *why* it resists resolution. We classify these resistant claims using the **Undecidability Taxonomy**.

### The Categories of the Unresolved
*   **SRL (Self-Referential Loop):** The claim is caught in a logical recursion (e.g., "This statement is false"). It cannot resolve because the act of evaluation changes the state of the claim.
*   **UNGROUNDED:** The claim has no connection to the substrate or existing stabilized Boxes. It is a circular dependency without an anchor. It is "floating" knowledge.
*   **VAGUE:** The claim suffers from boundary issues (Sorites paradox). The terms are not defined with enough precision to trigger a constraint violation or a stabilization.
*   **STC (Set-Theoretic Contradiction):** The claim violates fundamental laws of grouping or hierarchy. It is a "Russell's Paradox" type failure.

### Why Undetermined ≠ Failure
In unbounded systems, a failure to reach a "True" or "False" verdict is seen as a failure of the researcher. In CRF, reaching an `UNDETERMINED` state and correctly classifying it as `SRL` or `VAGUE` is a **successful resolution**. 

You have successfully identified a structural limit of the current system. You have mapped the boundary of what can be known. This is an "Ethical Refusal"—it is the system saying "I cannot answer this without making something up." In CRF, we value an honest refusal over a forced answer.

### The Danger of Misclassification
The most dangerous systemic error is misclassifying an undecidable claim as decidable. 
*   If you treat a `VAGUE` claim as `IS`, you introduce noise.
*   If you treat an `SRL` claim as `NOT`, you ignore a logical structural flaw that will likely reappear elsewhere.

Misclassification leads to "Semantic Pollution"—the gradual accumulation of half-truths and forced verdicts that eventually clog the resolution pipelines. By forcing every undetermined claim into a specific taxonomic category, we isolate the problem. We can then decide whether to ignore the claim (quarantine) or to invest resources in refining the system to handle that specific category of undecidability.

### Quarantine
Claims marked as `UNDETERMINED` are kept in a **Quarantine** area. They are not part of the active model and cannot be used as components in new Boxes. However, they are preserved as a record of the system's current limits. If a new operator tier is added or a constraint is refined, quarantined claims may be re-entered into the pipeline for a second attempt at resolution.
