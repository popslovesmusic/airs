/**
 * SID Diagram Dump Tool - emits diagram JSON for an expression.
 */

#include "../src/cpp/sid_ssp/sid_capi.hpp"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: sid_dump_expr <expr>\n";
        return 2;
    }

    const char* expr = argv[1];
    SidEngineHandle engine = sid_create_engine(64, 1.0);
    if (!engine) {
        std::cerr << "ERROR: unable to create engine\n";
        return 1;
    }

    if (!sid_set_diagram_expr(engine, expr, "dump")) {
        std::cerr << "ERROR: sid_set_diagram_expr failed\n";
        sid_destroy_engine(engine);
        return 1;
    }

    const char* json = sid_get_diagram_json(engine);
    if (!json) {
        std::cerr << "ERROR: sid_get_diagram_json failed\n";
        sid_destroy_engine(engine);
        return 1;
    }

    std::cout << json;
    sid_destroy_engine(engine);
    return 0;
}
