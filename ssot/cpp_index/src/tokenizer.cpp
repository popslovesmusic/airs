#include "tokenizer.h"

#include <array>
#include <vector>

#if defined(__AVX2__)
#include <immintrin.h>
#endif

namespace {

std::array<uint8_t, 256> build_token_table() {
    std::array<uint8_t, 256> table{};
    for (int c = 0; c < 256; ++c) {
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
            table[static_cast<size_t>(c)] = 1;
        }
    }
    return table;
}

const std::array<uint8_t, 256> kTokenTable = build_token_table();

void lowercase_ascii_inplace(std::string& s) {
    size_t i = 0;
#if defined(__AVX2__)
    const __m256i A1 = _mm256_set1_epi8(static_cast<char>('A' - 1));
    const __m256i Z1 = _mm256_set1_epi8(static_cast<char>('Z' + 1));
    const __m256i diff = _mm256_set1_epi8(32);
    for (; i + 32 <= s.size(); i += 32) {
        __m256i v = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(s.data() + i));
        __m256i geA = _mm256_cmpgt_epi8(v, A1);
        __m256i leZ = _mm256_cmpgt_epi8(Z1, v);
        __m256i mask = _mm256_and_si256(geA, leZ);
        __m256i add = _mm256_and_si256(mask, diff);
        v = _mm256_add_epi8(v, add);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(s.data() + i), v);
    }
#endif
    for (; i < s.size(); ++i) {
        char c = s[i];
        if (c >= 'A' && c <= 'Z') {
            s[i] = static_cast<char>(c + 32);
        }
    }
}

void split_terms(const std::string& text, std::vector<std::string>& out_terms) {
    std::string buffer = text;
    lowercase_ascii_inplace(buffer);

    const unsigned char* data = reinterpret_cast<const unsigned char*>(buffer.data());
    size_t n = buffer.size();
    size_t i = 0;
    while (i < n) {
        while (i < n && !kTokenTable[data[i]]) {
            ++i;
        }
        size_t start = i;
        while (i < n && kTokenTable[data[i]]) {
            ++i;
        }
        if (i > start) {
            out_terms.emplace_back(buffer.substr(start, i - start));
        }
    }
}

}  // namespace

void tokenize_to_counts(const std::string& text,
                        std::unordered_map<std::string, uint32_t>& counts,
                        uint32_t& token_count) {
    std::vector<std::string> terms;
    terms.reserve(64);
    split_terms(text, terms);
    token_count = static_cast<uint32_t>(terms.size());
    for (const auto& term : terms) {
        ++counts[term];
    }
}

void tokenize_to_terms(const std::string& text,
                       std::vector<std::string>& terms) {
    terms.clear();
    split_terms(text, terms);
}
