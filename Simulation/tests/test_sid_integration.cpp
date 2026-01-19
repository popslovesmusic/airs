/**
 * SID Integration Tests - Phase 5 expansion
 */

#include "../src/cpp/sid_ssp/sid_ast.hpp"
#include "../src/cpp/sid_ssp/sid_parser.hpp"
#include "../src/cpp/sid_ssp/sid_rewrite.hpp"
#include "../src/cpp/sid_ssp/sid_diagram.hpp"
#include "../src/cpp/sid_ssp/sid_validator.hpp"

#include <iostream>
#include <cassert>

using namespace sid;

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    void test_##name(); \
    void run_test_##name() { \
        std::cout << "Running: " << #name << "... "; \
        try { \
            test_##name(); \
            std::cout << "PASS\n"; \
            tests_passed++; \
        } catch (const std::exception& e) { \
            std::cout << "FAIL: " << e.what() << "\n"; \
            tests_failed++; \
        } \
    } \
    void test_##name()

static Diagram build_diagram_from_expr(const std::string& expr_text, const std::string& rule_id) {
    auto expr = parse_expression(expr_text);
    Diagram diagram;
    IDGenerator node_gen(rule_id + "_n");
    IDGenerator edge_gen(rule_id + "_e");
    Bindings bindings;
    build_expr(*expr, diagram, bindings, rule_id, node_gen, edge_gen);
    return diagram;
}

TEST(integration_parse_build_validate) {
    Diagram diagram = build_diagram_from_expr("C(P(Freedom), O(P(Choice)))", "rw");
    auto result = DiagramValidator::validate(diagram);
    assert(result.is_valid());
}

TEST(integration_rewrite_then_validate) {
    Diagram diagram = build_diagram_from_expr("C(P(Freedom), P(Choice))", "rw");
    auto pattern = parse_expression("C(P($x), P($y))");
    auto replacement = parse_expression("C(P($x), O(P($y)))");

    auto rewrite = apply_rewrite(diagram, *pattern, *replacement, "rw");
    assert(rewrite.applied);

    auto result = DiagramValidator::validate(diagram);
    assert(result.is_valid());
    assert(!diagram.has_cycle());
}

TEST(integration_no_match_no_change) {
    Diagram diagram = build_diagram_from_expr("P(Freedom)", "rw");
    auto pattern = parse_expression("C(P($x), P($y))");
    auto replacement = parse_expression("O(P($x))");

    auto result = apply_rewrite(diagram, *pattern, *replacement, "rw");
    assert(!result.applied);
    assert(diagram.node_count() == 2);
}

int main() {
    std::cout << "SID Integration Tests\n";
    std::cout << "=====================\n\n";

    run_test_integration_parse_build_validate();
    run_test_integration_rewrite_then_validate();
    run_test_integration_no_match_no_change();

    std::cout << "\n=====================\n";
    std::cout << "Results: " << tests_passed << " passed, " << tests_failed << " failed\n";

    return (tests_failed > 0) ? 1 : 0;
}
