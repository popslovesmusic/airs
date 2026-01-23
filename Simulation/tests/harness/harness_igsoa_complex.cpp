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
    auto result = harness::run_step_runner(runner, input, output);
    ASSERT_FALSE(result.hash.empty());
    ASSERT_NE(result.metrics.count("state_norm"), 0u);
    const double norm = result.metrics.at("state_norm");
    EXPECT_EQ(result.hash, "f1ecbfa6e26f8fff");
    EXPECT_DOUBLE_EQ(norm, 32.0);
}

TEST(IgsoaComplex, Step10HashMatchesGolden) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/dase_step_runner.exe";
    auto input = root / "Simulation/tests/fixtures/inputs/igsoa_complex_step_10.jsonl";
    auto output = root / "artifacts/validation/igsoa_complex/out_step_10.json";
    auto result = harness::run_step_runner(runner, input, output);
    ASSERT_FALSE(result.hash.empty());
    ASSERT_NE(result.metrics.count("state_norm"), 0u);
    const double norm = result.metrics.at("state_norm");
    EXPECT_EQ(result.hash, "f9a551d86d0a29c9");
    EXPECT_DOUBLE_EQ(norm, 32.0);
}

TEST(IgsoaComplex, StateNormFiniteAndPositive) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/dase_step_runner.exe";
    auto input = root / "Simulation/tests/fixtures/inputs/igsoa_complex_step_10.jsonl";
    auto output = root / "artifacts/validation/igsoa_complex/out_step_10.json";
    auto result = harness::run_step_runner(runner, input, output);
    ASSERT_FALSE(result.hash.empty());
    ASSERT_NE(result.metrics.count("state_norm"), 0u);
    const double norm = result.metrics.at("state_norm");
    ASSERT_TRUE(std::isfinite(norm));
    ASSERT_GT(norm, 0.0);
}

TEST(IgsoaComplex, DriftEnvelopeStateNorm) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/dase_step_runner.exe";
    auto input1 = root / "Simulation/tests/fixtures/inputs/igsoa_complex_step.jsonl";
    auto out1 = root / "artifacts/validation/igsoa_complex/out.json";
    auto input10 = root / "Simulation/tests/fixtures/inputs/igsoa_complex_step_10.jsonl";
    auto out10 = root / "artifacts/validation/igsoa_complex/out_step_10.json";
    auto r1 = harness::run_step_runner(runner, input1, out1);
    auto r10 = harness::run_step_runner(runner, input10, out10);
    ASSERT_NE(r1.metrics.count("state_norm"), 0u);
    ASSERT_NE(r10.metrics.count("state_norm"), 0u);
    const double n1 = r1.metrics.at("state_norm");
    const double n10 = r10.metrics.at("state_norm");
    ASSERT_GT(n1, 0.0);
    ASSERT_LE(n10, 1e6 * n1);
}

TEST(IgsoaComplex, AttractorPlaceholder) {
    GTEST_SKIP() << "TODO: implement attractor convergence and variance suppression tests.";
}

}  // namespace
