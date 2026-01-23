// Harness tests for sid_ternary engine family.
#include <gtest/gtest.h>
#include <fstream>
#include <string>

#include "harness_policy.hpp"
#include "harness_test_util.hpp"

namespace {

TEST(SidTernary, PolicyCoversExpectedValidations) {
    auto table = harness::load_default_policy();
    ASSERT_TRUE(table.is_allowed("sid_ternary", "I/N/U Consistency"));
    ASSERT_TRUE(table.is_allowed("sid_ternary", "Determinism"));
    ASSERT_TRUE(table.is_allowed("sid_ternary", "Boundary Handling"));
    EXPECT_TRUE(table.is_forbidden("sid_ternary", "Time Evolution"));
    EXPECT_TRUE(table.is_forbidden("sid_ternary", "Numerical Dynamics"));
    EXPECT_TRUE(table.is_forbidden("sid_ternary", "Variance Analysis"));
}

TEST(SidTernary, MetricsSmoke) {
    const auto fixture = harness::project_root() / "Simulation/tests/fixtures/sid_ternary_cases.txt";
    std::ifstream in(fixture);
    ASSERT_TRUE(in.is_open());
    std::string line;
    int rows = 0;
    int i_count = 0, n_count = 0, u_count = 0;
    std::getline(in, line);  // header
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto comma = line.find(',');
        ASSERT_NE(comma, std::string::npos);
        char label = line[comma + 1];
        if (label == 'I') ++i_count;
        if (label == 'N') ++n_count;
        if (label == 'U') ++u_count;
        ++rows;
    }
    harness::write_metrics_json("sid_ternary", "case_distribution",
                                {{"rows", static_cast<double>(rows)},
                                 {"I", static_cast<double>(i_count)},
                                 {"N", static_cast<double>(n_count)},
                                 {"U", static_cast<double>(u_count)}});
    EXPECT_EQ(rows, 8);
    EXPECT_EQ(i_count + n_count + u_count, rows);
}

TEST(SidTernary, ConsistencyPlaceholder) {
    GTEST_SKIP() << "TODO: implement I/N/U consistency and boundary handling tests.";
}

}  // namespace
