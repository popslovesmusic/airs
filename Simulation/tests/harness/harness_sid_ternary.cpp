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
    auto hash = harness::run_step_runner_and_hash(runner, input, output);
    ASSERT_FALSE(hash.empty());
    EXPECT_EQ(hash, "441585dd8f25f8df");
}

TEST(SidTernary, ConsistencyPlaceholder) {
    GTEST_SKIP() << "TODO: implement I/N/U consistency and boundary handling tests.";
}

}  // namespace
