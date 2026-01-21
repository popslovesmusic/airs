# Chapter 23: SSOT as Resolution Memory

A resolution system is useless if it cannot remember what it has resolved. In the Constraint Resolution Framework, this memory function is served by the **Single Source of Truth (SSOT)**. The SSOT is not a "database of facts"; it is a **ledger of constraints**.

### Provenance
Every entry in the SSOT must have a clear **Provenance**. We need to know:
*   Which resolution pipeline produced this Box?
*   Which constraints did it survive?
*   Which version of the framework (CRF ruleset) was in effect?

Without provenance, knowledge becomes "orphaned." We know *that* something is true, but we forget *why* it is true. Orphaned knowledge is dangerous because if the underlying constraints change, we do not know which facts need to be re-evaluated.

### Versioning
Knowledge is not static; it is versioned. The SSOT tracks the **Claim Lifecycle**. A claim moves from `PROPOSAL` → `TESTING` → `STABILIZED` → `DEPRECATED`. 
CRF explicitly bans "silent updates." You cannot simply overwrite a fact. You must issue a new version of the Box with a new provenance trail. This ensures that we can always roll back the state of the world to any previous point in time.

### Why Forgetting is Dangerous
Unbounded systems suffer from "amnesia." They solve the same problems over and over again because the solution was never formally banked. This wastes immense cognitive energy. 
The SSOT acts as a "ratchet." Once a problem is solved, it stays solved. The system prevents us from sliding back into lower-entropy states of confusion regarding that specific issue.

### Why Remembering Incorrectly is Worse
However, the only thing worse than forgetting is **remembering incorrectly**. If the SSOT is polluted with false or weak claims, it acts as a cancer. Other Boxes will link to the false Box, spreading the structural weakness. 
Therefore, the write-access to the SSOT is the most heavily guarded privilege in the entire system. Only claims that have survived the full Tri-Unity and the Operator Tiers are allowed to write to the ledger.
