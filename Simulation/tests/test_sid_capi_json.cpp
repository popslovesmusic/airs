/**
 * SID C API JSON Tests - Round-trip coverage
 */

#include "../src/cpp/sid_ssp/sid_capi.hpp"
#include "../dase_cli/src/json.hpp"

#include <iostream>
#include <cassert>

using json = nlohmann::json;

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

TEST(capi_json_round_trip_labels_and_dofs) {
    SidEngineHandle engine = sid_create_engine(8, 1.0);
    assert(engine != nullptr);

    json diagram = {
        {"id", "d1"},
        {"nodes", json::array({
            {{"id", "n1"}, {"op", "P"}, {"dof_refs", json::array({"A", "B"})}},
            {{"id", "n2"}, {"op", "C"}, {"inputs", json::array({"n1"})}}
        })},
        {"edges", json::array({
            {{"id", "e1"}, {"from", "n1"}, {"to", "n2"}, {"label", "custom"}, {"to_port", 0}}
        })}
    };

    std::string input_json = diagram.dump();
    assert(sid_set_diagram_json(engine, input_json.c_str()));

    std::string output_json = sid_get_diagram_json(engine);
    auto parsed = json::parse(output_json);

    bool found = false;
    for (const auto& node : parsed["nodes"]) {
        if (node.contains("id") && node["id"] == "n1") {
            assert(node.contains("dof_refs"));
            assert(node["dof_refs"].is_array());
            assert(node["dof_refs"].size() == 2);
            found = true;
        }
    }
    assert(found);

    bool label_ok = false;
    for (const auto& edge : parsed["edges"]) {
        if (edge.contains("id") && edge["id"] == "e1") {
            assert(edge.contains("label"));
            assert(edge["label"] == "custom");
            label_ok = true;
        }
    }
    assert(label_ok);

    sid_destroy_engine(engine);
}

int main() {
    std::cout << "SID C API JSON Tests\n";
    std::cout << "====================\n\n";

    run_test_capi_json_round_trip_labels_and_dofs();

    std::cout << "\n====================\n";
    std::cout << "Results: " << tests_passed << " passed, " << tests_failed << " failed\n";

    return (tests_failed > 0) ? 1 : 0;
}
