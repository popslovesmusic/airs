#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

void tokenize_to_counts(const std::string& text,
                        std::unordered_map<std::string, uint32_t>& counts,
                        uint32_t& token_count);

void tokenize_to_terms(const std::string& text,
                       std::vector<std::string>& terms);
