/**
 * SID Parser - Simplified C++ port
 *
 * NOTE: This file is deprecated. Use sid_parser_impl.hpp instead.
 */

#pragma once

#include "sid_ast.hpp"
#include <string>
#include <stdexcept>

namespace sid {

// ParseError is now defined in sid_ast.hpp

// Simplified parser stub - full implementation in Python original
// NOTE: This function is deprecated. Use parseExpression() from sid_parser_impl.hpp instead.
inline ExprPtr parse_expression(const std::string& text) {
    // Simplified: just create a basic atom for now
    // Full implementation would replicate Python tokenizer/parser logic
    if (text.empty()) throw ParseError("Empty expression");
    return make_atom(text);
}

} // namespace sid
