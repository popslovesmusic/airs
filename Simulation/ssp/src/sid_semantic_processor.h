#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Ternary role identifiers for semantic processors.
 * Role is immutable after creation (role lock invariant).
 */
typedef enum {
    SID_ROLE_I = 0,  /**< Admitted/Included role */
    SID_ROLE_N = 1,  /**< Excluded/Negated role */
    SID_ROLE_U = 2   /**< Undecided/Uncertain role */
} sid_role_t;

/**
 * Per-field semantic metrics computed during commit_step.
 */
typedef struct {
    double stability;   /**< Semantic headroom: 1 - clamp(load), range [0,1] */
    double coherence;   /**< Field uniformity: 1/(1+variance), range (0,1] */
    double divergence;  /**< Mean absolute neighbor difference, range [0,+inf) */
} sid_semantic_metrics_t;

/**
 * Opaque semantic state processor handle.
 */
typedef struct sid_ssp sid_ssp_t;

/* ========== Lifecycle ========== */

/**
 * Create a semantic state processor.
 *
 * @param role             Ternary role (I, N, or U) - immutable after creation
 * @param field_len        Number of field elements (must be > 0)
 * @param semantic_capacity Total semantic capacity (must be >= 0)
 * @return                 New SSP handle, or NULL on failure
 *
 * Thread safety: Not thread-safe
 * Memory ownership: Caller must call sid_ssp_destroy()
 */
sid_ssp_t* sid_ssp_create(
    sid_role_t role,
    uint64_t field_len,
    double semantic_capacity
);

/**
 * Destroy a semantic state processor and free all resources.
 *
 * @param ssp SSP handle (NULL safe)
 *
 * Thread safety: Not thread-safe
 */
void sid_ssp_destroy(sid_ssp_t* ssp);

/* ========== Engine-facing Interface ========== */

/**
 * Get writable field pointer for engine mutation.
 * Only the local engine should write to this field.
 *
 * @param ssp SSP handle (must not be NULL)
 * @return    Writable field array of length field_len
 *
 * Lifetime: Valid until sid_ssp_destroy()
 * Thread safety: Not thread-safe
 */
double* sid_ssp_field(sid_ssp_t* ssp);

/**
 * Commit current step and recompute metrics.
 * Call after engine has finished updating the field.
 *
 * @param ssp SSP handle (must not be NULL)
 *
 * Thread safety: Not thread-safe
 */
void sid_ssp_commit_step(sid_ssp_t* ssp);

/* ========== Mixer-facing Interface ========== */

/**
 * Get read-only field pointer for mixer observation.
 *
 * @param ssp SSP handle (must not be NULL)
 * @return    Read-only field array of length field_len
 *
 * Lifetime: Valid until sid_ssp_destroy()
 * Thread safety: Not thread-safe
 */
const double* sid_ssp_field_ro(const sid_ssp_t* ssp);

/**
 * Dual-mask collapse structure per spec 03_COLLAPSE_MASKS.md
 * Constraint: M_I(x) + M_N(x) <= 1.0 for all x
 */
typedef struct {
    const double* M_I;   /**< Admissible inclusion mask, range [0,1] */
    const double* M_N;   /**< Inadmissible exclusion mask, range [0,1] */
    uint64_t len;        /**< Must match field_len */
} sid_collapse_mask_t;

/**
 * Apply irreversible collapse to U field (legacy single-mask API).
 *
 * @param ssp    SSP handle (must be role U)
 * @param mask   Collapse mask array, length field_len, range [0,1]
 * @param amount Collapse intensity per cell, range [0,+inf)
 *
 * Thread safety: Not thread-safe
 */
void sid_ssp_apply_collapse(
    sid_ssp_t* ssp,
    const double* mask,
    double amount
);

/**
 * Apply irreversible dual-mask collapse to U field (spec-compliant).
 * Formula: U'(x) = U(x) - alpha * (M_I(x) + M_N(x)) * U(x)
 *
 * @param ssp   SSP handle (must be role U)
 * @param mask  Dual-mask structure (M_I and M_N)
 * @param alpha Collapse intensity factor, range [0,1] typically
 *
 * Enforces:
 * - M_I(x) + M_N(x) <= 1.0
 * - U'(x) >= 0.0
 *
 * Thread safety: Not thread-safe
 */
void sid_ssp_apply_collapse_mask(
    sid_ssp_t* ssp,
    const sid_collapse_mask_t* mask,
    double alpha
);

/**
 * Route mass from a source field into a destination SSP field.
 * Formula: dst[i] += alpha * mask[i] * src_field[i]
 *
 * @param ssp        Destination SSP (must not be NULL)
 * @param src_field  Source field array, length must match field_len
 * @param mask       Mask array, length must match field_len, range [0,1]
 * @param alpha      Routing intensity factor, must be >= 0
 *
 * Thread safety: Not thread-safe
 */
void sid_ssp_route_from_field(
    sid_ssp_t* ssp,
    const double* src_field,
    uint64_t src_len,
    const double* mask,
    double alpha
);

/**
 * Route mass from a source SSP into a destination SSP.
 * Requires matching field lengths.
 *
 * @param ssp_dst Destination SSP (must not be NULL)
 * @param ssp_src Source SSP (must not be NULL)
 * @param mask    Mask array, length must match field_len, range [0,1]
 * @param alpha   Routing intensity factor, must be >= 0
 *
 * Thread safety: Not thread-safe
 */
void sid_ssp_route_from_ssp(
    sid_ssp_t* ssp_dst,
    const sid_ssp_t* ssp_src,
    const double* mask,
    double alpha
);

/**
 * Scale an SSP field in-place.
 *
 * @param ssp   SSP handle (must not be NULL)
 * @param scale Scale factor, must be >= 0
 *
 * Thread safety: Not thread-safe
 */
void sid_ssp_scale_fields(
    sid_ssp_t* ssp,
    double scale
);

/**
 * Uniformly add mass to an SSP field (used for conservation correction).
 *
 * @param ssp              SSP handle (must not be NULL)
 * @param amount_per_cell  Amount to add to each cell, must be >= 0
 *
 * Thread safety: Not thread-safe
 */
void sid_ssp_add_uniform(
    sid_ssp_t* ssp,
    double amount_per_cell
);

/* ========== Observation Interface ========== */

/**
 * Get computed semantic metrics.
 *
 * @param ssp SSP handle (must not be NULL)
 * @return    Metrics structure (copy)
 *
 * Thread safety: Not thread-safe
 */
sid_semantic_metrics_t sid_ssp_metrics(const sid_ssp_t* ssp);

/**
 * Get immutable role.
 *
 * @param ssp SSP handle (must not be NULL)
 * @return    Role (I, N, or U)
 *
 * Thread safety: Not thread-safe
 */
sid_role_t sid_ssp_role(const sid_ssp_t* ssp);

/**
 * Get current step counter.
 *
 * @param ssp SSP handle (must not be NULL)
 * @return    Number of committed steps
 *
 * Thread safety: Not thread-safe
 */
uint64_t sid_ssp_step(const sid_ssp_t* ssp);

/**
 * Get field length.
 *
 * @param ssp SSP handle (must not be NULL)
 * @return    Number of field elements
 *
 * Thread safety: Not thread-safe
 */
uint64_t sid_ssp_field_len(const sid_ssp_t* ssp);

#ifdef __cplusplus
}
#endif
