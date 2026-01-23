// Policy loader implementation.
#include "harness_policy.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {

std::string trim(std::string s) {
    auto not_space = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
    return s;
}

std::string normalize(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return trim(out);
}

std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    bool in_quotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '\"') {
            in_quotes = !in_quotes;
            continue;
        }
        if (c == ',' && !in_quotes) {
            fields.push_back(trim(current));
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    fields.push_back(trim(current));
    return fields;
}

std::set<std::string> split_items(const std::string& items) {
    std::set<std::string> out;
    std::string token;
    std::stringstream ss(items);
    while (std::getline(ss, token, ',')) {
        auto norm = normalize(token);
        if (!norm.empty()) {
            out.insert(norm);
        }
    }
    return out;
}

}  // namespace

namespace harness {

void PolicyTable::add(ValidationPolicy policy) {
    table_[normalize(policy.engine_family)] = std::move(policy);
}

const ValidationPolicy* PolicyTable::find(const std::string& engine_family) const {
    auto it = table_.find(normalize(engine_family));
    if (it == table_.end()) {
        return nullptr;
    }
    return &it->second;
}

bool PolicyTable::is_allowed(const std::string& engine_family, const std::string& validation) const {
    auto* p = find(engine_family);
    if (!p) {
        return false;
    }
    return p->allowed.count(normalize(validation)) > 0;
}

bool PolicyTable::is_forbidden(const std::string& engine_family, const std::string& validation) const {
    auto* p = find(engine_family);
    if (!p) {
        return false;
    }
    return p->forbidden.count(normalize(validation)) > 0;
}

std::set<std::string> PolicyTable::allowed_for(const std::string& engine_family) const {
    auto* p = find(engine_family);
    if (!p) return {};
    return p->allowed;
}

std::set<std::string> PolicyTable::forbidden_for(const std::string& engine_family) const {
    auto* p = find(engine_family);
    if (!p) return {};
    return p->forbidden;
}

PolicyTable load_policy(const std::filesystem::path& csv_path) {
    PolicyTable table;
    std::ifstream in(csv_path);
    if (!in.is_open()) {
        throw std::runtime_error("Could not open policy file: " + csv_path.string());
    }

    std::string line;
    bool first = true;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto fields = split_csv_line(line);
        if (first) {  // skip header
            first = false;
            continue;
        }
        if (fields.size() < 4) {
            continue;  // malformed line
        }
        ValidationPolicy policy;
        policy.engine_family = normalize(fields[0]);
        policy.allowed = split_items(fields[2]);
        policy.forbidden = split_items(fields[3]);
        table.add(std::move(policy));
    }

    return table;
}

std::filesystem::path default_policy_path() {
#ifdef VALIDATION_TABLE_PATH
    return std::filesystem::path(VALIDATION_TABLE_PATH);
#else
    return std::filesystem::path("validation table.txt");
#endif
}

PolicyTable load_default_policy() {
    return load_policy(default_policy_path());
}

}  // namespace harness
