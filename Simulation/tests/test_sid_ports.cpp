/**
 * SID Port Tests - AST utils, AST->diagram, CRF, Stability
 */

#include "../src/cpp/sid_ssp/sid_ast_utils.hpp"
#include "../src/cpp/sid_ssp/sid_ast_to_diagram.hpp"
#include "../src/cpp/sid_ssp/sid_crf.hpp"
#include "../src/cpp/sid_ssp/sid_stability.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace sid;
using json = nlohmann::json;

static int tests_passed = 0;
static int tests_failed = 0;

#define EXPECT_NEAR(a, b, eps) \
    do { \
        if (std::fabs((a) - (b)) > (eps)) { \
            throw std::runtime_error("EXPECT_NEAR failed"); \
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

TEST(ast_utils_roundtrip) {
    auto expr = make_op("C", {make_op("P", {make_atom("x")}), make_atom("y")});
    json encoded = ast_utils::expr_to_json(*expr);
    auto decoded = ast_utils::json_to_expr(encoded);
    assert(expr_to_string(*expr) == expr_to_string(*decoded));
}

TEST(ast_to_diagram_atom) {
    auto atom = make_atom("x");
    json diagram = ast_to_diagram::expr_to_diagram(*atom, "d1");
    assert(diagram["nodes"].size() == 1);
    const auto& node = diagram["nodes"][0];
    assert(node.value("op", "") == "P");
    assert(node.contains("dof_refs"));
    assert(node["dof_refs"].size() == 1);
    assert(node["dof_refs"][0] == "x");
}

TEST(ast_to_diagram_edges) {
    auto expr = make_op("C", {make_op("P", {make_atom("x")}), make_op("P", {make_atom("y")})});
    json diagram = ast_to_diagram::expr_to_diagram(*expr, "d2");
    assert(diagram["nodes"].size() == 3);
    assert(diagram["edges"].size() == 2);

    bool found_c = false;
    for (const auto& node : diagram["nodes"]) {
        if (node.value("op", "") == "C") {
            found_c = true;
            assert(node.contains("inputs"));
            assert(node["inputs"].size() == 2);
        }
    }
    assert(found_c);

    for (const auto& edge : diagram["edges"]) {
        assert(edge.value("label", "") == "arg");
    }
}

TEST(crf_conflict_resolution) {
    json state = json::object();
    json conflict = json{{"constraint_id", "c1"}, {"type", "soft_violation"}};
    auto result = crf::attenuate(conflict, state, json::object());
    assert(result.action == "attenuate");
    assert(result.success);
    assert(result.new_state.has_value());
    assert(result.new_state->contains("attenuated_constraints"));

    auto unknown = crf::resolve_conflict("mystery", json::object(), state, json::object());
    assert(unknown.action == "halt");
    assert(!unknown.success);
}

TEST(crf_authorize_rewrite) {
    json diagram = json{
        {"nodes", json::array({json{{"id", "n1"}, {"op", "O"}}})},
        {"edges", json::array()}
    };
    json csi = json::object();
    json state = json::object();
    json rule = json{{"id", "r1"}};
    std::vector<json> constraints = {json{{"id", "c1"}, {"predicate", "collapse_irreversible"}, {"type", "hard"}}};

    auto auth = crf::authorize_rewrite(constraints, state, diagram, csi, rule);
    assert(!std::get<0>(auth));
    assert(!std::get<1>(auth).empty());
}

TEST(crf_assign_inu_labels_no_constraints) {
    json diagram = json{
        {"nodes", json::array({
            json{{"id", "n1"}, {"op", "P"}, {"dof_refs", json::array({"Freedom"})}},
            json{{"id", "n2"}, {"op", "O"}, {"irreversible", true}}
        })},
        {"edges", json::array({
            json{{"id", "e1"}, {"from", "n1"}, {"to", "n2"}, {"label", "arg"}}
        })}
    };
    json state = json::object();
    json csi = json{{"allowed_dofs", json::array({"Freedom"})}, {"allowed_pairs", json::array()}};
    std::vector<json> constraints;

    json labels = crf::assign_inu_labels(diagram, constraints, state, csi);
    assert(labels.value("n1", "") == "I");
    assert(labels.value("n2", "") == "I");
    assert(labels.value("e1", "") == "I");
}

TEST(crf_check_admissible_cases) {
    json state1 = json{{"inu_labels", json{{"n1", "I"}, {"n2", "I"}, {"e1", "I"}}}};
    auto ok1 = crf::check_admissible(state1);
    assert(ok1.first);

    json state2 = json{{"inu_labels", json{{"n1", "I"}, {"n2", "N"}, {"e1", "I"}}}};
    auto ok2 = crf::check_admissible(state2);
    assert(!ok2.first);

    json state3 = json{{"inu_labels", json{{"n1", "I"}, {"n2", "U"}, {"e1", "I"}}}};
    auto ok3 = crf::check_admissible(state3);
    assert(ok3.first);
}

TEST(crf_predicates_registered) {
    crf::ensure_default_predicates();
    auto& registry = crf::predicate_registry();
    assert(registry.find("no_cross_csi_interaction") != registry.end());
    assert(registry.find("collapse_irreversible") != registry.end());
    assert(registry.find("no_cycles") != registry.end());
    assert(registry.find("valid_compartment_transitions") != registry.end());
}

TEST(crf_no_cycles_predicate) {
    crf::ensure_default_predicates();
    auto& registry = crf::predicate_registry();
    auto it = registry.find("no_cycles");
    assert(it != registry.end());

    json diagram1 = json{
        {"nodes", json::array({
            json{{"id", "n1"}, {"op", "P"}},
            json{{"id", "n2"}, {"op", "O"}}
        })},
        {"edges", json::array({
            json{{"id", "e1"}, {"from", "n1"}, {"to", "n2"}}
        })}
    };
    auto ok1 = it->second(json::object(), diagram1, json::object());
    assert(ok1.first);

    json diagram2 = json{
        {"nodes", json::array({
            json{{"id", "n1"}, {"op", "P"}},
            json{{"id", "n2"}, {"op", "O"}}
        })},
        {"edges", json::array({
            json{{"id", "e1"}, {"from", "n1"}, {"to", "n2"}},
            json{{"id", "e2"}, {"from", "n2"}, {"to", "n1"}}
        })}
    };
    auto ok2 = it->second(json::object(), diagram2, json::object());
    assert(!ok2.first);
}

TEST(stability_metrics_and_loop) {
    json diagram = json{
        {"id", "d1"},
        {"nodes", json::array({
            json{{"id", "n1"}, {"op", "O"}},
            json{{"id", "n2"}, {"op", "C"}},
            json{{"id", "n3"}, {"op", "T"}, {"meta", json{{"target_compartment", "c1"}}}}
        })},
        {"edges", json::array()}
    };
    json state = json{
        {"id", "s1"},
        {"csi_id", "csi1"},
        {"inu_labels", json{{"n1", "I"}, {"n2", "I"}}},
        {"loop_history", json::array({
            json{{"inu_labels", json{{"n1", "I"}, {"n2", "U"}}}},
            json{{"inu_labels", json{{"n1", "I"}, {"n2", "I"}}}}
        })}
    };
    json pkg = json{
        {"states", json::array({state})},
        {"diagrams", json::array({diagram})}
    };

    json metrics = stability::compute_stability_metrics(pkg, "s1", "d1");
    assert(metrics.value("collapse_count", 0) == 1);
    assert(metrics.value("coupling_count", 0) == 1);
    assert(metrics.contains("transport_fidelity"));

    auto loop = stability::check_loop_convergence(state, 0.6);
    assert(loop.first);
}

TEST(stability_only_identity_rewrites) {
    std::vector<json> rules1 = {
        json{{"id", "r1"}, {"pattern", "P(x)"}, {"replacement", "P(x)"}},
        json{{"id", "r2"}, {"pattern_expr", "O(y)"}, {"replacement_expr", "O(y)"}}
    };
    auto ok1 = stability::check_only_identity_rewrites(rules1);
    assert(ok1.first);

    std::vector<json> rules2 = {
        json{{"id", "r1"}, {"pattern", "P(x)"}, {"replacement", "P(x)"}},
        json{{"id", "r2"}, {"pattern", "P(x)"}, {"replacement", "O(x)"}}
    };
    auto ok2 = stability::check_only_identity_rewrites(rules2);
    assert(!ok2.first);
}

TEST(stability_loop_convergence_cases) {
    json state1 = json{{"loop_history", json::array()}};
    auto ok1 = stability::check_loop_convergence(state1);
    assert(!ok1.first);

    json state2 = json{{"loop_history", json::array({
        json{{"inu_labels", json{{"n1", "I"}, {"n2", "I"}}}},
        json{{"inu_labels", json{{"n1", "I"}, {"n2", "I"}}}}
    })}};
    auto ok2 = stability::check_loop_convergence(state2);
    assert(ok2.first);

    json state3 = json{{"loop_history", json::array({
        json{{"inu_labels", json{{"n1", "I"}, {"n2", "I"}, {"n3", "I"}}}},
        json{{"inu_labels", json{{"n1", "I"}, {"n2", "N"}, {"n3", "U"}}}}
    })}};
    auto ok3 = stability::check_loop_convergence(state3, 0.1);
    assert(!ok3.first);

    json state4 = json{{"loop_history", json::array({
        json{{"inu_labels", json{{"n1", "I"}, {"n2", "I"}, {"n3", "I"}, {"n4", "I"}, {"n5", "I"}}}},
        json{{"inu_labels", json{{"n1", "I"}, {"n2", "I"}, {"n3", "I"}, {"n4", "I"}, {"n5", "U"}}}}
    })}};
    auto ok4 = stability::check_loop_convergence(state4, 0.3);
    assert(ok4.first);
}

TEST(stability_compute_metrics) {
    json pkg = json{
        {"states", json::array({
            json{
                {"id", "s1"},
                {"inu_labels", json{{"n1", "I"}, {"n2", "I"}, {"n3", "U"}, {"e1", "I"}}},
                {"loop_history", json::array({
                    json{{"inu_labels", json{{"n1", "I"}, {"n2", "I"}, {"n3", "U"}, {"e1", "I"}}}},
                    json{{"inu_labels", json{{"n1", "I"}, {"n2", "I"}, {"n3", "I"}, {"e1", "I"}}}}
                })}
            }
        })},
        {"diagrams", json::array({
            json{
                {"id", "d1"},
                {"nodes", json::array({
                    json{{"id", "n1"}, {"op", "P"}},
                    json{{"id", "n2"}, {"op", "O"}},
                    json{{"id", "n3"}, {"op", "C"}}
                })},
                {"edges", json::array({json{{"id", "e1"}, {"from", "n1"}, {"to", "n2"}}})}
            }
        })}
    };

    json metrics = stability::compute_stability_metrics(pkg, "s1", "d1");
    assert(metrics.contains("admissible_volume"));
    assert(metrics.contains("collapse_ratio"));
    assert(metrics.contains("gradient_coherence"));
    assert(metrics.contains("transport_fidelity"));
    assert(metrics.contains("loop_gain"));
    assert(metrics.value("collapse_count", 0) == 1);
    assert(metrics.value("coupling_count", 0) == 1);
    EXPECT_NEAR(metrics.value("collapse_ratio", 0.0), 1.0 / 3.0, 1e-6);
    EXPECT_NEAR(metrics.value("gradient_coherence", 0.0), 1.0 / 3.0, 1e-6);
}

TEST(stability_is_structurally_stable) {
    json pkg = json{
        {"states", json::array({
            json{
                {"id", "s1"},
                {"diagram_id", "d1"},
                {"csi_id", "csi1"},
                {"inu_labels", json{{"n1", "I"}}},
                {"loop_history", json::array({
                    json{{"inu_labels", json{{"n1", "I"}}}},
                    json{{"inu_labels", json{{"n1", "I"}}}}
                })}
            }
        })},
        {"diagrams", json::array({
            json{{"id", "d1"}, {"nodes", json::array({json{{"id", "n1"}, {"op", "P"}}})}, {"edges", json::array()}}
        })},
        {"csis", json::array({
            json{{"id", "csi1"}, {"allowed_dofs", json::array({"Freedom"})}, {"allowed_pairs", json::array()}}
        })},
        {"constraints", json::array()},
        {"rewrite_rules", json::array()}
    };

    auto stable = stability::is_structurally_stable(pkg, "s1", "d1");
    assert(std::get<0>(stable));
    assert(!std::get<1>(stable).empty());
}

TEST(stability_metrics_collapse_ratio) {
    json pkg = json{
        {"states", json::array({json{{"id", "s1"}, {"inu_labels", json::object()}}})},
        {"diagrams", json::array({
            json{
                {"id", "d1"},
                {"nodes", json::array({
                    json{{"id", "n1"}, {"op", "P"}},
                    json{{"id", "n2"}, {"op", "O"}},
                    json{{"id", "n3"}, {"op", "O"}},
                    json{{"id", "n4"}, {"op", "C"}}
                })},
                {"edges", json::array()}
            }
        })}
    };

    json metrics = stability::compute_stability_metrics(pkg, "s1", "d1");
    assert(metrics.value("collapse_count", 0) == 2);
    EXPECT_NEAR(metrics.value("collapse_ratio", 0.0), 0.5, 1e-6);
}

TEST(stability_metrics_transport_fidelity) {
    json pkg = json{
        {"states", json::array({json{{"id", "s1"}, {"inu_labels", json::object()}}})},
        {"diagrams", json::array({
            json{
                {"id", "d1"},
                {"nodes", json::array({
                    json{{"id", "n1"}, {"op", "T"}, {"meta", json{{"target_compartment", "c2"}}}},
                    json{{"id", "n2"}, {"op", "T"}, {"meta", json{{"target_compartment", "c3"}}}},
                    json{{"id", "n3"}, {"op", "T"}, {"meta", json::object()}},
                    json{{"id", "n4"}, {"op", "P"}}
                })},
                {"edges", json::array()}
            }
        })}
    };

    json metrics = stability::compute_stability_metrics(pkg, "s1", "d1");
    assert(metrics.value("transport_count", 0) == 3);
    EXPECT_NEAR(metrics.value("transport_fidelity", 0.0), 2.0 / 3.0, 1e-6);
}

int main() {
    std::cout << "SID Port Tests - Additional Modules\n";
    std::cout << "====================================\n\n";

    run_test_ast_utils_roundtrip();
    run_test_ast_to_diagram_atom();
    run_test_ast_to_diagram_edges();
    run_test_crf_conflict_resolution();
    run_test_crf_authorize_rewrite();
    run_test_crf_assign_inu_labels_no_constraints();
    run_test_crf_check_admissible_cases();
    run_test_crf_predicates_registered();
    run_test_crf_no_cycles_predicate();
    run_test_stability_metrics_and_loop();
    run_test_stability_only_identity_rewrites();
    run_test_stability_loop_convergence_cases();
    run_test_stability_compute_metrics();
    run_test_stability_is_structurally_stable();
    run_test_stability_metrics_collapse_ratio();
    run_test_stability_metrics_transport_fidelity();

    std::cout << "\n====================================\n";
    std::cout << "Results: " << tests_passed << " passed, " << tests_failed << " failed\n";

    return (tests_failed > 0) ? 1 : 0;
}
