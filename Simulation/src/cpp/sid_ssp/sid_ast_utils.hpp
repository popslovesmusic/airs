/**
 * SID AST Utils - C++ port from sids/sid_ast_utils.py
 *
 * Utility functions for AST manipulation
 */

#pragma once

#include "sid_ast.hpp"
#include <map>
#include <string>

namespace sid {

/**
 * Convert expression to dictionary-like representation (for serialization)
 */
inline std::map<std::string, std::string> expr_to_dict(const Expr& expr) {
    std::map<std::string, std::string> result;

    if (is_atom(expr)) {
        result["type"] = "atom";
        result["name"] = std::get<Atom>(expr).name;
    } else {
        const auto& op_expr = std::get<OpExpr>(expr);
        result["type"] = "op";
        result["op"] = op_expr.op;
        // Note: args would need recursive conversion for full serialization
    }

    return result;
}

} // namespace sid
