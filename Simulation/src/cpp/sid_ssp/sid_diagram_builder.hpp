/**
 * SID Diagram Builder - C++ Implementation
 *
 * Converts AST expressions to diagram representation.
 * Port from sids/sid_ast_to_diagram.py
 */

#pragma once

#include "sid_ast.hpp"
#include "sid_diagram.hpp"
#include <string>
#include <vector>
#include <unordered_set>
#include <stdexcept>

namespace sid {

/**
 * Build context for diagram construction
 */
struct DiagramBuildContext {
    std::vector<Node> nodes;
    std::vector<Edge> edges;
    int counter = 0;

    std::string nextId(const std::string& prefix) {
        counter++;
        return prefix + std::to_string(counter);
    }
};

/**
 * Build information returned during expression traversal
 */
struct BuildInfo {
    std::string node_id;  // Empty if no node created
    std::vector<std::string> atoms;
};

/**
 * Validate diagram structure
 *
 * Checks:
 * - All edges reference valid nodes
 * - All node inputs reference valid nodes
 *
 * @throws std::runtime_error on validation failure
 */
inline void validateDiagramStructure(const Diagram& diagram) {
    // Collect all node IDs
    std::unordered_set<std::string> node_ids;
    for (const auto& node : diagram.nodes()) {
        node_ids.insert(node.id);
    }

    // Check all edges reference valid nodes
    for (const auto& edge : diagram.edges()) {
        if (node_ids.find(edge.from) == node_ids.end()) {
            throw std::runtime_error("Edge " + edge.id + " references non-existent 'from' node: " + edge.from);
        }
        if (node_ids.find(edge.to) == node_ids.end()) {
            throw std::runtime_error("Edge " + edge.id + " references non-existent 'to' node: " + edge.to);
        }
    }

    // Check all node inputs reference valid nodes
    for (const auto& node : diagram.nodes()) {
        for (const auto& input_id : node.inputs) {
            if (node_ids.find(input_id) == node_ids.end()) {
                throw std::runtime_error("Node " + node.id + " references non-existent input node: " + input_id);
            }
        }
    }
}

/**
 * Build expression recursively
 *
 * @param expr AST node to build
 * @param ctx Build context
 * @return Build information (node_id and atoms)
 */
inline BuildInfo buildExpr(const ASTNode& expr, DiagramBuildContext& ctx) {
    // Handle Atom nodes
    if (expr.kind == ASTKind::Atom) {
        BuildInfo info;
        info.atoms.push_back(expr.atom_name);
        return info;
    }

    // Handle operator nodes
    if (expr.kind != ASTKind::Op) {
        throw std::runtime_error("Unknown expression type");
    }

    std::vector<std::string> atom_args;
    std::vector<std::string> input_ids;

    // Build all arguments
    for (const auto& arg : expr.args) {
        BuildInfo child_info = buildExpr(arg, ctx);

        // Collect atoms
        atom_args.insert(atom_args.end(), child_info.atoms.begin(), child_info.atoms.end());

        // Collect input nodes
        if (!child_info.node_id.empty()) {
            input_ids.push_back(child_info.node_id);
        }
    }

    // Create node
    std::string node_id = ctx.nextId("n");
    Node node;
    node.id = node_id;
    node.op = expr.op_name;
    node.inputs = input_ids;

    // Handle atom arguments based on operator semantics
    if (expr.op_name == "P" && atom_args.size() == 1 && input_ids.empty()) {
        // P operator with single atom argument -> set dof_refs
        node.dof_refs.push_back(atom_args[0]);
    } else if ((expr.op_name == "S+" || expr.op_name == "S-") && !atom_args.empty() && input_ids.empty()) {
        // Superposition operators with atom arguments -> use dof_refs
        node.dof_refs = atom_args;
    } else if (!atom_args.empty()) {
        // Other operators: store atoms in metadata for reference
        // (This is for tracking purposes; actual semantics use input edges)
        node.meta["atom_args"] = atom_args;
    }

    ctx.nodes.push_back(node);

    // Create edges for inputs
    for (const auto& input_id : input_ids) {
        std::string edge_id = ctx.nextId("e");
        Edge edge;
        edge.id = edge_id;
        edge.from = input_id;
        edge.to = node_id;
        edge.label = "arg";  // Standardized: all edges represent argument relationships
        ctx.edges.push_back(edge);
    }

    BuildInfo info;
    info.node_id = node_id;
    return info;
}

/**
 * Convert AST expression to diagram
 *
 * @param expr AST expression to convert
 * @param diagram_id Unique identifier for diagram
 * @param compartment_id Optional compartment identifier
 * @return Constructed diagram
 * @throws std::runtime_error on validation failure
 */
inline Diagram exprToDiagram(const ASTNode& expr,
                             const std::string& diagram_id = "d_expr",
                             const std::string& compartment_id = "") {
    DiagramBuildContext ctx;

    // Build expression tree
    BuildInfo info = buildExpr(expr, ctx);

    // If only an atom was provided (no operator node created), wrap in P operator
    if (info.node_id.empty() && !info.atoms.empty()) {
        std::string node_id = ctx.nextId("n");
        Node node;
        node.id = node_id;
        node.op = "P";
        node.dof_refs.push_back(info.atoms[0]);
        node.meta["atom_only"] = true;
        ctx.nodes.push_back(node);
        info.node_id = node_id;
    }

    // Create diagram
    Diagram diagram(diagram_id);

    // Set compartment if provided
    if (!compartment_id.empty()) {
        diagram.setCompartment(compartment_id);
    }

    // Add nodes and edges
    for (const auto& node : ctx.nodes) {
        diagram.addNode(node);
    }
    for (const auto& edge : ctx.edges) {
        diagram.addEdge(edge);
    }

    // Validate diagram structure
    validateDiagramStructure(diagram);

    return diagram;
}

} // namespace sid
