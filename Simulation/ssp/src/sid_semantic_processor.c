#include "sid_semantic_processor.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

struct sid_ssp {
    sid_role_t role;
    uint64_t step;
    uint64_t field_len;
    double capacity;
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

    /* Pass 1: Compute sum, variance, and divergence simultaneously */
    double sum = f[0];
    double sum_sq = f[0] * f[0];
    double div = 0.0;

    for (uint64_t i = 1; i < n; ++i) {
        sum += f[i];
        sum_sq += f[i] * f[i];
        div += fabs(f[i] - f[i - 1]);
    }

    /* Stability: semantic headroom per spec (clamped) */
    double load = (capacity > 0.0) ? (sum / capacity) : 1.0;
    out->stability = 1.0 - clamp01(load);

    /* Coherence: 1 / (1 + variance) using E[X^2] - E[X]^2 */
    double mean = sum / (double)n;
    double mean_sq = sum_sq / (double)n;
    double var = mean_sq - mean * mean;
    if (var < 0.0) var = 0.0;  /* Numerical safety */
    out->coherence = 1.0 / (1.0 + var);

    /* Divergence: mean absolute neighbor difference */
    if (n > 1) {
        out->divergence = div / (double)(n - 1);
    } else {
        out->divergence = 0.0;
    }
}

sid_ssp_t* sid_ssp_create(sid_role_t role, uint64_t field_len,
                          double semantic_capacity) {
    if (role != SID_ROLE_I && role != SID_ROLE_N && role != SID_ROLE_U)
        return NULL;
    if (field_len == 0)
        return NULL;
    if (semantic_capacity < 0.0)
        return NULL;

    /* Check for integer overflow in allocation size */
    const uint64_t MAX_FIELD_LEN = SIZE_MAX / sizeof(double);
    if (field_len > MAX_FIELD_LEN)
        return NULL;

    sid_ssp_t* ssp = (sid_ssp_t*)calloc(1, sizeof(sid_ssp_t));
    if (!ssp) return NULL;

    ssp->role = role;
    ssp->field_len = field_len;
    ssp->capacity = semantic_capacity;
    ssp->step = 0;

    ssp->field = (double*)calloc(field_len, sizeof(double));
    if (!ssp->field) {
        free(ssp);
        return NULL;
    }

    ssp->metrics.stability = 0.0;
    ssp->metrics.coherence = 0.0;
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

uint64_t sid_ssp_field_len(const sid_ssp_t* ssp) {
    assert(ssp);
    return ssp->field_len;
}

void sid_ssp_apply_collapse_mask(
    sid_ssp_t* ssp,
    const sid_collapse_mask_t* mask,
    double alpha
) {
    assert(ssp);
    assert(ssp->role == SID_ROLE_U);
    assert(mask);
    assert(mask->M_I && mask->M_N);
    assert(mask->len == ssp->field_len);
    assert(alpha >= 0.0);

    if (alpha > 1.0) alpha = 1.0;

    /* Spec: U'(x) = U(x) - alpha * (M_I(x) + M_N(x)) * U(x) */
    for (uint64_t i = 0; i < ssp->field_len; ++i) {
        double total_mask = mask->M_I[i] + mask->M_N[i];

        /* Enforce constraint: M_I(x) + M_N(x) <= 1 */
        if (total_mask > 1.0) total_mask = 1.0;
        if (total_mask < 0.0) total_mask = 0.0;

        double delta = alpha * total_mask * ssp->field[i];

        /* Enforce U'(x) >= 0 */
        if (delta > ssp->field[i]) delta = ssp->field[i];

        ssp->field[i] -= delta;
    }
}

void sid_ssp_route_from_field(
    sid_ssp_t* ssp,
    const double* src_field,
    uint64_t src_len,
    const double* mask,
    double alpha
) {
    assert(ssp);
    assert(src_field);
    assert(src_len == ssp->field_len);
    assert(mask);
    assert(alpha >= 0.0);

    for (uint64_t i = 0; i < ssp->field_len; ++i) {
        double m = mask[i];
        if (m < 0.0) m = 0.0;
        if (m > 1.0) m = 1.0;
        double delta = alpha * m * src_field[i];
        if (delta < 0.0) delta = 0.0;
        ssp->field[i] += delta;
    }
}

void sid_ssp_route_from_ssp(
    sid_ssp_t* ssp_dst,
    const sid_ssp_t* ssp_src,
    const double* mask,
    double alpha
) {
    assert(ssp_dst);
    assert(ssp_src);
    assert(mask);
    assert(alpha >= 0.0);
    assert(ssp_dst->field_len == ssp_src->field_len);

    const double* src_field = sid_ssp_field_ro(ssp_src);
    sid_ssp_route_from_field(ssp_dst, src_field, ssp_src->field_len, mask, alpha);
}

void sid_ssp_scale_fields(
    sid_ssp_t* ssp,
    double scale
) {
    assert(ssp);
    assert(scale >= 0.0);

    for (uint64_t i = 0; i < ssp->field_len; ++i) {
        ssp->field[i] *= scale;
    }
}

void sid_ssp_add_uniform(
    sid_ssp_t* ssp,
    double amount_per_cell
) {
    assert(ssp);
    assert(amount_per_cell >= 0.0);

    if (amount_per_cell <= 0.0) return;

    for (uint64_t i = 0; i < ssp->field_len; ++i) {
        ssp->field[i] += amount_per_cell;
    }
}
