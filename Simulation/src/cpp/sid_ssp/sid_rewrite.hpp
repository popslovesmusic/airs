/**
 * SID Rewrite Engine - C++ Implementation
 *
 * Pattern matching and diagram rewriting for semantic interaction diagrams.
 * Port from sids/sid_rewrite.py
 */

#pragma once

#include "sid_ast.hpp"
#include "sid_diagram.hpp"
#include "sid_parser_impl.hpp"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <tuple>
#include <stdexcept>
#include <algorithm>

namespace sid {

// Configuration constants
constexpr int MAX_REWRITE_ITERATIONS = 1000;  // Prevent infinite loops

/**
 * Result of a rewrite operation
 */
struct RewriteResult {
    bool applied;
    Diagram diagram;
    std::vector<std::string> messages;

    RewriteResult(bool app, const Diagram& diag, const std::vector<std::string>& msgs = {})
        : applied(app), diagram(diag), messages(msgs) {}
};

/**
 * Variable bindings map
 */
using Bindings = std::unordered_map<std::string, std::string>;

/**
 * Check if atom name represents a variable
 */
inline bool isVariable(const std::string& atom) {
    if (atom.empty()) return false;
    if (atom[0] == '$') return true;
    return atom.size() == 1 && std::islower(atom[0]);
}

/**
 * Generate next unique node ID
 */
inline std::string nextNodeId(const Diagram& diagram, const std::string& prefix) {
    std::unordered_set<std::string> existing;
    for (const auto& node : diagram.nodes()) {
        existing.insert(node.id);
    }

    int idx = 1;
    while (true) {
        std::string candidate = prefix + std::to_string(idx);
        if (existing.find(candidate) == existing.end()) {
            return candidate;
        }
        idx++;
    }
}

/**
 * Generate next unique edge ID
 */
inline std::string nextEdgeId(const Diagram& diagram, const std::string& prefix) {
    std::unordered_set<std::string> existing;
    for (const auto& edge : diagram.edges()) {
        existing.insert(edge.id);
    }

    int idx = 1;
    while (true) {
        std::string candidate = prefix + std::to_string(idx);
        if (existing.find(candidate) == existing.end()) {
            return candidate;
        }
        idx++;
    }
}

/**
 * Build expression into diagram
 *
 * Recursively constructs nodes and edges from AST.
 *
 * @param expr AST expression to build
 * @param diagram Diagram to modify
 * @param bindings Variable bindings for pattern variables
 * @param rule_id Rule identifier for naming new nodes
 * @return ID of root node created
 */
inline std::string buildExpr(const ASTNode& expr, Diagram& diagram,
                              const Bindings& bindings, const std::string& rule_id) {
    // Handle atoms
    if (expr.kind == ASTKind::Atom) {
        // Check if it's a variable
        if (isVariable(expr.atom_name)) {
            std::string var_name = expr.atom_name;
            if (var_name[0] == '$') {
                var_name = var_name.substr(1);  // Strip $
            }

            auto it = bindings.find(var_name);
            if (it == bindings.end()) {
                throw std::runtime_error("Unbound variable: " + expr.atom_name);
            }
            return it->second;
        }

        // Create P node for literal atom
        std::string node_id = nextNodeId(diagram, rule_id + "_n");
        Node node(node_id, "P");
        node.dof_refs.push_back(expr.atom_name);
        diagram.addNode(node);
        return node_id;
    }

    // Handle operator expressions
    if (expr.kind == ASTKind::Op) {
        // Build all arguments
        std::vector<std::string> input_ids;
        for (const auto& arg : expr.args) {
            std::string arg_id = buildExpr(arg, diagram, bindings, rule_id);
            input_ids.push_back(arg_id);
        }

        // Create operator node
        std::string node_id = nextNodeId(diagram, rule_id + "_n");
        Node node(node_id, expr.op_name);
        node.inputs = input_ids;

        // Special handling for P operator
        if (expr.op_name == "P" && !expr.args.empty() && expr.args[0].kind == ASTKind::Atom) {
            const auto& atom = expr.args[0];
            if (!isVariable(atom.atom_name)) {
                node.dof_refs.push_back(atom.atom_name);
            }
        }

        // Mark O (Collapse) operators as irreversible
        if (expr.op_name == "O") {
            node.irreversible = true;
        }

        diagram.addNode(node);

        // Create edges for inputs
        for (const auto& input_id : input_ids) {
            std::string edge_id = nextEdgeId(diagram, rule_id + "_e");
            Edge edge(edge_id, input_id, node_id, "arg");
            diagram.addEdge(edge);
        }

        return node_id;
    }

    throw std::runtime_error("Unknown expression type");
}

/**
 * Match expression pattern against diagram node
 *
 * Recursively matches AST pattern against diagram structure.
 *
 * @param expr Pattern expression
 * @param node_id Node to match against
 * @param diagram Diagram containing nodes
 * @param bindings Current variable bindings (modified in-place)
 * @param matched Set of matched node IDs (modified in-place)
 * @param bound_nodes Set of nodes bound to variables (modified in-place)
 * @return true if match succeeds
 */
inline bool matchExpr(const ASTNode& expr, const std::string& node_id,
                      const Diagram& diagram, Bindings& bindings,
                      std::unordered_set<std::string>& matched,
                      std::unordered_set<std::string>& bound_nodes) {
    const Node* node = diagram.find_node(node_id);
    if (!node) return false;

    // Handle atom patterns
    if (expr.kind == ASTKind::Atom) {
        if (isVariable(expr.atom_name)) {
            // Variable binding
            std::string var_name = expr.atom_name;
            if (var_name[0] == '$') {
                var_name = var_name.substr(1);
            }

            auto it = bindings.find(var_name);
            if (it != bindings.end() && it->second != node_id) {
                return false;  // Conflict
            }

            bindings[var_name] = node_id;
            bound_nodes.insert(node_id);
            return true;
        }

        // Literal atom match
        if (node->op == "P") {
            for (const auto& dof : node->dof_refs) {
                if (dof == expr.atom_name) return true;
            }
        }

        // Check metadata
        auto meta_it = node->meta.find("atom_args");
        if (meta_it != node->meta.end()) {
            if (std::holds_alternative<std::vector<std::string>>(meta_it->second)) {
                const auto& atom_args = std::get<std::vector<std::string>>(meta_it->second);
                for (const auto& arg : atom_args) {
                    if (arg == expr.atom_name) return true;
                }
            }
        }

        return false;
    }

    // Handle operator patterns
    if (expr.kind == ASTKind::Op) {
        if (node->op != expr.op_name) return false;

        matched.insert(node_id);

        // Match arguments
        if (!node->inputs.empty()) {
            if (node->inputs.size() < expr.args.size()) return false;

            for (size_t i = 0; i < expr.args.size(); ++i) {
                if (!matchExpr(expr.args[i], node->inputs[i], diagram,
                               bindings, matched, bound_nodes)) {
                    return false;
                }
            }
        } else {
            // No inputs - check for P operator with atom argument
            if (expr.op_name == "P" && !expr.args.empty()) {
                const auto& arg = expr.args[0];
                if (arg.kind == ASTKind::Atom) {
                    if (isVariable(arg.atom_name)) {
                        std::string var_name = arg.atom_name;
                        if (var_name[0] == '$') var_name = var_name.substr(1);

                        auto it = bindings.find(var_name);
                        if (it != bindings.end() && it->second != node_id) {
                            return false;
                        }

                        bindings[var_name] = node_id;
                        bound_nodes.insert(node_id);
                    } else {
                        // Check dof_refs
                        bool found = false;
                        for (const auto& dof : node->dof_refs) {
                            if (dof == arg.atom_name) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) return false;
                    }
                } else {
                    return false;
                }
            } else if (!expr.args.empty()) {
                return false;
            }
        }

        return true;
    }

    return false;
}

/**
 * Find expression match in diagram
 *
 * @param diagram Diagram to search
 * @param expr Pattern expression
 * @return Optional tuple of (root_id, bindings, matched_nodes, bound_nodes)
 */
inline std::optional<std::tuple<std::string, Bindings, std::unordered_set<std::string>, std::unordered_set<std::string>>>
findExprMatch(const Diagram& diagram, const ASTNode& expr) {
    for (const auto& node : diagram.nodes()) {
        Bindings bindings;
        std::unordered_set<std::string> matched;
        std::unordered_set<std::string> bound_nodes;

        if (matchExpr(expr, node.id, diagram, bindings, matched, bound_nodes)) {
            return std::make_tuple(node.id, bindings, matched, bound_nodes);
        }
    }

    return std::nullopt;
}

/**
 * Apply expression-based rewrite rule
 *
 * This is the core rewrite operation used by the SID engine.
 *
 * @param diagram Diagram to rewrite
 * @param pattern_text Pattern expression string (e.g., "P(Freedom)")
 * @param replacement_text Replacement expression string
 * @param rule_id Rule identifier
 * @return RewriteResult with updated diagram
 */
inline RewriteResult applyExprRewrite(const Diagram& diagram,
                                      const std::string& pattern_text,
                                      const std::string& replacement_text,
                                      const std::string& rule_id) {
    std::vector<std::string> messages;

    // Parse expressions
    ASTNode pattern_expr;
    ASTNode replacement_expr;

    try {
        pattern_expr = parseExpression(pattern_text);
        replacement_expr = parseExpression(replacement_text);
    } catch (const ParseError& e) {
        messages.push_back("ERROR: " + std::string(e.what()));
        return RewriteResult(false, diagram, messages);
    }

    // Find match
    auto match = findExprMatch(diagram, pattern_expr);
    if (!match.has_value()) {
        messages.push_back("Rewrite " + rule_id + " not applicable");
        return RewriteResult(false, diagram, messages);
    }

    auto [root_id, bindings, matched_nodes, bound_nodes] = match.value();

    // Create new diagram with rewrite applied
    Diagram new_diagram = diagram;  // Copy

    // Build replacement
    std::string new_root = buildExpr(replacement_expr, new_diagram, bindings, rule_id);

    // Compute nodes to remove (matched but not bound to variables)
    std::unordered_set<std::string> remove_nodes;
    for (const auto& node_id : matched_nodes) {
        if (bound_nodes.find(node_id) == bound_nodes.end()) {
            remove_nodes.insert(node_id);
        }
    }

    // Redirect edges pointing to removed nodes to new root
    std::vector<Edge> new_edges;
    for (const auto& edge : new_diagram.edges()) {
        if (remove_nodes.find(edge.to) != remove_nodes.end()) {
            // Redirect to new root
            Edge redirected = edge;
            redirected.to = new_root;
            new_edges.push_back(redirected);
        } else if (remove_nodes.find(edge.from) == remove_nodes.end() &&
                   remove_nodes.find(edge.to) == remove_nodes.end()) {
            // Keep edge
            new_edges.push_back(edge);
        }
        // Else: edge connects to removed node, discard it
    }

    // Remove matched nodes
    std::vector<Node> new_nodes;
    for (const auto& node : new_diagram.nodes()) {
        if (remove_nodes.find(node.id) == remove_nodes.end()) {
            new_nodes.push_back(node);
        }
    }

    // Update diagram
    new_diagram.nodes() = new_nodes;
    new_diagram.edges() = new_edges;
    new_diagram.mark_dirty();

    // Check for cycles
    if (new_diagram.has_cycle()) {
        messages.push_back("ERROR: Rewrite " + rule_id + " would introduce cycle");
        return RewriteResult(false, diagram, messages);
    }

    messages.push_back("Rewrite " + rule_id + " applied");
    return RewriteResult(true, new_diagram, messages);
}

/**
 * Check if rewrite rule is applicable to diagram
 *
 * @param diagram Diagram to check
 * @param pattern_text Pattern expression string
 * @return true if pattern matches diagram
 */
inline bool ruleApplicable(const Diagram& diagram, const std::string& pattern_text) {
    try {
        ASTNode pattern_expr = parseExpression(pattern_text);
        return findExprMatch(diagram, pattern_expr).has_value();
    } catch (const ParseError&) {
        return false;
    }
}

} // namespace sid
