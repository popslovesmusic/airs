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
    auto hash = harness::run_step_runner_and_hash(runner, input, output);
    ASSERT_FALSE(hash.empty());
    EXPECT_EQ(hash, "441585dd8f25f8df");
}

TEST(SidSsp, RewriteDeterminismPlaceholder) {
    GTEST_SKIP() << "TODO: implement rewrite determinism and invariant preservation tests.";
}

}  // namespace
