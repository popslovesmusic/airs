# SID Test Harness & Simulation Code: Detailed Examination

**Date**: 2026-01-23
**Status**: Step 5a Blocked - Diagram Load Failure + Build System Issues

---

## EXECUTIVE SUMMARY

The SID (Semantic Interaction Diagram) system is blocked on **Step 5a** (rewrite ordering validation) due to:

1. **Build system has incompatible MSVC flags** (/O2 optimization + /RTC1 runtime checks)
2. **sid_set_diagram_json() fails silently** for new Step 5a fixtures
3. **Working Release binaries exist** but cannot load the new Step 5a JSON diagrams

**Current State**:
- ✅ Steps 1-4 validated (basic SID execution)
- ✅ JSONL strict mode enforced
- ✅ Validator gating implemented
- ✅ Step 5a fixtures created (scenario_overlap + scenario_multiredeux)
- ❌ Cannot execute Step 5a tests (diagram load failure)
- ❌ Cannot rebuild Debug binaries (CMake flag conflict)

---

## ARCHITECTURE OVERVIEW

### Three-Layer Test System

**1. Python Black-Box Harness** (`scripts/test_cli_harness.py`)
- Spawns dase_cli.exe subprocesses
- Sends JSONL commands via stdin
- 10-second timeout per command
- Session-scoped (isolated test runs)
- Non-blocking I/O with thread-based queue readers

**2. GTest Harness** (`Simulation/tests/harness/`)
- `harness_sid_rewrite.cpp` - **Currently placeholder (GTEST_SKIP)**
- `harness_policy.cpp` - CSV-based validation loader
- `harness_test_util.cpp/hpp` - Shared test utilities
- Designed for policy-aware validation but not yet fully implemented
- Configured via VALIDATION_TABLE_PATH (validation table.txt)

**3. CLI Step Runner** (`Simulation/tests/engine_api/sid_step_runner.cpp`)
- Minimal wrapper spawning dase_cli.exe
- Normalizes output (strips timestamps, engine_id)
- Computes FNV-1a hash of traces for deterministic comparison
- Extracts metrics (active_nodes, total_mass) for validation

### Integration Flow

```
┌─────────────────────────────────────────┐
│  Python/GTest Test Harness              │
└──────────────┬──────────────────────────┘
               │ JSONL over stdin
               ▼
┌─────────────────────────────────────────┐
│     dase_cli.exe (main.cpp)             │
│  ┌─────────────────────────────────┐   │
│  │  CommandRouter::execute()        │   │
│  │  - 50+ registered commands       │   │
│  │  - Timing instrumentation        │   │
│  └──────────────┬──────────────────┘   │
│                 │                        │
│  ┌──────────────▼──────────────────┐   │
│  │  EngineManager                   │   │
│  │  - createEngine()                │   │
│  │  - sidSetDiagramJson()           │   │
│  │  - sidApplyRewrite()             │   │
│  │  - sidRunRewrites() [policy]     │   │
│  └──────────────┬──────────────────┘   │
└─────────────────┼────────────────────────┘
                  │
   ┌──────────────┼──────────────────┐
   │ C API Boundary (sid_capi.cpp)   │
   │  - sid_engine opaque handle      │
   │  - sid_set_diagram_json()        │
   │  - sid_apply_rewrite()           │
   │  - sid_last_rewrite_message()    │
   └──────────────┬──────────────────┘
                  │
   ┌──────────────▼──────────────────────────┐
   │  SidTernaryEngine (C++)                 │
   │  ┌──────────────────────────────────┐  │
   │  │  Mixer (I/N/U evolution)          │  │
   │  │  - step(*ssp_I, *ssp_N, *ssp_U)  │  │
   │  └──────────────────────────────────┘  │
   │  ┌──────────────────────────────────┐  │
   │  │  SemanticProcessor × 3            │  │
   │  │  - field(): vector<double>        │  │
   │  │  - commit_step()                  │  │
   │  └──────────────────────────────────┘  │
   │  ┌──────────────────────────────────┐  │
   │  │  Diagram                          │  │
   │  │  - nodes_, edges_                 │  │
   │  │  - adjacency_list_ (cached)       │  │
   │  │  - has_cycle() (iterative DFS)    │  │
   │  └──────────────────────────────────┘  │
   │  ┌──────────────────────────────────┐  │
   │  │  Rewrite Engine                   │  │
   │  │  - parse_expression()             │  │
   │  │  - apply_rewrite()                │  │
   │  │  - build_expr() → Diagram         │  │
   │  └──────────────────────────────────┘  │
   └─────────────────────────────────────────┘
```

---

## CRITICAL FINDINGS

### 1. Build System Bug (ROOT CAUSE)

**File**: `Simulation/CMakeLists.txt:105-108`

```cmake
set(DASE_COMPILE_FLAGS_MSVC
  /EHsc /bigobj /std:c++17 /O2 /Ob3 /Oi /Ot
  /fp:fast /GL /DNOMINMAX /openmp /MD
)
```

**Problem**: `/O2` (optimize for speed) is incompatible with `/RTC1` (runtime checks) which MSVC adds by default in Debug configurations.

**MSVC Error**: `C1189: #error: /RTC1 and /O2 are incompatible`

**Impact**:
- Clean Debug builds timeout or fail across multiple projects (dase_cli, sid_ssp_capi, dase_core)
- Mixed build artifacts in `build/Debug/` are unreliable
- Previous `build/Debug` deletion makes rebuilding impossible without fixing flags

**Affected Projects**:
- dase_cli
- sid_ssp_capi
- dase_core
- analysis_integration
- igsoa_gw_core

### 2. Diagram Load Failure

**Symptom** (from `problem.txt`):
- `sid_set_diagram_json` returns `EXECUTION_FAILED`
- `last_rewrite_message` is empty string
- Metrics stay at zero (I_mass=0, N_mass=0, U_mass=0)
- Even minimal diagram `{id, nodes:[{op:P,dof_refs:[]}], edges:[]}` fails

**Root Cause Analysis**:

#### Location: `Simulation/src/cpp/sid_ternary_engine.hpp:303-355`

```cpp
bool setDiagramJson(const std::string& json_str) {
  try {
    auto j = nlohmann::json::parse(json_str);

    if (!j.is_object()) {
      last_rewrite_message_ = "Diagram JSON must be an object";
      return false;
    }

    // Parse nodes and edges...
    for (const auto& jn : j.value("nodes", nlohmann::json::array())) {
      // ... build Node struct ...
      diagram_->add_node(node);
    }

    for (const auto& je : j.value("edges", nlohmann::json::array())) {
      // ... build Edge struct ...
      diagram_->add_edge(edge);
    }

    diagram_ = std::move(new_diagram);
    return true;

  } catch (const std::exception& e) {
    last_rewrite_message_ = "Diagram parse error: " + std::string(e.what());
    return false; // ← Should propagate error, but...
  }
}
```

#### C API Boundary Issue: `Simulation/src/cpp/sid_ssp/sid_capi.cpp:130`

```cpp
const char* sid_last_rewrite_message(sid_engine* eng) {
  if (!eng || !eng->engine) return "";
  static std::string msg_buffer;  // ← Static buffer!
  msg_buffer = eng->engine->last_rewrite_message();
  return msg_buffer.c_str();
}
```

**Potential Issues**:
1. Static buffer could be corrupted/stale in certain build configurations
2. Thread-unsafe (not relevant for single-threaded CLI, but still risky)
3. Exception might be thrown in Node/Edge construction but message lost
4. nlohmann::json parsing might succeed but structural validation fails silently

#### Command Router Validation: `dase_cli/src/command_router.cpp:1438-1461`

Pre-validates JSON before calling engine:
- Checks `diagram.is_object()`
- Checks `diagram["id"].is_string()`
- Validates `nodes[]` array structure
- Validates `edges[]` array structure

**This validation passes**, meaning the issue is in the C++ engine layer.

### 3. Step 5a Fixtures Analysis

**Created but Untested**:
- `fixtures/step5a/scenario_overlap/` - Overlapping rewrites (2 rules match same redex)
- `fixtures/step5a/scenario_multiredeux/` - Multi-redex ordering (inner vs outer first)

#### Fixture Design (from `step5ab.txt`)

**scenario_overlap** - Minimal overlapping rewrite set:
```
Diagram: S+(P(a), P(b))

Rules:
  R1: S+($x,$y) → C($x,$y)  ← Both rules match the root!
  R2: S+($x,$y) → D($x,$y)  ← Overlap (intentional)
  R3: C($x,$y) → T($x)      ← Converge to same normal form
  R4: D($x,$y) → T($x)      ← Converge to same normal form

Expected Behavior:
  - P1/P2/P3 may pick R1 vs R2 first (trace differs immediately)
  - Second step always collapses to same normal form: T(P(a))
  - steps = 2
  - rules_applied = 2
  - termination = fixed_point
  - Final semantic state equivalent across all policies
  - Conservation identical across all policies

Purpose: Test that different rule orderings produce identical final states
```

**scenario_multiredeux** - Multi-redex ordering fixture:
```
Diagram: S+(P(a), S+(P(b), P(c)))  ← Nested structure!

Redexes:
  - R₀ (outer): S+( ... , ... )
  - R₁ (inner): S+(P(b), P(c))

Rules:
  R1: S+($x,$y) → C($x,$y)
  R2: S+($x,$y) → D($x,$y)
  R3: C($x,$y) → T($x)
  R4: D($x,$y) → T($x)
  R5: T($x) → T($x)  ← Fixed-point marker

Example Trace A (outer-first):
  S+(P(a), S+(P(b),P(c)))
  → C(P(a), S+(P(b),P(c)))
  → T(P(a))
  (2 steps, inner redex skipped)

Example Trace B (inner-first):
  S+(P(a), S+(P(b),P(c)))
  → S+(P(a), C(P(b),P(c)))
  → S+(P(a), T(P(b)))
  → C(P(a), T(P(b)))
  → T(P(a))
  (4 steps, more rewrites)

Expected:
  - steps > 1
  - rules_applied > 1
  - termination = fixed_point
  - Final semantic state equivalent: T(P(a))
  - Conservation metrics identical
  - Trace divergence allowed and expected

Purpose: Test ordering sensitivity across multiple redex locations with invariant final state
```

**Acceptance Criteria** (governance-level):
- Across P1/P2/P3 policies: final semantic state must match
- Conservation metrics (I_mass, N_mass, U_mass, total_mass) must be identical
- Trace divergence is allowed (different step counts, different rule sequences)
- Termination must be `fixed_point` (not `horizon`)

**Failure Conditions**:
- Final term differs across policies
- Conservation metrics differ
- Non-termination (hits horizon_cap)
- Different fixed points reached

### 4. Policy Implementation

**File**: `dase_cli/src/command_router.cpp:1203-1320`

#### Policy Scheduler

```cpp
auto build_order = [](size_t n, const std::string& pol, std::mt19937& gen) {
  std::vector<size_t> idx(n);
  std::iota(idx.begin(), idx.end(), 0);
  if (pol == "P2") std::reverse(idx.begin(), idx.end());
  else if (pol == "P3") std::shuffle(idx.begin(), idx.end(), gen);
  return idx;
};
```

**P1 (lexical)**: Rules applied in array order [0, 1, 2, 3, ...]
**P2 (reverse)**: Rules applied in reverse order [N-1, N-2, ..., 0]
**P3 (shuffle)**: Seeded random permutation (seed from params)
**P4/P5**: Currently map to P1 (placeholders for future heuristics)

#### Rewrite Loop

```cpp
while (steps < horizon_cap) {
  auto order = build_order(rules.size(), policy, rng);
  bool applied_pass = false;

  for (size_t rule_idx : order) {
    const auto& rule = rules[rule_idx];
    bool applied = false;
    engine_manager->sidApplyRewrite(
      engine_id, pattern, replacement, rule_id,
      rule_metadata, applied, message
    );

    if (applied) {
      applied_pass = true;
      applied_total++;
      applied_trace.push_back(rule_id);
      steps++;
      if (steps >= horizon_cap) {
        horizon_hit = true;
        break;
      }
    }
  }

  if (!applied_pass) break; // Fixed point reached
}
```

**Termination Modes**:
- `fixed_point`: No rules applicable in entire pass through rule set
- `horizon`: Reached horizon_cap steps (indicates potential non-termination)

#### Metrics After Rewrites

```cpp
std::vector<double> psi_r, psi_i, field;
if (engine_manager->getAllNodeStates(engine_id, psi_r, psi_i, field)) {
  for (double v : field) {
    if (std::abs(v) > 1e-12) active_nodes += 1.0;
    total_mass += v;
  }
}
```

**Conservation Check**: `I_mass + N_mass + U_mass ≈ R_c` (within epsilon)

---

## SIMULATION CLI INTERFACE (dase_cli.exe)

### Command Protocol

**Entry Point**: `Simulation/dase_cli/src/main.cpp`
- Line-buffered JSON over stdin (JSONL protocol)
- Binary mode on Windows (no CRLF translation)
- Single-threaded command router (no thread safety required)
- Returns JSON response per command

### SID-Specific Commands

#### Engine Lifecycle
- `create_engine` (engine_type: "sid_ternary" or "sid_ssp")
  - sid_ternary: `num_nodes` (1-1048576), `R_c` (total mass)
  - sid_ssp: `num_nodes`, `capacity`, `role` (0=I, 1=N, 2=U)
- `destroy_engine` (engine_id)
- `list_engines` → [{engine_id, engine_type, num_nodes, config}]

#### Diagram Operations
- `sid_set_diagram_expr`: Load from expression string (e.g., "C(P(A),P(B))")
  - Uses recursive descent parser (sid_parser_impl.hpp)
  - Builds diagram from AST (sid_diagram_builder.hpp)
  - Stable ID generation (IDGenerator)

- `sid_set_diagram_json`: Load from JSON diagram structure
  - Schema: `{id: string, nodes: [], edges: []}`
  - Validates before calling engine
  - **Currently failing for Step 5a fixtures**
  - Returns `last_rewrite_message` on failure

- `sid_get_diagram_json`: Export current diagram as JSON

#### Rewrite Operations
- `sid_rewrite`: Apply single rule `{pattern, replacement, rule_id}`
  - Returns: `{applied: bool, message: string}`
  - Pattern syntax: `S+($x,$y)`, `C($x,$y)`, etc.

- `sid_run_rewrites`: Execute policy-driven rewrite sequence
  - Parameters: `{rules[], policy, horizon_cap, seed}`
  - Returns: `{steps, rules_applied, termination, applied_trace[], metrics}`
  - This is the main command for Step 5a validation

#### Evolution (Alpha Features)
- `sid_step` (alpha): Single mixer step (I/N/U field evolution)
- `sid_collapse` (alpha): Force collapse operation (U → I/N)
- `run_steps` (num_steps, iterations_per_node): Bulk stepping

#### Metrics
- `sid_metrics` → `{I_mass, N_mass, U_mass, is_conserved, instantaneous_gain, last_rewrite_applied, last_rewrite_message}`
- `sid_rewrite_events` (cursor, limit) → paginated event log
- `sid_wrapper_metrics` → wrapper state tracking

---

## SID ENGINE ARCHITECTURE

### C API Layer (`sid_ssp/sid_capi.cpp`)

**Opaque Handle Pattern**:
```c
struct sid_engine {
  std::unique_ptr<sid::SidTernaryEngine> engine;
};
```

**Key C API Functions**:
- `sid_create_engine(num_nodes, total_mass)` → sid_engine*
- `sid_destroy_engine(eng)`
- `sid_set_diagram_json(eng, json)` → bool
- `sid_get_diagram_json(eng)` → const char*
- `sid_apply_rewrite(eng, pattern, replacement, rule_id)` → bool
- `sid_last_rewrite_message(eng)` → const char* ← **Issue here**
- `sid_get_I_mass(eng)`, `sid_get_N_mass(eng)`, `sid_get_U_mass(eng)` → double

### C++ Engine Layer (`sid_ternary_engine.hpp`)

```cpp
class SidTernaryEngine {
private:
  std::unique_ptr<Mixer> mixer_;
  std::unique_ptr<SemanticProcessor> ssp_I_;
  std::unique_ptr<SemanticProcessor> ssp_N_;
  std::unique_ptr<SemanticProcessor> ssp_U_;
  std::unique_ptr<Diagram> diagram_;
  uint64_t step_count_;
  bool last_rewrite_applied_;
  std::string last_rewrite_message_;

public:
  bool step(double alpha);
  bool collapse(double alpha);
  bool applyRewrite(const std::string& pattern,
                    const std::string& replacement,
                    const std::string& rule_id);
  bool setDiagramJson(const std::string& json_str); // ← Failure point
  std::string getDiagramJson() const;
  std::string last_rewrite_message() const { return last_rewrite_message_; }
};
```

### Diagram Data Structures (`sid_diagram.hpp`)

```cpp
struct Node {
  std::string id;
  std::string op;  // P, S+, S-, C, T, O
  std::vector<std::string> inputs;
  std::vector<std::string> dof_refs;
  std::map<std::string, AttrValue> attributes;
  bool irreversible;
};

struct Edge {
  std::string id;
  std::string from;
  std::string to;
  std::string label;
  int port;
  std::map<std::string, AttrValue> attributes;
};

class Diagram {
private:
  std::vector<Node> nodes_;
  std::vector<Edge> edges_;
  std::map<std::string, std::vector<std::string>> adjacency_list_;

public:
  void add_node(const Node& node);
  void add_edge(const Edge& edge);
  bool has_cycle(); // Iterative DFS (fixes Python recursion bug)
  std::string to_json() const;
};
```

**Operators**:
- **P** (Primitive): Atomic elements
- **S+** (Simultaneous Add): Parallel composition
- **S-** (Simultaneous Subtract): Parallel anti-composition
- **C** (Compose): Sequential composition
- **T** (Transport): Transformation/motion
- **O** (Collapse): Quantum-like state collapse

---

## VALIDATION & INGRESS LAYER

### JSON Diagram Validation

**Two-Stage Validation**:

#### Stage 1: Command Router (`command_router.cpp:1438-1461`)
```cpp
// Pre-validation before calling engine
if (!diagram.is_object()) { error("must be object"); }
if (!diagram.contains("id") || !diagram["id"].is_string()) { error("needs id"); }

for (const auto& node : diagram.value("nodes", nlohmann::json::array())) {
  if (!node["id"].is_string()) { error("node needs id"); }
  if (node.contains("inputs") && !node["inputs"].is_array()) { error(); }
  if (node.contains("dof_refs") && !node["dof_refs"].is_array()) { error(); }
}

for (const auto& edge : diagram.value("edges", nlohmann::json::array())) {
  if (!edge["id"].is_string()) { error("edge needs id"); }
  if (!edge["from"].is_string() || !edge["to"].is_string()) { error(); }
}
```

**This stage passes for Step 5a fixtures.**

#### Stage 2: Engine-Level Parsing (`sid_ternary_engine.hpp:303-355`)
```cpp
bool setDiagramJson(const std::string& json_str) {
  try {
    auto j = nlohmann::json::parse(json_str);

    // Build new diagram
    auto new_diagram = std::make_unique<Diagram>();

    for (const auto& jn : j.value("nodes", nlohmann::json::array())) {
      Node node;
      node.id = jn.value("id", "");
      if (node.id.empty()) continue; // ← Silent skip!

      node.op = jn.value("op", "");
      node.irreversible = jn.value("irreversible", false);

      // Parse inputs array
      if (jn.contains("inputs") && jn["inputs"].is_array()) {
        for (const auto& inp : jn["inputs"]) {
          node.inputs.push_back(inp.get<std::string>());
        }
      }

      // Parse dof_refs array
      if (jn.contains("dof_refs") && jn["dof_refs"].is_array()) {
        for (const auto& dof : jn["dof_refs"]) {
          node.dof_refs.push_back(dof.get<std::string>());
        }
      }

      new_diagram->add_node(node);
    }

    // Similar for edges...

    diagram_ = std::move(new_diagram);
    return true;

  } catch (const std::exception& e) {
    last_rewrite_message_ = "Diagram parse error: " + std::string(e.what());
    return false;
  }
}
```

**Failure Modes**:
- JSON parse exception → "Diagram parse error: ..."
- Schema mismatch → "Diagram JSON must be an object"
- **Silent failures**: Empty node/edge IDs are skipped without error
- Possible exceptions in `add_node()` or `add_edge()` not being caught

### Package/Multi-Diagram Support

Command router supports `package` parameter for batch operations:
```json
{
  "command": "sid_set_diagram_json",
  "params": {
    "engine_id": "...",
    "package": {
      "diagrams": [{id, nodes, edges}, ...],
      "rules": [...]
    },
    "diagram_id": "optional_selector"
  }
}
```

### Expression-Based Ingress (Alternative)

Alternative to JSON: `sid_set_diagram_expr`
- Input: Expression string like `"C(P(Freedom), O(P(Choice)))"`
- Parser: `sid_parser_impl.hpp` (recursive descent)
- Builder: `sid_diagram_builder.hpp` (AST → Diagram)
- ID generation: IDGenerator for stable node/edge IDs
- **This path might work as workaround for JSON load issues**

---

## BUILD SYSTEM DETAILS

### CMake Configuration

**Root**: `Simulation/CMakeLists.txt`

#### Critical Bug (Lines 105-108)
```cmake
set(DASE_COMPILE_FLAGS_MSVC
  /EHsc /bigobj /std:c++17
  /O2 /Ob3 /Oi /Ot      # ← Optimization flags
  /fp:fast /GL           # ← Link-time optimization
  /DNOMINMAX /openmp /MD # ← Multi-threaded DLL
)
```

**Problem**: These flags are applied unconditionally to ALL build types.

**MSVC Debug Default**: Adds `/RTC1` (runtime error checks) automatically.

**Conflict**: `/O2` (optimize for speed) + `/RTC1` (runtime checks) are mutually exclusive.

**MSVC Compiler Error**:
```
C1189: #error: /RTC1 and /O2 are incompatible
```

**Affected Targets**:
- dase_cli
- sid_ssp_capi
- dase_core
- analysis_integration
- igsoa_gw_core
- All transitive dependencies

### Build Artifacts Status

**Working Binaries** (`Simulation/bin/`):
- `dase_cli.exe` (860KB, Jan 20) - Release build, **working CLI**
- `sid_cli.exe` (356KB, Jan 20) - SID-only wrapper

**Debug Build** (`build/Debug/`):
- `dase_cli.exe` (916KB, Jan 23) - **Inconsistent flags, unreliable**
- `dase_step_runner.exe` (889KB, Jan 23) - Built today
- Mixed .obj files from different build configurations
- **Not recommended for use**

**Release Build** (`build/Release/`):
- Likely clean if rebuilt
- Would have consistent optimization flags

### Problem Timeline (from `problem.txt`)

1. Added Step 5a fixtures (scenario_overlap, scenario_multiredeux)
2. Attempted to load diagrams via `sid_set_diagram_json`
3. Command returns `EXECUTION_FAILED`, `last_rewrite_message` empty
4. Metrics all zero (diagram not loaded)
5. Tried minimal diagram `{id, nodes:[{op:P,dof_refs:[]}], edges:[]}` - also failed
6. Copied `Simulation/dase_cli` contents to `build/Debug` - same failure
7. Attempted rebuild: `cmake --build build --config Debug`
8. Build timeout due to `/O2` and `/RTC1` incompatibility errors
9. Prior deletion of `build/Debug` now prevents successful rebuild

---

## RECOMMENDED SOLUTIONS

### Option A: Fix Build System (Recommended - Clean Solution)

#### 1. Fix CMakeLists.txt

**File**: `Simulation/CMakeLists.txt:105-108`

**Current**:
```cmake
set(DASE_COMPILE_FLAGS_MSVC
  /EHsc /bigobj /std:c++17 /O2 /Ob3 /Oi /Ot
  /fp:fast /GL /DNOMINMAX /openmp /MD
)
```

**Fixed**:
```cmake
# Conditional flags based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(DASE_COMPILE_FLAGS_MSVC
    /EHsc /bigobj /std:c++17
    /Od                    # No optimization for Debug
    /MDd                   # Debug runtime
    /DNOMINMAX
  )
else()
  set(DASE_COMPILE_FLAGS_MSVC
    /EHsc /bigobj /std:c++17
    /O2 /Ob3 /Oi /Ot      # Optimize for Release
    /fp:fast /GL           # Link-time optimization
    /DNOMINMAX /openmp /MD # Multi-threaded DLL
  )
endif()
```

#### 2. Clean Rebuild

```bash
# Remove inconsistent artifacts
rm -rf build

# Configure Release build
cmake -S Simulation -B build_release -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build_release --config Release

# Use working binary
build_release/dase_cli/Release/dase_cli.exe
```

#### 3. Test Step 5a

```bash
# Run via test harness with fixed binary
python scripts/test_cli_harness.py --binary build_release/dase_cli/Release/dase_cli.exe
```

### Option B: Debug Diagram Load Failure

Add instrumentation to isolate the failure point.

**File**: `Simulation/src/cpp/sid_ternary_engine.hpp:303-355`

```cpp
bool setDiagramJson(const std::string& json_str) {
  try {
    std::cerr << "[DEBUG] Starting JSON parse...\n";
    auto j = nlohmann::json::parse(json_str);
    std::cerr << "[DEBUG] JSON parsed successfully\n";
    std::cerr << "[DEBUG] Diagram ID: " << j.value("id", "MISSING") << "\n";

    if (!j.is_object()) {
      last_rewrite_message_ = "Diagram JSON must be an object";
      std::cerr << "[DEBUG] ERROR: Not an object\n";
      return false;
    }

    auto new_diagram = std::make_unique<Diagram>();
    std::cerr << "[DEBUG] Created new diagram\n";

    int node_count = 0;
    for (const auto& jn : j.value("nodes", nlohmann::json::array())) {
      std::string node_id = jn.value("id", "");
      std::cerr << "[DEBUG] Processing node " << node_count << ": " << node_id << "\n";

      if (node_id.empty()) {
        std::cerr << "[DEBUG] Skipping node with empty ID\n";
        continue;
      }

      Node node;
      node.id = node_id;
      node.op = jn.value("op", "");
      std::cerr << "[DEBUG] Node op: " << node.op << "\n";

      // ... rest of parsing ...

      std::cerr << "[DEBUG] Adding node to diagram...\n";
      new_diagram->add_node(node);
      std::cerr << "[DEBUG] Node added successfully\n";
      node_count++;
    }

    std::cerr << "[DEBUG] Total nodes added: " << node_count << "\n";

    // Similar for edges...

    std::cerr << "[DEBUG] Moving diagram to engine...\n";
    diagram_ = std::move(new_diagram);
    std::cerr << "[DEBUG] Diagram set successfully\n";
    return true;

  } catch (const std::exception& e) {
    std::cerr << "[DEBUG] EXCEPTION CAUGHT: " << e.what() << "\n";
    last_rewrite_message_ = "Diagram parse error: " + std::string(e.what());
    return false;
  }
}
```

**Then**:
1. Rebuild with instrumentation
2. Run CLI with stderr capture: `dase_cli.exe 2>debug.log`
3. Examine `debug.log` to identify exact failure point

### Option C: Workaround with Expression Syntax

Bypass JSON parsing entirely by using expression-based diagram loading.

**Instead of**:
```json
{
  "command": "sid_set_diagram_json",
  "params": {
    "engine_id": "sid_001",
    "diagram": {
      "id": "test",
      "nodes": [...],
      "edges": [...]
    }
  }
}
```

**Use**:
```json
{
  "command": "sid_set_diagram_expr",
  "params": {
    "engine_id": "sid_001",
    "expr": "S+(P(a),P(b))",
    "rule_id": "init"
  }
}
```

**Advantages**:
- Bypasses JSON diagram structure entirely
- Uses proven parser (sid_parser_impl.hpp)
- Generates stable IDs via IDGenerator
- Same internal Diagram representation

**Disadvantages**:
- Requires translating Step 5a fixtures from JSON to expression syntax
- Less flexible for complex diagrams with metadata

### Option D: Use Working Release Binary + Fix Fixtures

If the issue is in the fixture JSON format:

1. Use existing working binary: `Simulation/bin/dase_cli.exe`
2. Validate fixture JSON against schema
3. Test with minimal diagram first
4. Incrementally add complexity

**Minimal Test**:
```json
{
  "command": "sid_set_diagram_json",
  "params": {
    "engine_id": "sid_001",
    "diagram": {
      "id": "minimal_test",
      "nodes": [
        {"id": "n1", "op": "P", "dof_refs": ["a"]}
      ],
      "edges": []
    }
  }
}
```

---

## STEP 5A VALIDATION SPECIFICATION

### Objective

Prove that **final semantic outcome** is invariant under **admissible rewrite orderings**.

### Acceptance Criteria

For each scenario, running with P1/P2/P3 policies must produce:

1. **Termination**: All policies terminate before `horizon_cap` with `termination: "fixed_point"`
2. **Conservation**: Identical metrics across all policies:
   - `I_mass` (within epsilon)
   - `N_mass` (within epsilon)
   - `U_mass` (within epsilon)
   - `total_mass` (within epsilon)
3. **Semantic Equivalence**: Final diagram canonically equivalent
4. **Trace Divergence Allowed**: `steps` and `applied_trace` may differ

### Failure Modes

| Failure | Symptom | Likely Cause | Action |
|---------|---------|--------------|--------|
| **F1: Canon mismatch only** | Invariants match, diagram structure differs | Canonicalization too strict/unstable | Fix serialization/sorting |
| **F2: Invariants drift** | Mass totals differ across paths | Conservation enforcement missing | Add invariant checks at each step |
| **F3: Different fixed points** | Both canon + invariants differ | Rewrite system not confluent | Define admissibility constraints |
| **F4: Non-termination** | Hits horizon_cap | Cycles or oscillations | Add cycle detection |
| **F5: Order-dependent eligibility** | Eligible set differs too much | Matching logic depends on iteration order | Stabilize match enumeration |

### Test Matrix

**Minimum Test Set**:
- Scenarios: 3 (scenario_01, scenario_overlap, scenario_multiredeux)
- Policies: 3 (P1, P2, P3)
- Seeds (P3 only): 3
- Total runs: 2×3 + 1×9 = **15 runs**

**Expected Results** (from `step5ab.txt`):

#### scenario_overlap
```
P1: R1→R3 (2 steps) → T(P(a))
P2: R1→R3 (2 steps) → T(P(a))
P3: R2→R4 or R1→R3 (2 steps) → T(P(a))

All: steps=2, rules_applied=2, termination=fixed_point
     Final state: T(P(a))
     Conservation: identical
```

#### scenario_multiredeux
```
P1 (outer-first): 2-3 steps → T(P(a))
P2 (inner-first): 3-4 steps → T(P(a))
P3 (mixed): 2-4 steps → T(P(a))

All: steps>1, rules_applied>1, termination=fixed_point
     Final state: T(P(a))
     Conservation: identical
     Traces: divergent (expected)
```

---

## KEY FILES REFERENCE

### CLI Core
- `Simulation/dase_cli/src/main.cpp:1-110` - Entry point, JSONL protocol
- `Simulation/dase_cli/src/command_router.cpp:1203-1320` - Policy execution, rewrite loop
- `Simulation/dase_cli/src/command_router.cpp:1438-1461` - JSON validation (Stage 1)
- `Simulation/dase_cli/src/engine_manager.cpp` - SID engine lifecycle management

### Engine Core
- `Simulation/src/cpp/sid_ternary_engine.hpp:303-355` - **setDiagramJson (FAILURE POINT)**
- `Simulation/src/cpp/sid_ternary_engine.hpp:150-200` - applyRewrite, step, collapse
- `Simulation/src/cpp/sid_ssp/sid_capi.cpp:74-145` - C API boundary
- `Simulation/src/cpp/sid_ssp/sid_capi.cpp:130` - **sid_last_rewrite_message (static buffer issue)**

### Data Structures
- `Simulation/src/cpp/sid_ssp/sid_diagram.hpp:15-82` - Node/Edge structures
- `Simulation/src/cpp/sid_ssp/sid_diagram.hpp:100-200` - Diagram class (add_node, add_edge, has_cycle)
- `Simulation/src/cpp/sid_ssp/sid_rewrite.hpp` - Pattern matcher, rewrite application

### Parsing & Building
- `Simulation/src/cpp/sid_ssp/sid_parser_impl.hpp` - Recursive descent expression parser
- `Simulation/src/cpp/sid_ssp/sid_diagram_builder.hpp` - AST to Diagram conversion
- `Simulation/src/cpp/sid_ssp/sid_id_generator.hpp` - Stable ID generation

### Build System
- `Simulation/CMakeLists.txt:105-108` - **BUG: Unconditional /O2 flags**
- `Simulation/dase_cli/CMakeLists.txt` - CLI executable configuration
- `Simulation/tests/harness/CMakeLists.txt` - GTest suite configuration

### Test Infrastructure
- `Simulation/tests/harness/harness_sid_rewrite.cpp` - **Placeholder (GTEST_SKIP)**
- `Simulation/tests/harness/harness_policy.cpp` - Policy validation loader
- `Simulation/tests/engine_api/sid_step_runner.cpp` - CLI subprocess runner
- `scripts/test_cli_harness.py` - Python black-box test framework

### Fixtures
- `fixtures/step5a/scenario_overlap/diagram.json` - Overlapping rewrites test
- `fixtures/step5a/scenario_overlap/rules.json` - R1/R2 overlap, R3/R4 converge
- `fixtures/step5a/scenario_multiredeux/diagram.json` - Multi-redex ordering test
- `fixtures/step5a/scenario_multiredeux/rules.json` - Same rules, nested diagram

### Build Artifacts
- `Simulation/bin/dase_cli.exe` (860KB, Jan 20) - **Working Release binary**
- `Simulation/bin/sid_cli.exe` (356KB, Jan 20) - SID-only wrapper
- `build/Debug/dase_cli.exe` (916KB, Jan 23) - **Inconsistent, unreliable**
- `build/Release/` - Clean Release build (if rebuilt)

### Documentation
- `problem.txt` - Current blocker description
- `step5ab.txt` - Step 5a fixture specifications
- `step5a.txt` - Original Step 5a requirements
- `resume.txt` - Step 5a resume point after ingress fixes

---

## NEXT ACTIONS

### Immediate (Critical Path)

1. **Fix Build System** (30 minutes)
   - Edit `Simulation/CMakeLists.txt:105-108`
   - Add conditional flags for Debug vs Release
   - Rebuild Release configuration

2. **Validate Diagram Load** (1 hour)
   - Test minimal diagram with working Release binary
   - Add debug instrumentation if needed
   - Identify exact failure point in setDiagramJson

3. **Unblock Step 5a** (2 hours)
   - Either: Fix JSON loading
   - Or: Use expression-based workaround
   - Validate one scenario end-to-end (scenario_overlap)

### Short-term (Week 1)

4. **Complete Step 5a Validation** (1 day)
   - Run all 3 scenarios with P1/P2/P3
   - Verify conservation + semantic equivalence
   - Document results in validation ledger

5. **Implement harness_sid_rewrite.cpp** (1 day)
   - Remove GTEST_SKIP placeholder
   - Add policy comparison tests
   - Integrate with GTest suite

### Medium-term (Week 2-3)

6. **Canonicalization & Witness** (3 days)
   - Implement canonical diagram serialization
   - Add invariant vector computation
   - Generate golden witnesses

7. **Step 5b Planning** (1 day)
   - Define next validation phase
   - Identify additional scenarios
   - Plan P4/P5 policy implementations

---

## ARCHITECTURAL STRENGTHS

1. **Clean Separation of Concerns**
   - CLI ↔ C API ↔ C++ Engine boundaries well-defined
   - Command router decouples protocol from logic
   - Opaque handles prevent memory management issues

2. **Policy Abstraction**
   - Rewrite scheduling cleanly separated from pattern matching
   - Easy to add new policies (P4/P5) without engine changes
   - Deterministic testing via seeded randomness

3. **Conservation Tracking**
   - I/N/U mass verified at each step
   - Exact arithmetic (no epsilon for conservation in SID)
   - Metrics exposed via clean API

4. **Fixture Design**
   - JSON schema supports complex scenarios
   - Multi-redex testing enabled
   - Clear separation of diagram/rules/params

5. **Expression Parser Alternative**
   - Proven fallback for diagram loading
   - Stable ID generation
   - Same internal representation

---

## ARCHITECTURAL WEAKNESSES

1. **Static Buffer in C API**
   - `sid_last_rewrite_message()` uses static buffer
   - Thread-unsafe (not critical for CLI, but risky)
   - Potential for corruption in certain builds

2. **Silent Failures**
   - Empty node/edge IDs silently skipped
   - No validation of operator types (P, S+, etc.)
   - Missing cycle detection during load

3. **Build System**
   - Unconditional optimization flags cause Debug build failure
   - No separation of Debug/Release compiler flags
   - Mixed artifacts from failed builds

4. **Test Coverage Gaps**
   - `harness_sid_rewrite.cpp` is placeholder
   - No automated P1/P2/P3 comparison yet
   - Missing golden witness generation

5. **Error Propagation**
   - Exception messages may be lost at C API boundary
   - Empty `last_rewrite_message` on failure
   - No structured error codes

---

## CONCLUSION

The SID test harness and simulation architecture is **fundamentally sound** with clean separation of concerns and robust policy abstraction. The current blocker is a **trivial build system bug** combined with a **diagram load failure** that is either:

1. A fixture JSON format issue (fixable by validation)
2. A C++ exception being swallowed (fixable by instrumentation)
3. A build artifact inconsistency (fixable by clean rebuild)

**Recommended immediate action**: Fix the CMakeLists.txt flags, do a clean Release build, and test with the working binary. If diagram load still fails, add debug instrumentation to isolate the exact failure point.

Once unblocked, Step 5a validation should proceed smoothly as all the infrastructure is in place and the fixture design is solid.

---

**Report Generated**: 2026-01-23
**Next Review**: After CMakeLists.txt fix and clean rebuild
**Status**: BLOCKED → FIX IN PROGRESS
