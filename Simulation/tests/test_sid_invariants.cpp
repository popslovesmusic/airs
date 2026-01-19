/**
 * SID Invariant Tests - Runtime enforcement checks
 */

#include "../src/cpp/sid_ssp/sid_semantic_processor.hpp"
#include "../src/cpp/sid_ssp/sid_mixer.hpp"

#include <iostream>
#include <stdexcept>
#include <vector>

using namespace sid;

static int tests_passed = 0;
static int tests_failed = 0;

#define REQUIRE(cond, msg) \
    do { \
        if (!(cond)) { \
            throw std::runtime_error(msg); \
        } \
    } while (0)

#define TEST(name) \
    void test_##name(); \
    void run_test_##name() { \
        std::cout << "Running: " << #name << "... "; \
        try { \
            test_##name(); \
            std::cout << "PASS\n"; \
            tests_passed++; \
        } catch (const std::exception& e) { \
            std::cout << "FAIL: " << e.what() << "\n"; \
            tests_failed++; \
        } \
    } \
    void test_##name()

TEST(role_locking_violation) {
    SemanticProcessor ssp(Role::I, 4, 10.0);
    std::vector<double> mask(4, 0.5);

    bool threw = false;
    try {
        ssp.apply_collapse(mask, 1.0);
    } catch (const std::logic_error&) {
        threw = true;
    }
    REQUIRE(threw, "Expected role locking violation");
}

TEST(mask_validity_violation) {
    SemanticProcessor ssp(Role::U, 3, 10.0);
    CollapseMask mask(3);
    mask.mask_I[0] = 0.8;
    mask.mask_N[0] = 0.5;  // invalid sum > 1

    bool threw = false;
    try {
        ssp.apply_collapse_mask(mask, 0.5);
    } catch (const std::logic_error&) {
        threw = true;
    }
    REQUIRE(threw, "Expected mask validity violation");
}

TEST(conservation_violation) {
    const uint64_t len = 5;
    SemanticProcessor ssp_I(Role::I, len, 10.0);
    SemanticProcessor ssp_N(Role::N, len, 10.0);
    SemanticProcessor ssp_U(Role::U, len, 10.0);

    for (size_t i = 0; i < len; ++i) {
        ssp_I.field()[i] = 1.0;
        ssp_U.field()[i] = 0.0;
        ssp_N.field()[i] = 0.0;
    }

    Mixer mixer(1.0);

    bool threw = false;
    try {
        mixer.step(ssp_I, ssp_N, ssp_U);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    REQUIRE(threw, "Expected conservation violation");
}

TEST(mixer_boundedness_violation) {
    const uint64_t len = 10;
    const double total_mass = 100.0;

    SemanticProcessor ssp_I(Role::I, len, total_mass);
    SemanticProcessor ssp_N(Role::N, len, total_mass);
    SemanticProcessor ssp_U(Role::U, len, total_mass);

    for (size_t i = 0; i < len; ++i) {
        ssp_U.field()[i] = 0.1;
    }

    Mixer mixer(total_mass);

    bool threw = false;
    try {
        mixer.step(ssp_I, ssp_N, ssp_U);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    REQUIRE(threw, "Expected mixer boundedness violation");
}

int main() {
    std::cout << "SID Invariant Tests - Runtime Enforcement\n";
    std::cout << "=========================================\n\n";

    run_test_role_locking_violation();
    run_test_mask_validity_violation();
    run_test_conservation_violation();
    run_test_mixer_boundedness_violation();

    std::cout << "\n=========================================\n";
    std::cout << "Results: " << tests_passed << " passed, " << tests_failed << " failed\n";

    return (tests_failed > 0) ? 1 : 0;
}
