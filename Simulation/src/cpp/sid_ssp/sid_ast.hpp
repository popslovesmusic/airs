/**
 * SID AST - Abstract Syntax Tree for Semantic Interaction Diagrams
 *
 * C++ port of sids/sid_ast.py
 * Defines expression types for SID operator compositions
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <sstream>
#include <stdexcept>

namespace sid {

/**
 * Parse error exception
 */
class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& msg) : std::runtime_error(msg) {}
};

/**
 * AST Node kind
 */
enum class ASTKind {
    Atom,   // Atomic identifier
    Op      // Operator expression
};

/**
 * Unified AST Node structure
 *
 * Simpler alternative to variant-based Expr for easier pattern matching.
 */
struct ASTNode {
    ASTKind kind;

    // For Atom
    std::string atom_name;

    // For Op
    std::string op_name;
    std::vector<ASTNode> args;

    // Constructors
    ASTNode() : kind(ASTKind::Atom) {}

    // Create atom
    static ASTNode makeAtom(const std::string& name) {
        ASTNode node;
        node.kind = ASTKind::Atom;
        node.atom_name = name;
        return node;
    }

    // Create operator
    static ASTNode makeOp(const std::string& op, const std::vector<ASTNode>& arguments) {
        ASTNode node;
        node.kind = ASTKind::Op;
        node.op_name = op;
        node.args = arguments;
        return node;
    }
};

// Forward declarations
struct Atom;
struct OpExpr;

// Expression variant type
using Expr = std::variant<Atom, OpExpr>;
using ExprPtr = std::shared_ptr<Expr>;

/**
 * Atom - Atomic identifier (variable/constant)
 */
struct Atom {
    std::string name;

    Atom() = default;
    explicit Atom(const std::string& n) : name(n) {}
    explicit Atom(std::string&& n) : name(std::move(n)) {}

    bool operator==(const Atom& other) const {
        return name == other.name;
    }

    bool operator!=(const Atom& other) const {
        return !(*this == other);
    }
};

/**
 * OpExpr - Operation expression with operator and arguments
 */
struct OpExpr {
    std::string op;
    std::vector<ExprPtr> args;

    OpExpr() = default;
    OpExpr(const std::string& operation, std::vector<ExprPtr> arguments)
        : op(operation), args(std::move(arguments)) {}

    bool operator==(const OpExpr& other) const {
        if (op != other.op || args.size() != other.args.size()) {
            return false;
        }
        for (size_t i = 0; i < args.size(); ++i) {
            if (*args[i] != *other.args[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const OpExpr& other) const {
        return !(*this == other);
    }
};

// Comparison operators for Expr variant
inline bool operator==(const Expr& lhs, const Expr& rhs) {
    if (lhs.index() != rhs.index()) {
        return false;
    }
    if (std::holds_alternative<Atom>(lhs)) {
        return std::get<Atom>(lhs) == std::get<Atom>(rhs);
    } else {
        return std::get<OpExpr>(lhs) == std::get<OpExpr>(rhs);
    }
}

inline bool operator!=(const Expr& lhs, const Expr& rhs) {
    return !(lhs == rhs);
}

/**
 * Deep clone an expression
 */
inline ExprPtr expr_clone(const Expr& expr) {
    if (std::holds_alternative<Atom>(expr)) {
        return std::make_shared<Expr>(std::get<Atom>(expr));
    } else {
        const OpExpr& op_expr = std::get<OpExpr>(expr);
        std::vector<ExprPtr> cloned_args;
        cloned_args.reserve(op_expr.args.size());
        for (const auto& arg : op_expr.args) {
            cloned_args.push_back(expr_clone(*arg));
        }
        return std::make_shared<Expr>(OpExpr{op_expr.op, std::move(cloned_args)});
    }
}

/**
 * Convert expression to string representation
 */
inline std::string expr_to_string(const Expr& expr) {
    if (std::holds_alternative<Atom>(expr)) {
        return std::get<Atom>(expr).name;
    } else {
        const OpExpr& op_expr = std::get<OpExpr>(expr);
        std::ostringstream oss;
        oss << op_expr.op << "(";
        for (size_t i = 0; i < op_expr.args.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << expr_to_string(*op_expr.args[i]);
        }
        oss << ")";
        return oss.str();
    }
}

/**
 * Helper to create an Atom expression
 */
inline ExprPtr make_atom(const std::string& name) {
    return std::make_shared<Expr>(Atom{name});
}

/**
 * Helper to create an OpExpr expression
 */
inline ExprPtr make_op(const std::string& op, std::vector<ExprPtr> args) {
    return std::make_shared<Expr>(OpExpr{op, std::move(args)});
}

/**
 * Check if expression is an Atom
 */
inline bool is_atom(const Expr& expr) {
    return std::holds_alternative<Atom>(expr);
}

/**
 * Check if expression is an OpExpr
 */
inline bool is_op_expr(const Expr& expr) {
    return std::holds_alternative<OpExpr>(expr);
}

} // namespace sid
