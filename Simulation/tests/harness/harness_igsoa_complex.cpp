// Harness tests for igsoa_complex engine family.
#include <gtest/gtest.h>
#include <fstream>
#include <string>

#include "harness_policy.hpp"
#include "harness_test_util.hpp"

namespace {

TEST(IgsoaComplex, PolicyCoversExpectedValidations) {
    auto table = harness::load_default_policy();
    ASSERT_TRUE(table.is_allowed("igsoa_complex", "Attractor Convergence"));
    ASSERT_TRUE(table.is_allowed("igsoa_complex", "Constraint Satisfaction"));
    ASSERT_TRUE(table.is_allowed("igsoa_complex", "Variance Suppression"));
    EXPECT_TRUE(table.is_forbidden("igsoa_complex", "Mass Conservation"));
    EXPECT_TRUE(table.is_forbidden("igsoa_complex", "Diffusion Accuracy"));
    EXPECT_TRUE(table.is_forbidden("igsoa_complex", "Transport Fidelity"));
}

TEST(IgsoaComplex, MetricsSmoke) {
    const auto fixture = harness::project_root() / "Simulation/tests/fixtures/igsoa_complex_state.txt";
    std::ifstream in(fixture);
    ASSERT_TRUE(in.is_open());
    std::string line;
    double first_residual = -1.0;
    double last_residual = 0.0;
    int count = 0;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto pos = line.find("residual=");
        ASSERT_NE(pos, std::string::npos);
        double res = std::stod(line.substr(pos + 9));
        if (count == 0) first_residual = res;
        last_residual = res;
        ++count;
    }
    harness::write_metrics_json("igsoa_complex", "residual_drop",
                                {{"initial_residual", first_residual},
                                 {"final_residual", last_residual},
                                 {"iterations", static_cast<double>(count)}});
    EXPECT_GT(first_residual, 0.0);
    EXPECT_LT(last_residual, first_residual);
}

TEST(IgsoaComplex, AttractorPlaceholder) {
    GTEST_SKIP() << "TODO: implement attractor convergence and variance suppression tests.";
}

}  // namespace
