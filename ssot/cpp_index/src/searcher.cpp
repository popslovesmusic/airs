#include "common.h"
#include "tokenizer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct TermInfo {
    std::string term;
    uint64_t offset;
    uint32_t count;
};

struct Posting {
    uint32_t doc_id;
    uint32_t tf;
};

bool load_lexicon(const fs::path& path, std::vector<TermInfo>& lexicon) {
    std::ifstream in(path, std::ios::binary);
    if (!in.good()) return false;
    while (in.good()) {
        uint32_t len = 0;
        if (!in.read(reinterpret_cast<char*>(&len), sizeof(len))) {
            break;
        }
        TermInfo info;
        info.term.resize(len);
        if (!in.read(&info.term[0], len)) break;
        if (!in.read(reinterpret_cast<char*>(&info.offset), sizeof(info.offset))) break;
        if (!in.read(reinterpret_cast<char*>(&info.count), sizeof(info.count))) break;
        lexicon.push_back(std::move(info));
    }
    return true;
}

const TermInfo* find_term(const std::vector<TermInfo>& lexicon, const std::string& term) {
    auto it = std::lower_bound(
        lexicon.begin(), lexicon.end(), term,
        [](const TermInfo& a, const std::string& b) { return a.term < b; });
    if (it != lexicon.end() && it->term == term) {
        return &(*it);
    }
    return nullptr;
}

bool load_postings(std::ifstream& postings_in, const TermInfo& term,
                   std::vector<Posting>& out) {
    out.clear();
    postings_in.clear();
    postings_in.seekg(static_cast<std::streamoff>(term.offset), std::ios::beg);
    if (!postings_in.good()) return false;
    uint32_t doc_id = 0;
    for (uint32_t i = 0; i < term.count; ++i) {
        uint64_t delta = 0;
        uint64_t tf = 0;
        if (!read_varint(postings_in, delta)) return false;
        if (!read_varint(postings_in, tf)) return false;
        doc_id += static_cast<uint32_t>(delta);
        out.push_back(Posting{doc_id, static_cast<uint32_t>(tf)});
    }
    return true;
}

std::vector<uint32_t> intersect_doc_ids(const std::vector<uint32_t>& a,
                                        const std::vector<uint32_t>& b) {
    std::vector<uint32_t> out;
    size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            out.push_back(a[i]);
            ++i;
            ++j;
        } else if (a[i] < b[j]) {
            ++i;
        } else {
            ++j;
        }
    }
    return out;
}

int main(int argc, char** argv) {
    std::string index_dir = "ssot_index_cpp";
    std::string mode = "keyword";
    std::string query;
    size_t limit = 10;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--index" && i + 1 < argc) {
            index_dir = argv[++i];
        } else if (arg == "--mode" && i + 1 < argc) {
            mode = argv[++i];
        } else if (arg == "--limit" && i + 1 < argc) {
            limit = static_cast<size_t>(std::stoull(argv[++i]));
        } else if (arg == "--query" && i + 1 < argc) {
            query = argv[++i];
        } else {
            std::cerr << "Usage: searcher --index <dir> --mode keyword|full --limit N --query \"...\"\n";
            return 1;
        }
    }

    if (query.empty()) {
        std::cerr << "Query is empty.\n";
        return 1;
    }

    fs::path base(index_dir);
    std::vector<TermInfo> lexicon;
    if (!load_lexicon(base / "lexicon.bin", lexicon)) {
        std::cerr << "Failed to load lexicon.\n";
        return 1;
    }

    std::ifstream postings_in(base / "postings.bin", std::ios::binary);
    if (!postings_in.good()) {
        std::cerr << "Failed to open postings.bin\n";
        return 1;
    }

    std::ifstream doc_offsets_in(base / "docstore_offsets.bin", std::ios::binary);
    std::ifstream doc_data_in(base / "docstore_data.bin", std::ios::binary);
    std::ifstream doc_len_in(base / "docstore_doclen.bin", std::ios::binary);
    if (!doc_offsets_in.good() || !doc_data_in.good() || !doc_len_in.good()) {
        std::cerr << "Failed to open docstore files\n";
        return 1;
    }

    std::vector<uint64_t> doc_offsets;
    uint64_t off = 0;
    while (doc_offsets_in.read(reinterpret_cast<char*>(&off), sizeof(off))) {
        doc_offsets.push_back(off);
    }

    std::vector<uint32_t> doc_lens;
    uint32_t len = 0;
    while (doc_len_in.read(reinterpret_cast<char*>(&len), sizeof(len))) {
        doc_lens.push_back(len);
    }

    std::vector<std::string> terms;
    tokenize_to_terms(query, terms);
    if (terms.empty()) {
        std::cerr << "No valid terms in query.\n";
        return 1;
    }
    std::sort(terms.begin(), terms.end());
    terms.erase(std::unique(terms.begin(), terms.end()), terms.end());

    struct TermPostings {
        std::string term;
        uint32_t df;
        std::vector<Posting> postings;
    };

    std::vector<TermPostings> term_lists;
    for (const auto& term : terms) {
        const TermInfo* info = find_term(lexicon, term);
        if (!info) {
            std::cout << "No results.\n";
            return 0;
        }
        TermPostings tp;
        tp.term = term;
        tp.df = info->count;
        if (!load_postings(postings_in, *info, tp.postings)) {
            std::cerr << "Failed to load postings for term: " << term << "\n";
            return 1;
        }
        term_lists.push_back(std::move(tp));
    }

    std::sort(term_lists.begin(), term_lists.end(),
              [](const TermPostings& a, const TermPostings& b) {
                  return a.postings.size() < b.postings.size();
              });

    std::vector<uint32_t> candidate;
    candidate.reserve(term_lists.front().postings.size());
    for (const auto& p : term_lists.front().postings) {
        candidate.push_back(p.doc_id);
    }

    for (size_t i = 1; i < term_lists.size(); ++i) {
        std::vector<uint32_t> doc_ids;
        doc_ids.reserve(term_lists[i].postings.size());
        for (const auto& p : term_lists[i].postings) {
            doc_ids.push_back(p.doc_id);
        }
        candidate = intersect_doc_ids(candidate, doc_ids);
        if (candidate.empty()) {
            std::cout << "No results.\n";
            return 0;
        }
    }

    if (mode != "keyword" && mode != "full") {
        std::cerr << "Unknown mode: " << mode << "\n";
        return 1;
    }

    if (mode == "keyword") {
        size_t printed = 0;
        for (uint32_t doc_id : candidate) {
            if (doc_id >= doc_offsets.size()) continue;
            std::string id;
            std::string path;
            uint64_t next_offset = 0;
            if (!read_string_at(doc_data_in, doc_offsets[doc_id], id, &next_offset)) continue;
            if (!read_string_at(doc_data_in, next_offset, path)) continue;

            std::cout << id << "\t" << path << "\n";
            if (++printed >= limit) break;
        }
        return 0;
    }

    double avg_doc_len = 0.0;
    {
        std::ifstream meta_in(base / "index_meta.json");
        std::string line;
        while (std::getline(meta_in, line)) {
            if (line.find("\"avg_doc_len\"") != std::string::npos) {
                auto pos = line.find(':');
                if (pos != std::string::npos) {
                    avg_doc_len = std::stod(line.substr(pos + 1));
                }
            }
        }
    }
    if (avg_doc_len <= 0.0) {
        avg_doc_len = 1.0;
    }

    const double k1 = 1.2;
    const double b = 0.75;
    const double N = static_cast<double>(doc_offsets.size());

    std::vector<double> scores(candidate.size(), 0.0);
    for (const auto& tp : term_lists) {
        double df = static_cast<double>(tp.df);
        double idf = std::log((N - df + 0.5) / (df + 0.5) + 1.0);
        size_t i = 0;
        size_t j = 0;
        while (i < candidate.size() && j < tp.postings.size()) {
            uint32_t doc_id = candidate[i];
            const auto& p = tp.postings[j];
            if (doc_id == p.doc_id) {
                double dl = (doc_id < doc_lens.size()) ? doc_lens[doc_id] : avg_doc_len;
                double tf = static_cast<double>(p.tf);
                double denom = tf + k1 * (1.0 - b + b * dl / avg_doc_len);
                double score = idf * (tf * (k1 + 1.0) / denom);
                scores[i] += score;
                ++i;
                ++j;
            } else if (doc_id < p.doc_id) {
                ++i;
            } else {
                ++j;
            }
        }
    }

    using ScoredDoc = std::pair<double, uint32_t>;
    auto cmp = [](const ScoredDoc& a, const ScoredDoc& b) { return a.first > b.first; };
    std::priority_queue<ScoredDoc, std::vector<ScoredDoc>, decltype(cmp)> topk(cmp);

    for (size_t i = 0; i < candidate.size(); ++i) {
        if (topk.size() < limit) {
            topk.push({scores[i], candidate[i]});
        } else if (scores[i] > topk.top().first) {
            topk.pop();
            topk.push({scores[i], candidate[i]});
        }
    }

    std::vector<ScoredDoc> results;
    results.reserve(topk.size());
    while (!topk.empty()) {
        results.push_back(topk.top());
        topk.pop();
    }
    std::sort(results.begin(), results.end(),
              [](const ScoredDoc& a, const ScoredDoc& b) { return a.first > b.first; });

    for (const auto& res : results) {
        uint32_t doc_id = res.second;
        if (doc_id >= doc_offsets.size()) continue;
        std::string id;
        std::string path;
        uint64_t next_offset = 0;
        if (!read_string_at(doc_data_in, doc_offsets[doc_id], id, &next_offset)) continue;
        if (!read_string_at(doc_data_in, next_offset, path)) continue;

        std::cout << res.first << "\t" << id << "\t" << path << "\n";
    }

    return 0;
}
