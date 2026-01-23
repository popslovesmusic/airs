/**
 * Engine Manager - Manages lifecycle of DASE engines
 * Handles both Phase 4B (real) and IGSOA Complex (complex) engines
 *
 * @warning SINGLE-THREADED ONLY
 * This class is designed for single-threaded CLI use only.
 * All operations assume sequential command execution from stdin.
 * No thread safety mechanisms are implemented.
 * DO NOT use from multiple threads - undefined behavior will occur.
 */

#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <atomic>
#include <unordered_map>
#include "json.hpp"

// Engine instance wrapper
struct EngineInstance {
    enum class TypeTag {
        Unknown,
        Phase4B,
        IgsoaComplex,
        IgsoaComplex2D,
        IgsoaComplex3D,
        IgsoaGW,
        SatpHiggs1D,
        SatpHiggs2D,
        SatpHiggs3D,
        FFTWCache,
        SidTernary,
        SidSSP
    };

    std::string engine_id;
    std::string engine_type; // "phase4b", "igsoa_complex", or IGSOA lattice variants
    void* engine_handle;      // Opaque handle to DLL engine
    int num_nodes;
    double created_timestamp;
    int dimension_x;
    int dimension_y;
    int dimension_z;
    int sid_role;
    double R_c;
    double kappa;
    double gamma;
    double dt;
    double alpha;
    TypeTag type_tag;

    EngineInstance()
        : engine_handle(nullptr)
        , num_nodes(0)
        , created_timestamp(0)
        , dimension_x(0)
        , dimension_y(0)
        , dimension_z(0)
        , sid_role(2)
        , R_c(1.0)
        , kappa(1.0)
        , gamma(0.1)
        , dt(0.01)
        , alpha(0.1)
        , type_tag(TypeTag::Unknown) {}
};

class EngineManager {
public:
    EngineManager();
    ~EngineManager();

    // Engine lifecycle
    std::string createEngine(const std::string& engine_type,
                             int num_nodes,
                             double R_c = 1.0,
                             double kappa = 1.0,
                             double gamma = 0.1,
                             double dt = 0.01,
                             double alpha = 0.1,
                             int N_x = 0,
                             int N_y = 0,
                             int N_z = 0,
                             int sid_role = 2,
                             const std::string& engine_id_hint = "");
    bool destroyEngine(const std::string& engine_id);
    EngineInstance* getEngine(const std::string& engine_id);
    const EngineInstance* getEngineConst(const std::string& engine_id) const;

    // List all engines
    std::vector<EngineInstance*> listEngines();

    // Engine operations (Phase 4B)
    bool setNodeState(const std::string& engine_id, int node_index, double value, const std::string& field = "phi");
    double getNodeState(const std::string& engine_id, int node_index, const std::string& field = "phi");
    bool runMission(const std::string& engine_id, int num_steps, int iterations_per_node);

    // Engine operations (IGSOA Complex)
    bool setNodePsi(const std::string& engine_id, int node_index, double real, double imag);
    bool getNodePsi(const std::string& engine_id, int node_index, double& real_out, double& imag_out);

    // Bulk state extraction (for spectral analysis)
    bool getAllNodeStates(const std::string& engine_id,
                          std::vector<double>& psi_real,
                          std::vector<double>& psi_imag,
                          std::vector<double>& phi);

    // Bulk state extraction (for SATP+Higgs engines)
    bool getSatpState(const std::string& engine_id,
                      std::vector<double>& phi_out,
                      std::vector<double>& phi_dot_out,
                      std::vector<double>& h_out,
                      std::vector<double>& h_dot_out);

    // Bulk state initialization (for IGSOA engines)
    bool setIgsoaState(const std::string& engine_id,
                       const std::string& profile_type,
                       const nlohmann::json& params);

    // Bulk state initialization (for SATP+Higgs engines)
    bool setSatpState(const std::string& engine_id,
                      const std::string& profile_type,
                      const nlohmann::json& params);

    // 2D analysis helpers
    bool computeCenterOfMass2D(const std::string& engine_id,
                               double& x_cm_out,
                               double& y_cm_out);
    bool computeCenterOfMass3D(const std::string& engine_id,
                               double& x_cm_out,
                               double& y_cm_out,
                               double& z_cm_out);

    // Metrics
    struct EngineMetrics {
        double ns_per_op;
        double ops_per_sec;
        uint64_t total_operations;
        double speedup_factor;
    };

    EngineMetrics getMetrics(const std::string& engine_id);

    // SID ternary operations
    struct SidMetrics {
        double I_mass;
        double N_mass;
        double U_mass;
        double instantaneous_gain;
        bool is_conserved;
        bool last_rewrite_applied;
        std::string last_rewrite_message;
    };
    struct SidRewriteEvent {
        uint64_t event_id;
        std::string rule_id;
        bool applied;
        std::string message;
        nlohmann::json metadata;
        double timestamp;
    };
    struct SidWrapperState {
        double I_mass;
        double N_mass;
        double U_mass;
        uint64_t motion_applied_count;
        size_t event_cursor;
        nlohmann::json last_motion;
        bool initialized;
    };

    bool sidStep(const std::string& engine_id, double alpha);
    bool sidCollapse(const std::string& engine_id, double alpha);
    bool sidApplyRewrite(const std::string& engine_id,
                         const std::string& pattern,
                         const std::string& replacement,
                         const std::string& rule_id,
                         const nlohmann::json& rule_metadata,
                         bool& applied_out,
                         std::string& message_out);
    bool sidSetDiagramExpr(const std::string& engine_id,
                           const std::string& expr,
                           const std::string& rule_id,
                           std::string& message_out);
    bool sidSetDiagramJson(const std::string& engine_id,
                            const std::string& diagram_json,
                            std::string& message_out);
    bool sidGetDiagramJson(const std::string& engine_id,
                           std::string& diagram_json_out);
    SidMetrics getSidMetrics(const std::string& engine_id);
    bool getSidRewriteEvents(const std::string& engine_id,
                             size_t cursor,
                             size_t limit,
                             std::vector<SidRewriteEvent>& events_out);
    bool sidWrapperApplyMotion(const std::string& engine_id,
                               size_t max_events_to_process,
                               SidWrapperState& state_out);
    bool getSidWrapperMetrics(const std::string& engine_id, SidWrapperState& state_out);
    void recordSidRewriteEvent(const std::string& engine_id,
                               const std::string& rule_id,
                               bool applied,
                               const std::string& message,
                               const nlohmann::json& metadata);

private:
    std::map<std::string, std::unique_ptr<EngineInstance>> engines;
    std::unordered_map<std::string, std::vector<SidRewriteEvent>> sid_rewrite_events_;
    std::unordered_map<std::string, SidWrapperState> sid_wrapper_state_;
    // Simple counter for engine ID generation (single-threaded, no atomic needed)
    int next_engine_id;
    static std::atomic<bool> instance_created_;

    std::string generateEngineId();
    double getCurrentTimestamp();
};
