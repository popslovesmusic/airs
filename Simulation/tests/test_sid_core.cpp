/**
 * SID Core Tests - Basic validation and smoke tests
 * Tests Phase 1-2 components: AST, Parser, Diagram, Validator
 */

#include "../src/cpp/sid_ssp/sid_ast.hpp"
#include "../src/cpp/sid_ssp/sid_parser_impl.hpp"
#include "../src/cpp/sid_ssp/sid_diagram.hpp"
#include "../src/cpp/sid_ssp/sid_diagram_builder.hpp"
#include "../src/cpp/sid_ssp/sid_validator.hpp"
#include "../src/cpp/sid_ssp/sid_semantic_processor.hpp"
#include "../src/cpp/sid_ssp/sid_mixer.hpp"
#include "../src/cpp/sid_ssp/sid_rewrite.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace sid;

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define REQUIRE(cond, msg) \
    do { \
        if (!(cond)) { \
            throw std::runtime_error(msg); \
        } \
    } while (0)

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

static bool remove_node(Diagram& diagram, const std::string& node_id) {
    auto& nodes = diagram.nodes();
    auto& edges = diagram.edges();

    auto node_it = std::find_if(nodes.begin(), nodes.end(), [&](const Node& node) {
        return node.id == node_id;
    });
    if (node_it == nodes.end()) {
        return false;
    }

    nodes.erase(node_it);
    edges.erase(
        std::remove_if(edges.begin(), edges.end(), [&](const Edge& edge) {
            return edge.from == node_id || edge.to == node_id;
        }),
        edges.end());
    diagram.mark_dirty();
    return true;
}

static std::vector<std::string> get_source_nodes(const Diagram& diagram) {
    std::vector<std::string> sources;
    for (const auto& node : diagram.nodes()) {
        bool has_incoming = false;
        for (const auto& edge : diagram.edges()) {
            if (edge.to == node.id) {
                has_incoming = true;
                break;
            }
        }
        if (!has_incoming) {
            sources.push_back(node.id);
        }
    }
    return sources;
}

static std::vector<std::string> get_sink_nodes(const Diagram& diagram) {
    std::vector<std::string> sinks;
    for (const auto& node : diagram.nodes()) {
        bool has_outgoing = false;
        for (const auto& edge : diagram.edges()) {
            if (edge.from == node.id) {
                has_outgoing = true;
                break;
            }
        }
        if (!has_outgoing) {
            sinks.push_back(node.id);
        }
    }
    return sinks;
}

// ============================================================================
// AST Tests
// ============================================================================

TEST(ast_atom_creation) {
    auto atom = make_atom("Freedom");
    REQUIRE(atom->index() == 0, "Expected atom variant");
    REQUIRE(std::get<Atom>(*atom).name == "Freedom", "Atom name mismatch");
}

TEST(ast_op_creation) {
    auto arg1 = make_atom("x");
    auto arg2 = make_atom("y");
    auto op = make_op("Add", {arg1, arg2});

    REQUIRE(op->index() == 1, "Expected op variant");
    REQUIRE(std::get<OpExpr>(*op).op == "Add", "Op name mismatch");
    REQUIRE(std::get<OpExpr>(*op).args.size() == 2, "Op args mismatch");
}

TEST(ast_expr_to_string) {
    auto arg = make_atom("Freedom");
    auto op = make_op("P", {arg});

    std::string result = expr_to_string(*op);
    REQUIRE(result == "P(Freedom)", "Expr string mismatch");
}

TEST(ast_expr_clone) {
    auto original = make_op("P", {make_atom("Freedom")});
    auto cloned = expr_clone(*original);

    REQUIRE(cloned != nullptr, "Clone returned null");
    REQUIRE(expr_to_string(*original) == expr_to_string(*cloned), "Clone mismatch");
}

// ============================================================================
// Parser Tests
// ============================================================================

TEST(parser_simple_atom) {
    auto expr = parseExpression("Freedom");
    REQUIRE(expr.kind == ASTKind::Atom, "Expected atom");
    REQUIRE(expr.atom_name == "Freedom", "Atom name mismatch");
}

TEST(parser_simple_operator) {
    auto expr = parseExpression("P(Freedom)");
    REQUIRE(expr.kind == ASTKind::Op, "Expected operator");
    REQUIRE(expr.op_name == "P", "Operator name mismatch");
    REQUIRE(expr.args.size() == 1, "Operator arg count mismatch");
    REQUIRE(expr.args[0].kind == ASTKind::Atom, "Expected atom argument");
}

TEST(parser_nested_operators) {
    auto expr = parseExpression("C(P(Freedom), P(Justice))");
    REQUIRE(expr.kind == ASTKind::Op, "Expected operator");
    REQUIRE(expr.op_name == "C", "Operator name mismatch");
    REQUIRE(expr.args.size() == 2, "Operator arg count mismatch");
}

TEST(parser_multi_arg_operator) {
    auto expr = parseExpression("S+(Freedom, Justice, Peace)");
    REQUIRE(expr.kind == ASTKind::Op, "Expected operator");
    REQUIRE(expr.op_name == "S+", "Operator name mismatch");
    REQUIRE(expr.args.size() == 3, "Operator arg count mismatch");
}

TEST(parser_invalid_expression_throws) {
    bool caught = false;
    try {
        parseExpression("P(");
    } catch (const ParseError&) {
        caught = true;
    }
    REQUIRE(caught, "Expected parse error");
}

// ============================================================================
// Diagram Tests
// ============================================================================

TEST(diagram_add_nodes) {
    Diagram diagram;

    Node node1("n1", "Atom");
    Node node2("n2", "Add");

    diagram.add_node(node1);
    diagram.add_node(node2);

    REQUIRE(diagram.nodes().size() == 2, "Node count mismatch");
    REQUIRE(diagram.find_node("n1") != nullptr, "Node n1 missing");
    REQUIRE(diagram.find_node("n2") != nullptr, "Node n2 missing");
}

TEST(diagram_add_edges) {
    Diagram diagram;

    diagram.add_node(Node("n1", "Atom"));
    diagram.add_node(Node("n2", "Add"));

    Edge edge("e1", "n1", "n2", "arg");
    edge.port = 0;
    diagram.add_edge(edge);

    REQUIRE(diagram.edges().size() == 1, "Edge count mismatch");
}

TEST(diagram_get_inputs) {
    Diagram diagram;

    diagram.add_node(Node("n1", "Atom"));
    diagram.add_node(Node("n2", "Add"));
    diagram.add_node(Node("n3", "Mul"));

    Edge edge1("e1", "n1", "n2", "arg");
    edge1.port = 0;
    Edge edge2("e2", "n3", "n2", "arg");
    edge2.port = 1;
    diagram.add_edge(edge1);
    diagram.add_edge(edge2);

    auto inputs = diagram.get_inputs("n2");
    REQUIRE(inputs.size() == 2, "Input count mismatch");
    REQUIRE(inputs[0] == "n1", "Input order mismatch");
    REQUIRE(inputs[1] == "n3", "Input order mismatch");
}

TEST(diagram_cycle_detection_no_cycle) {
    Diagram diagram;

    diagram.add_node(Node("n1", "A"));
    diagram.add_node(Node("n2", "B"));
    diagram.add_node(Node("n3", "C"));

    diagram.add_edge(Edge("e1", "n1", "n2", "arg"));
    diagram.add_edge(Edge("e2", "n2", "n3", "arg"));

    REQUIRE(!diagram.has_cycle(), "Unexpected cycle detected");
}

TEST(diagram_cycle_detection_with_cycle) {
    Diagram diagram;

    diagram.add_node(Node("n1", "A"));
    diagram.add_node(Node("n2", "B"));
    diagram.add_node(Node("n3", "C"));

    diagram.add_edge(Edge("e1", "n1", "n2", "arg"));
    diagram.add_edge(Edge("e2", "n2", "n3", "arg"));
    diagram.add_edge(Edge("e3", "n3", "n1", "arg"));

    REQUIRE(diagram.has_cycle(), "Expected cycle not detected");
}

TEST(diagram_remove_node_cleans_edges) {
    Diagram diagram;

    diagram.add_node(Node("n1", "A"));
    diagram.add_node(Node("n2", "B"));
    diagram.add_node(Node("n3", "C"));

    diagram.add_edge(Edge("e1", "n1", "n2", "arg"));
    diagram.add_edge(Edge("e2", "n2", "n3", "arg"));
    diagram.add_edge(Edge("e3", "n1", "n3", "arg"));

    REQUIRE(remove_node(diagram, "n2"), "Remove node failed");
    REQUIRE(diagram.nodes().size() == 2, "Node count mismatch after removal");
    REQUIRE(diagram.edges().size() == 1, "Edge count mismatch after removal");
    REQUIRE(diagram.find_edge("e1") == nullptr, "Edge e1 should be removed");
    REQUIRE(diagram.find_edge("e2") == nullptr, "Edge e2 should be removed");
    REQUIRE(diagram.find_edge("e3") != nullptr, "Edge e3 should remain");
}

// ============================================================================
// Validator Tests
// ============================================================================

TEST(validator_valid_diagram) {
    Diagram diagram;

    diagram.add_node(Node("n1", "Atom"));
    diagram.add_node(Node("n2", "Add"));
    diagram.add_edge(Edge("e1", "n1", "n2", "arg"));

    DiagramValidator validator;
    auto errors = validator.validate(diagram);
    REQUIRE(errors.empty(), "Expected no validation errors");
}

TEST(validator_missing_node_reference) {
    Diagram diagram;

    diagram.add_node(Node("n1", "Atom"));
    diagram.add_edge(Edge("e1", "n1", "n2", "arg"));

    bool threw = false;
    try {
        validateDiagramStructure(diagram);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    REQUIRE(threw, "Expected missing node validation failure");
}

TEST(validator_get_source_nodes) {
    Diagram diagram;

    diagram.add_node(Node("n1", "Source"));
    diagram.add_node(Node("n2", "Process"));
    diagram.add_node(Node("n3", "Sink"));

    diagram.add_edge(Edge("e1", "n1", "n2", "arg"));
    diagram.add_edge(Edge("e2", "n2", "n3", "arg"));

    auto sources = get_source_nodes(diagram);
    REQUIRE(sources.size() == 1, "Source count mismatch");
    REQUIRE(sources[0] == "n1", "Source node mismatch");

    auto sinks = get_sink_nodes(diagram);
    REQUIRE(sinks.size() == 1, "Sink count mismatch");
    REQUIRE(sinks[0] == "n3", "Sink node mismatch");
}

// ============================================================================
// Parser -> Diagram Build Tests
// ============================================================================

TEST(parser_to_diagram_build) {
    ASTNode ast = parseExpression("C(P(Freedom), P(Justice))");
    Diagram diagram = exprToDiagram(ast, "d_build");

    REQUIRE(!diagram.nodes().empty(), "Diagram nodes missing");
    REQUIRE(!diagram.edges().empty(), "Diagram edges missing");
    REQUIRE(!diagram.has_cycle(), "Unexpected cycle in built diagram");
}

// ============================================================================
// Rewrite Tests
// ============================================================================

TEST(rewrite_apply_expr) {
    ASTNode ast = parseExpression("P(Freedom)");
    Diagram diagram = exprToDiagram(ast, "d_rewrite");

    auto result = applyExprRewrite(diagram, "P($x)", "O($x)", "rw1");
    REQUIRE(result.applied, "Rewrite should apply");

    bool found_o = false;
    for (const auto& node : result.diagram.nodes()) {
        if (node.op == "O") {
            found_o = true;
            break;
        }
    }
    REQUIRE(found_o, "Expected O node after rewrite");
}

TEST(rewrite_rejects_cycle) {
    Diagram diagram("d_cycle");
    Node n1("n1", "P");
    n1.dof_refs.push_back("A");
    Node n2("n2", "P");
    n2.dof_refs.push_back("B");
    diagram.add_node(n1);
    diagram.add_node(n2);
    diagram.add_edge(Edge("e1", "n1", "n2", "arg"));
    diagram.add_edge(Edge("e2", "n2", "n1", "arg"));

    auto result = applyExprRewrite(diagram, "P($x)", "O($x)", "rw_cycle");
    REQUIRE(!result.applied, "Rewrite should reject cycle");

    bool cycle_error = false;
    for (const auto& msg : result.messages) {
        if (msg.find("cycle") != std::string::npos) {
            cycle_error = true;
            break;
        }
    }
    REQUIRE(cycle_error, "Expected cycle rejection message");
}

// ============================================================================
// SSP Tests
// ============================================================================

TEST(ssp_semantic_processor_creation) {
    SemanticProcessor ssp(Role::I, 100, 0.0);
    REQUIRE(ssp.field_len() == 100, "Field length mismatch");
    REQUIRE(ssp.total_mass() == 0.0, "Expected zero mass");
}

TEST(ssp_add_uniform) {
    SemanticProcessor ssp(Role::U, 10, 100.0);
    ssp.add_uniform(5.0);

    REQUIRE(ssp.total_mass() == 50.0, "Uniform add mismatch");
}

TEST(ssp_scale_all) {
    SemanticProcessor ssp(Role::U, 10, 100.0);
    ssp.add_uniform(10.0);
    ssp.scale_all(0.5);

    REQUIRE(ssp.total_mass() == 50.0, "Scale mismatch");
}

TEST(ssp_mixer_conservation) {
    const uint64_t len = 100;
    const double total_mass = 100.0;

    SemanticProcessor ssp_I(Role::I, len, total_mass);
    SemanticProcessor ssp_N(Role::N, len, total_mass);
    SemanticProcessor ssp_U(Role::U, len, total_mass);

    for (size_t i = 0; i < len; ++i) {
        ssp_U.field()[i] = total_mass / static_cast<double>(len);
    }

    Mixer mixer(total_mass);
    mixer.step(ssp_I, ssp_N, ssp_U);

    REQUIRE(mixer.metrics().conservation_error <= mixer.config().eps_conservation,
            "Conservation error exceeded tolerance");
}

TEST(ssp_mixer_collapse) {
    const uint64_t len = 100;
    const double total_mass = 100.0;

    SemanticProcessor ssp_I(Role::I, len, total_mass);
    SemanticProcessor ssp_N(Role::N, len, total_mass);
    SemanticProcessor ssp_U(Role::U, len, total_mass);

    for (size_t i = 0; i < len; ++i) {
        ssp_U.field()[i] = total_mass / static_cast<double>(len);
    }

    Mixer mixer(total_mass);
    mixer.request_collapse(ssp_I, ssp_N, ssp_U);
    mixer.step(ssp_I, ssp_N, ssp_U);

    REQUIRE(mixer.metrics().conservation_error <= mixer.config().eps_conservation,
            "Conservation error exceeded tolerance after collapse");
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::cout << "SID Core Tests - Phase 1-2 Validation\n";
    std::cout << "=======================================\n\n";

    // AST tests
    run_test_ast_atom_creation();
    run_test_ast_op_creation();
    run_test_ast_expr_to_string();
    run_test_ast_expr_clone();

    // Parser tests
    run_test_parser_simple_atom();
    run_test_parser_simple_operator();
    run_test_parser_nested_operators();
    run_test_parser_multi_arg_operator();
    run_test_parser_invalid_expression_throws();

    // Diagram tests
    run_test_diagram_add_nodes();
    run_test_diagram_add_edges();
    run_test_diagram_get_inputs();
    run_test_diagram_cycle_detection_no_cycle();
    run_test_diagram_cycle_detection_with_cycle();
    run_test_diagram_remove_node_cleans_edges();

    // Validator tests
    run_test_validator_valid_diagram();
    run_test_validator_missing_node_reference();
    run_test_validator_get_source_nodes();

    // Parser -> diagram build
    run_test_parser_to_diagram_build();

    // Rewrite tests
    run_test_rewrite_apply_expr();
    run_test_rewrite_rejects_cycle();

    // SSP tests
    run_test_ssp_semantic_processor_creation();
    run_test_ssp_add_uniform();
    run_test_ssp_scale_all();
    run_test_ssp_mixer_conservation();
    run_test_ssp_mixer_collapse();

    std::cout << "\n=======================================\n";
    std::cout << "Results: " << tests_passed << " passed, " << tests_failed << " failed\n";

    return (tests_failed > 0) ? 1 : 0;
}
