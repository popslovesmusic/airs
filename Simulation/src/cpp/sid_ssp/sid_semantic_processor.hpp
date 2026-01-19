/**
 * SID Semantic Processor - C++ port from ssp/src/sid_semantic_processor.{h,c}
 *
 * Manages semantic state fields with RAII memory safety
 * Includes bug fixes from SSP_SIDS_CODE_REVIEW_BUGS.md
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

namespace sid {

/**
 * Ternary role identifiers for semantic processors
 */
enum class Role {
    I = 0,  // Admitted/Included role
    N = 1,  // Excluded/Negated role
    U = 2   // Undecided/Uncertain role
};

/**
 * Per-field semantic metrics computed during commit_step
 */
struct SemanticMetrics {
    double stability = 0.0;    // Semantic headroom: 1 - clamp(load), range [0,1]
    double coherence = 0.0;    // Field uniformity: 1/(1+variance), range (0,1]
    double divergence = 0.0;   // Mean absolute neighbor difference, range [0,+inf)
};

/**
 * Dual-mask collapse structure per spec 03_COLLAPSE_MASKS.md
 * Constraint: M_I(x) + M_N(x) <= 1.0 for all x
 *
 * BUG FIX (MEDIUM): Added is_valid() method for mask validation
 */
struct CollapseMask {
    std::vector<double> mask_I;  // Admissible inclusion mask, range [0,1]
    std::vector<double> mask_N;  // Inadmissible exclusion mask, range [0,1]

    CollapseMask(size_t len) : mask_I(len, 0.0), mask_N(len, 0.0) {}

    /**
     * BUG FIX: Validate mask constraints
     */
    bool is_valid() const {
        if (mask_I.size() != mask_N.size()) return false;

        for (size_t i = 0; i < mask_I.size(); ++i) {
            if (mask_I[i] < 0.0 || mask_I[i] > 1.0) return false;
            if (mask_N[i] < 0.0 || mask_N[i] > 1.0) return false;
            if (mask_I[i] + mask_N[i] > 1.0) return false;
        }

        return true;
    }
};

/**
 * Semantic State Processor - RAII wrapper for SSP field
 */
class SemanticProcessor {
private:
    Role role_;
    uint64_t step_ = 0;
    uint64_t field_len_;
    double capacity_;
    std::vector<double> field_;
    SemanticMetrics metrics_;

    static double clamp01(double x) {
        if (x < 0.0) return 0.0;
        if (x > 1.0) return 1.0;
        return x;
    }

    void compute_metrics() {
        if (field_.empty()) {
            metrics_.stability = 0.0;
            metrics_.coherence = 0.0;
            metrics_.divergence = 0.0;
            return;
        }

        // Pass 1: Compute sum, variance, and divergence
        double sum = field_[0];
        double sum_sq = field_[0] * field_[0];
        double div = 0.0;

        for (size_t i = 1; i < field_.size(); ++i) {
            sum += field_[i];
            sum_sq += field_[i] * field_[i];
            div += std::abs(field_[i] - field_[i - 1]);
        }

        // Stability: semantic headroom
        double load = (capacity_ > 0.0) ? (sum / capacity_) : 1.0;
        metrics_.stability = 1.0 - clamp01(load);

        // Coherence: 1 / (1 + variance)
        double mean = sum / static_cast<double>(field_.size());
        double mean_sq = sum_sq / static_cast<double>(field_.size());
        double var = mean_sq - mean * mean;
        if (var < 0.0) var = 0.0;  // Numerical safety
        metrics_.coherence = 1.0 / (1.0 + var);

        // Divergence: mean absolute neighbor difference
        if (field_.size() > 1) {
            metrics_.divergence = div / static_cast<double>(field_.size() - 1);
        } else {
            metrics_.divergence = 0.0;
        }
    }

public:
    SemanticProcessor(Role role, uint64_t field_len, double semantic_capacity)
        : role_(role), field_len_(field_len), capacity_(semantic_capacity), field_(field_len, 0.0) {
        if (field_len == 0) {
            throw std::logic_error("SemanticProcessor field_len must be positive");
        }
        if (semantic_capacity < 0.0) {
            throw std::logic_error("SemanticProcessor capacity must be non-negative");
        }
    }

    // Accessors
    Role role() const { return role_; }
    uint64_t step() const { return step_; }
    uint64_t field_len() const { return field_len_; }
    double capacity() const { return capacity_; }
    const SemanticMetrics& metrics() const { return metrics_; }

    // Field access
    std::vector<double>& field() { return field_; }
    const std::vector<double>& field() const { return field_; }

    /**
     * Commit current step and recompute metrics
     */
    void commit_step() {
        compute_metrics();
        ++step_;
    }

    /**
     * Apply irreversible collapse to U field (legacy single-mask API)
     *
     * BUG FIX (MEDIUM): Added mask range validation
     */
    void apply_collapse(const std::vector<double>& mask, double amount) {
        if (role_ != Role::U) {
            throw std::logic_error("apply_collapse requires Role::U");
        }
        if (mask.size() != field_len_) {
            throw std::logic_error("apply_collapse mask length mismatch");
        }

        for (size_t i = 0; i < field_len_; ++i) {
            // BUG FIX: Validate mask range
            if (mask[i] < 0.0 || mask[i] > 1.0) {
                throw std::logic_error("apply_collapse mask values must be in [0,1]");
            }
            double m = mask[i];
            double delta = m * amount;
            if (delta < 0.0) delta = 0.0;
            if (delta > field_[i]) delta = field_[i];
            field_[i] -= delta;
        }
    }

    /**
     * Apply irreversible dual-mask collapse to U field (spec-compliant)
     * Formula: U'(x) = U(x) - alpha * (M_I(x) + M_N(x)) * U(x)
     *
     * BUG FIX (MEDIUM): Validates CollapseMask constraints
     */
    void apply_collapse_mask(const CollapseMask& mask, double alpha) {
        if (role_ != Role::U) {
            throw std::logic_error("apply_collapse_mask requires Role::U");
        }
        if (mask.mask_I.size() != field_len_ || mask.mask_N.size() != field_len_) {
            throw std::logic_error("apply_collapse_mask mask length mismatch");
        }
        if (!mask.is_valid()) {
            throw std::logic_error("apply_collapse_mask requires valid masks");
        }
        if (alpha < 0.0) {
            throw std::logic_error("apply_collapse_mask alpha must be non-negative");
        }

        if (alpha > 1.0) alpha = 1.0;

        for (size_t i = 0; i < field_len_; ++i) {
            double total_mask = mask.mask_I[i] + mask.mask_N[i];
            if (total_mask > 1.0) total_mask = 1.0;
            if (total_mask < 0.0) total_mask = 0.0;

            double delta = alpha * total_mask * field_[i];
            if (delta > field_[i]) delta = field_[i];

            field_[i] -= delta;
        }
    }

    /**
     * Route mass from a source field into this field
     * Formula: dst[i] += alpha * mask[i] * src_field[i]
     *
     * BUG FIX (MEDIUM): Added non-negative field validation
     */
    void route_from_field(const std::vector<double>& src_field, 
                          const std::vector<double>& mask, 
                          double alpha) {
        if (src_field.size() != field_len_ || mask.size() != field_len_) {
            throw std::logic_error("route_from_field length mismatch");
        }
        if (alpha < 0.0) {
            throw std::logic_error("route_from_field alpha must be non-negative");
        }

        for (size_t i = 0; i < field_len_; ++i) {
            if (mask[i] < 0.0 || mask[i] > 1.0) {
                throw std::logic_error("route_from_field mask values must be in [0,1]");
            }
            double m = mask[i];
            double delta = alpha * m * src_field[i];
            // BUG FIX: Validate non-negative source
            if (delta < 0.0) delta = 0.0;
            field_[i] += delta;
        }
    }

    /**
     * Scale field in-place
     */
    void scale_all(double scale) {
        if (scale < 0.0) {
            throw std::logic_error("scale_all scale must be non-negative");
        }

        for (auto& val : field_) {
            val *= scale;
        }
    }

    /**
     * Uniformly add mass to field (used for conservation correction)
     */
    void add_uniform(double amount_per_cell) {
        if (amount_per_cell < 0.0) {
            throw std::logic_error("add_uniform amount must be non-negative");
        }

        if (amount_per_cell <= 0.0) return;

        for (auto& val : field_) {
            val += amount_per_cell;
        }
    }

    /**
     * Compute total mass in field
     */
    double total_mass() const {
        double sum = 0.0;
        for (const auto& val : field_) {
            sum += val;
        }
        return sum;
    }
};

} // namespace sid
