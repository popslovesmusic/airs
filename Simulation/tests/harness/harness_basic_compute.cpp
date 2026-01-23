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
    auto hash = harness::run_step_runner_and_hash(runner, input, output);
    ASSERT_FALSE(hash.empty());
    EXPECT_EQ(hash, "abe8b580ae633901");
}

TEST(BasicComputeSubstrate, DeterminismPlaceholder) {
    GTEST_SKIP() << "TODO: implement determinism and stability checks with real fixtures.";
}

}  // namespace
