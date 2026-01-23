// Example harness tests validating policy parsing and metrics plumbing.
#include <gtest/gtest.h>

#include "harness_policy.hpp"
#include "harness_test_util.hpp"

#include <filesystem>
#include <fstream>

namespace {

TEST(PolicyTable, LoadsValidationTable) {
    auto table = harness::load_default_policy();
    // Spot-check expected allow/deny entries.
    EXPECT_TRUE(table.is_allowed("satp_higgs", "Vacuum Stability"));
    EXPECT_TRUE(table.is_allowed("igsoa_gw", "Echo Structure"));
    EXPECT_TRUE(table.is_allowed("sid_ternary", "Boundary Handling"));

    EXPECT_TRUE(table.is_forbidden("satp_higgs", "Mass Conservation"));
    EXPECT_TRUE(table.is_forbidden("igsoa_complex", "Transport Fidelity"));
    EXPECT_TRUE(table.is_forbidden("sid_ssp", "Numerical Accuracy"));
}

TEST(PolicyTable, UnknownEngineIsSafe) {
    auto table = harness::load_default_policy();
    EXPECT_FALSE(table.is_allowed("nonexistent_engine", "Determinism"));
    EXPECT_FALSE(table.is_forbidden("nonexistent_engine", "Determinism"));
}

TEST(MetricsWriter, WritesJsonToMetricsRoot) {
    const std::string engine = "basic_compute_substrate";
    const std::string test_case = "determinism_smoke";

    harness::write_metrics_json(engine, test_case,
                                {{"hash_match", 1.0}, {"runtime_ms", 0.0}},
                                {{"status", "pass"}});

    auto dir = harness::engine_metrics_dir(engine);
    bool found = false;
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;
        auto name = entry.path().filename().string();
        if (name.find("determinism_smoke") != std::string::npos) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST(DeterminismHelpers, HashIsStable) {
    std::vector<uint8_t> data{1, 2, 3, 4, 5};
    auto h1 = harness::hash_bytes(data);
    auto h2 = harness::hash_bytes(data);
    EXPECT_EQ(h1, h2);
}

}  // namespace
