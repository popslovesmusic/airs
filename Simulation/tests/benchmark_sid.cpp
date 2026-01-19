/**
 * SID Benchmarks - Minimal performance harness
 */

#include "../src/cpp/sid_ssp/sid_parser.hpp"
#include "../src/cpp/sid_ssp/sid_rewrite.hpp"
#include "../src/cpp/sid_ssp/sid_diagram.hpp"

#include <chrono>
#include <iostream>

using namespace sid;

static Diagram build_diagram_from_expr(const std::string& expr_text, const std::string& rule_id) {
    auto expr = parse_expression(expr_text);
    Diagram diagram;
    IDGenerator node_gen(rule_id + "_n");
    IDGenerator edge_gen(rule_id + "_e");
    Bindings bindings;
    build_expr(*expr, diagram, bindings, rule_id, node_gen, edge_gen);
    return diagram;
}

int main() {
    try {
        using clock = std::chrono::high_resolution_clock;

    const int parse_iters = 10000;
    auto parse_start = clock::now();
    for (int i = 0; i < parse_iters; ++i) {
        auto expr = parse_expression("C(P(Freedom), O(P(Choice)))");
        (void)expr;
    }
    auto parse_end = clock::now();
    auto parse_ms = std::chrono::duration_cast<std::chrono::milliseconds>(parse_end - parse_start).count();

    Diagram diagram = build_diagram_from_expr("S+(P(A), P(B), P(C))", "rw");
    auto cycle_start = clock::now();
    bool has_cycle = diagram.has_cycle();
    auto cycle_end = clock::now();
    auto cycle_us = std::chrono::duration_cast<std::chrono::microseconds>(cycle_end - cycle_start).count();

    Diagram large_diagram;
    const int large_nodes = 10000;
    for (int i = 0; i < large_nodes; ++i) {
        large_diagram.add_node(Node("n" + std::to_string(i), "N"));
    }
    for (int i = 0; i < large_nodes - 1; ++i) {
        large_diagram.add_edge(Edge("e" + std::to_string(i),
                                    "n" + std::to_string(i),
                                    "n" + std::to_string(i + 1),
                                    0, 0));
    }

    auto large_cycle_start = clock::now();
    bool large_has_cycle = large_diagram.has_cycle();
    auto large_cycle_end = clock::now();
    auto large_cycle_ms = std::chrono::duration_cast<std::chrono::milliseconds>(large_cycle_end - large_cycle_start).count();

    auto pattern = parse_expression("S+(P($x), P($y), P($z))");
    auto replacement = parse_expression("S+(P($x), O(P($y)), P($z))");

    auto rewrite_start = clock::now();
    auto rewrite_result = apply_rewrite(diagram, *pattern, *replacement, "rw");
    auto rewrite_end = clock::now();
    auto rewrite_us = std::chrono::duration_cast<std::chrono::microseconds>(rewrite_end - rewrite_start).count();

    std::cout << "SID Benchmarks\n";
    std::cout << "==============\n";
    std::cout << "Parse iterations: " << parse_iters << " in " << parse_ms << " ms\n";
    std::cout << "Cycle detection: " << (has_cycle ? "true" : "false") << " in " << cycle_us << " us\n";
    std::cout << "Cycle detection (10k): " << (large_has_cycle ? "true" : "false") << " in " << large_cycle_ms << " ms\n";
    std::cout << "Rewrite applied: " << (rewrite_result.applied ? "true" : "false") << " in " << rewrite_us << " us\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Benchmark failed: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Benchmark failed: unknown error\n";
        return 1;
    }
}
