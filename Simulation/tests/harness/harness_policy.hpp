// Policy loader for validation allow/deny lists.
#pragma once

#include <filesystem>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace harness {

struct ValidationPolicy {
    std::string engine_family;
    std::set<std::string> allowed;
    std::set<std::string> forbidden;
};

class PolicyTable {
public:
    void add(ValidationPolicy policy);

    // Returns nullptr if engine is unknown.
    const ValidationPolicy* find(const std::string& engine_family) const;

    bool is_allowed(const std::string& engine_family, const std::string& validation) const;
    bool is_forbidden(const std::string& engine_family, const std::string& validation) const;

    std::set<std::string> allowed_for(const std::string& engine_family) const;
    std::set<std::string> forbidden_for(const std::string& engine_family) const;

private:
    std::unordered_map<std::string, ValidationPolicy> table_;
};

// Load from CSV with columns: Engine Family,Type of Engine,Allowed...,Forbidden...
PolicyTable load_policy(const std::filesystem::path& csv_path);
PolicyTable load_default_policy();
std::filesystem::path default_policy_path();

}  // namespace harness
