/**
 * SID Ternary Engine
 *
 * Complete engine for Semantic Interaction Diagrams with I/N/U ternary fields.
 * Manages Semantic State Processors (SSP) with conservation-governed evolution.
 */

#pragma once

#include "sid_ssp/sid_mixer.hpp"
#include "sid_ssp/sid_semantic_processor.hpp"
#include "sid_ssp/sid_diagram.hpp"
#include "sid_ssp/sid_parser_impl.hpp"
#include "sid_ssp/sid_diagram_builder.hpp"
#include "sid_ssp/sid_rewrite.hpp"
#include <vector>
#include <string>
#include <memory>
#include <cmath>

namespace sid {

/**
 * SID Ternary Engine Configuration
 */
struct SidEngineConfig {
    size_t num_nodes = 100;
    double total_mass = 1000.0;
    double eps_conservation = 1e-6;
    double eps_delta = 1e-6;
    uint64_t K_stable = 5;
    double ema_alpha = 0.1;
};

/**
 * SID Ternary Engine
 *
 * Manages I/N/U ternary fields with mixer-driven evolution and conservation.
 *
 * Core Concepts:
 * - I (Is): Admissible, affirmed, coherent states
 * - N (Not): Excluded, forbidden, contradictory states
 * - U (Unknown): Unresolved, undecided, open states
 * - Conservation: I + N + U = C (total mass constant)
 */
class SidTernaryEngine {
private:
    // Configuration
    size_t num_nodes_;
    double total_mass_;

    // Core components
    std::unique_ptr<Mixer> mixer_;
    std::unique_ptr<SemanticProcessor> ssp_I_;
    std::unique_ptr<SemanticProcessor> ssp_N_;
    std::unique_ptr<SemanticProcessor> ssp_U_;
    std::unique_ptr<Diagram> diagram_;

    // Evolution state
    uint64_t step_count_ = 0;

    // Rewrite tracking
    bool last_rewrite_applied_ = false;
    std::string last_rewrite_message_;

    // Initialization flag
    bool initialized_ = false;

public:
    /**
     * Constructor
     *
     * @param num_nodes Number of field nodes
     * @param total_mass Total conserved mass C
     * @param config Optional mixer configuration
     */
    SidTernaryEngine(size_t num_nodes, double total_mass,
                     const MixerConfig& mixer_config = MixerConfig{})
        : num_nodes_(num_nodes)
        , total_mass_(total_mass)
    {
        // Create mixer
        mixer_ = std::make_unique<Mixer>(total_mass, mixer_config);

        // Create SSP fields with initial equal distribution
        double initial_mass_per_field = total_mass / 3.0;

        ssp_I_ = std::make_unique<SemanticProcessor>(
            Role::I,
            num_nodes,
            initial_mass_per_field
        );

        ssp_N_ = std::make_unique<SemanticProcessor>(
            Role::N,
            num_nodes,
            initial_mass_per_field
        );

        ssp_U_ = std::make_unique<SemanticProcessor>(
            Role::U,
            num_nodes,
            initial_mass_per_field
        );

        // Create empty diagram
        diagram_ = std::make_unique<Diagram>("sid_engine_diagram");

        // Initialize fields with uniform distribution
        initializeUniformFields();

        initialized_ = true;
    }

    ~SidTernaryEngine() = default;

    // Prevent copying
    SidTernaryEngine(const SidTernaryEngine&) = delete;
    SidTernaryEngine& operator=(const SidTernaryEngine&) = delete;

    /**
     * Initialize fields with uniform distribution
     */
    void initializeUniformFields() {
        double mass_per_node = total_mass_ / (3.0 * num_nodes_);

        auto& field_I = ssp_I_->field();
        auto& field_N = ssp_N_->field();
        auto& field_U = ssp_U_->field();

        for (size_t i = 0; i < num_nodes_; ++i) {
            field_I[i] = mass_per_node;
            field_N[i] = mass_per_node;
            field_U[i] = mass_per_node;
        }

        // Commit initial state
        ssp_I_->commit_step();
        ssp_N_->commit_step();
        ssp_U_->commit_step();
    }

    /**
     * Execute one evolution step
     *
     * @param alpha Step parameter (currently unused, reserved for future)
     */
    void step(double alpha = 1.0) {
        if (!initialized_) return;

        // Run mixer step (evolves I/N/U fields)
        mixer_->step(*ssp_I_, *ssp_N_, *ssp_U_);

        // Commit field updates
        ssp_I_->commit_step();
        ssp_N_->commit_step();
        ssp_U_->commit_step();

        step_count_++;
    }

    /**
     * Force collapse operation
     *
     * Moves mass from U to I/N based on collapse masks.
     *
     * @param alpha Collapse strength parameter (0.0 to 1.0)
     */
    void collapse(double alpha = 1.0) {
        if (!initialized_) return;

        // Clamp alpha to valid range
        alpha = std::max(0.0, std::min(1.0, alpha));

        auto& field_I = ssp_I_->field();
        auto& field_N = ssp_N_->field();
        auto& field_U = ssp_U_->field();

        // Simple collapse: move fraction of U to I (admissible)
        // In full implementation, this would use collapse masks from diagram
        for (size_t i = 0; i < num_nodes_; ++i) {
            double u_transfer = field_U[i] * alpha * 0.5; // Half to I
            double n_transfer = field_U[i] * alpha * 0.5; // Half to N

            field_U[i] -= (u_transfer + n_transfer);
            field_I[i] += u_transfer;
            field_N[i] += n_transfer;
        }

        // Commit changes
        ssp_I_->commit_step();
        ssp_N_->commit_step();
        ssp_U_->commit_step();

        step_count_++;
    }

    /**
     * Get total mass in I field (admissible states)
     */
    double getIMass() const {
        if (!initialized_) return 0.0;

        double sum = 0.0;
        const auto& field = ssp_I_->field();
        for (size_t i = 0; i < num_nodes_; ++i) {
            sum += field[i];
        }
        return sum;
    }

    /**
     * Get total mass in N field (excluded states)
     */
    double getNMass() const {
        if (!initialized_) return 0.0;

        double sum = 0.0;
        const auto& field = ssp_N_->field();
        for (size_t i = 0; i < num_nodes_; ++i) {
            sum += field[i];
        }
        return sum;
    }

    /**
     * Get total mass in U field (undecided states)
     */
    double getUMass() const {
        if (!initialized_) return 0.0;

        double sum = 0.0;
        const auto& field = ssp_U_->field();
        for (size_t i = 0; i < num_nodes_; ++i) {
            sum += field[i];
        }
        return sum;
    }

    /**
     * Get instantaneous loop gain from mixer
     */
    double getInstantaneousGain() const {
        if (!initialized_) return 0.0;
        return mixer_->metrics().loop_gain;
    }

    /**
     * Check if mass conservation holds
     *
     * @param tolerance Conservation error tolerance
     * @return true if |I+N+U - C| < tolerance
     */
    bool isConserved(double tolerance) const {
        if (!initialized_) return false;

        double total = getIMass() + getNMass() + getUMass();
        return std::abs(total - total_mass_) < tolerance;
    }

    /**
     * Get conservation error
     *
     * @return |I+N+U - C|
     */
    double getConservationError() const {
        if (!initialized_) return 0.0;

        double total = getIMass() + getNMass() + getUMass();
        return std::abs(total - total_mass_);
    }

    /**
     * Get mixer metrics
     */
    MixerMetrics getMetrics() const {
        if (!initialized_) return MixerMetrics{};
        return mixer_->metrics();
    }

    /**
     * Get current step count
     */
    uint64_t getStepCount() const {
        return step_count_;
    }

    /**
     * Check if transport is ready (mixer stability reached)
     */
    bool isTransportReady() const {
        if (!initialized_) return false;
        return mixer_->metrics().transport_ready;
    }

    /**
     * Set diagram from JSON string
     *
     * @param json_str JSON diagram representation
     * @return true if successful
     */
    bool setDiagramJson(const std::string& json_str) {
        // TODO: Implement JSON parsing and diagram loading
        // For now, just track that it was called
        last_rewrite_message_ = "Diagram JSON loading not yet implemented";
        return false;
    }

    /**
     * Get diagram as JSON string
     *
     * @return JSON representation of current diagram
     */
    std::string getDiagramJson() const {
        // TODO: Implement diagram serialization
        // For now, return minimal valid JSON
        return "{\"id\":\"" + diagram_->id() + "\",\"nodes\":[],\"edges\":[]}";
    }

    /**
     * Apply rewrite rule to diagram
     *
     * @param pattern Pattern expression to match
     * @param replacement Replacement expression
     * @param rule_id Rule identifier
     * @return true if rewrite was applied
     */
    bool applyRewrite(const std::string& pattern,
                      const std::string& replacement,
                      const std::string& rule_id) {
        if (!diagram_) {
            last_rewrite_message_ = "No diagram loaded";
            last_rewrite_applied_ = false;
            return false;
        }

        try {
            // Apply expression-based rewrite
            RewriteResult result = applyExprRewrite(*diagram_, pattern, replacement, rule_id);

            // Update diagram if rewrite was applied
            if (result.applied) {
                diagram_ = std::make_unique<Diagram>(result.diagram);
            }

            // Collect messages
            last_rewrite_message_ = "";
            for (const auto& msg : result.messages) {
                if (!last_rewrite_message_.empty()) {
                    last_rewrite_message_ += "; ";
                }
                last_rewrite_message_ += msg;
            }

            last_rewrite_applied_ = result.applied;
            return result.applied;

        } catch (const std::exception& e) {
            last_rewrite_message_ = std::string("Rewrite error: ") + e.what();
            last_rewrite_applied_ = false;
            return false;
        }
    }

    /**
     * Set diagram from expression string
     *
     * @param expr Expression to parse (e.g., "P(Freedom)")
     * @param rule_id Rule identifier
     * @return true if successful
     */
    bool setDiagramExpr(const std::string& expr, const std::string& rule_id) {
        try {
            // Parse expression to AST
            ASTNode ast = parseExpression(expr);

            // Convert AST to diagram
            Diagram new_diagram = exprToDiagram(ast, rule_id);

            // Replace current diagram
            diagram_ = std::make_unique<Diagram>(new_diagram);

            last_rewrite_message_ = "Diagram set from expression: " + expr;
            last_rewrite_applied_ = true;
            return true;

        } catch (const ParseError& e) {
            last_rewrite_message_ = std::string("Parse error: ") + e.what();
            last_rewrite_applied_ = false;
            return false;
        } catch (const std::exception& e) {
            last_rewrite_message_ = std::string("Error: ") + e.what();
            last_rewrite_applied_ = false;
            return false;
        }
    }

    /**
     * Get last rewrite status
     */
    bool lastRewriteApplied() const {
        return last_rewrite_applied_;
    }

    /**
     * Get last rewrite message
     */
    std::string lastRewriteMessage() const {
        return last_rewrite_message_;
    }

    /**
     * Get number of nodes
     */
    size_t getNumNodes() const {
        return num_nodes_;
    }

    /**
     * Get total conserved mass
     */
    double getTotalMass() const {
        return total_mass_;
    }

    /**
     * Access I field (read-only)
     */
    const std::vector<double>& getIField() const {
        return ssp_I_->field();
    }

    /**
     * Access N field (read-only)
     */
    const std::vector<double>& getNField() const {
        return ssp_N_->field();
    }

    /**
     * Access U field (read-only)
     */
    const std::vector<double>& getUField() const {
        return ssp_U_->field();
    }
};

} // namespace sid
