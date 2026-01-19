/**
 * SID Rewrite Tests - Phase 5 expansion
 */

#include "../src/cpp/sid_ssp/sid_ast.hpp"
#include "../src/cpp/sid_ssp/sid_parser.hpp"
#include "../src/cpp/sid_ssp/sid_rewrite.hpp"
#include "../src/cpp/sid_ssp/sid_diagram.hpp"

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

TEST(rewrite_nested_patterns) {
    Diagram diagram = build_diagram_from_expr("C(P(Freedom), O(P(Choice)))", "rw");
    auto pattern = parse_expression("C(P(Freedom), O(P(Choice)))");

    auto match = find_expr_match(diagram, *pattern);
    assert(match.has_value());
}

TEST(rewrite_variable_binding) {
    Diagram diagram = build_diagram_from_expr("P(Freedom)", "rw");
    auto pattern = parse_expression("P($x)");
    auto replacement = parse_expression("O($x)");

    auto result = apply_rewrite(diagram, *pattern, *replacement, "rw");
    assert(result.applied);

    bool has_p = false;
    bool has_o = false;
    for (const auto& [id, node] : diagram.nodes()) {
        if (node.type == "P") has_p = true;
        if (node.type == "O") has_o = true;
    }
    assert(!has_p);
    assert(has_o);
}

TEST(rewrite_multiple_matches) {
    Diagram diagram;
    IDGenerator node_gen("rw_n");
    IDGenerator edge_gen("rw_e");
    Bindings bindings;

    build_expr(*parse_expression("P(A)"), diagram, bindings, "rw", node_gen, edge_gen);
    build_expr(*parse_expression("P(B)"), diagram, bindings, "rw", node_gen, edge_gen);

    auto pattern = parse_expression("P($x)");
    auto replacement = parse_expression("O($x)");

    auto result = apply_rewrite_until_fixpoint(diagram, *pattern, *replacement, "rw");
    assert(result.converged);
    assert(result.iterations == 2);

    for (const auto& [id, node] : diagram.nodes()) {
        assert(node.type != "P");
    }
}

int main() {
    std::cout << "SID Rewrite Tests\n";
    std::cout << "=================\n\n";

    run_test_rewrite_nested_patterns();
    run_test_rewrite_variable_binding();
    run_test_rewrite_multiple_matches();

    std::cout << "\n=================\n";
    std::cout << "Results: " << tests_passed << " passed, " << tests_failed << " failed\n";

    return (tests_failed > 0) ? 1 : 0;
}
