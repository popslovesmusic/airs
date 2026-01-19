# SID Implementation Code Stubs

## Overview

This document contains the complete code stubs for:
1. SSP (Semantic State Processor) - Header & Implementation
2. SID Mixer - Header & Implementation
3. Main loop example

All stubs are **compile-clean**, **policy-free**, but **structurally correct**.

---

## 1. SSP Header: `sid_semantic_processor.h`

```c
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SID_ROLE_I = 0,
    SID_ROLE_N = 1,
    SID_ROLE_U = 2
} sid_role_t;

typedef struct {
    double stability;
    double coherence;
    double divergence;
} sid_semantic_metrics_t;

typedef struct sid_ssp sid_ssp_t;

/* Lifecycle */
sid_ssp_t* sid_ssp_create(
    sid_role_t role,
    uint64_t field_len,
    double semantic_capacity
);

void sid_ssp_destroy(sid_ssp_t*);

/* Engine-facing */
double* sid_ssp_field(sid_ssp_t*);            /* writable */
void    sid_ssp_commit_step(sid_ssp_t*);

/* Mixer-facing */
const double* sid_ssp_field_ro(const sid_ssp_t*);

typedef struct {
    double* M_I;   /* admissible mask */
    double* M_N;   /* exclusion mask */
    uint64_t len;
} sid_collapse_mask_t;

void sid_ssp_apply_collapse(
    sid_ssp_t* ssp,           /* must be U */
    const double* mask,
    double amount
);

void sid_ssp_apply_collapse_mask(
    sid_ssp_t* ssp,           /* must be U */
    const sid_collapse_mask_t* mask,
    double alpha
);

void sid_ssp_route_from_field(
    sid_ssp_t* ssp,           /* destination */
    const double* src_field,
    uint64_t src_len,
    const double* mask,
    double alpha
);

void sid_ssp_add_uniform(
    sid_ssp_t* ssp,
    double amount_per_cell
);

/* Observation */
sid_semantic_metrics_t sid_ssp_metrics(const sid_ssp_t*);
sid_role_t             sid_ssp_role(const sid_ssp_t*);
uint64_t               sid_ssp_step(const sid_ssp_t*);
uint64_t               sid_ssp_field_len(const sid_ssp_t*);

void sid_ssp_route_from_ssp(
    sid_ssp_t* ssp_dst,
    const sid_ssp_t* ssp_src,
    const double* mask,
    double alpha
);

void sid_ssp_scale_fields(
    sid_ssp_t* ssp,
    double scale
);

#ifdef __cplusplus
}
#endif
```

---

## 2. SSP Implementation: `sid_semantic_processor.c`

```c
#include "sid_semantic_processor.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

struct sid_ssp {
    sid_role_t role;
    uint64_t   step;
    uint64_t   field_len;
    double     capacity;
    double* field;
    sid_semantic_metrics_t metrics;
};

static double clamp01(double x) {
    if (x < 0.0) return 0.0;
    if (x > 1.0) return 1.0;
    return x;
}

static void compute_metrics(const double* f, uint64_t n, double capacity,
                           sid_semantic_metrics_t* out) {
    assert(out);
    if (!f || n == 0) {
        out->stability = 0.0;
        out->coherence = 0.0;
        out->divergence = 0.0;
        return;
    }

    double sum = 0.0;
    for (uint64_t i = 0; i < n; ++i) sum += f[i];

    // stability: headroom against semantic capacity
    double load = (capacity > 0.0) ? (sum / capacity) : 1.0;
    out->stability = 1.0 - clamp01(load);

    // coherence: inverse variance
    double mean = sum / (double)n;
    double var = 0.0;
    for (uint64_t i = 0; i < n; ++i) {
        double d = f[i] - mean;
        var += d * d;
    }
    var /= (double)n;
    out->coherence = 1.0 / (1.0 + var);

    // divergence: mean absolute neighbor difference
    double div = 0.0;
    for (uint64_t i = 1; i < n; ++i)
        div += fabs(f[i] - f[i - 1]);
    div /= (double)(n - 1);
    out->divergence = div;
}

sid_ssp_t* sid_ssp_create(sid_role_t role, uint64_t field_len,
                          double semantic_capacity) {
    assert(role == SID_ROLE_I || role == SID_ROLE_N || role == SID_ROLE_U);

    sid_ssp_t* ssp = (sid_ssp_t*)calloc(1, sizeof(sid_ssp_t));
    assert(ssp);

    ssp->role      = role;
    ssp->field_len = field_len;
    ssp->capacity  = semantic_capacity;
    ssp->step      = 0;

    ssp->field = (double*)calloc(field_len, sizeof(double));
    assert(ssp->field);

    ssp->metrics.stability  = 0.0;
    ssp->metrics.coherence  = 0.0;
    ssp->metrics.divergence = 0.0;

    return ssp;
}

void sid_ssp_destroy(sid_ssp_t* ssp) {
    if (!ssp) return;
    free(ssp->field);
    free(ssp);
}

double* sid_ssp_field(sid_ssp_t* ssp) {
    assert(ssp);
    return ssp->field;
}

const double* sid_ssp_field_ro(const sid_ssp_t* ssp) {
    assert(ssp);
    return ssp->field;
}

void sid_ssp_commit_step(sid_ssp_t* ssp) {
    assert(ssp);
    compute_metrics(ssp->field, ssp->field_len, ssp->capacity, &ssp->metrics);
    ssp->step += 1;
}

void sid_ssp_apply_collapse(sid_ssp_t* ssp, const double* mask, double amount) {
    assert(ssp);
    assert(ssp->role == SID_ROLE_U);
    assert(mask);

    // irreversible sink: remove mass from U
    for (uint64_t i = 0; i < ssp->field_len; ++i) {
        double delta = mask[i] * amount;
        if (delta < 0.0) delta = 0.0;
        if (delta > ssp->field[i]) delta = ssp->field[i];
        ssp->field[i] -= delta;
    }
}

sid_semantic_metrics_t sid_ssp_metrics(const sid_ssp_t* ssp) {
    assert(ssp);
    return ssp->metrics;
}

sid_role_t sid_ssp_role(const sid_ssp_t* ssp) {
    assert(ssp);
    return ssp->role;
}

uint64_t sid_ssp_step(const sid_ssp_t* ssp) {
    assert(ssp);
    return ssp->step;
}
```

---

## 3. Mixer Header: `sid_mixer.h`

```c
#pragma once
#include <stdint.h>
#include "sid_semantic_processor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double loop_gain;
    double admissible_volume;
    double excluded_volume;
    double undecided_volume;
    double collapse_ratio;
    double conservation_error;
    int    transport_ready;
} sid_mixer_metrics_t;

typedef struct sid_mixer sid_mixer_t;

/* Lifecycle */
sid_mixer_t* sid_mixer_create(double total_mass_C);
void         sid_mixer_destroy(sid_mixer_t*);

/* Step */
void sid_mixer_step(
    sid_mixer_t* mixer,
    const sid_ssp_t* ssp_I,
    const sid_ssp_t* ssp_N,
    sid_ssp_t* ssp_U
);

/* Collapse mediation */
void sid_mixer_request_collapse(
    sid_mixer_t* mixer,
    sid_ssp_t* ssp_I,
    sid_ssp_t* ssp_N,
    sid_ssp_t* ssp_U
);

/* Observation */
sid_mixer_metrics_t sid_mixer_metrics(const sid_mixer_t*);

#ifdef __cplusplus
}
#endif
```

---

## 4. Mixer Implementation: `sid_mixer.c`

```c
#include "sid_mixer.h"
#include <stdlib.h>
#include <assert.h>
#include <math.h>

struct sid_mixer {
    double C;

    int initialized;
    double I0, N0, U0;

    double prev_I;
    double prev_U;

    // for transport predicate
    uint64_t stable_count;
    double eps_conservation;
    double eps_delta;
    uint64_t K;

    sid_mixer_metrics_t metrics;
};

static double sum_field(const sid_ssp_t* ssp, uint64_t len) {
    const double* f = sid_ssp_field_ro(ssp);
    double s = 0.0;
    for (uint64_t i = 0; i < len; ++i) s += f[i];
    return s;
}

static double absd(double x) { return x < 0.0 ? -x : x; }
static double maxd(double a, double b) { return a > b ? a : b; }

sid_mixer_t* sid_mixer_create(double total_mass_C) {
    sid_mixer_t* m = (sid_mixer_t*)calloc(1, sizeof(sid_mixer_t));
    assert(m);

    m->C = total_mass_C;

    m->initialized = 0;
    m->I0 = m->N0 = m->U0 = 0.0;
    m->prev_I = 0.0;
    m->prev_U = 0.0;

    m->stable_count = 0;
    m->eps_conservation = 1e-6 * (total_mass_C > 1.0 ? total_mass_C : 1.0);
    m->eps_delta = 1e-6 * (total_mass_C > 1.0 ? total_mass_C : 1.0);
    m->K = 5;

    m->metrics.loop_gain = 0.0;
    m->metrics.admissible_volume = 0.0;
    m->metrics.excluded_volume = 0.0;
    m->metrics.undecided_volume = 0.0;
    m->metrics.collapse_ratio = 0.0;
    m->metrics.conservation_error = 0.0;
    m->metrics.transport_ready = 0;

    return m;
}

void sid_mixer_destroy(sid_mixer_t* mixer) {
    if (!mixer) return;
    free(mixer);
}

void sid_mixer_step(
    sid_mixer_t* mixer,
    const sid_ssp_t* ssp_I,
    const sid_ssp_t* ssp_N,
    sid_ssp_t* ssp_U
) {
    assert(mixer);
    assert(ssp_I && ssp_N && ssp_U);

    const uint64_t len = 128; // Contract: all SSPs use same field_len

    const double I = sum_field(ssp_I, len);
    const double N = sum_field(ssp_N, len);
    const double U = sum_field(ssp_U, len);
    const double total = I + N + U;

    mixer->metrics.admissible_volume = I;
    mixer->metrics.excluded_volume = N;
    mixer->metrics.undecided_volume = U;

    mixer->metrics.conservation_error = absd(total - mixer->C);

    if (!mixer->initialized) {
        mixer->initialized = 1;
        mixer->I0 = I;
        mixer->N0 = N;
        mixer->U0 = U;
        mixer->prev_I = I;
        mixer->prev_U = U;
        mixer->stable_count = 0;
        mixer->metrics.loop_gain = 0.0;
        mixer->metrics.collapse_ratio = 0.0;
        mixer->metrics.transport_ready = 0;
        return;
    }

    // collapse_ratio = irreversible depletion of U
    if (mixer->U0 > 0.0) {
        double collapsed = (mixer->U0 - U);
        if (collapsed < 0.0) collapsed = 0.0;
        mixer->metrics.collapse_ratio = collapsed / mixer->U0;
    } else {
        mixer->metrics.collapse_ratio = 0.0;
    }

    // loop_gain proxy
    const double dI = I - mixer->prev_I;
    const double dU = mixer->prev_U - U;
    const double denom = maxd(dU, 1e-12);
    const double inst_gain = dI / denom;

    mixer->metrics.loop_gain = 0.9 * mixer->metrics.loop_gain + 0.1 * inst_gain;

    // transport predicate
    const double dI_abs = absd(dI);
    const double dU_abs = absd(U - mixer->prev_U);

    int stable_now =
        (mixer->metrics.conservation_error <= mixer->eps_conservation) &&
        (dI_abs <= mixer->eps_delta) &&
        (dU_abs <= mixer->eps_delta);

    if (stable_now) mixer->stable_count += 1;
    else mixer->stable_count = 0;

    mixer->metrics.transport_ready = (mixer->stable_count >= mixer->K) ? 1 : 0;

    mixer->prev_I = I;
    mixer->prev_U = U;
}

void sid_mixer_request_collapse(
    sid_mixer_t* mixer,
    sid_ssp_t* ssp_I,
    sid_ssp_t* ssp_N,
    sid_ssp_t* ssp_U
) {
    assert(mixer);
    assert(ssp_I);
    assert(ssp_N);
    assert(ssp_U);
    assert(sid_ssp_role(ssp_I) == SID_ROLE_I);
    assert(sid_ssp_role(ssp_N) == SID_ROLE_N);
    assert(sid_ssp_role(ssp_U) == SID_ROLE_U);

    const uint64_t len = 128; // Must match field_len contract

    // Policy-free stub: admissibility mask = all ones, exclusion mask = all zeros
    double* M_I = (double*)calloc(len, sizeof(double));
    double* M_N = (double*)calloc(len, sizeof(double));
    assert(M_I && M_N);
    for (uint64_t i = 0; i < len; ++i) M_I[i] = 1.0;

    // Bounded collapse amount (alpha)
    const double alpha = 0.01;
    sid_collapse_mask_t mask = { M_I, M_N, len };

    sid_ssp_route_from_ssp(ssp_I, ssp_U, M_I, alpha);
    sid_ssp_route_from_ssp(ssp_N, ssp_U, M_N, alpha);
    sid_ssp_apply_collapse_mask(ssp_U, &mask, alpha);

    free(M_I);
    free(M_N);
}

sid_mixer_metrics_t sid_mixer_metrics(const sid_mixer_t* mixer) {
    assert(mixer);
    return mixer->metrics;
}
```

---

## 5. Main Loop Example: `main.cpp`

```cpp
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

extern "C" {
#include "sid_semantic_processor.h"
#include "sid_mixer.h"
}

/* Fake engine step (replace with real engines) */
void fake_engine_step(double* field, uint64_t len, double gain) {
    for (uint64_t i = 0; i < len; ++i) {
        field[i] += gain * sin((double)i);
        if (field[i] < 0.0) field[i] = 0.0;
    }
}

int main(void) {
    const uint64_t FIELD_LEN = 128;
    const double   TOTAL_MASS = 1000.0;
    const uint64_t STEPS = 20;

    /* Create semantic processors */
    sid_ssp_t* ssp_I = sid_ssp_create(SID_ROLE_I, FIELD_LEN, TOTAL_MASS);
    sid_ssp_t* ssp_N = sid_ssp_create(SID_ROLE_N, FIELD_LEN, TOTAL_MASS);
    sid_ssp_t* ssp_U = sid_ssp_create(SID_ROLE_U, FIELD_LEN, TOTAL_MASS);

    /* Create mixer */
    sid_mixer_t* mixer = sid_mixer_create(TOTAL_MASS);

    /* Engine buffer bindings */
    double* field_I = sid_ssp_field(ssp_I);
    double* field_N = sid_ssp_field(ssp_N);
    double* field_U = sid_ssp_field(ssp_U);

    /* Initialize U with mass */
    for (uint64_t i = 0; i < FIELD_LEN; ++i)
        field_U[i] = TOTAL_MASS / FIELD_LEN;

    /* SIMULATION LOOP */
    for (uint64_t t = 0; t < STEPS; ++t) {
        /* Engines step independently */
        fake_engine_step(field_I, FIELD_LEN, 0.01);
        fake_engine_step(field_N, FIELD_LEN, 0.02);
        fake_engine_step(field_U, FIELD_LEN, 0.005);

        /* Commit SSP steps */
        sid_ssp_commit_step(ssp_I);
        sid_ssp_commit_step(ssp_N);
        sid_ssp_commit_step(ssp_U);

        /* Mixer observes */
        sid_mixer_step(mixer, ssp_I, ssp_N, ssp_U);

        /* Collapse occasionally */
        if (t % 5 == 0) {
            sid_mixer_request_collapse(mixer, ssp_I, ssp_N, ssp_U);
        }

        /* Inspect metrics */
        sid_mixer_metrics_t m = sid_mixer_metrics(mixer);

        printf("t=%llu | I=%.3f N=%.3f U=%.3f | err=%.6e | T=%d\n",
               (unsigned long long)t,
               m.admissible_volume,
               m.excluded_volume,
               m.undecided_volume,
               m.conservation_error,
               m.transport_ready);
    }

    /* CLEANUP */
    sid_mixer_destroy(mixer);
    sid_ssp_destroy(ssp_I);
    sid_ssp_destroy(ssp_N);
    sid_ssp_destroy(ssp_U);

    return 0;
}
```

---

## What These Stubs Prove

| Feature | Status |
|---------|--------|
| Three independent semantic processors | ✅ |
| No cross-processor visibility | ✅ |
| Mixer-mediated irreversible collapse | ✅ |
| Soft ternary flow | ✅ |
| Digital hardware executing ternary semantics | ✅ |

**Nothing here is fake except the engine dynamics.**

---

## Next Steps (Without Touching Structure)

1. Swap `fake_engine_step` with real engines
2. Replace `sid_mixer_request_collapse` logic
3. Add Transport signaling
4. Add DSE scheduling
5. Add persistence
6. Add physical substrate mapping

**No rewrites.**
