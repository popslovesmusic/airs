#pragma once
#include <stdint.h>
#include "sid_semantic_processor.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SID Mixer observable metrics (emitted each step).
 * Per spec 02_MIXER_SPECIFICATION.md
 */
typedef struct {
    double loop_gain;           /**< Feedback amplification I<->U, smoothed */
    double admissible_volume;   /**< Total mass in I */
    double excluded_volume;     /**< Total mass in N */
    double undecided_volume;    /**< Total mass in U */
    double collapse_ratio;      /**< (U0 - U) / U0, irreversible depletion */
    double conservation_error;  /**< |(I + N + U) - C| */
    int transport_ready;        /**< 1 if stability predicate met, 0 otherwise */
} sid_mixer_metrics_t;

/**
 * Opaque mixer handle.
 */
typedef struct sid_mixer sid_mixer_t;

/**
 * Mixer tuning parameters for transport predicate and loop gain.
 * All parameters must be validated before use.
 */
typedef struct {
    double eps_conservation;  /**< Conservation error tolerance, must be >= 0 */
    double eps_delta;         /**< Delta change tolerance for stability, must be >= 0 */
    uint64_t K;               /**< Consecutive stable steps required, must be > 0 */
    double ema_alpha;         /**< EMA smoothing factor, range [0,1] */
} sid_mixer_config_t;

/* ========== Lifecycle ========== */

/**
 * Create mixer with default configuration.
 * Default config:
 * - eps_conservation = 1e-6 * max(C, 1.0)
 * - eps_delta = 1e-6 * max(C, 1.0)
 * - K = 5
 * - ema_alpha = 0.1
 *
 * @param total_mass_C Total conserved mass, must be > 0
 * @return             New mixer handle, or NULL on failure
 *
 * Thread safety: Not thread-safe
 * Memory ownership: Caller must call sid_mixer_destroy()
 */
sid_mixer_t* sid_mixer_create(double total_mass_C);

/**
 * Create mixer with custom configuration.
 *
 * @param total_mass_C Total conserved mass, must be > 0
 * @param config       Configuration parameters (all validated)
 * @return             New mixer handle, or NULL on failure
 *
 * Thread safety: Not thread-safe
 * Memory ownership: Caller must call sid_mixer_destroy()
 */
sid_mixer_t* sid_mixer_create_ex(double total_mass_C, const sid_mixer_config_t* config);

/**
 * Destroy mixer and free all resources.
 *
 * @param mixer Mixer handle (NULL safe)
 *
 * Thread safety: Not thread-safe
 */
void sid_mixer_destroy(sid_mixer_t* mixer);

/* ========== Mixer Operations ========== */

/**
 * Execute one mixer observation step.
 * - Reads all three SSP fields
 * - Computes conservation error
 * - Updates loop gain
 * - Evaluates transport predicate
 *
 * @param mixer Mixer handle (must not be NULL)
 * @param ssp_I SSP with role I (validated)
 * @param ssp_N SSP with role N (validated)
 * @param ssp_U SSP with role U (validated); may be collapsed for conservation correction
 *
 * Requires: All SSPs must have same field_len
 * Thread safety: Not thread-safe
 */
void sid_mixer_step(
    sid_mixer_t* mixer,
    const sid_ssp_t* ssp_I,
    const sid_ssp_t* ssp_N,
    sid_ssp_t* ssp_U
);

/**
 * Request collapse of undecided field (policy-free stub).
 * Uses uniform mask (all 1.0) and small fixed amount.
 *
 * @param mixer Mixer handle (must not be NULL)
 * @param ssp_I SSP with role I (must not be NULL)
 * @param ssp_N SSP with role N (must not be NULL)
 * @param ssp_U SSP with role U (must not be NULL)
 *
 * Note: This is a stub implementation. Future versions will
 *       use admissibility-based dual masks.
 *
 * Thread safety: Not thread-safe
 */
void sid_mixer_request_collapse(
    sid_mixer_t* mixer,
    sid_ssp_t* ssp_I,
    sid_ssp_t* ssp_N,
    sid_ssp_t* ssp_U
);

/* ========== Observation ========== */

/**
 * Get current mixer metrics.
 *
 * @param mixer Mixer handle (must not be NULL)
 * @return      Metrics structure (copy)
 *
 * Thread safety: Not thread-safe
 */
sid_mixer_metrics_t sid_mixer_metrics(const sid_mixer_t* mixer);

/**
 * Get mixer configuration parameters.
 *
 * @param mixer Mixer handle (must not be NULL)
 * @return      Configuration structure (copy)
 *
 * Thread safety: Not thread-safe
 */
sid_mixer_config_t sid_mixer_config(const sid_mixer_t* mixer);

#ifdef __cplusplus
}
#endif
