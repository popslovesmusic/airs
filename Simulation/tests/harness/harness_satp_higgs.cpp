// Harness tests for satp_higgs engine family.
#include <gtest/gtest.h>
#include <fstream>
#include <string>

#include "harness_policy.hpp"
#include "harness_test_util.hpp"

namespace {

TEST(SatpHiggs, PolicyCoversExpectedValidations) {
    auto table = harness::load_default_policy();
    ASSERT_TRUE(table.is_allowed("satp_higgs", "Vacuum Stability"));
    ASSERT_TRUE(table.is_allowed("satp_higgs", "Symmetry Breaking"));
    ASSERT_TRUE(table.is_allowed("satp_higgs", "Phase Transition Detection"));
    ASSERT_TRUE(table.is_allowed("satp_higgs", "Boundedness"));
    EXPECT_TRUE(table.is_forbidden("satp_higgs", "Mass Conservation"));
    EXPECT_TRUE(table.is_forbidden("satp_higgs", "Diffusion Accuracy"));
    EXPECT_TRUE(table.is_forbidden("satp_higgs", "Random Walk"));
}

TEST(SatpHiggs, StepHashMatchesGolden) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/dase_step_runner.exe";
    auto input = root / "Simulation/tests/fixtures/inputs/satp_higgs_step.jsonl";
    auto output = root / "artifacts/validation/satp_higgs/out.json";
    auto result = harness::run_step_runner(runner, input, output);
    ASSERT_FALSE(result.hash.empty());
    ASSERT_NE(result.metrics.count("state_norm"), 0u);
    const double norm = result.metrics.at("state_norm");
    EXPECT_EQ(result.hash, "14650fb0739d0383");
    EXPECT_DOUBLE_EQ(norm, 0.0);
}

TEST(SatpHiggs, Step10HashMatchesGolden) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/dase_step_runner.exe";
    auto input = root / "Simulation/tests/fixtures/inputs/satp_higgs_step_10.jsonl";
    auto output = root / "artifacts/validation/satp_higgs/out_step_10.json";
    auto result = harness::run_step_runner(runner, input, output);
    ASSERT_FALSE(result.hash.empty());
    ASSERT_NE(result.metrics.count("state_norm"), 0u);
    const double norm = result.metrics.at("state_norm");
    EXPECT_EQ(result.hash, "9cd18930a3a8c474");
    EXPECT_DOUBLE_EQ(norm, 0.0);
}

TEST(SatpHiggs, VacuumStabilityPlaceholder) {
    GTEST_SKIP() << "TODO: implement vacuum stability and phase transition tests with fixtures.";
}

}  // namespace
