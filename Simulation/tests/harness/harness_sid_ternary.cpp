// Harness tests for sid_ternary engine family.
#include <gtest/gtest.h>
#include <fstream>
#include <string>

#include "harness_policy.hpp"
#include "harness_test_util.hpp"

namespace {

TEST(SidTernary, PolicyCoversExpectedValidations) {
    auto table = harness::load_default_policy();
    ASSERT_TRUE(table.is_allowed("sid_ternary", "I/N/U Consistency"));
    ASSERT_TRUE(table.is_allowed("sid_ternary", "Determinism"));
    ASSERT_TRUE(table.is_allowed("sid_ternary", "Boundary Handling"));
    EXPECT_TRUE(table.is_forbidden("sid_ternary", "Time Evolution"));
    EXPECT_TRUE(table.is_forbidden("sid_ternary", "Numerical Dynamics"));
    EXPECT_TRUE(table.is_forbidden("sid_ternary", "Variance Analysis"));
}

TEST(SidTernary, StepHashMatchesGolden) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/sid_step_runner.exe";
    auto input = root / "Simulation/tests/fixtures/inputs/sid_ternary_step.jsonl";
    auto output = root / "artifacts/validation/sid_ternary/out.json";
    auto result = harness::run_step_runner(runner, input, output);
    ASSERT_FALSE(result.hash.empty());
    ASSERT_NE(result.metrics.count("active_nodes"), 0u);
    const double active = result.metrics.at("active_nodes");
    EXPECT_EQ(result.hash, "956d1239323d716f");
    EXPECT_DOUBLE_EQ(active, 1024.0);
}

TEST(SidTernary, Step10HashMatchesGolden) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/sid_step_runner.exe";
    auto input = root / "Simulation/tests/fixtures/inputs/sid_ternary_step_10.jsonl";
    auto output = root / "artifacts/validation/sid_ternary/out_step_10.json";
    auto result = harness::run_step_runner(runner, input, output);
    ASSERT_FALSE(result.hash.empty());
    ASSERT_NE(result.metrics.count("active_nodes"), 0u);
    const double active = result.metrics.at("active_nodes");
    EXPECT_EQ(result.hash, "225c0af20e263657");
    EXPECT_DOUBLE_EQ(active, 1024.0);
}

TEST(SidTernary, ActiveNodesWithinBounds) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/sid_step_runner.exe";
    auto input = root / "Simulation/tests/fixtures/inputs/sid_ternary_step_10.jsonl";
    auto output = root / "artifacts/validation/sid_ternary/out_step_10.json";
    auto result = harness::run_step_runner(runner, input, output);
    ASSERT_FALSE(result.hash.empty());
    ASSERT_NE(result.metrics.count("active_nodes"), 0u);
    const double active = result.metrics.at("active_nodes");
    ASSERT_GE(active, 0.0);
    ASSERT_LE(active, 1024.0);  // max_nodes for fixture
}

}  // namespace
