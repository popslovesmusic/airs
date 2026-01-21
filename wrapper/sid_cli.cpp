// SID-only lightweight CLI wrapper
// Provides JSON commands for sid_ternary and sid_ssp, with an external
// semantic-motion layer that adjusts wrapper-owned mass_state for opt-in
// rewrites (mode == "semantic_motion"). Base engine remains symbolic-only.

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <cmath>
#include <optional>
#include "../Simulation/src/cpp/sid_ssp/sid_capi.hpp"
#include "json.hpp"

using json = nlohmann::json;

struct Event {
    uint64_t event_id;
    std::string rule_id;
    bool applied;
    std::string message;
    json metadata;
    double timestamp;
};

struct WrapperState {
    double I_mass{1.0 / 3.0};
    double N_mass{1.0 / 3.0};
    double U_mass{1.0 / 3.0};
    uint64_t motion_applied_count{0};
    uint64_t motion_refused_count{0};
    uint64_t rewrite_calls{0};
    uint64_t rewrites_applied{0};
    size_t event_cursor{0};
    json last_motion;
    bool initialized{false};
};

struct EngineEntry {
    enum class Type { Ternary, SSP };
    Type type;
    void* handle{nullptr};
    double alpha{0.1};
    double R_c{1.0};
    int sid_role{2};
    uint64_t ssp_steps{0};
    std::vector<Event> events;
    WrapperState wrapper;
};

static double now_seconds() {
    using clock = std::chrono::steady_clock;
    return std::chrono::duration<double>(clock::now().time_since_epoch()).count();
}

static bool masses_finite(double I, double N, double U) {
    return std::isfinite(I) && std::isfinite(N) && std::isfinite(U);
}

class SidCli {
public:
    json handle(const json& cmd) {
        const std::string name = cmd.value("command", "");
        if (name == "sid_create") return handleCreate(cmd.value("params", json::object()));
        if (name == "sid_destroy") return handleDestroy(cmd.value("params", json::object()));
        if (name == "sid_metrics") return handleMetrics(cmd.value("params", json::object()));
        if (name == "sid_set_diagram_expr") return handleSetDiagramExpr(cmd.value("params", json::object()));
        if (name == "sid_set_diagram_json") return handleSetDiagramJson(cmd.value("params", json::object()));
        if (name == "sid_get_diagram_json") return handleGetDiagramJson(cmd.value("params", json::object()));
        if (name == "sid_rewrite") return handleRewrite(cmd.value("params", json::object()));
        if (name == "sid_rewrite_events") return handleEvents(cmd.value("params", json::object()));
        if (name == "sid_wrapper_apply_motion") return handleApplyMotion(cmd.value("params", json::object()));
        if (name == "sid_wrapper_metrics") return handleWrapperMetrics(cmd.value("params", json::object()));
        if (name == "sid_run") return handleRun(cmd.value("params", json::object())); // SSP steps
        return error(name, "Unknown command", "UNKNOWN_COMMAND");
    }

private:
    std::unordered_map<std::string, EngineEntry> engines_;
    int next_id_{1};

    std::string genId() {
        std::ostringstream oss;
        oss << "sid_" << std::setw(3) << std::setfill('0') << next_id_++;
        return oss.str();
    }

    json handleCreate(const json& p) {
        const std::string type = p.value("engine_type", "sid_ternary");
        const int num_nodes = p.value("num_nodes", 8);
        const double R_c = p.value("R_c", p.value("capacity", 1.0));
        const double alpha = p.value("alpha", 0.1);
        const int role = p.value("role", 2);

        if (num_nodes <= 0) return error("sid_create", "num_nodes must be > 0", "INVALID_PARAMETER");
        if (R_c <= 0.0) return error("sid_create", "R_c/capacity must be > 0", "INVALID_PARAMETER");

        EngineEntry entry{};
        entry.R_c = R_c;
        entry.alpha = alpha;

        if (type == "sid_ternary") {
            entry.type = EngineEntry::Type::Ternary;
            entry.handle = sid_create_engine(static_cast<uint64_t>(num_nodes), R_c);
            if (!entry.handle) return error("sid_create", "sid_ternary create failed", "ENGINE_CREATE_FAILED");
            // seed wrapper from engine metrics
            double I = sid_get_I_mass(static_cast<sid_engine*>(entry.handle));
            double N = sid_get_N_mass(static_cast<sid_engine*>(entry.handle));
            double U = sid_get_U_mass(static_cast<sid_engine*>(entry.handle));
            double total = I + N + U;
            if (total > 0.0) { I /= total; N /= total; U /= total; }
            entry.wrapper.I_mass = I;
            entry.wrapper.N_mass = N;
            entry.wrapper.U_mass = U;
            entry.wrapper.initialized = true;
        } else if (type == "sid_ssp") {
            entry.type = EngineEntry::Type::SSP;
            entry.sid_role = role;
            entry.handle = sid_ssp_create(role, static_cast<unsigned long>(num_nodes), R_c);
            if (!entry.handle) return error("sid_create", "sid_ssp create failed", "ENGINE_CREATE_FAILED");
        } else {
            return error("sid_create", "unsupported engine_type", "INVALID_ENGINE");
        }

        const std::string id = genId();
        engines_[id] = std::move(entry);
        return success("sid_create", {{"engine_id", id}, {"engine_type", type}, {"num_nodes", num_nodes}, {"alpha", alpha}, {"R_c", R_c}, {"role", role}});
    }

    json handleDestroy(const json& p) {
        const std::string id = p.value("engine_id", "");
        auto it = engines_.find(id);
        if (it == engines_.end()) return error("sid_destroy", "engine not found", "ENGINE_NOT_FOUND");
        if (it->second.type == EngineEntry::Type::Ternary) {
            sid_destroy_engine(static_cast<sid_engine*>(it->second.handle));
        } else {
            sid_ssp_destroy(static_cast<sid_ssp_t*>(it->second.handle));
        }
        engines_.erase(it);
        return success("sid_destroy", {{"engine_id", id}, {"destroyed", true}});
    }

    json handleMetrics(const json& p) {
        const std::string id = p.value("engine_id", "");
        auto it = engines_.find(id);
        if (it == engines_.end()) return error("sid_metrics", "engine not found", "ENGINE_NOT_FOUND");
        if (it->second.type == EngineEntry::Type::Ternary) {
            auto* h = static_cast<sid_engine*>(it->second.handle);
            json res = {
                {"engine_id", id},
                {"I_mass", sid_get_I_mass(h)},
                {"N_mass", sid_get_N_mass(h)},
                {"U_mass", sid_get_U_mass(h)},
                {"instantaneous_gain", sid_get_instantaneous_gain(h)},
                {"is_conserved", sid_is_conserved(h, 1e-6)},
                {"last_rewrite_applied", sid_last_rewrite_applied(h)},
                {"last_rewrite_message", sid_last_rewrite_message(h) ? sid_last_rewrite_message(h) : ""}
            };
            return success("sid_metrics", res);
        } else {
            // SSP: expose steps committed from wrapper counter
            json res = {
                {"engine_id", id},
                {"engine_type", "sid_ssp"},
                {"steps_committed", it->second.ssp_steps}
            };
            return success("sid_metrics", res);
        }
    }

    json handleSetDiagramExpr(const json& p) {
        const std::string id = p.value("engine_id", "");
        const std::string expr = p.value("expr", "");
        const std::string rule_id = p.value("rule_id", "init");
        auto it = engines_.find(id);
        if (it == engines_.end() || it->second.type != EngineEntry::Type::Ternary) {
            return error("sid_set_diagram_expr", "engine not found or not sid_ternary", "ENGINE_NOT_FOUND");
        }
        if (expr.empty()) return error("sid_set_diagram_expr", "expr required", "MISSING_PARAMETER");
        bool ok = sid_set_diagram_expr(static_cast<sid_engine*>(it->second.handle), expr.c_str(), rule_id.c_str());
        const char* msg = sid_last_rewrite_message(static_cast<sid_engine*>(it->second.handle));
        return ok ? success("sid_set_diagram_expr", {{"engine_id", id}, {"rule_id", rule_id}, {"message", msg ? msg : ""}})
                  : error("sid_set_diagram_expr", "set_diagram_expr failed", "EXECUTION_FAILED");
    }

    json handleSetDiagramJson(const json& p) {
        const std::string id = p.value("engine_id", "");
        auto it = engines_.find(id);
        if (it == engines_.end() || it->second.type != EngineEntry::Type::Ternary) {
            return error("sid_set_diagram_json", "engine not found or not sid_ternary", "ENGINE_NOT_FOUND");
        }
        std::string diag_str;
        if (p.contains("diagram_json") && p["diagram_json"].is_string()) {
            diag_str = p["diagram_json"].get<std::string>();
        } else if (p.contains("diagram")) {
            diag_str = p["diagram"].dump();
        } else {
            return error("sid_set_diagram_json", "diagram or diagram_json required", "MISSING_PARAMETER");
        }
        bool ok = sid_set_diagram_json(static_cast<sid_engine*>(it->second.handle), diag_str.c_str());
        const char* msg = sid_last_rewrite_message(static_cast<sid_engine*>(it->second.handle));
        return ok ? success("sid_set_diagram_json", {{"engine_id", id}, {"message", msg ? msg : ""}})
                  : error("sid_set_diagram_json", "set_diagram_json failed", "EXECUTION_FAILED");
    }

    json handleGetDiagramJson(const json& p) {
        const std::string id = p.value("engine_id", "");
        auto it = engines_.find(id);
        if (it == engines_.end() || it->second.type != EngineEntry::Type::Ternary) {
            return error("sid_get_diagram_json", "engine not found or not sid_ternary", "ENGINE_NOT_FOUND");
        }
        const char* diag = sid_get_diagram_json(static_cast<sid_engine*>(it->second.handle));
        if (!diag) return error("sid_get_diagram_json", "null diagram", "EXECUTION_FAILED");
        json parsed;
        try { parsed = json::parse(diag); } catch (...) { parsed = json::object(); }
        return success("sid_get_diagram_json", {{"engine_id", id}, {"diagram", parsed}});
    }

    json handleRewrite(const json& p) {
        const std::string id = p.value("engine_id", "");
        auto it = engines_.find(id);
        if (it == engines_.end() || it->second.type != EngineEntry::Type::Ternary) {
            return error("sid_rewrite", "engine not found or not sid_ternary", "ENGINE_NOT_FOUND");
        }
        const std::string pattern = p.value("pattern", "");
        const std::string replacement = p.value("replacement", "");
        const std::string rule_id = p.value("rule_id", "rw");
        if (pattern.empty() || replacement.empty()) return error("sid_rewrite", "pattern/replacement required", "MISSING_PARAMETER");
        json metadata = p.value("rule_metadata", json::object());
        if (p.contains("mode")) metadata["mode"] = p["mode"];
        if (p.contains("epsilon")) metadata["epsilon"] = p["epsilon"];

        bool applied = sid_apply_rewrite(static_cast<sid_engine*>(it->second.handle), pattern.c_str(), replacement.c_str(), rule_id.c_str());
        const char* msg = sid_last_rewrite_message(static_cast<sid_engine*>(it->second.handle));
        std::string message = msg ? msg : "";

        // record event
        Event ev{};
        ev.event_id = static_cast<uint64_t>(it->second.events.size());
        ev.rule_id = rule_id;
        ev.applied = applied;
        ev.message = message;
        ev.metadata = metadata;
        ev.timestamp = now_seconds();
        it->second.events.push_back(ev);
        it->second.wrapper.rewrite_calls++;
        if (applied) it->second.wrapper.rewrites_applied++;

        json res = {{"engine_id", id}, {"rule_id", rule_id}, {"applied", applied}, {"message", message}};
        if (!metadata.empty()) res["rule_metadata"] = metadata;
        return success("sid_rewrite", res);
    }

    json handleEvents(const json& p) {
        const std::string id = p.value("engine_id", "");
        size_t cursor = static_cast<size_t>(p.value("cursor", 0));
        size_t limit = static_cast<size_t>(p.value("limit", 100));
        auto it = engines_.find(id);
        if (it == engines_.end()) return error("sid_rewrite_events", "engine not found", "ENGINE_NOT_FOUND");
        auto& evs = it->second.events;
        if (cursor > evs.size()) cursor = evs.size();
        size_t end = evs.size();
        if (limit > 0 && cursor + limit < end) end = cursor + limit;
        json out = json::array();
        for (size_t i = cursor; i < end; ++i) {
            const auto& ev = evs[i];
            out.push_back({{"event_id", ev.event_id}, {"rule_id", ev.rule_id}, {"applied", ev.applied}, {"message", ev.message}, {"timestamp", ev.timestamp}, {"metadata", ev.metadata}});
        }
        return success("sid_rewrite_events", {{"engine_id", id}, {"events", out}, {"next_cursor", cursor + out.size()}});
    }

    json handleApplyMotion(const json& p) {
        const std::string id = p.value("engine_id", "");
        size_t max_events = static_cast<size_t>(p.value("max_events", 0));
        auto it = engines_.find(id);
        if (it == engines_.end() || it->second.type != EngineEntry::Type::Ternary) {
            return error("sid_wrapper_apply_motion", "engine not found or not sid_ternary", "ENGINE_NOT_FOUND");
        }
        auto& entry = it->second;
        auto& w = entry.wrapper;
        auto& evs = entry.events;
        size_t start = w.event_cursor;
        size_t end = evs.size();
        if (max_events > 0 && start + max_events < end) end = start + max_events;

        auto apply_motion = [&](double eps, const std::string& rule_id) {
            if (eps <= 0.0) {
                w.last_motion = {{"rule_id", rule_id}, {"applied", false}, {"reason", "epsilon_invalid"}};
                return;
            }
            if (w.U_mass + 1e-12 < eps) {
                w.motion_refused_count++;
                w.last_motion = {{"rule_id", rule_id}, {"applied", false}, {"reason", "guard_failed"}};
                return;
            }
            w.U_mass -= eps;
            w.I_mass += eps;
            double total = w.I_mass + w.N_mass + w.U_mass;
            if (total > 0.0) {
                w.I_mass /= total;
                w.N_mass /= total;
                w.U_mass /= total;
            }
            w.motion_applied_count++;
            w.last_motion = {{"rule_id", rule_id}, {"applied", true}, {"reason", "applied"}};
        };

        for (size_t i = start; i < end; ++i) {
            const auto& ev = evs[i];
            w.event_cursor = i + 1;
            bool semantic_motion = false;
            double epsilon = 0.0;
            if (ev.metadata.is_object()) {
                auto mode_it = ev.metadata.find("mode");
                if (mode_it != ev.metadata.end() && mode_it->is_string() && *mode_it == "semantic_motion") {
                    semantic_motion = true;
                }
                if (ev.metadata.value("semantic_motion", false)) {
                    semantic_motion = true;
                }
                if (ev.metadata.contains("epsilon") && ev.metadata["epsilon"].is_number()) {
                    epsilon = ev.metadata["epsilon"].get<double>();
                }
            }
            if (semantic_motion && ev.applied) {
                if (epsilon <= 0.0) epsilon = entry.alpha;
                apply_motion(epsilon, ev.rule_id);
            }
        }

        return wrapperMetricsResponse("sid_wrapper_apply_motion", id, w);
    }

    json handleWrapperMetrics(const json& p) {
        const std::string id = p.value("engine_id", "");
        auto it = engines_.find(id);
        if (it == engines_.end() || it->second.type != EngineEntry::Type::Ternary) {
            return error("sid_wrapper_metrics", "engine not found or not sid_ternary", "ENGINE_NOT_FOUND");
        }
        return wrapperMetricsResponse("sid_wrapper_metrics", id, it->second.wrapper);
    }

    json handleRun(const json& p) {
        const std::string id = p.value("engine_id", "");
        int steps = p.value("steps", 1);
        auto it = engines_.find(id);
        if (it == engines_.end() || it->second.type != EngineEntry::Type::SSP) {
            return error("sid_run", "engine not found or not sid_ssp", "ENGINE_NOT_FOUND");
        }
        auto* ssp = static_cast<sid_ssp_t*>(it->second.handle);
        for (int i = 0; i < steps; ++i) sid_ssp_commit_step(ssp);
        it->second.ssp_steps += static_cast<uint64_t>(steps);
        return success("sid_run", {{"engine_id", id}, {"steps_completed", steps}, {"total_steps", it->second.ssp_steps}});
    }

    json wrapperMetricsResponse(const std::string& cmd, const std::string& id, const WrapperState& w) {
        double total = w.I_mass + w.N_mass + w.U_mass;
        bool conserved = std::abs(total - 1.0) < 1e-9 && w.I_mass >= -1e-12 && w.N_mass >= -1e-12 && w.U_mass >= -1e-12;
        json res = {
            {"engine_id", id},
            {"I_mass", w.I_mass},
            {"N_mass", w.N_mass},
            {"U_mass", w.U_mass},
            {"is_conserved_wrapper", conserved},
            {"motion_applied_count", w.motion_applied_count},
            {"motion_refused_count", w.motion_refused_count},
            {"rewrite_calls", w.rewrite_calls},
            {"rewrites_applied", w.rewrites_applied},
            {"event_cursor", w.event_cursor},
            {"last_motion", w.last_motion}
        };
        return success(cmd, res);
    }

    json success(const std::string& cmd, const json& result, double exec_ms = 0.0) {
        return json{{"command", cmd}, {"status", "success"}, {"result", result}, {"execution_time_ms", exec_ms}};
    }

    json error(const std::string& cmd, const std::string& msg, const std::string& code) {
        return json{{"command", cmd}, {"status", "error"}, {"error", msg}, {"error_code", code}, {"execution_time_ms", 0}};
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    SidCli cli;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        try {
            auto cmd = json::parse(line);
            auto resp = cli.handle(cmd);
            std::cout << resp.dump() << std::endl;
        } catch (const std::exception& e) {
            json resp = {{"status", "error"}, {"error", std::string("parse/exec error: ") + e.what()}, {"error_code", "INTERNAL_ERROR"}, {"execution_time_ms", 0}};
            std::cout << resp.dump() << std::endl;
        }
    }
    return 0;
}
