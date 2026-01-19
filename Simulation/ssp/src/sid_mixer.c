#include "sid_mixer.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

struct sid_mixer {
    double C;

    int initialized;
    double I0, N0, U0;

    double prev_I;
    double prev_U;

    uint64_t stable_count;
    double eps_conservation;
    double eps_delta;
    uint64_t K;
    double ema_alpha;

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
    sid_mixer_config_t default_config = {
        .eps_conservation = 1e-6 * (total_mass_C > 1.0 ? total_mass_C : 1.0),
        .eps_delta = 1e-6 * (total_mass_C > 1.0 ? total_mass_C : 1.0),
        .K = 5,
        .ema_alpha = 0.1
    };
    return sid_mixer_create_ex(total_mass_C, &default_config);
}

sid_mixer_t* sid_mixer_create_ex(double total_mass_C, const sid_mixer_config_t* config) {
    if (total_mass_C <= 0.0)
        return NULL;
    if (!config)
        return NULL;
    if (config->eps_conservation < 0.0 || config->eps_delta < 0.0)
        return NULL;
    if (config->K == 0)
        return NULL;
    if (config->ema_alpha < 0.0 || config->ema_alpha > 1.0)
        return NULL;

    sid_mixer_t* m = (sid_mixer_t*)calloc(1, sizeof(sid_mixer_t));
    if (!m) return NULL;

    m->C = total_mass_C;
    m->initialized = 0;
    m->I0 = m->N0 = m->U0 = 0.0;
    m->prev_I = 0.0;
    m->prev_U = 0.0;

    m->stable_count = 0;
    m->eps_conservation = config->eps_conservation;
    m->eps_delta = config->eps_delta;
    m->K = config->K;
    m->ema_alpha = config->ema_alpha;

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

    const uint64_t len = sid_ssp_field_len(ssp_U);

    /* Validate all SSPs have same field length */
    assert(sid_ssp_field_len(ssp_I) == len);
    assert(sid_ssp_field_len(ssp_N) == len);

    /* Validate roles */
    assert(sid_ssp_role(ssp_I) == SID_ROLE_I);
    assert(sid_ssp_role(ssp_N) == SID_ROLE_N);
    assert(sid_ssp_role(ssp_U) == SID_ROLE_U);

    const double I = sum_field(ssp_I, len);
    const double N = sum_field(ssp_N, len);
    double U = sum_field(ssp_U, len);
    double total = I + N + U;

    /* Spec: mixer corrects conservation via SSP(U) when total deviates from C */
    if (total > mixer->C && U > 0.0) {
        const double excess = total - mixer->C;
        double alpha = excess / U;
        if (alpha > 1.0) alpha = 1.0;

        const uint64_t MAX_LEN = SIZE_MAX / sizeof(double);
        if (len <= MAX_LEN && alpha > 0.0) {
            double* M_I = (double*)calloc(len, sizeof(double));
            double* M_N = (double*)calloc(len, sizeof(double));
            if (M_I && M_N) {
                for (uint64_t i = 0; i < len; ++i) M_I[i] = 1.0;
                sid_collapse_mask_t mask = { M_I, M_N, len };
                sid_ssp_route_from_ssp(ssp_I, ssp_U, M_I, alpha);
                sid_ssp_route_from_ssp(ssp_N, ssp_U, M_N, alpha);
                sid_ssp_apply_collapse_mask(ssp_U, &mask, alpha);
            }
            free(M_I);
            free(M_N);
        }

        /* Recompute U and total after correction */
        U = sum_field(ssp_U, len);
        total = I + N + U;
        if (total > mixer->C) {
            const double I_now = sum_field(ssp_I, len);
            const double N_now = sum_field(ssp_N, len);
            const double IN = I_now + N_now;
            if (IN > 0.0) {
                double scale = (mixer->C - U) / IN;
                if (scale < 0.0) scale = 0.0;
                if (scale < 1.0) {
                    sid_ssp_scale_fields(ssp_I, scale);
                    sid_ssp_scale_fields(ssp_N, scale);
                }
            }
            U = sum_field(ssp_U, len);
            total = sum_field(ssp_I, len) + sum_field(ssp_N, len) + U;
        }
    } else if (total < mixer->C) {
        const double deficit = mixer->C - total;
        if (U > 0.0) {
            const double alpha = deficit / U;
            sid_ssp_scale_fields(ssp_U, 1.0 + alpha);
        } else {
            const double amount_per_cell = deficit / (double)len;
            sid_ssp_add_uniform(ssp_U, amount_per_cell);
        }

        U = sum_field(ssp_U, len);
        total = I + N + U;
    }

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

    if (mixer->U0 > 0.0) {
        double collapsed = (mixer->U0 - U);
        if (collapsed < 0.0) collapsed = 0.0;
        mixer->metrics.collapse_ratio = collapsed / mixer->U0;
    } else {
        mixer->metrics.collapse_ratio = 0.0;
    }

    const double dI = I - mixer->prev_I;
    const double dU = mixer->prev_U - U;
    const double denom = maxd(dU, 1e-12);
    const double inst_gain = dI / denom;

    mixer->metrics.loop_gain = (1.0 - mixer->ema_alpha) * mixer->metrics.loop_gain + mixer->ema_alpha * inst_gain;

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

    const uint64_t len = sid_ssp_field_len(ssp_U);

    /* Check for integer overflow in allocation size */
    const uint64_t MAX_LEN = SIZE_MAX / sizeof(double);
    if (len > MAX_LEN) return;

    double* M_I = (double*)calloc(len, sizeof(double));
    double* M_N = (double*)calloc(len, sizeof(double));
    if (!M_I || !M_N) {
        free(M_I);
        free(M_N);
        return;
    }

    for (uint64_t i = 0; i < len; ++i) M_I[i] = 1.0;

    /* Policy-free stub: uniform admissibility, small alpha */
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

sid_mixer_config_t sid_mixer_config(const sid_mixer_t* mixer) {
    assert(mixer);
    sid_mixer_config_t config = {
        .eps_conservation = mixer->eps_conservation,
        .eps_delta = mixer->eps_delta,
        .K = mixer->K,
        .ema_alpha = mixer->ema_alpha
    };
    return config;
}
