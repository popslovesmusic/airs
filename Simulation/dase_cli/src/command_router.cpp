/**
 * Command Router Implementation
 */

#include "command_router.h"
#include "engine_manager.h"
#include "../../src/cpp/satp_higgs_engine_1d.h"
#include "../../src/cpp/satp_higgs_engine_2d.h"
#include "../../src/cpp/satp_higgs_engine_3d.h"
#include "analysis_router.h"
#include "python_bridge.h"
#include "engine_fft_analysis.h"
#include <chrono>
#include <cstdint>

// Global analysis router (initialized after engine_manager)
static std::unique_ptr<dase::AnalysisRouter> g_analysis_router;

CommandRouter::CommandRouter()
    : engine_manager(std::make_unique<EngineManager>()) {

    // Initialize analysis router
    g_analysis_router = std::make_unique<dase::AnalysisRouter>(engine_manager.get());

    // Register command handlers
    command_handlers["get_capabilities"] = [this](const json& p) { return handleGetCapabilities(p); };
    command_handlers["describe_engine"] = [this](const json& p) { return handleDescribeEngine(p); };
    command_handlers["list_engines"] = [this](const json& p) { return handleListEngines(p); };
    command_handlers["create_engine"] = [this](const json& p) { return handleCreateEngine(p); };
    command_handlers["destroy_engine"] = [this](const json& p) { return handleDestroyEngine(p); };
    command_handlers["set_node_state"] = [this](const json& p) { return handleSetNodeState(p); };
    command_handlers["get_node_state"] = [this](const json& p) { return handleGetNodeState(p); };
    command_handlers["set_igsoa_state"] = [this](const json& p) { return handleSetIgsoaState(p); };
    command_handlers["set_satp_state"] = [this](const json& p) { return handleSetSatpState(p); };
    command_handlers["run_mission"] = [this](const json& p) { return handleRunMission(p); };
    command_handlers["run_mission_with_snapshots"] = [this](const json& p) { return handleRunMissionWithSnapshots(p); };
    command_handlers["run_benchmark"] = [this](const json& p) { return handleRunBenchmark(p); };
    command_handlers["get_metrics"] = [this](const json& p) { return handleGetMetrics(p); };
    command_handlers["get_state"] = [this](const json& p) { return handleGetState(p); };
    command_handlers["get_satp_state"] = [this](const json& p) { return handleGetSatpState(p); };
    command_handlers["get_center_of_mass"] = [this](const json& p) { return handleGetCenterOfMass(p); };
    command_handlers["sid_step"] = [this](const json& p) { return handleSidStep(p); };
    command_handlers["sid_collapse"] = [this](const json& p) { return handleSidCollapse(p); };
    command_handlers["sid_rewrite"] = [this](const json& p) { return handleSidRewrite(p); };
    command_handlers["sid_metrics"] = [this](const json& p) { return handleSidMetrics(p); };
    command_handlers["sid_set_diagram_expr"] = [this](const json& p) { return handleSidSetDiagramExpr(p); };
    command_handlers["sid_set_diagram_json"] = [this](const json& p) { return handleSidSetDiagramJson(p); };
    command_handlers["sid_get_diagram_json"] = [this](const json& p) { return handleSidGetDiagramJson(p); };

    // Register analysis commands
    command_handlers["check_analysis_tools"] = [this](const json& p) { return handleCheckAnalysisTools(p); };
    command_handlers["python_analyze"] = [this](const json& p) { return handlePythonAnalyze(p); };
    command_handlers["engine_fft"] = [this](const json& p) { return handleEngineFFT(p); };
    command_handlers["analyze_fields"] = [this](const json& p) { return handleAnalyzeFields(p); };
}

CommandRouter::~CommandRouter() = default;

json CommandRouter::execute(const json& command) {
    auto start_time = std::chrono::high_resolution_clock::now();

    try {
        // Extract command name
        if (!command.contains("command")) {
            return createErrorResponse("", "Missing 'command' field", "MISSING_COMMAND");
        }

        std::string cmd_name = command["command"].get<std::string>();

        // Get parameters (optional)
        json params = command.contains("params") ? command["params"] : json::object();

        // Find and execute handler
        auto it = command_handlers.find(cmd_name);
        if (it == command_handlers.end()) {
            return createErrorResponse(cmd_name, "Unknown command: " + cmd_name, "UNKNOWN_COMMAND");
        }

        // Execute command
        json result = it->second(params);

        // Calculate execution time
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        double execution_time_ms = duration.count() / 1000.0;

        // Add execution time to result
        result["execution_time_ms"] = execution_time_ms;

        return result;

    } catch (const std::exception& e) {
        return createErrorResponse("", std::string("Exception: ") + e.what(), "INTERNAL_ERROR");
    }
}

json CommandRouter::handleGetCapabilities(const json& params) {
    json result = {
        {"version", "1.0.0"},
        {"status", "prototype"},
        {"engines", json::array({"phase4b", "igsoa_complex", "igsoa_complex_2d", "igsoa_complex_3d", "satp_higgs_1d", "satp_higgs_2d", "satp_higgs_3d", "sid_ternary", "igsoa_gw"})},
        {"cpu_features", {
            {"avx2", true},
            {"avx512", false},
            {"fma", true}
        }},
        {"max_nodes", 1048576}
    };

    return createSuccessResponse("get_capabilities", result, 0);
}

json CommandRouter::handleDescribeEngine(const json& params) {
    // Extract engine name
    if (!params.contains("engine_name")) {
        return createErrorResponse("describe_engine", "Missing 'engine_name' parameter", "MISSING_PARAMETER");
    }

    std::string engine_name = params["engine_name"].get<std::string>();

    // IGSOA Gravitational Wave Engine
    if (engine_name == "igsoa_gw") {
        json description = {
            {"engine", "igsoa_gw"},
            {"display_name", "IGSOA Gravitational Wave Engine"},
            {"description", "Fractional-order wave equation solver for gravitational wave propagation in asymmetric spacetime"},
            {"version", "1.0.0"},
            {"parameters", {
                {"grid_nx", {
                    {"type", "integer"},
                    {"default", 32},
                    {"range", json::array({16, 128})},
                    {"description", "Grid points in X dimension"}
                }},
                {"grid_ny", {
                    {"type", "integer"},
                    {"default", 32},
                    {"range", json::array({16, 128})},
                    {"description", "Grid points in Y dimension"}
                }},
                {"grid_nz", {
                    {"type", "integer"},
                    {"default", 32},
                    {"range", json::array({16, 128})},
                    {"description", "Grid points in Z dimension"}
                }},
                {"dx", {
                    {"type", "float"},
                    {"default", 0.1},
                    {"range", json::array({0.01, 1.0})},
                    {"units", "meters"},
                    {"description", "Grid spacing in X dimension"}
                }},
                {"dy", {
                    {"type", "float"},
                    {"default", 0.1},
                    {"range", json::array({0.01, 1.0})},
                    {"units", "meters"},
                    {"description", "Grid spacing in Y dimension"}
                }},
                {"dz", {
                    {"type", "float"},
                    {"default", 0.1},
                    {"range", json::array({0.01, 1.0})},
                    {"units", "meters"},
                    {"description", "Grid spacing in Z dimension"}
                }},
                {"dt", {
                    {"type", "float"},
                    {"default", 0.001},
                    {"range", json::array({0.0001, 0.01})},
                    {"units", "seconds"},
                    {"description", "Time step (must satisfy CFL condition)"}
                }},
                {"alpha_min", {
                    {"type", "float"},
                    {"default", 1.0},
                    {"range", json::array({1.0, 2.0})},
                    {"description", "Minimum fractional memory order (near horizon)"}
                }},
                {"alpha_max", {
                    {"type", "float"},
                    {"default", 2.0},
                    {"range", json::array({1.0, 2.0})},
                    {"description", "Maximum fractional memory order (flat spacetime)"}
                }},
                {"R_c_default", {
                    {"type", "float"},
                    {"default", 1.0},
                    {"range", json::array({0.1, 10.0})},
                    {"description", "Default coupling constant"}
                }},
                {"kappa", {
                    {"type", "float"},
                    {"default", 1.0},
                    {"range", json::array({0.0, 10.0})},
                    {"description", "Asymmetry potential scale"}
                }},
                {"lambda", {
                    {"type", "float"},
                    {"default", 0.1},
                    {"range", json::array({0.0, 1.0})},
                    {"description", "Self-interaction strength"}
                }},
                {"soe_rank", {
                    {"type", "integer"},
                    {"default", 12},
                    {"range", json::array({4, 32})},
                    {"description", "Sum-of-exponentials approximation rank"}
                }}
            }},
            {"equations", json::array({
                {
                    {"name", "fractional_wave"},
                    {"latex", "\\partial^2_t \\psi - D^\\alpha_t \\psi - \\nabla^2 \\psi - V(\\delta\\Phi) \\psi = S"},
                    {"description", "Fractional wave equation for symmetry field"},
                    {"editable_terms", json::array({"V", "S"})}
                },
                {
                    {"name", "field_evolution"},
                    {"latex", "\\partial_t \\delta\\Phi = \\dot{\\Phi}"},
                    {"description", "Field time evolution"}
                }
            })},
            {"boundary_conditions", {
                {"types", json::array({"periodic", "dirichlet", "neumann"})},
                {"default", "periodic"},
                {"description", "Boundary condition types for field edges"}
            }},
            {"initial_conditions", {
                {"types", json::array({"gaussian", "plane_wave", "custom"})},
                {"default", "gaussian"},
                {"description", "Initial field configuration"}
            }},
            {"output_metrics", json::array({
                "simulation_time",
                "total_energy",
                "max_amplitude",
                "mean_amplitude",
                "max_gradient",
                "h_plus",
                "h_cross"
            })}
        };

        return createSuccessResponse("describe_engine", description, 0);
    }

    // IGSOA Complex Engine (existing engines)
    if (engine_name == "igsoa_complex") {
        json description = {
            {"engine", "igsoa_complex"},
            {"display_name", "IGSOA Complex 1D"},
            {"description", "1D complex-valued IGSOA dynamics engine"},
            {"version", "1.0.0"},
            {"parameters", {
                {"num_nodes", {
                    {"type", "integer"},
                    {"default", 2048},
                    {"range", json::array({64, 1048576})},
                    {"description", "Number of nodes in 1D lattice"}
                }},
                {"R_c", {
                    {"type", "float"},
                    {"default", 1.0},
                    {"range", json::array({0.1, 10.0})},
                    {"description", "Coupling constant"}
                }},
                {"kappa", {
                    {"type", "float"},
                    {"default", 1.0},
                    {"range", json::array({0.0, 10.0})},
                    {"description", "Field coupling strength"}
                }},
                {"gamma", {
                    {"type", "float"},
                    {"default", 0.1},
                    {"range", json::array({0.0, 1.0})},
                    {"description", "Damping coefficient"}
                }},
                {"dt", {
                    {"type", "float"},
                    {"default", 0.01},
                    {"range", json::array({0.001, 0.1})},
                    {"units", "arbitrary"},
                    {"description", "Time step"}
                }}
            }},
            {"equations", json::array({
                {
                    {"name", "igsoa_evolution"},
                    {"latex", "\\partial_t \\psi = -i H \\psi"},
                    {"description", "IGSOA field evolution"}
                }
            })},
            {"boundary_conditions", {
                {"types", json::array({"periodic"})},
                {"default", "periodic"}
            }}
        };

        return createSuccessResponse("describe_engine", description, 0);
    }

    if (engine_name == "sid_ternary") {
        json description = {
            {"engine", "sid_ternary"},
            {"display_name", "SID Ternary Mixer"},
            {"description", "Ternary I/N/U semantic mixer with optional rewrite operations"},
            {"version", "1.0.0"},
            {"parameters", {
                {"num_nodes", {
                    {"type", "integer"},
                    {"default", 1024},
                    {"range", json::array({1, 1048576})},
                    {"description", "Number of cells in each I/N/U field"}
                }},
                {"R_c", {
                    {"type", "float"},
                    {"default", 1.0},
                    {"range", json::array({0.000001, 1000.0})},
                    {"description", "Total mass / conservation constant"}
                }}
            }},
            {"commands", json::array({"sid_step", "sid_collapse", "sid_rewrite", "sid_metrics", "sid_set_diagram_expr", "sid_set_diagram_json", "sid_get_diagram_json"})}
        };

        return createSuccessResponse("describe_engine", description, 0);
    }

    // Unknown engine
    return createErrorResponse("describe_engine",
                              "Unknown engine: " + engine_name + ". Available engines: igsoa_gw, igsoa_complex, igsoa_complex_2d, igsoa_complex_3d, phase4b, satp_higgs_1d, satp_higgs_2d, satp_higgs_3d, sid_ternary.",
                              "UNKNOWN_ENGINE");
}

json CommandRouter::handleListEngines(const json& params) {
    auto engine_list = engine_manager->listEngines();

    json engines_array = json::array();
    for (auto* engine : engine_list) {
        json engine_json = {
            {"engine_id", engine->engine_id},
            {"engine_type", engine->engine_type},
            {"num_nodes", engine->num_nodes}
        };

        if (engine->dimension_x > 0 && engine->dimension_y > 0) {
            json dims = {
                {"N_x", engine->dimension_x},
                {"N_y", engine->dimension_y}
            };
            if (engine->dimension_z > 0) {
                dims["N_z"] = engine->dimension_z;
            }
            engine_json["dimensions"] = dims;
        }

        if (engine->engine_type.find("igsoa_complex") != std::string::npos) {
            engine_json["config"] = {
                {"R_c", engine->R_c},
                {"kappa", engine->kappa},
                {"gamma", engine->gamma},
                {"dt", engine->dt}
            };
        } else if (engine->engine_type == "sid_ternary") {
            engine_json["config"] = {
                {"R_c", engine->R_c}
            };
        }

        engines_array.push_back(engine_json);
    }

    json result = {
        {"engines", engines_array},
        {"total", engine_list.size()}
    };

    return createSuccessResponse("list_engines", result, 0);
}

json CommandRouter::handleCreateEngine(const json& params) {
    // Extract parameters with defaults
    std::string engine_type = params.value("engine_type", "phase4b");
    int num_nodes = params.value("num_nodes", 1024);

    // Extract R_c with explicit fallback logic (avoid nested .value() which can throw on null)
    double R_c = 1.0;
    if (params.contains("R_c_default") && !params["R_c_default"].is_null()) {
        R_c = params["R_c_default"].get<double>();
    } else if (params.contains("R_c") && !params["R_c"].is_null()) {
        R_c = params["R_c"].get<double>();
    }

    double kappa = params.value("kappa", 1.0);
    double gamma = params.value("gamma", 0.1);
    double dt = params.value("dt", 0.01);
    int N_x = params.value("N_x", params.value("width", 0));
    int N_y = params.value("N_y", params.value("height", 0));
    int N_z = params.value("N_z", params.value("depth", 0));

    // Validate physics parameters
    if (R_c <= 0.0 || !std::isfinite(R_c)) {
        return createErrorResponse("create_engine",
                                   "Invalid R_c parameter. Must be positive and finite.",
                                   "INVALID_PARAMETER");
    }
    if (kappa <= 0.0 || !std::isfinite(kappa)) {
        return createErrorResponse("create_engine",
                                   "Invalid kappa parameter. Must be positive and finite.",
                                   "INVALID_PARAMETER");
    }
    if (gamma < 0.0 || !std::isfinite(gamma)) {
        return createErrorResponse("create_engine",
                                   "Invalid gamma parameter. Must be non-negative and finite.",
                                   "INVALID_PARAMETER");
    }
    if (dt <= 0.0 || !std::isfinite(dt)) {
        return createErrorResponse("create_engine",
                                   "Invalid dt parameter. Must be positive and finite.",
                                   "INVALID_PARAMETER");
    }

    if (engine_type == "igsoa_complex_2d") {
        if (N_x <= 0 || N_y <= 0) {
            return createErrorResponse("create_engine",
                                       "Invalid 2D dimensions. N_x and N_y must be greater than 0.",
                                       "INVALID_DIMENSIONS");
        }

        int64_t expected_nodes = static_cast<int64_t>(N_x) * static_cast<int64_t>(N_y);
        if (expected_nodes <= 0 || expected_nodes > 1048576) {
            return createErrorResponse("create_engine",
                                       "Requested lattice size exceeds limits (max 1048576 nodes).",
                                       "INVALID_DIMENSIONS");
        }

        num_nodes = static_cast<int>(expected_nodes);
    } else if (engine_type == "igsoa_complex_3d") {
        if (N_x <= 0 || N_y <= 0 || N_z <= 0) {
            return createErrorResponse("create_engine",
                                       "Invalid 3D dimensions. N_x, N_y, and N_z must be greater than 0.",
                                       "INVALID_DIMENSIONS");
        }

        int64_t expected_nodes = static_cast<int64_t>(N_x) * static_cast<int64_t>(N_y) * static_cast<int64_t>(N_z);
        if (expected_nodes <= 0 || expected_nodes > 1048576) {
            return createErrorResponse("create_engine",
                                       "Requested lattice size exceeds limits (max 1048576 nodes).",
                                       "INVALID_DIMENSIONS");
        }

        num_nodes = static_cast<int>(expected_nodes);
    }

    // Create engine
    std::string engine_id = engine_manager->createEngine(
        engine_type,
        num_nodes,
        R_c,
        kappa,
        gamma,
        dt,
        N_x,
        N_y,
        N_z
    );

    if (engine_id.empty()) {
        return createErrorResponse("create_engine", "Failed to create engine.", "ENGINE_CREATE_FAILED");
    }

    json result = {
        {"engine_id", engine_id},
        {"engine_type", engine_type},
        {"num_nodes", num_nodes},
        {"R_c", R_c},
        {"kappa", kappa},
        {"gamma", gamma},
        {"dt", dt}
    };

    if (engine_type == "igsoa_complex_2d") {
        result["N_x"] = N_x;
        result["N_y"] = N_y;
    } else if (engine_type == "igsoa_complex_3d") {
        result["N_x"] = N_x;
        result["N_y"] = N_y;
        result["N_z"] = N_z;
    }

    return createSuccessResponse("create_engine", result, 0);
}

json CommandRouter::handleDestroyEngine(const json& params) {
    if (!params.contains("engine_id")) {
        return createErrorResponse("destroy_engine", "Missing 'engine_id' parameter.", "MISSING_PARAMETER");
    }

    std::string engine_id = params["engine_id"].get<std::string>();
    bool success = engine_manager->destroyEngine(engine_id);

    if (!success) {
        return createErrorResponse("destroy_engine",
                                   "Engine not found: " + engine_id + ".",
                                   "ENGINE_NOT_FOUND");
    }

    json result = {
        {"engine_id", engine_id},
        {"destroyed", true}
    };

    return createSuccessResponse("destroy_engine", result, 0);
}

json CommandRouter::handleSetNodeState(const json& params) {
    std::string engine_id = params.value("engine_id", "");
    int node_index = params.value("node_index", 0);
    double value = params.value("value", 0.0);

    bool success = engine_manager->setNodeState(engine_id, node_index, value);

    if (!success) {
        return createErrorResponse("set_node_state",
                                   "Failed to set node state.",
                                   "EXECUTION_FAILED");
    }

    json result = {
        {"node_index", node_index},
        {"value", value},
        {"updated", true}
    };

    return createSuccessResponse("set_node_state", result, 0);
}

json CommandRouter::handleGetNodeState(const json& params) {
    std::string engine_id = params.value("engine_id", "");
    int node_index = params.value("node_index", 0);

    double value = engine_manager->getNodeState(engine_id, node_index);

    json result = {
        {"node_index", node_index},
        {"value", value}
    };

    return createSuccessResponse("get_node_state", result, 0);
}

json CommandRouter::handleRunMission(const json& params) {
    std::string engine_id = params.value("engine_id", "");
    int num_steps = params.value("num_steps", 0);
    int iterations_per_node = params.value("iterations_per_node", 30);

    bool success = engine_manager->runMission(engine_id, num_steps, iterations_per_node);

    if (!success) {
        return createErrorResponse("run_mission",
                                   "Mission execution failed.",
                                   "EXECUTION_FAILED");
    }

    json result = {
        {"steps_completed", num_steps},
        {"total_operations", static_cast<double>(num_steps) * iterations_per_node * 1024}
    };

    return createSuccessResponse("run_mission", result, 0);
}

json CommandRouter::handleRunMissionWithSnapshots(const json& params) {
    std::string engine_id = params.value("engine_id", "");
    int num_steps = params.value("num_steps", 0);
    int iterations_per_node = params.value("iterations_per_node", 30);
    int snapshot_interval = params.value("snapshot_interval", 1);

    json snapshots = json::array();

    for (int step = snapshot_interval; step <= num_steps; step += snapshot_interval) {
        int steps_to_run = snapshot_interval;
        bool success = engine_manager->runMission(engine_id, steps_to_run, iterations_per_node);

        if (!success) {
            return createErrorResponse("run_mission_with_snapshots",
                                       "Mission execution failed at step " + std::to_string(step),
                                       "EXECUTION_FAILED");
        }

        json snapshot;
        snapshot["timestep"] = step;

        std::vector<double> psi_real, psi_imag, phi;
        bool success_state = engine_manager->getAllNodeStates(engine_id, psi_real, psi_imag, phi);

        if (!success_state) {
            return createErrorResponse("run_mission_with_snapshots",
                                       "Failed to get state at step " + std::to_string(step),
                                       "STATE_CAPTURE_FAILED");
        }

        snapshot["num_nodes"] = psi_real.size();
        snapshot["psi_real"] = psi_real;
        snapshot["psi_imag"] = psi_imag;
        snapshot["phi"] = phi;

        snapshots.push_back(snapshot);
    }

    json result = {
        {"steps_completed", num_steps},
        {"snapshot_count", snapshots.size()},
        {"snapshots", snapshots}
    };

    return createSuccessResponse("run_mission_with_snapshots", result, 0);
}

json CommandRouter::handleRunBenchmark(const json& params) {
    // STUB: Benchmark command not implemented
    // Returning stub response to indicate feature unavailability
    json result = {
        {"status", "stub"},
        {"message", "Benchmark command is not implemented. Use get_metrics on a real engine instead."},
        {"benchmark_type", "not_available"}
    };

    return createSuccessResponse("run_benchmark", result, 0);
}

json CommandRouter::handleGetMetrics(const json& params) {
    std::string engine_id = params.value("engine_id", "");

    // Get engine instance to retrieve actual engine type
    auto* instance = engine_manager->getEngine(engine_id);
    std::string engine_type = instance ? instance->engine_type : "unknown";

    auto metrics = engine_manager->getMetrics(engine_id);

    json result = {
        {"engine_type", engine_type},  // Use actual engine type from instance
        {"ns_per_op", metrics.ns_per_op},
        {"ops_per_sec", metrics.ops_per_sec},
        {"total_operations", metrics.total_operations}
    };

    return createSuccessResponse("get_metrics", result, 0);
}

json CommandRouter::handleSetIgsoaState(const json& params) {
    // Extract engine_id
    if (!params.contains("engine_id")) {
        return createErrorResponse("set_igsoa_state", "Missing 'engine_id' parameter", "MISSING_PARAMETER");
    }

    std::string engine_id = params["engine_id"].get<std::string>();

    // Extract profile type
    if (!params.contains("profile_type")) {
        return createErrorResponse("set_igsoa_state", "Missing 'profile_type' parameter", "MISSING_PARAMETER");
    }

    std::string profile_type = params["profile_type"].get<std::string>();

    // Extract profile parameters
    if (!params.contains("params")) {
        return createErrorResponse("set_igsoa_state", "Missing 'params' parameter", "MISSING_PARAMETER");
    }

    json profile_params = params["params"];

    // Call engine manager to set state
    if (!engine_manager->setIgsoaState(engine_id, profile_type, profile_params)) {
        return createErrorResponse("set_igsoa_state",
                                   "Failed to set state (wrong engine type, invalid engine_id, or unsupported profile)",
                                   "STATE_SET_FAILED");
    }

    json result = {
        {"profile_type", profile_type},
        {"applied", true}
    };

    return createSuccessResponse("set_igsoa_state", result, 0);
}

json CommandRouter::handleSetSatpState(const json& params) {
    // Extract engine_id
    if (!params.contains("engine_id")) {
        return createErrorResponse("set_satp_state", "Missing 'engine_id' parameter", "MISSING_PARAMETER");
    }

    std::string engine_id = params["engine_id"].get<std::string>();

    // Extract profile type
    if (!params.contains("profile_type")) {
        return createErrorResponse("set_satp_state", "Missing 'profile_type' parameter", "MISSING_PARAMETER");
    }

    std::string profile_type = params["profile_type"].get<std::string>();

    // Extract profile parameters
    if (!params.contains("params")) {
        return createErrorResponse("set_satp_state", "Missing 'params' parameter", "MISSING_PARAMETER");
    }

    json profile_params = params["params"];

    // Call engine manager to set state
    if (!engine_manager->setSatpState(engine_id, profile_type, profile_params)) {
        return createErrorResponse("set_satp_state",
                                   "Failed to set SATP state (wrong engine type, invalid engine_id, or unsupported profile)",
                                   "STATE_SET_FAILED");
    }

    json result = {
        {"profile_type", profile_type},
        {"applied", true}
    };

    return createSuccessResponse("set_satp_state", result, 0);
}

json CommandRouter::handleGetState(const json& params) {
    // Extract engine_id
    if (!params.contains("engine_id")) {
        return createErrorResponse("get_state", "Missing 'engine_id' parameter", "MISSING_PARAMETER");
    }

    std::string engine_id = params["engine_id"].get<std::string>();

    // Extract all node states
    std::vector<double> psi_real, psi_imag, phi;

    if (!engine_manager->getAllNodeStates(engine_id, psi_real, psi_imag, phi)) {
        return createErrorResponse("get_state", "Failed to extract state (wrong engine type or invalid engine_id)", "STATE_EXTRACTION_FAILED");
    }

    // Return state arrays
    json result = {
        {"num_nodes", psi_real.size()},
        {"psi_real", psi_real},
        {"psi_imag", psi_imag},
        {"phi", phi}
    };

    if (auto* instance = engine_manager->getEngine(engine_id)) {
        result["engine_type"] = instance->engine_type;
        if (instance->dimension_x > 0 && instance->dimension_y > 0) {
            json dims = {
                {"N_x", instance->dimension_x},
                {"N_y", instance->dimension_y}
            };
            if (instance->dimension_z > 0) {
                dims["N_z"] = instance->dimension_z;
            }
            result["dimensions"] = dims;
        }
    }

    return createSuccessResponse("get_state", result, 0);
}

json CommandRouter::handleGetSatpState(const json& params) {
    // Extract engine_id
    if (!params.contains("engine_id")) {
        return createErrorResponse("get_satp_state", "Missing 'engine_id' parameter", "MISSING_PARAMETER");
    }

    std::string engine_id = params["engine_id"].get<std::string>();

    // Extract SATP+Higgs field states
    std::vector<double> phi, phi_dot, h, h_dot;

    if (!engine_manager->getSatpState(engine_id, phi, phi_dot, h, h_dot)) {
        return createErrorResponse("get_satp_state", "Failed to extract SATP state (wrong engine type or invalid engine_id)", "STATE_EXTRACTION_FAILED");
    }

    // Compute additional diagnostics
    double phi_rms = 0.0, h_rms = 0.0, total_energy = 0.0;
    size_t num_nodes = phi.size();

    // Get engine for energy computation
    if (auto* instance = engine_manager->getEngine(engine_id)) {
        if (instance->engine_type == "satp_higgs_1d") {
            auto* engine = static_cast<dase::satp_higgs::SATPHiggsEngine1D*>(instance->engine_handle);
            total_energy = engine->computeTotalEnergy();
        } else if (instance->engine_type == "satp_higgs_2d") {
            auto* engine = static_cast<dase::satp_higgs::SATPHiggsEngine2D*>(instance->engine_handle);
            total_energy = engine->computeTotalEnergy();
        } else if (instance->engine_type == "satp_higgs_3d") {
            auto* engine = static_cast<dase::satp_higgs::SATPHiggsEngine3D*>(instance->engine_handle);
            total_energy = engine->computeTotalEnergy();
        }
    }

    for (size_t i = 0; i < num_nodes; i++) {
        phi_rms += phi[i] * phi[i];
        h_rms += h[i] * h[i];
    }

    // Guard against division by zero (should not occur, but defensive)
    if (num_nodes > 0) {
        phi_rms = std::sqrt(phi_rms / static_cast<double>(num_nodes));
        h_rms = std::sqrt(h_rms / static_cast<double>(num_nodes));
    }

    // Return state arrays with diagnostics
    json result = {
        {"num_nodes", num_nodes},
        {"phi", phi},
        {"phi_dot", phi_dot},
        {"h", h},
        {"h_dot", h_dot},
        {"diagnostics", {
            {"phi_rms", phi_rms},
            {"h_rms", h_rms},
            {"total_energy", total_energy}
        }}
    };

    if (auto* instance = engine_manager->getEngine(engine_id)) {
        result["engine_type"] = instance->engine_type;
        if (instance->engine_type == "satp_higgs_1d") {
            result["time"] = static_cast<dase::satp_higgs::SATPHiggsEngine1D*>(instance->engine_handle)->getTime();
        } else if (instance->engine_type == "satp_higgs_2d") {
            result["time"] = static_cast<dase::satp_higgs::SATPHiggsEngine2D*>(instance->engine_handle)->getTime();
        } else if (instance->engine_type == "satp_higgs_3d") {
            result["time"] = static_cast<dase::satp_higgs::SATPHiggsEngine3D*>(instance->engine_handle)->getTime();
        } else {
            result["time"] = 0.0;
        }
    }

    return createSuccessResponse("get_satp_state", result, 0);
}

json CommandRouter::handleGetCenterOfMass(const json& params) {
    if (!params.contains("engine_id")) {
        return createErrorResponse("get_center_of_mass",
                                   "Missing 'engine_id' parameter",
                                   "MISSING_PARAMETER");
    }

    std::string engine_id = params["engine_id"].get<std::string>();
    auto* instance = engine_manager->getEngine(engine_id);
    if (!instance) {
        return createErrorResponse("get_center_of_mass",
                                   "Engine does not exist",
                                   "ENGINE_NOT_FOUND");
    }

    json result = {
        {"engine_id", engine_id}
    };

    if (instance->engine_type == "igsoa_complex_2d") {
        double x_cm = 0.0;
        double y_cm = 0.0;
        if (!engine_manager->computeCenterOfMass2D(engine_id, x_cm, y_cm)) {
            return createErrorResponse("get_center_of_mass",
                                       "Unable to compute 2D center of mass",
                                       "INTERNAL_ERROR");
        }
        result["x_cm"] = x_cm;
        result["y_cm"] = y_cm;
    } else if (instance->engine_type == "igsoa_complex_3d") {
        double x_cm = 0.0;
        double y_cm = 0.0;
        double z_cm = 0.0;
        if (!engine_manager->computeCenterOfMass3D(engine_id, x_cm, y_cm, z_cm)) {
            return createErrorResponse("get_center_of_mass",
                                       "Unable to compute 3D center of mass",
                                       "INTERNAL_ERROR");
        }
        result["x_cm"] = x_cm;
        result["y_cm"] = y_cm;
        result["z_cm"] = z_cm;
    } else {
        return createErrorResponse("get_center_of_mass",
                                   "Engine is not an IGSOA lattice",
                                   "INVALID_ENGINE_TYPE");
    }

    if (instance->dimension_x > 0 && instance->dimension_y > 0) {
        json dims = {
            {"N_x", instance->dimension_x},
            {"N_y", instance->dimension_y}
        };
        if (instance->dimension_z > 0) {
            dims["N_z"] = instance->dimension_z;
        }
        result["dimensions"] = dims;
    }

    result["engine_type"] = instance->engine_type;

    return createSuccessResponse("get_center_of_mass", result, 0);
}

json CommandRouter::handleSidStep(const json& params) {
    std::string engine_id = params.value("engine_id", "");
    double alpha = params.value("alpha", 0.0);

    if (engine_id.empty()) {
        return createErrorResponse("sid_step", "Missing engine_id", "MISSING_PARAMETER");
    }

    if (!engine_manager->sidStep(engine_id, alpha)) {
        return createErrorResponse("sid_step",
                                   "SID step failed (invalid engine or parameters)",
                                   "EXECUTION_FAILED");
    }

    json result = {
        {"engine_id", engine_id},
        {"alpha", alpha}
    };

    return createSuccessResponse("sid_step", result, 0);
}

json CommandRouter::handleSidCollapse(const json& params) {
    std::string engine_id = params.value("engine_id", "");
    double alpha = params.value("alpha", 0.0);

    if (engine_id.empty()) {
        return createErrorResponse("sid_collapse", "Missing engine_id", "MISSING_PARAMETER");
    }

    if (!engine_manager->sidCollapse(engine_id, alpha)) {
        return createErrorResponse("sid_collapse",
                                   "SID collapse failed (invalid engine or parameters)",
                                   "EXECUTION_FAILED");
    }

    json result = {
        {"engine_id", engine_id},
        {"alpha", alpha}
    };

    return createSuccessResponse("sid_collapse", result, 0);
}

json CommandRouter::handleSidRewrite(const json& params) {
    std::string engine_id = params.value("engine_id", "");
    std::string pattern = params.value("pattern", "");
    std::string replacement = params.value("replacement", "");
    std::string rule_id = params.value("rule_id", "rw");

    if (engine_id.empty()) {
        return createErrorResponse("sid_rewrite", "Missing engine_id", "MISSING_PARAMETER");
    }
    if (pattern.empty() || replacement.empty()) {
        return createErrorResponse("sid_rewrite", "Missing pattern or replacement", "MISSING_PARAMETER");
    }

    bool applied = false;
    std::string message;
    if (!engine_manager->sidApplyRewrite(engine_id, pattern, replacement, rule_id, applied, message)) {
        return createErrorResponse("sid_rewrite",
                                   "SID rewrite failed (invalid engine or parameters)",
                                   "EXECUTION_FAILED");
    }

    json result = {
        {"engine_id", engine_id},
        {"rule_id", rule_id},
        {"applied", applied},
        {"message", message}
    };

    return createSuccessResponse("sid_rewrite", result, 0);
}

json CommandRouter::handleSidMetrics(const json& params) {
    std::string engine_id = params.value("engine_id", "");

    if (engine_id.empty()) {
        return createErrorResponse("sid_metrics", "Missing engine_id", "MISSING_PARAMETER");
    }

    auto metrics = engine_manager->getSidMetrics(engine_id);
    json result = {
        {"engine_id", engine_id},
        {"I_mass", metrics.I_mass},
        {"N_mass", metrics.N_mass},
        {"U_mass", metrics.U_mass},
        {"is_conserved", metrics.is_conserved},
        {"instantaneous_gain", metrics.instantaneous_gain},
        {"last_rewrite_applied", metrics.last_rewrite_applied},
        {"last_rewrite_message", metrics.last_rewrite_message}
    };

    return createSuccessResponse("sid_metrics", result, 0);
}

json CommandRouter::handleSidSetDiagramExpr(const json& params) {
    std::string engine_id = params.value("engine_id", "");
    std::string expr = params.value("expr", "");
    std::string rule_id = params.value("rule_id", "init");

    if (engine_id.empty()) {
        return createErrorResponse("sid_set_diagram_expr", "Missing engine_id", "MISSING_PARAMETER");
    }
    if (expr.empty()) {
        return createErrorResponse("sid_set_diagram_expr", "Missing expr", "MISSING_PARAMETER");
    }

    std::string message;
    if (!engine_manager->sidSetDiagramExpr(engine_id, expr, rule_id, message)) {
        return createErrorResponse("sid_set_diagram_expr",
                                   "SID diagram set failed (invalid engine or expression)",
                                   "EXECUTION_FAILED");
    }

    json result = {
        {"engine_id", engine_id},
        {"rule_id", rule_id},
        {"message", message}
    };

    return createSuccessResponse("sid_set_diagram_expr", result, 0);
}

json CommandRouter::handleSidSetDiagramJson(const json& params) {
    std::string engine_id = params.value("engine_id", "");

    if (engine_id.empty()) {
        return createErrorResponse("sid_set_diagram_json", "Missing engine_id", "MISSING_PARAMETER");
    }

    json diagram;

    if (params.contains("diagram")) {
        diagram = params["diagram"];
    } else if (params.contains("diagram_json")) {
        if (!params["diagram_json"].is_string()) {
            return createErrorResponse("sid_set_diagram_json", "diagram_json must be a string", "INVALID_PARAMETER");
        }
        try {
            diagram = json::parse(params["diagram_json"].get<std::string>());
        } catch (const std::exception& e) {
            return createErrorResponse("sid_set_diagram_json",
                                       std::string("Invalid diagram_json: ") + e.what(),
                                       "INVALID_PARAMETER");
        }
    } else if (params.contains("package")) {
        const auto& pkg = params["package"];
        if (!pkg.is_object()) {
            return createErrorResponse("sid_set_diagram_json", "package must be an object", "INVALID_PARAMETER");
        }
        if (!pkg.contains("diagrams") || !pkg["diagrams"].is_array() || pkg["diagrams"].empty()) {
            return createErrorResponse("sid_set_diagram_json", "package missing diagrams", "INVALID_PARAMETER");
        }
        std::string diagram_id = params.value("diagram_id", "");
        if (!diagram_id.empty()) {
            bool found = false;
            for (const auto& candidate : pkg["diagrams"]) {
                if (candidate.contains("id") && candidate["id"].is_string() &&
                    candidate["id"].get<std::string>() == diagram_id) {
                    diagram = candidate;
                    found = true;
                    break;
                }
            }
            if (!found) {
                return createErrorResponse("sid_set_diagram_json", "diagram_id not found in package", "INVALID_PARAMETER");
            }
        } else {
            diagram = pkg["diagrams"].at(0);
        }
    } else {
        return createErrorResponse("sid_set_diagram_json",
                                   "Missing diagram, diagram_json, or package",
                                   "MISSING_PARAMETER");
    }

    if (!diagram.is_object()) {
        return createErrorResponse("sid_set_diagram_json", "diagram must be an object", "INVALID_PARAMETER");
    }

    std::string message;
    if (!engine_manager->sidSetDiagramJson(engine_id, diagram.dump(), message)) {
        return createErrorResponse("sid_set_diagram_json",
                                   "SID diagram JSON set failed",
                                   "EXECUTION_FAILED");
    }

    json result = {
        {"engine_id", engine_id},
        {"message", message}
    };

    return createSuccessResponse("sid_set_diagram_json", result, 0);
}

json CommandRouter::handleSidGetDiagramJson(const json& params) {
    std::string engine_id = params.value("engine_id", "");

    if (engine_id.empty()) {
        return createErrorResponse("sid_get_diagram_json", "Missing engine_id", "MISSING_PARAMETER");
    }

    std::string diagram_json;
    if (!engine_manager->sidGetDiagramJson(engine_id, diagram_json)) {
        return createErrorResponse("sid_get_diagram_json",
                                   "SID diagram export failed",
                                   "EXECUTION_FAILED");
    }

    json diagram;
    try {
        diagram = json::parse(diagram_json);
    } catch (const std::exception& e) {
        return createErrorResponse("sid_get_diagram_json",
                                   std::string("Invalid diagram JSON: ") + e.what(),
                                   "EXECUTION_FAILED");
    }

    json result = {
        {"engine_id", engine_id},
        {"diagram", diagram}
    };

    return createSuccessResponse("sid_get_diagram_json", result, 0);
}

// ============================================================================
// ANALYSIS COMMANDS
// ============================================================================

json CommandRouter::handleCheckAnalysisTools(const json& params) {
    json result = g_analysis_router->checkToolAvailability();
    return createSuccessResponse("check_analysis_tools", result, 0);
}

json CommandRouter::handlePythonAnalyze(const json& params) {
    // Extract parameters
    std::string engine_id = params.value("engine_id", "");
    std::string script = params.value("script", "");
    std::map<std::string, std::string> args;

    if (params.contains("args")) {
        for (auto& [key, value] : params["args"].items()) {
            args[key] = value.is_string() ? value.get<std::string>() : value.dump();
        }
    }

    if (engine_id.empty()) {
        return createErrorResponse("python_analyze", "Missing engine_id", "MISSING_PARAMETER");
    }

    if (script.empty()) {
        return createErrorResponse("python_analyze", "Missing script", "MISSING_PARAMETER");
    }

    try {
        auto result_data = g_analysis_router->quickPythonAnalysis(engine_id, script, args);

        json result = {
            {"success", result_data.success},
            {"exit_code", result_data.exit_code},
            {"execution_time_ms", result_data.execution_time_ms},
            {"generated_files", result_data.generated_files}
        };

        if (!result_data.success) {
            result["error"] = result_data.error_message;
            result["stderr"] = result_data.stderr_output;
        }

        return createSuccessResponse("python_analyze", result, result_data.execution_time_ms);

    } catch (const std::exception& e) {
        return createErrorResponse("python_analyze", std::string("Analysis failed: ") + e.what(), "ANALYSIS_ERROR");
    }
}

json CommandRouter::handleEngineFFT(const json& params) {
    // Extract parameters
    std::string engine_id = params.value("engine_id", "");
    std::string field = params.value("field", "psi_real");

    if (engine_id.empty()) {
        return createErrorResponse("engine_fft", "Missing engine_id", "MISSING_PARAMETER");
    }

    try {
        auto fft_result = g_analysis_router->quickFFT(engine_id, field);
        json result = dase::analysis::EngineFFTAnalysis::toJSON(fft_result);

        return createSuccessResponse("engine_fft", result, fft_result.execution_time_ms);

    } catch (const std::exception& e) {
        return createErrorResponse("engine_fft", std::string("FFT failed: ") + e.what(), "FFT_ERROR");
    }
}

json CommandRouter::handleAnalyzeFields(const json& params) {
    // Extract parameters
    std::string engine_id = params.value("engine_id", "");
    std::string analysis_type_str = params.value("analysis_type", "combined");

    if (engine_id.empty()) {
        return createErrorResponse("analyze_fields", "Missing engine_id", "MISSING_PARAMETER");
    }

    // Parse analysis type
    dase::AnalysisType analysis_type;
    if (analysis_type_str == "python") {
        analysis_type = dase::AnalysisType::PYTHON_ONLY;
    } else if (analysis_type_str == "julia") {
        analysis_type = dase::AnalysisType::JULIA_EFA_ONLY;
    } else if (analysis_type_str == "engine") {
        analysis_type = dase::AnalysisType::ENGINE_ONLY;
    } else {
        analysis_type = dase::AnalysisType::COMBINED_ALL;
    }

    // Build configuration
    dase::AnalysisConfig config;
    config.type = analysis_type;

    if (params.contains("config")) {
        const auto& cfg = params["config"];

        // Python configuration
        if (cfg.contains("python")) {
            config.python.enabled = cfg["python"].value("enabled", false);
            if (cfg["python"].contains("scripts")) {
                for (const auto& script : cfg["python"]["scripts"]) {
                    config.python.scripts.push_back(script.get<std::string>());
                }
            }
            config.python.output_dir = cfg["python"].value("output_dir", "analysis_output");

            if (cfg["python"].contains("args")) {
                for (auto& [key, value] : cfg["python"]["args"].items()) {
                    config.python.args[key] = value.is_string() ? value.get<std::string>() : value.dump();
                }
            }
        }

        // Julia EFA configuration
        if (cfg.contains("julia_efa")) {
            config.julia_efa.enabled = cfg["julia_efa"].value("enabled", false);
            config.julia_efa.policy_path = cfg["julia_efa"].value("policy_path", "");
            config.julia_efa.log_path = cfg["julia_efa"].value("log_path", "");
        }

        // Engine analysis configuration
        if (cfg.contains("engine")) {
            config.engine.enabled = cfg["engine"].value("enabled", false);
            config.engine.compute_fft = cfg["engine"].value("compute_fft", false);

            if (cfg["engine"].contains("fields_to_analyze")) {
                for (const auto& field : cfg["engine"]["fields_to_analyze"]) {
                    config.engine.fields_to_analyze.push_back(field.get<std::string>());
                }
            }
        }

        config.enable_cross_validation = cfg.value("enable_cross_validation", true);
    }

    try {
        auto combined_result = g_analysis_router->routeAnalysis(engine_id, config);

        json result = {
            {"success", combined_result.success},
            {"total_execution_time_ms", combined_result.total_execution_time_ms}
        };

        if (!combined_result.success) {
            result["error"] = combined_result.error_message;
        }

        // Add Python results
        if (combined_result.python.executed) {
            json python_results = json::array();
            for (const auto& script_result : combined_result.python.script_results) {
                python_results.push_back({
                    {"success", script_result.success},
                    {"exit_code", script_result.exit_code},
                    {"generated_files", script_result.generated_files}
                });
            }
            result["python"] = {
                {"executed", true},
                {"scripts", python_results}
            };
        }

        // Add Julia EFA results
        if (combined_result.julia_efa.executed) {
            result["julia_efa"] = {
                {"executed", true},
                {"routing", combined_result.julia_efa.routing_decision},
                {"result", combined_result.julia_efa.efa_result}
            };
        }

        // Add engine analysis results
        if (combined_result.engine.executed) {
            json fft_results = json::array();
            for (const auto& fft : combined_result.engine.fft_results) {
                fft_results.push_back(dase::analysis::EngineFFTAnalysis::toJSON(fft));
            }
            result["engine"] = {
                {"executed", true},
                {"fft_results", fft_results}
            };
        }

        // Add validation results
        if (combined_result.validation.performed) {
            result["validation"] = {
                {"performed", true},
                {"all_checks_passed", combined_result.validation.all_checks_passed},
                {"consistency_checks", combined_result.validation.consistency_checks}
            };
        }

        return createSuccessResponse("analyze_fields", result, combined_result.total_execution_time_ms);

    } catch (const std::exception& e) {
        return createErrorResponse("analyze_fields", std::string("Analysis failed: ") + e.what(), "ANALYSIS_ERROR");
    }
}

json CommandRouter::createSuccessResponse(const std::string& command,
                                                 const json& result,
                                                 double execution_time_ms) {
    return {
        {"status", "success"},
        {"command", command},
        {"result", result},
        {"execution_time_ms", execution_time_ms}
    };
}

json CommandRouter::createErrorResponse(const std::string& command,
                                               const std::string& error,
                                               const std::string& error_code) {
    json response = {
        {"status", "error"},
        {"error", error},
        {"error_code", error_code},
        {"execution_time_ms", 0}
    };

    if (!command.empty()) {
        response["command"] = command;
    }

    return response;
}
