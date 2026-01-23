// Harness tests for igsoa_gw engine family.
#include <gtest/gtest.h>
#include <fstream>
#include <string>

#include "harness_policy.hpp"
#include "harness_test_util.hpp"

namespace {

TEST(IgsoaGw, PolicyCoversExpectedValidations) {
    auto table = harness::load_default_policy();
    ASSERT_TRUE(table.is_allowed("igsoa_gw", "Signal Arrival"));
    ASSERT_TRUE(table.is_allowed("igsoa_gw", "Echo Structure"));
    ASSERT_TRUE(table.is_allowed("igsoa_gw", "Resonance Spectrum"));
    ASSERT_TRUE(table.is_allowed("igsoa_gw", "Symmetry Response"));
    EXPECT_TRUE(table.is_forbidden("igsoa_gw", "Diffusion Convergence"));
    EXPECT_TRUE(table.is_forbidden("igsoa_gw", "Steady State"));
    EXPECT_TRUE(table.is_forbidden("igsoa_gw", "Semantic Tests"));
}

TEST(IgsoaGw, StepHashMatchesGolden) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/dase_step_runner.exe";
    auto input = root / "Simulation/tests/fixtures/inputs/gw_step.jsonl";
    auto output = root / "artifacts/validation/gw/out.json";
    auto result = harness::run_step_runner(runner, input, output);
    ASSERT_FALSE(result.hash.empty());
    ASSERT_NE(result.metrics.count("state_norm"), 0u);
    const double norm = result.metrics.at("state_norm");
    EXPECT_EQ(result.hash, "f8813f85bb44d6ef");
    EXPECT_DOUBLE_EQ(norm, 1.45005e-87);
}

TEST(IgsoaGw, Step10HashMatchesGolden) {
    auto root = harness::project_root();
    auto runner = root / "build/Debug/dase_step_runner.exe";
    auto input = root / "Simulation/tests/fixtures/inputs/gw_step_10.jsonl";
    auto output = root / "artifacts/validation/gw/out_step_10.json";
    auto result = harness::run_step_runner(runner, input, output);
    ASSERT_FALSE(result.hash.empty());
    ASSERT_NE(result.metrics.count("state_norm"), 0u);
    const double norm = result.metrics.at("state_norm");
    EXPECT_EQ(result.hash, "2ff6e74376eddfa2");
    EXPECT_DOUBLE_EQ(norm, 4.04971e-88);
}

TEST(IgsoaGw, EchoStructurePlaceholder) {
    GTEST_SKIP() << "TODO: implement echo structure/resonance spectrum tests with fixtures.";
}

}  // namespace
