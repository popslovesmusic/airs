/**
 * SID C API Implementation - Wraps C++ classes for C compatibility
 */

#include "sid_capi.hpp"
#include "sid_diagram.hpp"
#include "sid_mixer.hpp"
#include "sid_semantic_processor.hpp"
#include "../sid_ternary_engine.hpp"
#include <string>
#include <memory>

// Engine handle structure
struct sid_engine {
    std::unique_ptr<sid::SidTernaryEngine> engine;
};

extern "C" {

/* Diagram operations */
sid_diagram_t* sid_diagram_create(const char* diagram_id) {
    return reinterpret_cast<sid_diagram_t*>(new sid::Diagram(diagram_id));
}

void sid_diagram_destroy(sid_diagram_t* diagram) {
    delete reinterpret_cast<sid::Diagram*>(diagram);
}

int sid_diagram_has_cycle(const sid_diagram_t* diagram) {
    return reinterpret_cast<const sid::Diagram*>(diagram)->has_cycle() ? 1 : 0;
}

/* Mixer operations */
sid_mixer_t* sid_mixer_create(double total_mass_C) {
    return reinterpret_cast<sid_mixer_t*>(new sid::Mixer(total_mass_C));
}

void sid_mixer_destroy(sid_mixer_t* mixer) {
    delete reinterpret_cast<sid::Mixer*>(mixer);
}

void sid_mixer_step(sid_mixer_t* mixer, sid_ssp_t* ssp_I, sid_ssp_t* ssp_N, sid_ssp_t* ssp_U) {
    auto* m = reinterpret_cast<sid::Mixer*>(mixer);
    auto* i = reinterpret_cast<sid::SemanticProcessor*>(ssp_I);
    auto* n = reinterpret_cast<sid::SemanticProcessor*>(ssp_N);
    auto* u = reinterpret_cast<sid::SemanticProcessor*>(ssp_U);
    m->step(*i, *n, *u);
}

/* SSP operations */
sid_ssp_t* sid_ssp_create(int role, unsigned long field_len, double capacity) {
    return reinterpret_cast<sid_ssp_t*>(
        new sid::SemanticProcessor(static_cast<sid::Role>(role), field_len, capacity));
}

void sid_ssp_destroy(sid_ssp_t* ssp) {
    delete reinterpret_cast<sid::SemanticProcessor*>(ssp);
}

double* sid_ssp_field(sid_ssp_t* ssp) {
    auto& field = reinterpret_cast<sid::SemanticProcessor*>(ssp)->field();
    return field.data();
}

void sid_ssp_commit_step(sid_ssp_t* ssp) {
    reinterpret_cast<sid::SemanticProcessor*>(ssp)->commit_step();
}

/* Engine operations */
sid_engine* sid_create_engine(uint64_t num_nodes, double total_mass) {
    try {
        auto* eng = new sid_engine();
        eng->engine = std::make_unique<sid::SidTernaryEngine>(
            static_cast<size_t>(num_nodes),
            total_mass
        );
        return eng;
    } catch (...) {
        return nullptr;
    }
}

void sid_destroy_engine(sid_engine* eng) {
    delete eng;
}

void sid_step(sid_engine* eng, double alpha) {
    if (eng && eng->engine) {
        eng->engine->step(alpha);
    }
}

void sid_collapse(sid_engine* eng, double alpha) {
    if (eng && eng->engine) {
        eng->engine->collapse(alpha);
    }
}

double sid_get_I_mass(sid_engine* eng) {
    return (eng && eng->engine) ? eng->engine->getIMass() : 0.0;
}

double sid_get_N_mass(sid_engine* eng) {
    return (eng && eng->engine) ? eng->engine->getNMass() : 0.0;
}

double sid_get_U_mass(sid_engine* eng) {
    return (eng && eng->engine) ? eng->engine->getUMass() : 0.0;
}

double sid_get_instantaneous_gain(sid_engine* eng) {
    return (eng && eng->engine) ? eng->engine->getInstantaneousGain() : 0.0;
}

bool sid_is_conserved(sid_engine* eng, double tolerance) {
    return (eng && eng->engine) ? eng->engine->isConserved(tolerance) : false;
}

bool sid_last_rewrite_applied(sid_engine* eng) {
    return (eng && eng->engine) ? eng->engine->lastRewriteApplied() : false;
}

const char* sid_last_rewrite_message(sid_engine* eng) {
    if (eng && eng->engine) {
        static std::string msg_buffer;
        msg_buffer = eng->engine->lastRewriteMessage();
        return msg_buffer.c_str();
    }
    return "";
}

bool sid_apply_rewrite(sid_engine* eng, const char* pattern,
                       const char* replacement, const char* rule_id) {
    if (eng && eng->engine && pattern && replacement && rule_id) {
        return eng->engine->applyRewrite(pattern, replacement, rule_id);
    }
    return false;
}

bool sid_set_diagram_expr(sid_engine* eng, const char* expr, const char* rule_id) {
    if (eng && eng->engine && expr && rule_id) {
        return eng->engine->setDiagramExpr(expr, rule_id);
    }
    return false;
}

bool sid_set_diagram_json(sid_engine* eng, const char* json) {
    if (eng && eng->engine && json) {
        return eng->engine->setDiagramJson(json);
    }
    return false;
}

const char* sid_get_diagram_json(sid_engine* eng) {
    if (eng && eng->engine) {
        static std::string json_buffer;
        json_buffer = eng->engine->getDiagramJson();
        return json_buffer.c_str();
    }
    return "{}";
}

}  // extern "C"
