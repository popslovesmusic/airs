# Engine Integration - Replacing Fake Engines

## Overview

Engines are created by **DASE CLI** (JSON-driven), not by SID directly.
- SID only attaches to existing engines
- SID never calls engine constructors
- DASE issues handles and shared buffers

## Correct Control Flow

```
AIRS
 └─ DASE (CLI, JSON-driven)
     └─ DSE (engine substrate)
         └─ SID
             ├─ SSP(I)
             ├─ SSP(N)
             ├─ SSP(U)
             └─ Mixer
```

## DASE CLI → Engine Creation (JSON)

### Example: `create_engines.json`

```json
{
  "command": "create_engines",
  "engine_set": "SID_ternary_v0",
  "engines": [
    {
      "id": "engine_I",
      "type": "igsoa_2d",
      "role": "I",
      "field_len": 128,
      "expose": {
        "field": true,
        "stability": true
      }
    },
    {
      "id": "engine_N",
      "type": "igsoa_2d",
      "role": "N",
      "field_len": 128,
      "expose": {
        "field": true,
        "stability": true
      }
    },
    {
      "id": "engine_U",
      "type": "igsoa_2d",
      "role": "U",
      "field_len": 128,
      "expose": {
        "field": true,
        "stability": true,
        "writable": true
      }
    }
  ]
}
```

## Buffer Binding Modes

### Mode 1: Engine Owns Buffer (Zero-Copy View)
Best if your `*_capi` already exposes: `const double* engine_field_ptr(engine*)`.

- Engine allocates & updates its field
- SSP just "points at it" read-only
- Mixer reads SSP view pointers

**Pros:**
- ✅ Zero copy
- ✅ No engine changes

**Cons:**
- ❗ SSP must support "external field pointer"

### Mode 2: SSP Owns Buffer (Engine Writes Into It)
Best if your engine can be configured to write into a caller-provided buffer.

- SSP allocates field
- Engine gets pointer and writes into it during step

**Pros:**
- ✅ Keeps SSP the canonical carrier

**Cons:**
- ❗ Requires engine API support (set/output buffer)

## Engine Adapter Interface (Generic)

Avoids guessing exact engine symbols. Implement glue once per engine family.

### Header: `sid_engine_adapter.h`

```c
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sid_engine sid_engine_t;

typedef struct {
    /* lifecycle */
    sid_engine_t* (*create)(uint64_t field_len);
    void         (*destroy)(sid_engine_t*);

    /* stepping */
    void (*step)(sid_engine_t*);

    /* field exposure (pick one mode) */
    const double* (*field_ro)(const sid_engine_t*);
    double*       (*field_rw)(sid_engine_t*); /* optional */

    /* optional health metric */
    double (*stability)(const sid_engine_t*);
} sid_engine_vtbl_t;

sid_engine_t* sid_engine_wrap(const sid_engine_vtbl_t* vtbl, uint64_t field_len);
void          sid_engine_unwrap(sid_engine_t*);

void          sid_engine_step(sid_engine_t*);
const double* sid_engine_field_ro(const sid_engine_t*);
double        sid_engine_stability(const sid_engine_t*);

#ifdef __cplusplus
}
#endif
```

### Implementation: `sid_engine_adapter.c`

```c
#include "sid_engine_adapter.h"
#include <stdlib.h>
#include <assert.h>

struct sid_engine {
    const sid_engine_vtbl_t* vtbl;
    void* impl;
    uint64_t field_len;
};

sid_engine_t* sid_engine_wrap(const sid_engine_vtbl_t* vtbl, uint64_t field_len) {
    assert(vtbl && vtbl->create && vtbl->destroy && vtbl->step);
    sid_engine_t* e = (sid_engine_t*)calloc(1, sizeof(sid_engine_t));
    assert(e);
    e->vtbl = vtbl;
    e->field_len = field_len;
    e->impl = (void*)vtbl->create(field_len);
    assert(e->impl);
    return e;
}

void sid_engine_unwrap(sid_engine_t* e) {
    if (!e) return;
    e->vtbl->destroy((sid_engine_t*)e->impl);
    free(e);
}

void sid_engine_step(sid_engine_t* e) {
    assert(e && e->vtbl && e->impl);
    e->vtbl->step((sid_engine_t*)e->impl);
}

const double* sid_engine_field_ro(const sid_engine_t* e) {
    assert(e && e->vtbl && e->impl);
    assert(e->vtbl->field_ro);
    return e->vtbl->field_ro((const sid_engine_t*)e->impl);
}

double sid_engine_stability(const sid_engine_t* e) {
    assert(e && e->vtbl && e->impl);
    if (!e->vtbl->stability) return 1.0;
    return e->vtbl->stability((const sid_engine_t*)e->impl);
}
```

## Example Glue for Real Engine

### File: `sid_engine_glue_igsoa_2d.c`

Replace placeholder symbols with your actual C-API symbols from `igsoa_capi_2d.h`.

```c
#include "sid_engine_adapter.h"

/* Include your real CAPI header */
#include "igsoa_capi_2d.h"

/* Wrapper impl type */
typedef struct {
    igsoa2d_handle* h;
} igsoa2d_wrap;

/* vtbl functions */

static sid_engine_t* igsoa2d_create(uint64_t field_len) {
    (void)field_len; /* engine may have its own sizing */
    igsoa2d_wrap* w = (igsoa2d_wrap*)calloc(1, sizeof(igsoa2d_wrap));
    w->h = igsoa2d_create_default();          /* <--- REPLACE */
    igsoa2d_init(w->h);                       /* <--- REPLACE */
    return (sid_engine_t*)w;
}

static void igsoa2d_destroy(sid_engine_t* impl) {
    igsoa2d_wrap* w = (igsoa2d_wrap*)impl;
    if (!w) return;
    igsoa2d_destroy(w->h);                    /* <--- REPLACE */
    free(w);
}

static void igsoa2d_step(sid_engine_t* impl) {
    igsoa2d_wrap* w = (igsoa2d_wrap*)impl;
    igsoa2d_step(w->h);                       /* <--- REPLACE */
}

static const double* igsoa2d_field_ro(const sid_engine_t* impl) {
    const igsoa2d_wrap* w = (const igsoa2d_wrap*)impl;
    return igsoa2d_field_ptr(w->h);           /* <--- REPLACE */
}

static double igsoa2d_stability(const sid_engine_t* impl) {
    const igsoa2d_wrap* w = (const igsoa2d_wrap*)impl;
    return igsoa2d_stability_metric(w->h);    /* <--- REPLACE (or return 1.0) */
}

/* Exported vtbl */
const sid_engine_vtbl_t SID_ENGINE_IGSOA2D = {
    .create    = igsoa2d_create,
    .destroy   = igsoa2d_destroy,
    .step      = igsoa2d_step,
    .field_ro  = igsoa2d_field_ro,
    .field_rw  = 0,
    .stability = igsoa2d_stability
};
```

## Updated Main Loop (Mode 1: Engine-Owned Buffers)

```cpp
#include <stdio.h>
#include <stdint.h>

extern "C" {
#include "sid_semantic_processor.h"
#include "sid_mixer.h"
#include "sid_engine_adapter.h"
}

/* vtbls implemented in glue files */
extern "C" {
extern const sid_engine_vtbl_t SID_ENGINE_IGSOA2D;
}

int main() {
    const uint64_t FIELD_LEN = 128;
    const double   TOTAL_MASS = 1000.0;
    const uint64_t STEPS = 200;

    /* SSPs: create with capacity, then attach engine-owned buffers (RO) */
    sid_ssp_t* ssp_I = sid_ssp_create(SID_ROLE_I, FIELD_LEN, TOTAL_MASS);
    sid_ssp_t* ssp_N = sid_ssp_create(SID_ROLE_N, FIELD_LEN, TOTAL_MASS);
    sid_ssp_t* ssp_U = sid_ssp_create(SID_ROLE_U, FIELD_LEN, TOTAL_MASS);

    sid_mixer_t* mixer = sid_mixer_create(TOTAL_MASS);

    /* Wrap engines */
    sid_engine_t* eng_I = sid_engine_wrap(&SID_ENGINE_IGSOA2D, FIELD_LEN);
    sid_engine_t* eng_N = sid_engine_wrap(&SID_ENGINE_IGSOA2D, FIELD_LEN);
    sid_engine_t* eng_U = sid_engine_wrap(&SID_ENGINE_IGSOA2D, FIELD_LEN);

    /* Attach SSP read-only views to engine buffers */
    sid_ssp_attach_external_field_ro(ssp_I, sid_engine_field_ro(eng_I), FIELD_LEN);
    sid_ssp_attach_external_field_ro(ssp_N, sid_engine_field_ro(eng_N), FIELD_LEN);
    sid_ssp_attach_external_field_ro(ssp_U, sid_engine_field_ro(eng_U), FIELD_LEN);

    for (uint64_t t = 0; t < STEPS; ++t) {
        sid_engine_step(eng_I);
        sid_engine_step(eng_N);
        sid_engine_step(eng_U);

        sid_ssp_commit_step(ssp_I);
        sid_ssp_commit_step(ssp_N);
        sid_ssp_commit_step(ssp_U);

        sid_mixer_step(mixer, ssp_I, ssp_N, ssp_U);

        if (t % 10 == 0) {
            /* Collapse via SSP(U) */
            sid_mixer_request_collapse(mixer, ssp_I, ssp_N, ssp_U);
        }

        sid_mixer_metrics_t m = sid_mixer_metrics(mixer);
        printf("t=%llu I=%.6f N=%.6f U=%.6f err=%.6e T=%d\n",
               (unsigned long long)t,
               m.admissible_volume,
               m.excluded_volume,
               m.undecided_volume,
               m.conservation_error,
               m.transport_ready);
    }

    sid_engine_unwrap(eng_I);
    sid_engine_unwrap(eng_N);
    sid_engine_unwrap(eng_U);

    sid_mixer_destroy(mixer);
    sid_ssp_destroy(ssp_I);
    sid_ssp_destroy(ssp_N);
    sid_ssp_destroy(ssp_U);

    return 0;
}
```

## Fastest Practical Wiring Path (No Refactors)

1. **I/N**: Engine-owned fields, SSP attaches RO (zero-copy)
2. **U**: SSP-owned field, engine writes into it (or post-step copy if needed)
3. **Mixer**: Collapse mutates SSP(U) buffer directly
4. **Engine(U)**: Reads SSP(U) buffer next step

This gives you full ternary irreversibility immediately.

## Collapse Needs Writable U

If U is attached RO (Mode 1), mixer can't mutate it.

### Two Clean Options:

1. **Switch only U to Mode 2** (SSP owns U buffer; engine writes into it)
2. **Add engine API call** like `engine_apply_collapse(mask, alpha)` for U

**Least invasive**: U uses SSP-owned buffer; I/N can remain engine-owned.

## Minimal "U Writable" Adapter Extension

Add to vtbl:

```c
void (*set_output_buffer)(sid_engine_t* impl, double* out, uint64_t len);
```

Then for U:
```c
// Allocate SSP(U) buffer
// Call set_output_buffer(eng_U, sid_ssp_field(ssp_U), FIELD_LEN)
```

No copies.
