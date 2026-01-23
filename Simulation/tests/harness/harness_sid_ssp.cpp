// Harness tests for sid_ssp engine family.
#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <vector>

#include "harness_policy.hpp"
#include "harness_test_util.hpp"

namespace {

TEST(SidSsp, PolicyCoversExpectedValidations) {
    auto table = harness::load_default_policy();
    ASSERT_TRUE(table.is_allowed("sid_ssp", "Rewrite Determinism"));
    ASSERT_TRUE(table.is_allowed("sid_ssp", "Invariant Preservation"));
    ASSERT_TRUE(table.is_allowed("sid_ssp", "Constraint Satisfaction"));
    EXPECT_TRUE(table.is_forbidden("sid_ssp", "Numerical Accuracy"));
    EXPECT_TRUE(table.is_forbidden("sid_ssp", "Diffusion"));
    EXPECT_TRUE(table.is_forbidden("sid_ssp", "Transport"));
}

TEST(SidSsp, StepHashMatchesGolden) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/sid_step_runner.exe";
    auto input = root / "Simulation/tests/fixtures/inputs/sid_ssp_step.jsonl";
    auto output = root / "artifacts/validation/sid_ssp/out.json";
    auto result = harness::run_step_runner(runner, input, output);
    ASSERT_FALSE(result.hash.empty());
    ASSERT_NE(result.metrics.count("active_nodes"), 0u);
    const double active = result.metrics.at("active_nodes");
    EXPECT_EQ(result.hash, "8d447646765728c0");
    EXPECT_DOUBLE_EQ(active, 1024.0);
}

TEST(SidSsp, Step10HashMatchesGolden) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/sid_step_runner.exe";
    auto input = root / "Simulation/tests/fixtures/inputs/sid_ssp_step_10.jsonl";
    auto output = root / "artifacts/validation/sid_ssp/out_step_10.json";
    auto result = harness::run_step_runner(runner, input, output);
    ASSERT_FALSE(result.hash.empty());
    ASSERT_NE(result.metrics.count("active_nodes"), 0u);
    const double active = result.metrics.at("active_nodes");
    EXPECT_EQ(result.hash, "cfd8763f9c08d782");
    EXPECT_DOUBLE_EQ(active, 1024.0);
}

TEST(SidSsp, RewriteDeterminismPlaceholder) {
    GTEST_SKIP() << "TODO: implement rewrite determinism and invariant preservation tests.";
}

}  // namespace
