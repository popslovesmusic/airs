/**
 * SID Diagram - Core SIDS graph data structures
 *
 * Port of Python diagram structures from sids/*.py
 * Defines graph nodes, edges, and diagram operations with iterative algorithms
 */

#pragma once

#include <algorithm>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <variant>
#include <vector>

namespace sid {

/**
 * Ternary values for semantic states (I/N/U)
 */
enum class Ternary {
    I,  // Included/Admissible
    N,  // Negated/Excluded
    U   // Undecided/Uncertain
};

inline std::string ternary_to_string(Ternary t) {
    switch (t) {
        case Ternary::I: return "I";
        case Ternary::N: return "N";
        case Ternary::U: return "U";
    }
    return "?";
}

/**
 * Attribute value type - can hold various types
 */
using AttrValue = std::variant<int, double, std::string, Ternary, bool>;

/**
 * Node in a semantic interaction diagram
 */
struct Node {
    std::string id;
    std::string op;  // Operator: P, S+, S-, O, C, T
    std::vector<std::string> inputs;  // Input node IDs
    std::vector<std::string> dof_refs;  // DOF references
    std::map<std::string, AttrValue> attributes;
    std::map<std::string, std::variant<bool, std::vector<std::string>>> meta;  // Metadata storage
    bool irreversible = false;  // For O (Collapse) operators

    Node() = default;
    explicit Node(const std::string& node_id, const std::string& operation = "")
        : id(node_id), op(operation) {}
};

/**
 * Edge in a semantic interaction diagram
 */
struct Edge {
    std::string id;
    std::string from;
    std::string to;
    std::string label;  // Edge type/label
    int port = 0;  // Port number for ordering
    std::map<std::string, AttrValue> attributes;

    Edge() = default;
    Edge(const std::string& edge_id, const std::string& from_id,
         const std::string& to_id, const std::string& edge_label = "arg")
        : id(edge_id), from(from_id), to(to_id), label(edge_label) {}
};

/**
 * Diagram - Directed graph of nodes and edges
 *
 * Implements iterative cycle detection (fixes CRITICAL bug from Python version)
 */
class Diagram {
private:
    std::string id_;
    std::string compartment_id_;
    std::vector<Node> nodes_;
    std::vector<Edge> edges_;

    // Cached adjacency lists (rebuilt when diagram changes)
    mutable std::map<std::string, std::vector<std::string>> adjacency_list_;
    mutable std::map<std::string, std::vector<std::string>> reverse_adjacency_list_;
    mutable bool adjacency_dirty_ = true;

    void rebuild_adjacency() const {
        if (!adjacency_dirty_) return;

        adjacency_list_.clear();
        reverse_adjacency_list_.clear();

        for (const auto& edge : edges_) {
            adjacency_list_[edge.from].push_back(edge.to);
            reverse_adjacency_list_[edge.to].push_back(edge.from);
        }

        adjacency_dirty_ = false;
    }

public:
    Diagram() = default;
    explicit Diagram(const std::string& diagram_id) : id_(diagram_id) {}

    // Accessors
    const std::string& id() const { return id_; }
    void set_id(const std::string& new_id) { id_ = new_id; }

    const std::string& compartment_id() const { return compartment_id_; }
    void set_compartment_id(const std::string& comp_id) { compartment_id_ = comp_id; }
    void setCompartment(const std::string& comp_id) { compartment_id_ = comp_id; }  // Alias for compatibility

    const std::vector<Node>& nodes() const { return nodes_; }
    std::vector<Node>& nodes() { return nodes_; }

    const std::vector<Edge>& edges() const { return edges_; }
    std::vector<Edge>& edges() { return edges_; }

    // Node operations
    void add_node(const Node& node) {
        nodes_.push_back(node);
    }

    void addNode(const Node& node) {  // CamelCase alias for compatibility
        nodes_.push_back(node);
    }

    Node* find_node(const std::string& node_id) {
        for (auto& node : nodes_) {
            if (node.id == node_id) return &node;
        }
        return nullptr;
    }

    const Node* find_node(const std::string& node_id) const {
        for (const auto& node : nodes_) {
            if (node.id == node_id) return &node;
        }
        return nullptr;
    }

    // Edge operations
    void add_edge(const Edge& edge) {
        edges_.push_back(edge);
        adjacency_dirty_ = true;
    }

    void addEdge(const Edge& edge) {  // CamelCase alias for compatibility
        edges_.push_back(edge);
        adjacency_dirty_ = true;
    }

    Edge* find_edge(const std::string& edge_id) {
        for (auto& edge : edges_) {
            if (edge.id == edge_id) return &edge;
        }
        return nullptr;
    }

    const Edge* find_edge(const std::string& edge_id) const {
        for (const auto& edge : edges_) {
            if (edge.id == edge_id) return &edge;
        }
        return nullptr;
    }

    /**
     * Get input edges to a node, sorted by port number
     * This ensures deterministic ordering
     */
    std::vector<std::string> get_inputs(const std::string& node_id) const {
        std::vector<std::pair<int, std::string>> port_edges;

        for (const auto& edge : edges_) {
            if (edge.to == node_id) {
                port_edges.push_back({edge.port, edge.from});
            }
        }

        // Sort by port number
        std::sort(port_edges.begin(), port_edges.end());

        std::vector<std::string> result;
        result.reserve(port_edges.size());
        for (const auto& [port, from_id] : port_edges) {
            result.push_back(from_id);
        }

        return result;
    }

    /**
     * Get output edges from a node
     */
    std::vector<std::string> get_outputs(const std::string& node_id) const {
        std::vector<std::string> result;

        for (const auto& edge : edges_) {
            if (edge.from == node_id) {
                result.push_back(edge.to);
            }
        }

        return result;
    }

    /**
     * Iterative cycle detection using DFS
     *
     * CRITICAL BUG FIX: Python version used recursive DFS which could hit
     * recursion limit. This iterative version scales to large graphs.
     */
    bool has_cycle() const {
        rebuild_adjacency();

        std::set<std::string> visited;
        std::set<std::string> rec_stack;

        // Get all node IDs
        std::vector<std::string> all_nodes;
        for (const auto& node : nodes_) {
            all_nodes.push_back(node.id);
        }

        // Try starting DFS from each unvisited node
        for (const auto& start : all_nodes) {
            if (visited.count(start)) continue;

            // Iterative DFS with explicit stack
            std::stack<std::pair<std::string, bool>> dfs_stack;
            dfs_stack.push({start, false});

            while (!dfs_stack.empty()) {
                auto [node_id, backtracking] = dfs_stack.top();
                dfs_stack.pop();

                if (backtracking) {
                    // Remove from recursion stack when done exploring
                    rec_stack.erase(node_id);
                    continue;
                }

                if (visited.count(node_id)) continue;

                // Check if already in recursion stack (cycle detected)
                if (rec_stack.count(node_id)) {
                    return true;
                }

                visited.insert(node_id);
                rec_stack.insert(node_id);

                // Mark for backtracking
                dfs_stack.push({node_id, true});

                // Push neighbors
                auto it = adjacency_list_.find(node_id);
                if (it != adjacency_list_.end()) {
                    for (const auto& neighbor : it->second) {
                        if (!visited.count(neighbor)) {
                            dfs_stack.push({neighbor, false});
                        }
                    }
                }
            }
        }

        return false;
    }

    /**
     * Mark adjacency lists as dirty (call after modifying edges)
     */
    void mark_dirty() {
        adjacency_dirty_ = true;
    }
};

} // namespace sid
