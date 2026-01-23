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

TEST(SidSsp, MetricsSmoke) {
    const auto fixture = harness::project_root() / "Simulation/tests/fixtures/sid_ssp_ast.txt";
    std::ifstream in(fixture);
    ASSERT_TRUE(in.is_open());
    std::string content((std::istreambuf_iterator<char>(in)), {});
    auto h1 = harness::hash_bytes(std::vector<uint8_t>(content.begin(), content.end()));
    auto h2 = harness::hash_bytes(std::vector<uint8_t>(content.begin(), content.end()));
    EXPECT_EQ(h1, h2);
    harness::write_metrics_json("sid_ssp", "ast_hash",
                                {{"hash_match", h1 == h2 ? 1.0 : 0.0}},
                                {{"hash", h1}});
}

TEST(SidSsp, RewriteDeterminismPlaceholder) {
    GTEST_SKIP() << "TODO: implement rewrite determinism and invariant preservation tests.";
}

}  // namespace
