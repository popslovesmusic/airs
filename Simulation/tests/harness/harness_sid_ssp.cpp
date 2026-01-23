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
    ASSERT_NE(result.metrics.count("total_mass"), 0u);
    const double active = result.metrics.at("active_nodes");
    const double mass = result.metrics.at("total_mass");
    EXPECT_EQ(result.hash, "c541819c637d6b0");
    EXPECT_DOUBLE_EQ(active, 1024.0);
    EXPECT_DOUBLE_EQ(mass, 1024.0);
}

TEST(SidSsp, Step10HashMatchesGolden) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/sid_step_runner.exe";
    auto input = root / "Simulation/tests/fixtures/inputs/sid_ssp_step_10.jsonl";
    auto output = root / "artifacts/validation/sid_ssp/out_step_10.json";
    auto result = harness::run_step_runner(runner, input, output);
    ASSERT_FALSE(result.hash.empty());
    ASSERT_NE(result.metrics.count("active_nodes"), 0u);
    ASSERT_NE(result.metrics.count("total_mass"), 0u);
    const double active = result.metrics.at("active_nodes");
    const double mass = result.metrics.at("total_mass");
    EXPECT_EQ(result.hash, "cd2074f5eb51562a");
    EXPECT_DOUBLE_EQ(active, 1024.0);
    EXPECT_DOUBLE_EQ(mass, 1024.0);
}

TEST(SidSsp, ActiveNodesWithinBounds) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/sid_step_runner.exe";
    auto input = root / "Simulation/tests/fixtures/inputs/sid_ssp_step_10.jsonl";
    auto output = root / "artifacts/validation/sid_ssp/out_step_10.json";
    auto result = harness::run_step_runner(runner, input, output);
    ASSERT_FALSE(result.hash.empty());
    ASSERT_NE(result.metrics.count("active_nodes"), 0u);
    const double active = result.metrics.at("active_nodes");
    ASSERT_GE(active, 0.0);
    ASSERT_LE(active, 1024.0);  // fixture max_nodes
}

TEST(SidSsp, TotalMassConservedBetweenStep1AndStep10) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/sid_step_runner.exe";
    auto input1 = root / "Simulation/tests/fixtures/inputs/sid_ssp_step.jsonl";
    auto out1 = root / "artifacts/validation/sid_ssp/out.json";
    auto input10 = root / "Simulation/tests/fixtures/inputs/sid_ssp_step_10.jsonl";
    auto out10 = root / "artifacts/validation/sid_ssp/out_step_10.json";
    auto r1 = harness::run_step_runner(runner, input1, out1);
    auto r10 = harness::run_step_runner(runner, input10, out10);
    ASSERT_NE(r1.metrics.count("total_mass"), 0u);
    ASSERT_NE(r10.metrics.count("total_mass"), 0u);
    EXPECT_DOUBLE_EQ(r1.metrics.at("total_mass"), r10.metrics.at("total_mass"));
}

}  // namespace
