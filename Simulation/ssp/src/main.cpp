#include <math.h>
#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include "sid_mixer.h"
#include "sid_semantic_processor.h"
}

static void fake_engine_step(double* field, uint64_t len, double gain) {
    for (uint64_t i = 0; i < len; ++i) {
        field[i] += gain * sin((double)i);
        if (field[i] < 0.0) field[i] = 0.0;
    }
}

int main(void) {
    const uint64_t FIELD_LEN = 128;
    const double TOTAL_MASS = 1000.0;
    const uint64_t STEPS = 20;

    sid_ssp_t* ssp_I = sid_ssp_create(SID_ROLE_I, FIELD_LEN, TOTAL_MASS);
    sid_ssp_t* ssp_N = sid_ssp_create(SID_ROLE_N, FIELD_LEN, TOTAL_MASS);
    sid_ssp_t* ssp_U = sid_ssp_create(SID_ROLE_U, FIELD_LEN, TOTAL_MASS);

    sid_mixer_t* mixer = sid_mixer_create(TOTAL_MASS);

    double* field_I = sid_ssp_field(ssp_I);
    double* field_N = sid_ssp_field(ssp_N);
    double* field_U = sid_ssp_field(ssp_U);

    for (uint64_t i = 0; i < FIELD_LEN; ++i) {
        field_U[i] = TOTAL_MASS / (double)FIELD_LEN;
    }

    for (uint64_t t = 0; t < STEPS; ++t) {
        fake_engine_step(field_I, FIELD_LEN, 0.01);
        fake_engine_step(field_N, FIELD_LEN, 0.02);
        fake_engine_step(field_U, FIELD_LEN, 0.005);

        sid_ssp_commit_step(ssp_I);
        sid_ssp_commit_step(ssp_N);
        sid_ssp_commit_step(ssp_U);

        sid_mixer_step(mixer, ssp_I, ssp_N, ssp_U);

        if (t % 5 == 0) {
            sid_mixer_request_collapse(mixer, ssp_I, ssp_N, ssp_U);
        }

        sid_mixer_metrics_t m = sid_mixer_metrics(mixer);

        printf("t=%llu | I=%.3f N=%.3f U=%.3f | err=%.6e | T=%d\n",
               (unsigned long long)t,
               m.admissible_volume,
               m.excluded_volume,
               m.undecided_volume,
               m.conservation_error,
               m.transport_ready);
    }

    sid_mixer_destroy(mixer);
    sid_ssp_destroy(ssp_I);
    sid_ssp_destroy(ssp_N);
    sid_ssp_destroy(ssp_U);

    return 0;
}
