/**
 * SID C API Header - Exports for DASE CLI integration
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sid_diagram sid_diagram_t;
typedef struct sid_mixer sid_mixer_t;
typedef struct sid_ssp sid_ssp_t;
typedef struct sid_engine sid_engine;

/* Diagram operations */
sid_diagram_t* sid_diagram_create(const char* diagram_id);
void sid_diagram_destroy(sid_diagram_t* diagram);
int sid_diagram_has_cycle(const sid_diagram_t* diagram);

/* Mixer operations */
sid_mixer_t* sid_mixer_create(double total_mass_C);
void sid_mixer_destroy(sid_mixer_t* mixer);
void sid_mixer_step(sid_mixer_t* mixer, sid_ssp_t* ssp_I, sid_ssp_t* ssp_N, sid_ssp_t* ssp_U);

/* SSP operations */
sid_ssp_t* sid_ssp_create(int role, unsigned long field_len, double capacity);
void sid_ssp_destroy(sid_ssp_t* ssp);
double* sid_ssp_field(sid_ssp_t* ssp);
void sid_ssp_commit_step(sid_ssp_t* ssp);

/* Engine lifecycle */
sid_engine* sid_create_engine(uint64_t num_nodes, double total_mass);
void sid_destroy_engine(sid_engine* eng);

/* Evolution operations */
void sid_step(sid_engine* eng, double alpha);
void sid_collapse(sid_engine* eng, double alpha);

/* Metrics and queries */
double sid_get_I_mass(sid_engine* eng);
double sid_get_N_mass(sid_engine* eng);
double sid_get_U_mass(sid_engine* eng);
double sid_get_instantaneous_gain(sid_engine* eng);
bool sid_is_conserved(sid_engine* eng, double tolerance);

/* Rewrite system */
bool sid_apply_rewrite(sid_engine* eng, const char* pattern,
                       const char* replacement, const char* rule_id);
bool sid_last_rewrite_applied(sid_engine* eng);
const char* sid_last_rewrite_message(sid_engine* eng);

/* Diagram management */
bool sid_set_diagram_expr(sid_engine* eng, const char* expr, const char* rule_id);
bool sid_set_diagram_json(sid_engine* eng, const char* json);
const char* sid_get_diagram_json(sid_engine* eng);

#ifdef __cplusplus
}
#endif
