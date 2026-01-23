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

TEST(BasicComputeSubstrate, MetricsSmoke) {
    // Hash fixture to ensure deterministic read of kernel baseline.
    const auto fixture = harness::project_root() / "Simulation/tests/fixtures/basic_kernel.txt";
    std::vector<uint8_t> data;
    std::ifstream in(fixture, std::ios::binary);
    ASSERT_TRUE(in.is_open());
    data.assign(std::istreambuf_iterator<char>(in), {});
    auto h = harness::hash_bytes(data);
    EXPECT_EQ(h, "ae84256407f5f343");
    harness::write_metrics_json("basic_compute_substrate", "kernel_hash",
                                {{"hash_match", h == "ae84256407f5f343" ? 1.0 : 0.0}},
                                {{"hash", h}});
}

TEST(BasicComputeSubstrate, DeterminismPlaceholder) {
    GTEST_SKIP() << "TODO: implement determinism and stability checks with real fixtures.";
}

}  // namespace
