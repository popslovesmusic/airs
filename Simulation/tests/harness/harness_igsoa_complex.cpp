// Harness tests for igsoa_complex engine family.
#include <gtest/gtest.h>
#include <fstream>
#include <string>

#include "harness_policy.hpp"
#include "harness_test_util.hpp"

namespace {

TEST(IgsoaComplex, PolicyCoversExpectedValidations) {
    auto table = harness::load_default_policy();
    ASSERT_TRUE(table.is_allowed("igsoa_complex", "Attractor Convergence"));
    ASSERT_TRUE(table.is_allowed("igsoa_complex", "Constraint Satisfaction"));
    ASSERT_TRUE(table.is_allowed("igsoa_complex", "Variance Suppression"));
    EXPECT_TRUE(table.is_forbidden("igsoa_complex", "Mass Conservation"));
    EXPECT_TRUE(table.is_forbidden("igsoa_complex", "Diffusion Accuracy"));
    EXPECT_TRUE(table.is_forbidden("igsoa_complex", "Transport Fidelity"));
}

TEST(IgsoaComplex, StepHashMatchesGolden) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/dase_step_runner.exe";
    auto input = root / "Simulation/tests/fixtures/inputs/igsoa_complex_step.jsonl";
    auto output = root / "artifacts/validation/igsoa_complex/out.json";
    auto hash = harness::run_step_runner_and_hash(runner, input, output);
    ASSERT_FALSE(hash.empty());
    EXPECT_EQ(hash, "bc2b53c7df81da87");
}

TEST(IgsoaComplex, AttractorPlaceholder) {
    GTEST_SKIP() << "TODO: implement attractor convergence and variance suppression tests.";
}

}  // namespace
