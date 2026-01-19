/**
 * SID Regression Tests - Bug fixes coverage
 */

#include "../src/cpp/sid_ssp/sid_ast.hpp"
#include "../src/cpp/sid_ssp/sid_parser.hpp"
#include "../src/cpp/sid_ssp/sid_rewrite.hpp"
#include "../src/cpp/sid_ssp/sid_diagram.hpp"
#include "../src/cpp/sid_ssp/sid_mixer.hpp"
#include "../src/cpp/sid_ssp/sid_semantic_processor.hpp"

#include <iostream>
#include <cassert>
#include <unordered_set>

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

TEST(regression_off_by_one_matching) {
    Diagram diagram = build_diagram_from_expr("S+(P(A), P(B))", "rw");
    auto pattern = parse_expression("S+(P($x))");
    auto replacement = parse_expression("S+(P($x))");

    auto result = apply_rewrite(diagram, *pattern, *replacement, "rw");
    assert(!result.applied);
}

TEST(regression_id_generator_edges) {
    Diagram diagram = build_diagram_from_expr("P(A)", "rw");
    auto pattern = parse_expression("P($x)");
    auto replacement = parse_expression("O(P($x))");

    auto result = apply_rewrite(diagram, *pattern, *replacement, "rw");
    assert(result.applied);

    std::unordered_set<std::string> node_ids;
    for (const auto& [id, node] : diagram.nodes()) {
        node_ids.insert(id);
    }

    for (const auto& [edge_id, edge] : diagram.edges()) {
        assert(node_ids.count(edge_id) == 0);
        assert(edge_id.rfind("rw_e", 0) == 0);
    }
}

TEST(regression_unbounded_growth) {
    Mixer mixer(10, 100.0);

    mixer.U().scale_all(0.001);
    double before = mixer.U().total_mass();
    mixer.step(0.5);
    double after = mixer.U().total_mass();

    assert(before < 1.0);
    assert(after <= 2.0);
    assert(mixer.is_conserved(1e-6));
}

TEST(regression_literal_lowercase_not_variable) {
    Diagram diagram = build_diagram_from_expr("P(Bar)", "rw");
    auto pattern = parse_expression("P(foo)");

    auto match = find_expr_match(diagram, *pattern);
    assert(!match.has_value());
}

TEST(regression_bound_node_retained) {
    Diagram diagram = build_diagram_from_expr("P(Freedom)", "rw");
    auto pattern = parse_expression("P($x)");
    auto replacement = parse_expression("O($x)");

    auto result = apply_rewrite(diagram, *pattern, *replacement, "rw");
    assert(result.applied);

    bool found_atom = false;
    for (const auto& [id, node] : diagram.nodes()) {
        if (node.type == "Atom") {
            found_atom = true;
            break;
        }
    }
    assert(found_atom);
}

TEST(regression_operator_identifier_disambiguation) {
    auto expr = parse_expression("Peace");
    assert(expr->is_atom());
    assert(expr->as_atom()->name == "Peace");
}

TEST(regression_parser_arity_enforced) {
    bool threw = false;
    try {
        parse_expression("C");
    } catch (const ParseError&) {
        threw = true;
    }
    assert(threw);
}

TEST(regression_atom_matches_dof_ref) {
    Diagram diagram = build_diagram_from_expr("P(Freedom)", "rw");
    auto pattern = parse_expression("Freedom");

    auto match = find_expr_match(diagram, *pattern);
    assert(match.has_value());
}

TEST(regression_remove_edge_updates_inputs) {
    Diagram diagram;
    diagram.add_node(Node("n1", "A"));
    diagram.add_node(Node("n2", "B"));
    diagram.add_edge(Edge("e1", "n1", "n2", 0, 0));

    assert(diagram.get_inputs("n2").size() == 1);
    assert(diagram.remove_edge("e1"));
    assert(diagram.get_inputs("n2").empty());
}

TEST(regression_mask_validation) {
    CollapseMask mask(2);
    mask.mask_I[0] = 0.5;
    mask.mask_N[0] = 0.5;
    mask.mask_I[1] = -0.1;
    mask.mask_N[1] = 1.1;

    assert(!mask.is_valid());
}

TEST(regression_route_mask_clamp) {
    SemanticProcessor src(2);
    SemanticProcessor dst(2);
    src.field_mut()[0] = 1.0;
    src.field_mut()[1] = 1.0;

    std::vector<double> mask = {-1.0, 2.0};
    src.route_to(dst, mask, 1.0);

    assert(dst.field()[0] == 0.0);
    assert(dst.field()[1] == 1.0);
}

TEST(regression_cycle_detection_iterative) {
    Diagram diagram;

    const int count = 2000;
    for (int i = 0; i < count; ++i) {
        diagram.add_node(Node("n" + std::to_string(i), "N"));
    }
    for (int i = 0; i < count - 1; ++i) {
        diagram.add_edge(Edge("e" + std::to_string(i),
                              "n" + std::to_string(i),
                              "n" + std::to_string(i + 1),
                              0, 0));
    }

    assert(!diagram.has_cycle());
}

int main() {
    std::cout << "SID Regression Tests\n";
    std::cout << "====================\n\n";

    run_test_regression_off_by_one_matching();
    run_test_regression_id_generator_edges();
    run_test_regression_unbounded_growth();
    run_test_regression_literal_lowercase_not_variable();
    run_test_regression_bound_node_retained();
    run_test_regression_operator_identifier_disambiguation();
    run_test_regression_parser_arity_enforced();
    run_test_regression_atom_matches_dof_ref();
    run_test_regression_remove_edge_updates_inputs();
    run_test_regression_mask_validation();
    run_test_regression_route_mask_clamp();
    run_test_regression_cycle_detection_iterative();

    std::cout << "\n====================\n";
    std::cout << "Results: " << tests_passed << " passed, " << tests_failed << " failed\n";

    return (tests_failed > 0) ? 1 : 0;
}
