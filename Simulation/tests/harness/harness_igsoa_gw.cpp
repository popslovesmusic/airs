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

TEST(IgsoaGw, MetricsSmoke) {
    const auto fixture = harness::project_root() / "Simulation/tests/fixtures/igsoa_gw_echo.csv";
    std::ifstream in(fixture);
    ASSERT_TRUE(in.is_open());
    std::string header;
    std::getline(in, header);
    double peak = 0.0;
    double peak_time = 0.0;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto comma = line.find(',');
        ASSERT_NE(comma, std::string::npos);
        double t = std::stod(line.substr(0, comma));
        double amp = std::stod(line.substr(comma + 1));
        if (amp > peak) {
            peak = amp;
            peak_time = t;
        }
    }
    harness::write_metrics_json("igsoa_gw", "echo_peak",
                                {{"peak_amplitude", peak}, {"peak_time", peak_time}});
    EXPECT_NEAR(peak, 0.60, 1e-6);
    EXPECT_NEAR(peak_time, 0.50, 1e-6);
}

TEST(IgsoaGw, EchoStructurePlaceholder) {
    GTEST_SKIP() << "TODO: implement echo structure/resonance spectrum tests with fixtures.";
}

}  // namespace
