# SID Semantic Processor - Complete Documentation

## Quick Navigation

This directory contains the complete specifications, code stubs, and documentation for the **SID (Semantic Interaction Diagrams)** framework - a ternary semantic processing system.

### Documentation Index

#### Core Architecture
- **[00_ARCHITECTURE_OVERVIEW.md](docs/00_ARCHITECTURE_OVERVIEW.md)** - System hierarchy, core concepts, architectural invariants

#### Specifications
- **[01_SSP_SPECIFICATION.md](docs/01_SSP_SPECIFICATION.md)** - Semantic State Processor (SSP) formal spec
- **[02_MIXER_SPECIFICATION.md](docs/02_MIXER_SPECIFICATION.md)** - SID Mixer formal spec
- **[03_COLLAPSE_MASKS.md](docs/03_COLLAPSE_MASKS.md)** - Collapse mask formalization

#### Implementation
- **[04_CODE_STUBS.md](docs/04_CODE_STUBS.md)** - Complete C/C++ code stubs (compile-ready)
- **[05_ENGINE_INTEGRATION.md](docs/05_ENGINE_INTEGRATION.md)** - How to replace fake engines with real ones

#### Philosophy Essays
- **[essay_1_ontology_of_decision.md](docs/essay_1_ontology_of_decision.md)** - From potential to actuality
- **[essay_2_architecture_of_meaning.md](docs/essay_2_architecture_of_meaning.md)** - Isolation, coupling, mediation
- **[essay_3_ternary_logic_metaphysics.md](docs/essay_3_ternary_logic_metaphysics.md)** - Why ternary logic is necessary
- **[essay_4_content_under_constraint.md](docs/essay_4_content_under_constraint.md)** - Conservation laws and semantic physics
- **[essay_5_conflict_ambivalence_resolution.md](docs/essay_5_conflict_ambivalence_resolution.md)** - How contradictions are managed
- **[essay_6_future_of_content.md](docs/essay_6_future_of_content.md)** - From digital to analog substrates

#### Source Materials
- **[ssp.txt](docs/ssp.txt)** - Original conversation/specification dump
- **[ternary.txt](docs/ternary.txt)** - Ternary logic hardware mapping

---

## What is SID?

**SID (Semantic Interaction Diagrams)** is a ternary semantic processing framework that:

- Treats content as **conserved semantic mass** in three states (potential, affirmed, rejected)
- Implements **irreversible collapse** from undecided to committed states
- Enforces **semantic isolation** between processors
- Uses a **mixer** for principled conflict resolution
- Runs ternary logic on digital hardware

### The Three States

```
0 (∅)   - Potential/Undecided
+1 (S+) - Positive Assertion/Affirmed
-1 (S-) - Negative Assertion/Rejected
```

### The Conservation Law

```
I(t) + N(t) + U(t) = C    ∀t
```

Where:
- **I** = Admitted/stabilized content
- **N** = Excluded/forbidden content
- **U** = Undecided/reservoir
- **C** = Total semantic capacity (constant)

---

## System Hierarchy

```
AIRS (AI Research Suite)
  └─ DASE (Dynamic Autonomous Simulation Environment)
      └─ DSE (Dynamic Simulation Engine)
          └─ SID (Semantic Interaction Diagrams)
              ├─ SSP(I) - Admitted Processor
              ├─ SSP(N) - Excluded Processor
              ├─ SSP(U) - Undecided Processor
              └─ Mixer - Semantic Arbiter
```

---

## Getting Started

### Beginner: Build and Run the Demo

This repo now includes a small C/C++ demo in `src/` that compiles into a single executable.

#### Prerequisites
- CMake 3.16+
- A C/C++ compiler toolchain (on Windows: Visual Studio with C++ workloads)

#### Build (from the repo root)

```sh
cmake -S . -B build
cmake --build build --config Release
```

#### Run

```sh
build/Release/sid_ssp_demo.exe
```

You should see a short log of `I/N/U` volumes and conservation error per timestep.

#### Where the code lives
- Demo entry point: `src/main.cpp`
- SSP implementation: `src/sid_semantic_processor.c` and `src/sid_semantic_processor.h`
- Mixer implementation: `src/sid_mixer.c` and `src/sid_mixer.h`

#### Next steps after the demo
- Replace the fake engine in `src/main.cpp` with real engines
- Follow `05_ENGINE_INTEGRATION.md` for adapter and buffer wiring

#### Troubleshooting (Windows)
- **CMake can't find a compiler**: Open the "x64 Native Tools Command Prompt for VS" and rerun the build commands.
- **MSBuild or cl.exe not found**: Install the "Desktop development with C++" workload in Visual Studio Installer.
- **Build succeeds but executable missing**: Check `build/Release/` for `sid_ssp_demo.exe` (Release config is required).
- **Linker errors about math functions**: Ensure you're building as C/C++ with the provided `CMakeLists.txt`.

### For Implementers

1. Read **[00_ARCHITECTURE_OVERVIEW.md](docs/00_ARCHITECTURE_OVERVIEW.md)** for the big picture
2. Review **[04_CODE_STUBS.md](docs/04_CODE_STUBS.md)** for runnable code
3. Check **[05_ENGINE_INTEGRATION.md](docs/05_ENGINE_INTEGRATION.md)** to connect real engines

### For Researchers

1. Start with **[essay_1_ontology_of_decision.md](essay_1_ontology_of_decision.md)**
2. Continue through the essay series (1-6) for philosophical foundations
3. Dive into specs for formal details

### For System Architects

1. **[00_ARCHITECTURE_OVERVIEW.md](docs/00_ARCHITECTURE_OVERVIEW.md)** - System boundaries
2. **[01_SSP_SPECIFICATION.md](docs/01_SSP_SPECIFICATION.md)** - Processor contracts
3. **[02_MIXER_SPECIFICATION.md](docs/02_MIXER_SPECIFICATION.md)** - Mixer responsibilities

---

## Key Architectural Decisions

### 1. SSP is Not a "Drive"
It's a **semantic processing unit**, not storage media. The name was corrected from "SID Drive" to "Semantic State Processor (SSP)".

### 2. Mixer is Semantic Arbiter, Not Blender
The mixer does **not** average or blend. It enforces ternary invariants and issues collapse directives based on formal rules.

### 3. Engines Are Created by DASE CLI
SID does **not** instantiate engines directly. DASE issues handles via JSON-driven CLI.

### 4. Irreversibility is Fundamental
Once content collapses from U to I or N, it **cannot** return. This is enforced both logically and (eventually) physically.

### 5. Isolation is Non-Negotiable
SSPs cannot observe each other. All coordination happens through the mixer.

---

## Architectural Invariants (Hard Rules)

```
✓ Role Lock:           SSP roles (I/N/U) are immutable
✓ Single Writer:       Only local engine mutates SSP field
✓ No Cross-Visibility: SSP(x) cannot observe SSP(y)
✓ Irreversibility:     U→I, U→N allowed; I→U, N→U, I↔N forbidden
✓ Conservation:        I(t) + N(t) + U(t) = C
```

Violating any of these is a **category error**, not a bug.

---

## Current Status

### What Exists

- ✅ Complete formal specifications (SSP, Mixer, Collapse Masks)
- ✅ Compile-ready C/C++ stubs
- ✅ Philosophical foundations (6-essay series)
- ✅ Engine adapter interface design
- ✅ Real metrics implementation (stability, coherence, divergence)
- ✅ Conservation tracking and transport predicates

### What's Next

1. Replace fake engines with real DASE engines
2. Implement admissibility-shaped collapse masks
3. Add Transport as phase transition trigger
4. Connect to DSE scheduling layer
5. Add persistence
6. Explore native ternary substrates (spintronic, analog, quantum)

---

## One-Line Contracts

**SID**: The irreversible semantic coupler that converts undecided dynamics into admissible or excluded structure under conserved ternary law.

**SSP**: A bounded, role-locked state substrate that preserves engine dynamics while enforcing semantic isolation.

**Mixer**: A semantic arbiter that enforces ternary conservation and irreversible collapse through predicate-based adjudication.

**Collapse Mask**: A bounded semantic eligibility field that irreversibly transfers undecided mass into admissible or excluded structure.

---

## License & Attribution

Part of the **AIRS (AI Research Suite)** / **DASE (Dynamic Autonomous Simulation Environment)** project.

Original specifications derived from conversations about ternary logic, semantic processing, and computational metaphysics.

---

## Contact & Contributing

For questions about specifications, see the individual markdown files.

For implementation questions, start with **[04_CODE_STUBS.md](docs/04_CODE_STUBS.md)** and **[05_ENGINE_INTEGRATION.md](docs/05_ENGINE_INTEGRATION.md)**.

For philosophical discussions, read the essay series in order (1→6).
