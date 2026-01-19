/**
 * SID Mixer - C++ port from ssp/src/sid_mixer.{h,c}
 *
 * Manages ternary I/N/U fields with conservation constraint
 * Includes bug fixes from SSP_SIDS_CODE_REVIEW_BUGS.md
 */

#pragma once

#include "sid_semantic_processor.hpp"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <sstream>

namespace sid {

/**
 * Mixer observable metrics (emitted each step)
 */
struct MixerMetrics {
    double loop_gain = 0.0;            // Feedback amplification I<->U, smoothed
    double admissible_volume = 0.0;    // Total mass in I
    double excluded_volume = 0.0;      // Total mass in N
    double undecided_volume = 0.0;     // Total mass in U
    double collapse_ratio = 0.0;       // (U0 - U) / U0, irreversible depletion
    double conservation_error = 0.0;   // |(I + N + U) - C|
    bool transport_ready = false;      // Stability predicate met
};

/**
 * Mixer tuning parameters
 */
struct MixerConfig {
    double eps_conservation = 1e-6;  // Conservation error tolerance
    double eps_delta = 1e-6;         // Delta change tolerance for stability
    uint64_t K = 5;                  // Consecutive stable steps required
    double ema_alpha = 0.1;          // EMA smoothing factor, range [0,1]
};

/**
 * BUG FIX (HIGH): Maximum scale factor to prevent unbounded field growth
 */
constexpr double MAX_SCALE_FACTOR = 10.0;

/**
 * Mixer - Manages I/N/U ternary state with conservation
 */
class Mixer {
private:
    double C_;  // Total conserved mass

    bool initialized_ = false;
    double I0_ = 0.0, N0_ = 0.0, U0_ = 0.0;
    double prev_I_ = 0.0;
    double prev_U_ = 0.0;

    uint64_t stable_count_ = 0;
    MixerConfig config_;
    MixerMetrics metrics_;

    static double absd(double x) { return x < 0.0 ? -x : x; }
    static double maxd(double a, double b) { return a > b ? a : b; }
    static double mind(double a, double b) { return a < b ? a : b; }

public:
    Mixer(double total_mass_C, const MixerConfig& config = MixerConfig{})
        : C_(total_mass_C), config_(config) {
        if (C_ <= 0.0) {
            throw std::logic_error("Mixer total mass must be positive");
        }
        if (config_.eps_conservation < 0.0) {
            throw std::logic_error("Mixer eps_conservation must be non-negative");
        }
        if (config_.eps_delta < 0.0) {
            throw std::logic_error("Mixer eps_delta must be non-negative");
        }
        if (config_.K == 0) {
            throw std::logic_error("Mixer K must be positive");
        }
        if (config_.ema_alpha < 0.0 || config_.ema_alpha > 1.0) {
            throw std::logic_error("Mixer ema_alpha must be in [0,1]");
        }

        // Adjust defaults based on total mass
        if (config_.eps_conservation == 1e-6) {
            config_.eps_conservation = 1e-6 * maxd(C_, 1.0);
        }
        if (config_.eps_delta == 1e-6) {
            config_.eps_delta = 1e-6 * maxd(C_, 1.0);
        }
    }

    const MixerMetrics& metrics() const { return metrics_; }
    const MixerConfig& config() const { return config_; }

    /**
     * Execute one mixer observation step
     *
     * BUG FIX (HIGH): Added MAX_SCALE_FACTOR limit to prevent unbounded growth
     */
    void step(const SemanticProcessor& ssp_I, 
              const SemanticProcessor& ssp_N,
              SemanticProcessor& ssp_U) {
        if (ssp_I.role() != Role::I || ssp_N.role() != Role::N || ssp_U.role() != Role::U) {
            throw std::logic_error("Mixer role mismatch for I/N/U processors");
        }

        const uint64_t len = ssp_U.field_len();
        if (ssp_I.field_len() != len || ssp_N.field_len() != len) {
            throw std::logic_error("Mixer field length mismatch");
        }

        double I = ssp_I.total_mass();
        double N = ssp_N.total_mass();
        double U = ssp_U.total_mass();
        double total = I + N + U;
        double total_before = total;

        // Conservation correction
        if (total > C_ && U > 0.0) {
            const double excess = total - C_;
            double alpha = excess / U;
            if (alpha > 1.0) alpha = 1.0;

            // Apply collapse to remove excess (simplified version)
            std::vector<double> uniform_mask(len, 1.0);
            ssp_U.apply_collapse(uniform_mask, alpha * U / len);

            U = ssp_U.total_mass();
            total = I + N + U;
        } else if (total < C_) {
            const double deficit = C_ - total;
            if (U > 0.0) {
                // BUG FIX (HIGH): Limit scale factor
                double scale = 1.0 + (deficit / U);
                if (scale > MAX_SCALE_FACTOR) {
                    std::ostringstream oss;
                    oss << "Mixer scale factor exceeded cap: scale=" << scale
                        << " cap=" << MAX_SCALE_FACTOR;
                    throw std::runtime_error(oss.str());
                }
                ssp_U.scale_all(scale);
            } else {
                const double amount_per_cell = deficit / static_cast<double>(len);
                ssp_U.add_uniform(amount_per_cell);
            }

            U = ssp_U.total_mass();
            total = I + N + U;
        }

        metrics_.admissible_volume = I;
        metrics_.excluded_volume = N;
        metrics_.undecided_volume = U;
        metrics_.conservation_error = absd(total - C_);
        if (metrics_.conservation_error > config_.eps_conservation) {
            std::ostringstream oss;
            oss << "Conservation violation: before_total=" << total_before
                << " after_total=" << total << " target=" << C_;
            throw std::runtime_error(oss.str());
        }

        if (!initialized_) {
            initialized_ = true;
            I0_ = I;
            N0_ = N;
            U0_ = U;
            prev_I_ = I;
            prev_U_ = U;
            stable_count_ = 0;
            metrics_.loop_gain = 0.0;
            metrics_.collapse_ratio = 0.0;
            metrics_.transport_ready = false;
            return;
        }

        // Collapse ratio
        if (U0_ > 0.0) {
            double collapsed = (U0_ - U);
            if (collapsed < 0.0) collapsed = 0.0;
            metrics_.collapse_ratio = collapsed / U0_;
        } else {
            metrics_.collapse_ratio = 0.0;
        }

        // Loop gain calculation
        // BUG FIX (LOW): Improved denominator protection
        const double dI = I - prev_I_;
        const double dU = prev_U_ - U;
        const double denom = maxd(absd(dU), 1e-12);
        const double inst_gain = dI / denom;

        metrics_.loop_gain = (1.0 - config_.ema_alpha) * metrics_.loop_gain + 
                             config_.ema_alpha * inst_gain;

        // Stability check
        const double dI_abs = absd(dI);
        const double dU_abs = absd(U - prev_U_);

        bool stable_now =
            (metrics_.conservation_error <= config_.eps_conservation) &&
            (dI_abs <= config_.eps_delta) &&
            (dU_abs <= config_.eps_delta);

        if (stable_now) {
            stable_count_ += 1;
        } else {
            stable_count_ = 0;
        }

        metrics_.transport_ready = (stable_count_ >= config_.K);

        prev_I_ = I;
        prev_U_ = U;
    }

    /**
     * Request collapse of undecided field (policy-free stub)
     */
    void request_collapse(SemanticProcessor& ssp_I,
                          SemanticProcessor& ssp_N,
                          SemanticProcessor& ssp_U) {
        if (ssp_I.role() != Role::I || ssp_N.role() != Role::N || ssp_U.role() != Role::U) {
            throw std::logic_error("request_collapse role mismatch for I/N/U processors");
        }

        const uint64_t len = ssp_U.field_len();

        // Policy-free stub: uniform admissibility, small alpha
        std::vector<double> uniform_mask(len, 1.0);
        const double alpha = 0.01;

        ssp_U.apply_collapse(uniform_mask, alpha);
    }
};

} // namespace sid
