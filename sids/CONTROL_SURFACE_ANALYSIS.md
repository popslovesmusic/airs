# Control Surface Analysis
**Date:** 2026-01-17
**Purpose:** Document all user-facing interfaces and control points

---

## Executive Summary

**Control Surface Type:** Command-Line Interface (CLI) Tools
**Exposure Level:** Local System Only
**Attack Surface:** Minimal (no network exposure)

The SID framework provides **5 CLI tools** and **7 Python modules** for programmatic access. All interfaces are local, with no network services, web APIs, or remote access capabilities.

---

## 1. CLI Tools (Primary Control Surface)

### Entry Points

All CLI tools follow the pattern:
```bash
python <tool_name>.py <arguments>
```

User controls the entire execution environment (working directory, file paths, arguments).

---

### 1.1 sid_ast_cli.py - Expression Parser

**Purpose:** Parse SID expressions into AST or diagram format

**Command:**
```bash
python sid_ast_cli.py "C(P(Freedom), O(Choice))" --mode diagram
```

**Input Surface:**
| Parameter | Type | Required | Validation | User Control |
|-----------|------|----------|------------|--------------|
| `expression` | String | Yes | Regex parser | Full - any string |
| `--mode` | Choice | No | `ast` or `diagram` | Limited - 2 choices |
| `--diagram-id` | String | No | None | Full - any string |
| `--compartment-id` | String | No | None | Full - any string |

**Output:**
- JSON to stdout (safe, no file writes)

**Attack Vectors:**
- ✅ Malformed expressions → Caught by parser (ParseError)
- ✅ Very long expressions → Limited by Python memory (no explicit limit)
- ✅ Special characters → Regex validates, only alphanumeric + operators allowed

**Risk:** **VERY LOW** - Read-only operation, no file access

---

### 1.2 sid_validate_cli.py - Package Validator

**Purpose:** Validate SID package files

**Command:**
```bash
python sid_validate_cli.py package.json
```

**Input Surface:**
| Parameter | Type | Required | Validation | User Control |
|-----------|------|----------|------------|--------------|
| `input` | File path | Yes | None | Full - any path |

**File Operations:**
- **Read:** Opens JSON file at user-specified path
- **Write:** None

**Attack Vectors:**
- ✅ Path traversal (read) → User can read any file they have permissions for
  - **Risk:** LOW - user controls their system
  - **Mitigation:** Not needed (CLI tool, local use)
- ✅ Malformed JSON → Caught by `json.load()` (raises JSONDecodeError)
- ✅ Large files → Limited by Python memory
- ✅ Malicious JSON content → Validated by `validate_package()`, no code execution

**Risk:** **LOW** - Read-only, user controls paths

---

### 1.3 sid_rewrite_cli.py - Rewrite Engine

**Purpose:** Apply rewrite rules to diagrams

**Command:**
```bash
python sid_rewrite_cli.py input.json state_0 d_initial -o output.json
```

**Input Surface:**
| Parameter | Type | Required | Validation | User Control |
|-----------|------|----------|------------|--------------|
| `input` | File path | Yes | None | Full - any path |
| `state_id` | String | Yes | None | Full - any string |
| `diagram_id` | String | Yes | None | Full - any string |
| `-o/--output` | File path | No | None | Full - any path |

**File Operations:**
- **Read:** Opens JSON file at user-specified path
- **Write:** Writes JSON to output path (defaults to overwriting input)

**Attack Vectors:**
- ✅ Path traversal (read) → User can read any accessible file
- ✅ Path traversal (write) → ⚠️ User can write to any path with permissions
  - **Risk:** LOW - user controls their system, would only harm themselves
  - **Example:** `-o ../../../../etc/passwd` (would fail on permissions)
  - **Mitigation:** Optional path validation (see SECURITY_CODE_REVIEW.md)
- ✅ Malformed JSON → Caught by parser
- ✅ Invalid state/diagram IDs → Validation returns error, no crash

**Risk:** **LOW-MEDIUM** - Can write files, but user-controlled environment

---

### 1.4 sid_pipeline_cli.py - Full Pipeline

**Purpose:** Validate → Rewrite → Re-validate in one step

**Command:**
```bash
python sid_pipeline_cli.py input.json state_0 d_initial -o output.json
```

**Input Surface:**
| Parameter | Type | Required | Validation | User Control |
|-----------|------|----------|------------|--------------|
| `input` | File path | Yes | None | Full - any path |
| `state_id` | String | Yes | None | Full - any string |
| `diagram_id` | String | Yes | None | Full - any string |
| `-o/--output` | File path | No | None | Full - any path |

**File Operations:**
- **Read:** Opens JSON file at user-specified path
- **Write:** Writes JSON to output path

**Attack Vectors:**
- Same as `sid_rewrite_cli.py` (combines validate + rewrite)

**Risk:** **LOW-MEDIUM** - Same as rewrite tool

---

### 1.5 sid_stability_cli.py - Stability Checker

**Purpose:** Check structural stability and compute metrics

**Command:**
```bash
python sid_stability_cli.py package.json state_0 d_initial --metrics
```

**Input Surface:**
| Parameter | Type | Required | Validation | User Control |
|-----------|------|----------|------------|--------------|
| `input` | File path | Yes | None | Full - any path |
| `state_id` | String | Yes | None | Full - any string |
| `diagram_id` | String | Yes | None | Full - any string |
| `--tolerance` | Float | No | Must be > 0 | Limited - positive float |
| `--metrics` | Flag | No | Boolean | Limited - on/off |

**File Operations:**
- **Read:** Opens JSON file at user-specified path
- **Write:** None

**Output:**
- Text to stdout (safe)

**Attack Vectors:**
- ✅ Path traversal (read) → User can read any accessible file
- ✅ Malformed JSON → Caught by parser
- ✅ Invalid tolerance → argparse validates type

**Risk:** **VERY LOW** - Read-only operation

---

### 1.6 sid_stress_cli.py - Stress Tester

**Purpose:** Run randomized stress tests

**Command:**
```bash
python sid_stress_cli.py --runs 200 --seed 1234 --report output.json
```

**Input Surface:**
| Parameter | Type | Required | Validation | User Control |
|-----------|------|----------|------------|--------------|
| `--runs` | Integer | No | Positive int | Limited - positive number |
| `--seed` | Integer | No | Any int | Full - any integer |
| `--report` | File path | No | None | Full - any path |

**File Operations:**
- **Read:** None (generates test data)
- **Write:** Writes JSON report to specified path

**Attack Vectors:**
- ✅ Path traversal (write) → ⚠️ User can write to any path with permissions
- ✅ Large run count → Could consume CPU/memory (user controls, would DOS themselves)

**Risk:** **LOW** - Write-only, user-controlled

---

## 2. Programmatic API (Secondary Control Surface)

### Python Module Imports

Users can import and use modules directly:

```python
from sid_parser import parse_expression
from sid_validator import validate_package
from sid_rewrite import apply_rewrites_to_package
from sid_crf import authorize_rewrite, assign_inu_labels
from sid_stability import is_structurally_stable
```

---

### 2.1 sid_parser Module

**Function:** `parse_expression(text: str) -> Expr`

**Input:** String expression
**Output:** AST (Expr object)
**Raises:** `ParseError` on invalid input

**User Control:**
- Full control over input string
- No file access
- No code execution

**Attack Surface:**
- Malformed expressions → Exception raised
- Very long expressions → Memory limited

---

### 2.2 sid_validator Module

**Function:** `validate_package(pkg: dict) -> Tuple[List[str], List[str]]`

**Input:** Python dict (already parsed JSON)
**Output:** (errors, warnings) tuple

**User Control:**
- Full control over package structure
- No file access in this function (caller handles files)

**Attack Surface:**
- Malformed package → Returns errors (doesn't crash)
- Invalid references → Validation catches

---

### 2.3 sid_rewrite Module

**Function:** `apply_rewrites_to_package(pkg: dict, state_id: str, diagram_id: str) -> RewriteResult`

**Input:**
- Package dict
- State ID string
- Diagram ID string

**Output:** RewriteResult with modified diagram

**User Control:**
- Full control over package structure
- Controls which state/diagram to rewrite

**Attack Surface:**
- Invalid IDs → Returns error result
- Malicious rewrite rules → ✅ Validated by CRF authorization
- Infinite loops → ✅ Protected by iteration limit (1000)
- Cycles → ✅ Detected and rejected

---

### 2.4 sid_crf Module

**Function:** `authorize_rewrite(constraints, state, diagram, csi, rewrite_rule) -> Tuple[bool, List[str], List[str]]`

**Input:** Multiple dicts and lists
**Output:** (authorized, errors, warnings)

**User Control:**
- Full control over constraints, state, diagram
- Controls authorization logic via package structure

**Attack Surface:**
- Malformed constraints → Validation errors returned
- No code execution
- Pure function (no side effects)

---

### 2.5 sid_stability Module

**Function:** `is_structurally_stable(pkg, state_id, diagram_id, tolerance=1e-6, require_all=False) -> Tuple[bool, List[str], str]`

**Input:**
- Package dict
- State/diagram IDs
- Tolerance float
- Boolean flag

**Output:** (is_stable, satisfied_conditions, message)

**User Control:**
- Full control over inputs
- Read-only analysis

**Attack Surface:**
- Invalid inputs → Returns False with error message
- No file access
- No side effects

---

## 3. File System Interface

### Read Operations

**Locations:**
- `sid_validator.py:load_package(path)`
- `sid_validate_cli.py:load_json(path)`
- `sid_rewrite_cli.py:load_json(path)`
- `sid_pipeline_cli.py:load_json(path)`

**Pattern:**
```python
def load_package(path: str) -> dict:
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)
```

**User Control:** Full - can specify any path

**Validation:** None (relies on OS permissions)

**Risk Assessment:**
- ✅ User can read any file they have permissions for
- ✅ This is expected behavior for a CLI tool
- ✅ No elevation of privileges
- ✅ No reading without user's explicit request

**Verdict:** **ACCEPTABLE** - User controls their own file system

---

### Write Operations

**Locations:**
- `sid_rewrite_cli.py:write_json(path, data)`
- `sid_pipeline_cli.py:write_json(path, data)`
- `sid_stress_cli.py` (report writing)

**Pattern:**
```python
def write_json(path: str, data: dict) -> None:
    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, sort_keys=True)
        f.write("\n")
```

**User Control:** Full - can specify any path

**Validation:** None (relies on OS permissions)

**Potential Issues:**
- ⚠️ Path traversal: User could specify `../../../../etc/passwd`
  - **Mitigation:** OS permissions prevent unauthorized writes
  - **Risk:** User can only harm their own files
  - **Severity:** LOW (user-controlled environment)

**Verdict:** **ACCEPTABLE with optional enhancement**

---

## 4. Data Flow Analysis

### Input → Processing → Output

```
┌─────────────────────────────────────────────────────────────┐
│                      CONTROL SURFACE                         │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────┐     ┌──────────┐     ┌─────────────────┐
│   CLI Tools     │────▶│  Parser  │────▶│   Validator     │
│  (User Input)   │     │          │     │                 │
└─────────────────┘     └──────────┘     └─────────────────┘
        │                                          │
        │                                          ▼
        │                                 ┌─────────────────┐
        │                                 │  CRF/Authorize  │
        │                                 │                 │
        │                                 └─────────────────┘
        │                                          │
        ▼                                          ▼
┌─────────────────┐                      ┌─────────────────┐
│  File System    │                      │  Rewrite Engine │
│  (Read/Write)   │◀─────────────────────│                 │
└─────────────────┘                      └─────────────────┘
```

**Trust Boundaries:**
1. **User Input → Parser:** Untrusted input, validated by parser
2. **Parser → Validator:** Trusted AST, validated structure
3. **Validator → CRF:** Trusted structure, checked constraints
4. **CRF → Rewrite:** Authorized operations only
5. **Rewrite → File System:** Trusted data, user-controlled paths

---

## 5. Attack Surface Summary

### External Attack Surface

**Network Exposure:** ✅ **NONE**
- No network services
- No listening sockets
- No HTTP/REST APIs
- No remote access

**File System Exposure:** ⚠️ **LOCAL ONLY**
- Read: User-specified paths (CLI tool, expected)
- Write: User-specified paths (⚠️ minor risk)

**Process Execution:** ✅ **NONE**
- No subprocess calls
- No shell command execution
- No `eval()`/`exec()`

---

### Internal Attack Surface

**Memory Safety:** ✅ **PYTHON-MANAGED**
- No manual memory management
- No buffer overflows
- No use-after-free

**Resource Consumption:**
- ✅ Iteration limits (1000)
- ✅ Pattern match limits (1000)
- ✅ Loop history limits (100)
- ⚠️ No explicit JSON file size limit
- ⚠️ No explicit expression length limit

**Data Validation:**
- ✅ Parser validates syntax
- ✅ Validator validates structure
- ✅ CRF validates constraints
- ✅ Type hints enforce types

---

## 6. Control Points (Where Users Control Behavior)

### 6.1 Configuration Files

**Format:** JSON packages
**Location:** User-specified
**Contents:**
- DOFs (Degrees of Freedom)
- Compartments
- CSIs (Causal Spheres of Influence)
- Diagrams
- States
- Constraints
- Rewrite rules

**User Control:** **FULL**
- User creates entire package structure
- User defines all constraints
- User defines all rewrite rules
- User controls authorization logic

**Validation:**
- ✅ Structure validated by `validate_package()`
- ✅ References checked
- ✅ Constraints evaluated
- ✅ No code execution from package contents

---

### 6.2 Command-Line Arguments

**Controlled by:** User
**Validation:** argparse library
**Examples:**
```bash
--mode ast|diagram     # Validated by choices
--tolerance 1e-6       # Validated as float
--runs 200             # Validated as int
-o output.json         # ⚠️ No path validation
```

---

### 6.3 Expression Syntax

**Format:** SID expression language
**Example:** `C(P(Freedom), O(Choice))`

**Operators Allowed:**
- P, S+, S-, O, C, T (whitelist)

**Atoms Allowed:**
- `[A-Za-z_][A-Za-z0-9_]*` (alphanumeric + underscore)
- Optional `$` prefix for variables

**Validation:**
- ✅ Regex-based tokenization
- ✅ Operator whitelist
- ✅ Arity checking
- ✅ No code execution

**User Control:**
- User provides expression string
- Parser validates before processing
- Invalid expressions raise ParseError

---

## 7. Privilege Model

### Required Permissions

**To Run:**
- Python interpreter execution
- Read access to input files
- Write access to output directory (for tools that write)

**NOT Required:**
- Administrative/root privileges
- Network access
- Special system capabilities

**Privilege Escalation:**
- ✅ **NONE** - Tool runs with user's permissions
- ✅ No setuid/setgid
- ✅ No privilege elevation

---

## 8. Trust Model

### What Is Trusted

1. **Python Interpreter** - Assumed secure
2. **Standard Library** - json, re, argparse, etc.
3. **Operating System** - File permissions, memory management
4. **User** - Controls their own environment

### What Is NOT Trusted

1. **User Input** - Expressions validated by parser
2. **JSON Files** - Structure validated by validator
3. **Rewrite Rules** - Authorized by CRF

### Validation Layers

```
User Input
    ↓
[Layer 1: Parser] ← Syntax validation, operator whitelist
    ↓
[Layer 2: Validator] ← Structure validation, reference checking
    ↓
[Layer 3: CRF] ← Constraint validation, authorization
    ↓
[Layer 4: Rewrite Engine] ← Cycle detection, iteration limits
    ↓
Output
```

---

## 9. Control Surface Hardening

### Already Implemented ✅

1. **Input Validation**
   - Regex-based parser (no code execution)
   - Operator whitelist
   - Arity validation
   - Type checking

2. **Resource Limits**
   - Iteration limits (1000)
   - Pattern match limits (1000)
   - Loop history limits (100)

3. **Immutability**
   - Frozen dataclasses
   - Pure functions
   - Deep copying

4. **Error Handling**
   - Structured errors
   - No information leakage
   - Graceful degradation

### Optional Enhancements ⚠️

1. **Path Validation** (LOW priority)
   ```python
   def validate_output_path(path: str) -> str:
       abs_path = os.path.abspath(path)
       # Optional: restrict to current directory
       if not abs_path.startswith(os.getcwd()):
           raise ValueError("Output must be in current directory")
       return abs_path
   ```

2. **Input Size Limits** (LOW priority)
   ```python
   MAX_FILE_SIZE = 100 * 1024 * 1024  # 100MB
   MAX_EXPRESSION_LENGTH = 10000
   ```

3. **Logging** (OPTIONAL)
   ```python
   logger.info(f"Reading: {input_path}")
   logger.info(f"Writing: {output_path}")
   ```

---

## 10. Comparison with Other Systems

### CLI Tools (Similar Risk Profile)

| Tool | Network | File Access | Code Exec | Risk |
|------|---------|-------------|-----------|------|
| **SID Framework** | No | Read/Write | No | Very Low |
| git | No | Read/Write | No | Very Low |
| jq | No | Read/Write | No | Very Low |
| gcc | No | Read/Write | Yes* | Low |
| npm | Yes | Read/Write | Yes* | Medium |

*Code execution is the tool's purpose (compiler/package manager)

### Web Services (Different Risk Profile)

| Tool | Network | File Access | Code Exec | Risk |
|------|---------|-------------|-----------|------|
| **SID Framework** | No | Read/Write | No | Very Low |
| REST API | Yes | Varies | No | Medium |
| Web Server | Yes | Read/Write | Varies | Medium-High |
| Database | Yes | Read/Write | Varies | Medium-High |

**Conclusion:** SID framework has similar risk to standard CLI utilities like `git` or `jq`.

---

## 11. Deployment Scenarios

### Scenario 1: Local Development (Primary Use Case)

**Environment:** Developer's workstation
**User:** Developer
**Risk:** Very Low
- User controls entire environment
- No multi-user concerns
- No network exposure
- Standard CLI tool usage

**Recommendation:** ✅ No additional hardening needed

---

### Scenario 2: CI/CD Pipeline

**Environment:** Automated build/test system
**User:** CI system (non-interactive)
**Risk:** Low
- Controlled environment
- Trusted inputs (source control)
- No user interaction
- Isolated container/VM

**Recommendation:**
- ⚠️ Validate input file paths (prevent accidental overwrites)
- ✅ Use explicit output paths (avoid `-o` defaults)
- ✅ Set resource limits at container level

---

### Scenario 3: Shared Server (Multi-user)

**Environment:** Shared development server
**User:** Multiple users
**Risk:** Low-Medium
- Users can only access their own files (OS permissions)
- No privilege escalation
- Standard Unix file permissions apply

**Recommendation:**
- ⚠️ **Implement path validation** (restrict outputs to user's home directory)
- ⚠️ Set umask appropriately (file permissions)
- ✅ Use per-user directories

---

### Scenario 4: Web Service Wrapper (NOT RECOMMENDED)

**Environment:** HTTP API wrapping SID tools
**User:** Remote/untrusted
**Risk:** High
- Network exposure
- Untrusted input
- Potential DOS attacks
- Path traversal risk

**Recommendation:**
- ⚠️ **DO NOT** expose directly as web service
- ✅ If needed, implement full API gateway with:
  - Input sanitization
  - Path whitelisting
  - Rate limiting
  - Authentication
  - Resource quotas
  - Sandboxing (containers)

---

## 12. Control Surface Matrix

| Interface | Exposure | Input Type | Validation | File Access | Risk |
|-----------|----------|------------|------------|-------------|------|
| **sid_ast_cli** | Local | String | Regex parser | None | Very Low |
| **sid_validate_cli** | Local | File path | JSON schema | Read | Low |
| **sid_rewrite_cli** | Local | File path | Full validation | Read/Write | Low-Med |
| **sid_pipeline_cli** | Local | File path | Full validation | Read/Write | Low-Med |
| **sid_stability_cli** | Local | File path | Full validation | Read | Very Low |
| **sid_stress_cli** | Local | Integers | Type check | Write | Low |
| **Python API** | Local | Objects | Type hints | None* | Very Low |

*File access handled by caller

---

## 13. Recommendations

### For Current Use (CLI Tool)

**Status:** ✅ **PRODUCTION READY AS-IS**

The control surface is appropriate for a CLI tool:
- Minimal exposure (local only)
- Standard Unix permissions apply
- User controls their environment
- No privilege escalation

**Optional Enhancements:**
1. Path validation (LOW priority)
2. Input size limits (LOW priority)
3. Logging (OPTIONAL)

---

### For Future Use (If Network-Exposed)

**Status:** ⚠️ **REQUIRES ADDITIONAL HARDENING**

If exposing as a web service or API:
1. ✅ **Mandatory path validation** - Whitelist allowed directories
2. ✅ **Input sanitization** - Reject suspicious patterns
3. ✅ **Rate limiting** - Prevent DOS
4. ✅ **Authentication** - Verify users
5. ✅ **Sandboxing** - Containers/VMs
6. ✅ **Resource quotas** - CPU/memory limits
7. ✅ **Audit logging** - Track all operations

---

## Conclusion

### Control Surface Assessment

**Type:** Command-Line Interface
**Exposure:** Local Only
**Attack Surface:** Minimal
**Risk Level:** Very Low

The SID framework has a **well-defined, minimal control surface** appropriate for a CLI tool:

✅ **Strengths:**
- No network exposure
- Input validation at all layers
- Resource consumption limits
- Immutable data structures
- No code execution from user input
- Standard Unix security model

⚠️ **Minor Considerations:**
- Optional path validation for writes
- Optional input size limits
- Both LOW priority for CLI use

### Deployment Verdict

**✅ APPROVED FOR PRODUCTION** - CLI Tool Use
**⚠️ REQUIRES HARDENING** - If Network-Exposed (not current use case)

The control surface is **secure and appropriate** for the intended use case (local CLI tool). No changes required for production deployment.

---

*Control surface analysis completed: 2026-01-17*
*Framework Status: Production Ready - Minimal Attack Surface*
