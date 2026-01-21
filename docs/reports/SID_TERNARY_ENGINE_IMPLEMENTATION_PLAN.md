# SID Ternary Engine - Implementation Plan

**Date:** January 19, 2026
**Status:** Incomplete - Requires Implementation
**Priority:** MEDIUM (feature enhancement, not blocking)

---

## Executive Summary

The SID (Semantic Interaction Diagrams) ternary engine integration is **partially complete** - the API surface exists but the underlying implementation is missing. This document provides a complete roadmap to finish the integration.

**Current State:**
- ✅ API defined in engine_manager.h
- ✅ Commands wired in command_router.cpp
- ✅ C++ SID SSP runtime exists (Mixer, SSP, Diagram classes)
- ✅ Python SID tooling complete and production-ready
- ❌ Engine class not implemented
- ❌ C API functions not exported
- ❌ Integration layer incomplete

**Completion Estimate:** 8-16 hours of focused development

---

## What is SID?

**Semantic Interaction Diagrams** is a diagrammatic framework for modeling:
- Agency and goal emergence
- Constraint-governed systems
- I/N/U (Is/Not/Unknown) ternary logic
- Rewrite-based transformations

**Key Components:**
1. **Operators:** P (Projection), S+/S- (Polarity), O (Collapse), C (Coupling), T (Transport)
2. **I/N/U Fields:** Ternary state with mass conservation (I + N + U = C)
3. **Mixer:** Manages field evolution and stability detection
4. **CRF:** Constraint Resolution Framework with 6 resolution procedures
5. **Rewrite System:** Pattern-based diagram transformations

---

## Current Implementation Status

### ✅ What Exists

**1. Python Implementation (Complete)**
- Location: `D:\airs\sids\`
- Status: Production-ready, fully tested
- Components:
  - `sid_parser.py` - Expression parsing
  - `sid_ast_to_diagram.py` - AST to diagram conversion
  - `sid_rewrite.py` - Rewrite rules and application
  - `sid_stability.py` - Stability checking
  - `sid_validator.py` - Package validation
  - `sid_crf.py` - Constraint resolution
  - CLI tools for all operations

**2. C++ SID SSP Runtime (Partial)**
- Location: `D:\airs\Simulation\src\cpp\sid_ssp\`
- Status: Runtime kernel only (~30% of Python features)
- Components:
  - `sid_mixer.hpp` - Ternary field mixer (complete)
  - `sid_semantic_processor.hpp` - SSP field management (complete)
  - `sid_diagram.hpp` - Diagram representation (complete)
  - `sid_capi.hpp` - C API (basic exports only)
  - **Missing:** Parser, rewrite engine, CRF, validation

**3. API Surface (Complete)**
- Location: `D:\airs\Simulation\dase_cli\src\`
- Status: Defined but not implemented
- Functions:
  - `sidStep()` - Execute one mixer step
  - `sidCollapse()` - Force O (collapse) operation
  - `sidApplyRewrite()` - Apply rewrite rule
  - `sidSetDiagramExpr()` - Parse expression and set diagram
  - `sidSetDiagramJson()` - Load diagram from JSON
  - `sidGetDiagramJson()` - Export diagram to JSON
  - `getSidMetrics()` - Get I/N/U masses and metrics

**4. Command Handlers (Complete)**
- Location: `D:\airs\Simulation\dase_cli\src\command_router.cpp`
- Commands registered:
  - `sid_step`
  - `sid_collapse`
  - `sid_rewrite`
  - `sid_metrics`
  - `sid_set_diagram_expr`
  - `sid_set_diagram_json`
  - `sid_get_diagram_json`

### ❌ What's Missing

**Critical Components:**

1. **SID Ternary Engine Class**
   - No `SidTernaryEngine` header file exists
   - Needs to wrap Mixer + Diagram + metadata
   - Must manage I/N/U field arrays
   - Handle evolution and metrics

2. **Engine C API Functions**
   - `sid_create_engine()` - Create engine instance
   - `sid_destroy_engine()` - Cleanup
   - `sid_step()` - Evolution step
   - `sid_collapse()` - Collapse operation
   - `sid_apply_rewrite()` - Apply rewrite
   - `sid_set_diagram_expr()` - Parse and set
   - `sid_set_diagram_json()` - Load from JSON
   - `sid_get_diagram_json()` - Export to JSON
   - `sid_get_I_mass()`, `sid_get_N_mass()`, `sid_get_U_mass()` - Field queries
   - `sid_get_instantaneous_gain()` - Mixer metric
   - `sid_is_conserved()` - Conservation check
   - `sid_last_rewrite_applied()` - Rewrite status
   - `sid_last_rewrite_message()` - Rewrite message

3. **Expression Parser (C++)**
   - Python has full parser in `sid_parser.py`
   - C++ has stub in `sid_parser.hpp`
   - Needs to parse: `P(Freedom)`, `O(S+(Peace))`, etc.

4. **Rewrite Engine (C++)**
   - Python has full implementation in `sid_rewrite.py`
   - C++ has stub in `sid_rewrite.hpp`
   - Needs pattern matching and substitution

5. **Integration Layer**
   - Wire SID engine to EngineManager
   - Handle opaque void* properly
   - Manage lifetime correctly

---

## Implementation Approaches

### Approach 1: **Full C++ Implementation** (Complex, 40-80 hours)

**Pros:**
- Native performance
- No Python dependency
- Consistent with other engines

**Cons:**
- Large amount of work
- Duplicates working Python code
- Risk of bugs during port

**Steps:**
1. Implement expression parser in C++
2. Implement rewrite engine in C++
3. Create SidTernaryEngine class
4. Implement C API functions
5. Wire to EngineManager
6. Test thoroughly

**Recommendation:** ❌ **NOT RECOMMENDED** - Too much work for uncertain benefit

---

### Approach 2: **Python Bridge** (Medium, 16-24 hours)

**Pros:**
- Reuses working Python implementation
- Faster to implement
- Leverages existing tests

**Cons:**
- Python runtime dependency
- FFI overhead
- More complex deployment

**Steps:**
1. Create Python C API bridge (similar to python_bridge.cpp)
2. Wrap Python SID functions
3. Create thin C++ SidTernaryEngine wrapper
4. Implement C API functions (call Python)
5. Wire to EngineManager
6. Test integration

**Recommendation:** ⚠️ **VIABLE** - Good if Python already embedded

---

### Approach 3: **Minimal C++ Engine** (Recommended, 8-16 hours)

**Pros:**
- Focuses on core functionality
- Uses existing C++ Mixer/SSP
- No Python dependency
- Matches AIRS architecture

**Cons:**
- Limited features initially
- No parser/rewrite at first

**Steps:**
1. Create `SidTernaryEngine` class (basic)
2. Implement core C API functions
3. Support JSON diagram input only (no parser)
4. Support manual field initialization
5. Implement mixer evolution
6. Wire to EngineManager
7. **Later:** Add parser and rewrites if needed

**Recommendation:** ✅ **RECOMMENDED** - Pragmatic, achievable

---

## Recommended Implementation Plan

### Phase 1: Core Engine (4-6 hours)

**Goal:** Get basic SID engine working with JSON input

**Tasks:**

1. **Create `sid_ternary_engine.hpp`** (2 hours)
   ```cpp
   class SidTernaryEngine {
   private:
       sid::Mixer mixer_;
       sid::Diagram diagram_;
       std::vector<sid::SSP> ssp_fields_; // I, N, U
       size_t num_nodes_;
       double R_c_;

   public:
       SidTernaryEngine(size_t num_nodes, double total_mass);

       // Core operations
       void step(double alpha);
       void collapse(double alpha);

       // State access
       double getIMass() const;
       double getNMass() const;
       double getUMass() const;
       bool isConserved(double tolerance) const;

       // Diagram management
       bool setDiagramJson(const std::string& json);
       std::string getDiagramJson() const;

       // Metrics
       sid::MixerMetrics getMetrics() const;
   };
   ```

2. **Implement C API in `sid_capi.cpp`** (2 hours)
   ```cpp
   // Add to existing sid_capi.cpp

   // Engine handle (opaque)
   struct sid_engine {
       std::unique_ptr<SidTernaryEngine> engine;
   };

   extern "C" {
       sid_engine* sid_create_engine(uint64_t num_nodes, double total_mass);
       void sid_destroy_engine(sid_engine* eng);
       void sid_step(sid_engine* eng, double alpha);
       // ... etc
   }
   ```

3. **Update `sid_capi.hpp`** (30 min)
   - Add engine function declarations
   - Export `sid_engine` opaque handle type

4. **Wire to `engine_manager.cpp`** (1 hour)
   - Replace stub implementations
   - Cast void* to sid_engine*
   - Call C API functions

5. **Test basic functionality** (30 min)
   - Create engine
   - Initialize fields
   - Run steps
   - Check conservation

**Deliverable:** Working sid_ternary engine with JSON diagram support

---

### Phase 2: Metrics and Queries (2-3 hours)

**Goal:** Full metrics and state query support

**Tasks:**

1. **Implement all metric functions** (1 hour)
   - `sid_get_I_mass()`
   - `sid_get_N_mass()`
   - `sid_get_U_mass()`
   - `sid_get_instantaneous_gain()`
   - `sid_is_conserved()`

2. **Add state export** (1 hour)
   - `sid_get_diagram_json()` - full diagram export
   - Include node states, masses, metrics

3. **Test metrics** (1 hour)
   - Verify conservation
   - Check mixer metrics
   - Validate JSON output

**Deliverable:** Complete metrics and monitoring

---

### Phase 3: Expression Parser (Optional, 4-8 hours)

**Goal:** Support `sidSetDiagramExpr("P(Freedom)")`

**Decision Point:** Only implement if needed for workflows

**Tasks:**

1. **Port Python parser to C++** (3-4 hours)
   - From `sids/sid_parser.py`
   - Recursive descent parser
   - AST generation

2. **Implement AST to Diagram** (2-3 hours)
   - From `sids/sid_ast_to_diagram.py`
   - Node/edge generation
   - Type checking

3. **Wire to C API** (1 hour)
   - `sid_set_diagram_expr()`

**Alternative:** Call Python parser via bridge if Python embedded

**Deliverable:** Expression parsing support

---

### Phase 4: Rewrite System (Optional, 8-12 hours)

**Goal:** Support `sidApplyRewrite(pattern, replacement, rule_id)`

**Decision Point:** Only if diagram transformation workflows needed

**Tasks:**

1. **Port pattern matcher** (4-6 hours)
   - From `sids/sid_rewrite.py`
   - Subgraph isomorphism
   - Constraint checking

2. **Port rewrite application** (3-4 hours)
   - Node replacement
   - Edge rewiring
   - Mass redistribution

3. **Wire to C API** (1 hour)
   - `sid_apply_rewrite()`
   - Track last rewrite status

4. **Test rewrites** (2 hours)
   - Pattern matching
   - Application correctness
   - Conservation preservation

**Deliverable:** Full rewrite system

---

## Minimal Viable Implementation

**If you want the quickest path to a working engine:**

### Minimum Scope (4 hours)

1. **SidTernaryEngine class** - Basic wrapper around Mixer
2. **JSON diagram input only** - No parser
3. **Core evolution** - step() and collapse()
4. **Basic metrics** - I/N/U masses
5. **C API** - Essential functions only

**What You Get:**
- ✅ Engine creates and runs
- ✅ Fields evolve with mixer
- ✅ Conservation maintained
- ✅ Metrics available
- ✅ Listed in capabilities
- ❌ No expression parsing
- ❌ No rewrite system
- ❌ Manual JSON diagrams only

**Good For:**
- Proof of concept
- Testing integration
- Simple workflows

---

## Implementation Code Skeleton

### `sid_ternary_engine.hpp`

```cpp
#pragma once

#include "sid_mixer.hpp"
#include "sid_semantic_processor.hpp"
#include "sid_diagram.hpp"
#include <vector>
#include <string>
#include <memory>

namespace sid {

/**
 * SID Ternary Engine
 *
 * Manages I/N/U ternary fields with diagram-driven evolution
 */
class SidTernaryEngine {
private:
    Mixer mixer_;
    Diagram diagram_;

    // SSP fields for I, N, U compartments
    std::unique_ptr<SSP> ssp_I_;
    std::unique_ptr<SSP> ssp_N_;
    std::unique_ptr<SSP> ssp_U_;

    size_t num_nodes_;
    double total_mass_;

    // Rewrite tracking
    bool last_rewrite_applied_ = false;
    std::string last_rewrite_message_;

public:
    SidTernaryEngine(size_t num_nodes, double total_mass)
        : mixer_(total_mass)
        , num_nodes_(num_nodes)
        , total_mass_(total_mass)
    {
        // Create SSP fields
        ssp_I_ = std::make_unique<SSP>(
            SSP::Role::INITIAL,
            num_nodes,
            total_mass / 3.0  // Initial distribution
        );
        ssp_N_ = std::make_unique<SSP>(
            SSP::Role::EXCLUDED,
            num_nodes,
            total_mass / 3.0
        );
        ssp_U_ = std::make_unique<SSP>(
            SSP::Role::UNDECIDED,
            num_nodes,
            total_mass / 3.0
        );
    }

    // Evolution
    void step(double alpha) {
        mixer_.step(ssp_I_.get(), ssp_N_.get(), ssp_U_.get());

        // Commit field updates
        ssp_I_->commit_step();
        ssp_N_->commit_step();
        ssp_U_->commit_step();
    }

    void collapse(double alpha) {
        // Force collapse: move U -> I or N based on bias
        // TODO: Implement collapse logic
    }

    // Metrics
    double getIMass() const {
        double sum = 0.0;
        const double* field = ssp_I_->field();
        for (size_t i = 0; i < num_nodes_; ++i) {
            sum += field[i];
        }
        return sum;
    }

    double getNMass() const {
        double sum = 0.0;
        const double* field = ssp_N_->field();
        for (size_t i = 0; i < num_nodes_; ++i) {
            sum += field[i];
        }
        return sum;
    }

    double getUMass() const {
        double sum = 0.0;
        const double* field = ssp_U_->field();
        for (size_t i = 0; i < num_nodes_; ++i) {
            sum += field[i];
        }
        return sum;
    }

    bool isConserved(double tolerance) const {
        double total = getIMass() + getNMass() + getUMass();
        return std::abs(total - total_mass_) < tolerance;
    }

    double getInstantaneousGain() const {
        return mixer_.getMetrics().loop_gain;
    }

    // Diagram management
    bool setDiagramJson(const std::string& json_str) {
        // TODO: Parse JSON and update diagram_
        return false;
    }

    std::string getDiagramJson() const {
        // TODO: Serialize diagram_ to JSON
        return "{}";
    }

    // Rewrite tracking
    bool lastRewriteApplied() const {
        return last_rewrite_applied_;
    }

    std::string lastRewriteMessage() const {
        return last_rewrite_message_;
    }

    MixerMetrics getMetrics() const {
        return mixer_.getMetrics();
    }
};

} // namespace sid
```

### `sid_capi.cpp` (additions)

```cpp
#include "sid_capi.hpp"
#include "sid_ternary_engine.hpp"
#include <memory>

// Engine handle
struct sid_engine {
    std::unique_ptr<sid::SidTernaryEngine> engine;
};

extern "C" {

sid_engine* sid_create_engine(uint64_t num_nodes, double total_mass) {
    try {
        auto* eng = new sid_engine();
        eng->engine = std::make_unique<sid::SidTernaryEngine>(
            static_cast<size_t>(num_nodes),
            total_mass
        );
        return eng;
    } catch (...) {
        return nullptr;
    }
}

void sid_destroy_engine(sid_engine* eng) {
    delete eng;
}

void sid_step(sid_engine* eng, double alpha) {
    if (eng && eng->engine) {
        eng->engine->step(alpha);
    }
}

void sid_collapse(sid_engine* eng, double alpha) {
    if (eng && eng->engine) {
        eng->engine->collapse(alpha);
    }
}

double sid_get_I_mass(sid_engine* eng) {
    return (eng && eng->engine) ? eng->engine->getIMass() : 0.0;
}

double sid_get_N_mass(sid_engine* eng) {
    return (eng && eng->engine) ? eng->engine->getNMass() : 0.0;
}

double sid_get_U_mass(sid_engine* eng) {
    return (eng && eng->engine) ? eng->engine->getUMass() : 0.0;
}

double sid_get_instantaneous_gain(sid_engine* eng) {
    return (eng && eng->engine) ? eng->engine->getInstantaneousGain() : 0.0;
}

bool sid_is_conserved(sid_engine* eng, double tolerance) {
    return (eng && eng->engine) ? eng->engine->isConserved(tolerance) : false;
}

bool sid_last_rewrite_applied(sid_engine* eng) {
    return (eng && eng->engine) ? eng->engine->lastRewriteApplied() : false;
}

const char* sid_last_rewrite_message(sid_engine* eng) {
    if (eng && eng->engine) {
        static std::string msg_buffer;
        msg_buffer = eng->engine->lastRewriteMessage();
        return msg_buffer.c_str();
    }
    return "";
}

bool sid_apply_rewrite(sid_engine* eng, const char* pattern,
                       const char* replacement, const char* rule_id) {
    // TODO: Implement rewrite
    return false;
}

bool sid_set_diagram_expr(sid_engine* eng, const char* expr, const char* rule_id) {
    // TODO: Implement parser
    return false;
}

bool sid_set_diagram_json(sid_engine* eng, const char* json) {
    if (eng && eng->engine) {
        return eng->engine->setDiagramJson(json);
    }
    return false;
}

const char* sid_get_diagram_json(sid_engine* eng) {
    if (eng && eng->engine) {
        static std::string json_buffer;
        json_buffer = eng->engine->getDiagramJson();
        return json_buffer.c_str();
    }
    return "{}";
}

} // extern "C"
```

### `sid_capi.hpp` (additions)

```cpp
// Add to existing file

// Forward declare opaque handle
typedef struct sid_engine sid_engine;

#ifdef __cplusplus
extern "C" {
#endif

// Engine lifecycle
sid_engine* sid_create_engine(uint64_t num_nodes, double total_mass);
void sid_destroy_engine(sid_engine* eng);

// Evolution
void sid_step(sid_engine* eng, double alpha);
void sid_collapse(sid_engine* eng, double alpha);

// Metrics
double sid_get_I_mass(sid_engine* eng);
double sid_get_N_mass(sid_engine* eng);
double sid_get_U_mass(sid_engine* eng);
double sid_get_instantaneous_gain(sid_engine* eng);
bool sid_is_conserved(sid_engine* eng, double tolerance);

// Rewrite system
bool sid_apply_rewrite(sid_engine* eng, const char* pattern,
                       const char* replacement, const char* rule_id);
bool sid_last_rewrite_applied(sid_engine* eng);
const char* sid_last_rewrite_message(sid_engine* eng);

// Diagram management
bool sid_set_diagram_expr(sid_engine* eng, const char* expr, const char* rule_id);
bool sid_set_diagram_json(sid_engine* eng, const char* json);
const char* sid_get_diagram_json(sid_engine* eng);

#ifdef __cplusplus
}
#endif
```

### `engine_manager.cpp` (replace stubs)

```cpp
// In createEngine():
} else if (engine_type == "sid_ternary") {
    handle = sid_create_engine(
        static_cast<uint64_t>(num_nodes),
        R_c  // Use R_c as total mass
    );
    if (!handle) {
        return "";
    }

// In destroyEngine():
} else if (it->second->engine_type == "sid_ternary") {
    sid_destroy_engine(static_cast<sid_engine*>(it->second->engine_handle));
}

// In sidStep():
bool EngineManager::sidStep(const std::string& engine_id, double alpha) {
    auto* instance = getEngine(engine_id);
    if (!instance || !instance->engine_handle) {
        return false;
    }
    if (instance->engine_type != "sid_ternary") {
        return false;
    }
    sid_step(static_cast<sid_engine*>(instance->engine_handle), alpha);
    return true;
}

// ... similar for other functions
```

---

## Testing Strategy

### Unit Tests (C++)

1. **Test Mixer Evolution**
   - Initialize I/N/U fields
   - Run steps
   - Verify conservation

2. **Test Engine Creation**
   - Create engine
   - Verify initialization
   - Destroy cleanly

3. **Test Metrics**
   - Query masses
   - Check gain calculation
   - Verify conservation

### Integration Tests (CLI)

```bash
# Create engine
echo '{"command":"create_engine","engine_type":"sid_ternary","num_nodes":100,"R_c":1000.0}' | dase_cli.exe

# Get initial metrics
echo '{"command":"sid_metrics","engine_id":"engine_001"}' | dase_cli.exe

# Run evolution
echo '{"command":"sid_step","engine_id":"engine_001","alpha":0.1}' | dase_cli.exe

# Check conservation
echo '{"command":"sid_metrics","engine_id":"engine_001"}' | dase_cli.exe
```

---

## Recommendations

### For Quick Proof-of-Concept
**Use Approach 3 - Minimal C++ Engine (Phase 1 only)**
- 4-6 hours of work
- Working engine with JSON input
- No parser/rewrite initially

### For Full Feature Parity
**Use Approach 3 - Phases 1-4**
- 24-40 hours of work
- Complete feature set
- Matches Python capabilities

### For Production Deployment
**Use Approach 2 - Python Bridge** (if Python already embedded)
- Reuses tested Python code
- Faster to production
- Lower risk

---

## Next Steps

**Immediate Actions:**

1. **Decide Scope:**
   - Minimal (Phase 1 only)?
   - Full (Phases 1-4)?
   - Python bridge?

2. **Create Work Branch:**
   ```bash
   git checkout -b feature/sid-ternary-engine
   ```

3. **Start Phase 1:**
   - Create `sid_ternary_engine.hpp`
   - Implement C API
   - Wire to engine_manager

4. **Test Incrementally:**
   - Build after each component
   - Test via CLI
   - Verify metrics

**Questions to Answer:**

1. **Do you need expression parsing?** (Adds 4-8 hours)
2. **Do you need rewrite system?** (Adds 8-12 hours)
3. **Is Python embedded/available?** (Enables bridge approach)
4. **What's the primary use case?** (Determines scope)

---

## Conclusion

The SID ternary engine is **75% complete** in terms of architecture:
- ✅ API designed
- ✅ Commands wired
- ✅ Runtime kernel exists
- ❌ Integration layer missing

**Recommended Path:** Implement minimal C++ engine (Phase 1, 4-6 hours) to get working functionality, then decide on additional phases based on actual usage needs.

The Python implementation exists and is production-ready, so there's no urgency - the feature can be completed incrementally as workflows require it.

---

**Status:** Ready for implementation
**Blocker:** None - all dependencies exist
**Risk:** Low - well-defined scope

