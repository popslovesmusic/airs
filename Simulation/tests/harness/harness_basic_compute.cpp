// Harness tests for basic_compute_substrate engine family.
#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <vector>

#include "harness_policy.hpp"
#include "harness_test_util.hpp"

namespace {

TEST(BasicComputeSubstrate, PolicyCoversExpectedValidations) {
    auto table = harness::load_default_policy();
    ASSERT_TRUE(table.is_allowed("basic_compute_substrate", "Determinism"));
    ASSERT_TRUE(table.is_allowed("basic_compute_substrate", "Numerical Stability"));
    ASSERT_TRUE(table.is_allowed("basic_compute_substrate", "Performance Scaling"));
    ASSERT_TRUE(table.is_allowed("basic_compute_substrate", "Memory Safety"));
    EXPECT_TRUE(table.is_forbidden("basic_compute_substrate", "Diffusion"));
    EXPECT_TRUE(table.is_forbidden("basic_compute_substrate", "Transport"));
    EXPECT_TRUE(table.is_forbidden("basic_compute_substrate", "Semantics"));
}

TEST(BasicComputeSubstrate, StepHashMatchesGolden) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/dase_step_runner.exe";
    auto input = root / "Simulation/tests/fixtures/inputs/basic_compute_step.jsonl";
    auto output = root / "artifacts/validation/basic_compute_substrate/out.json";
    auto result = harness::run_step_runner(runner, input, output);
    ASSERT_FALSE(result.hash.empty());
    ASSERT_NE(result.metrics.count("state_norm"), 0u);
    const double norm = result.metrics.at("state_norm");
    EXPECT_EQ(result.hash, "956aeb0623414a47");
    EXPECT_DOUBLE_EQ(norm, 1024.0);
}

TEST(BasicComputeSubstrate, Step10HashMatchesGolden) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/dase_step_runner.exe";
    auto input = root / "Simulation/tests/fixtures/inputs/basic_compute_step_10.jsonl";
    auto output = root / "artifacts/validation/basic_compute_substrate/out_step_10.json";
    auto result = harness::run_step_runner(runner, input, output);
    ASSERT_FALSE(result.hash.empty());
    ASSERT_NE(result.metrics.count("state_norm"), 0u);
    const double norm = result.metrics.at("state_norm");
    EXPECT_EQ(result.hash, "6cd1323345fee7b1");
    EXPECT_DOUBLE_EQ(norm, 1.26765e30);
}

TEST(BasicComputeSubstrate, StateNormFiniteAndPositive) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/dase_step_runner.exe";
    auto input = root / "Simulation/tests/fixtures/inputs/basic_compute_step_10.jsonl";
    auto output = root / "artifacts/validation/basic_compute_substrate/out_step_10.json";
    auto result = harness::run_step_runner(runner, input, output);
    ASSERT_FALSE(result.hash.empty());
    ASSERT_NE(result.metrics.count("state_norm"), 0u);
    const double norm = result.metrics.at("state_norm");
    ASSERT_TRUE(std::isfinite(norm));
    ASSERT_GT(norm, 0.0);
}

TEST(BasicComputeSubstrate, DeterminismPlaceholder) {
    GTEST_SKIP() << "TODO: implement determinism and stability checks with real fixtures.";
}

}  // namespace
