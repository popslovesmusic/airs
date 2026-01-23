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

TEST(SatpHiggs, MetricsSmoke) {
    const auto fixture = harness::project_root() / "Simulation/tests/fixtures/satp_state.txt";
    std::ifstream in(fixture);
    ASSERT_TRUE(in.is_open());
    std::string line;
    double last_phi = 0.0;
    int count = 0;
    while (std::getline(in, line)) {
        auto pos = line.find("phi=");
        ASSERT_NE(pos, std::string::npos);
        double phi = std::stod(line.substr(pos + 4));
        if (count == 0) last_phi = phi;
        EXPECT_GE(phi, last_phi);
        last_phi = phi;
        ++count;
    }
    harness::write_metrics_json("satp_higgs", "vacuum_stability_trend",
                                {{"samples", static_cast<double>(count)}},
                                {{"status", "monotonic_non_decreasing"}});
}

TEST(SatpHiggs, VacuumStabilityPlaceholder) {
    GTEST_SKIP() << "TODO: implement vacuum stability and phase transition tests with fixtures.";
}

}  // namespace
