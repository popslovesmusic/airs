/**
 * SID AST to Diagram - C++ port from sids/sid_ast_to_diagram.py
 *
 * Converts AST expressions to diagram representations
 */

#pragma once

#include "sid_ast.hpp"
#include "sid_diagram.hpp"
#include <optional>
#include <sstream>

namespace sid {

/**
 * Diagram builder with ID generation
 */
class DiagramBuilder {
private:
    Diagram diagram_;
    int counter_ = 0;

public:
    explicit DiagramBuilder(const std::string& diagram_id) {
        diagram_.set_id(diagram_id);
    }

    std::string next_id(const std::string& prefix) {
        ++counter_;
        std::ostringstream oss;
        oss << prefix << counter_;
        return oss.str();
    }

    Diagram& diagram() { return diagram_; }
    const Diagram& diagram() const { return diagram_; }
};

/**
 * Convert AST expression to diagram
 *
 * BUG FIX (HIGH): Fixed IndexError on empty atoms (line 94 in Python)
 */
inline Diagram expr_to_diagram(const Expr& expr, 
                                const std::string& diagram_id = "d_expr",
                                const std::string& compartment_id = "") {
    DiagramBuilder builder(diagram_id);

    std::function<std::optional<std::string>(const Expr&)> build_expr;
    
    build_expr = [&](const Expr& node_expr) -> std::optional<std::string> {
        if (is_atom(node_expr)) {
            // Atoms don't create nodes directly, they're handled by parent operators
            return std::nullopt;
        }

        const auto& op_expr = std::get<OpExpr>(node_expr);
        std::vector<std::string> atom_args;
        std::vector<std::string> input_ids;

        for (const auto& arg : op_expr.args) {
            if (is_atom(*arg)) {
                atom_args.push_back(std::get<Atom>(*arg).name);
            } else {
                auto child_id = build_expr(*arg);
                if (child_id) {
                    input_ids.push_back(*child_id);
                }
            }
        }

        std::string node_id = builder.next_id("n");
        Node node(node_id, op_expr.op);
        node.inputs = input_ids;

        // Handle atom arguments based on operator semantics
        if (op_expr.op == "P" && atom_args.size() == 1 && input_ids.empty()) {
            node.dof_refs = {atom_args[0]};
        } else if ((op_expr.op == "S+" || op_expr.op == "S-") && 
                   !atom_args.empty() && input_ids.empty()) {
            node.dof_refs = atom_args;
        }

        builder.diagram().add_node(node);

        // Create edges for inputs
        for (const auto& input_id : input_ids) {
            std::string edge_id = builder.next_id("e");
            Edge edge(edge_id, input_id, node_id, "arg");
            builder.diagram().add_edge(edge);
        }

        return node_id;
    };

    auto node_id = build_expr(expr);

    // BUG FIX: Handle bare atom case
    if (!node_id && is_atom(expr)) {
        std::string nid = builder.next_id("n");
        Node node(nid, "P");
        node.dof_refs = {std::get<Atom>(expr).name};
        builder.diagram().add_node(node);
    }

    if (!compartment_id.empty()) {
        builder.diagram().set_compartment_id(compartment_id);
    }

    return builder.diagram();
}

} // namespace sid
