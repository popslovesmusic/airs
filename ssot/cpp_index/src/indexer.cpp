#include "common.h"
#include "tokenizer.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include <sqlite3.h>

namespace fs = std::filesystem;

struct Entry {
    std::string term;
    uint32_t doc_id;
    uint32_t tf;
};

struct LexEntry {
    std::string term;
    uint64_t postings_offset;
    uint32_t postings_count;
};

struct ChunkReader {
    std::ifstream in;
    Entry current;
    bool valid = false;

    explicit ChunkReader(const fs::path& path) : in(path, std::ios::binary) {
        advance();
    }

    void advance() {
        if (!in.good()) {
            valid = false;
            return;
        }
        uint32_t term_len = 0;
        if (!in.read(reinterpret_cast<char*>(&term_len), sizeof(term_len))) {
            valid = false;
            return;
        }
        current.term.resize(term_len);
        if (!in.read(&current.term[0], term_len)) {
            valid = false;
            return;
        }
        if (!in.read(reinterpret_cast<char*>(&current.doc_id), sizeof(current.doc_id))) {
            valid = false;
            return;
        }
        if (!in.read(reinterpret_cast<char*>(&current.tf), sizeof(current.tf))) {
            valid = false;
            return;
        }
        valid = true;
    }
};

struct ReaderCmp {
    bool operator()(const ChunkReader* a, const ChunkReader* b) const {
        if (a->current.term != b->current.term) {
            return a->current.term > b->current.term;
        }
        return a->current.doc_id > b->current.doc_id;
    }
};

void write_chunk(std::vector<Entry>& entries, const fs::path& path) {
    std::sort(entries.begin(), entries.end(),
              [](const Entry& a, const Entry& b) {
                  if (a.term != b.term) return a.term < b.term;
                  return a.doc_id < b.doc_id;
              });
    std::ofstream out(path, std::ios::binary);
    for (const auto& e : entries) {
        uint32_t len = static_cast<uint32_t>(e.term.size());
        out.write(reinterpret_cast<const char*>(&len), sizeof(len));
        out.write(e.term.data(), len);
        out.write(reinterpret_cast<const char*>(&e.doc_id), sizeof(e.doc_id));
        out.write(reinterpret_cast<const char*>(&e.tf), sizeof(e.tf));
    }
    entries.clear();
    entries.shrink_to_fit();
}

int main(int argc, char** argv) {
    std::string db_path = "ssot_parallel.db";
    std::string out_dir = "ssot_index_cpp";
    size_t chunk_limit = 1000000;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--db" && i + 1 < argc) {
            db_path = argv[++i];
        } else if (arg == "--out" && i + 1 < argc) {
            out_dir = argv[++i];
        } else if (arg == "--chunk" && i + 1 < argc) {
            chunk_limit = static_cast<size_t>(std::stoull(argv[++i]));
        } else {
            std::cerr << "Usage: indexer --db <path> --out <dir> [--chunk N]\n";
            return 1;
        }
    }

    fs::path out_path(out_dir);
    fs::create_directories(out_path);
    fs::path tmp_path = out_path / "tmp";
    fs::create_directories(tmp_path);

    sqlite3* db = nullptr;
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Failed to open DB: " << sqlite3_errmsg(db) << "\n";
        return 1;
    }

    const char* sql =
        "SELECT f.id, f.content, m.file_path "
        "FROM documents_fts f JOIN documents_meta m ON f.id = m.id;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare SQL: " << sqlite3_errmsg(db) << "\n";
        sqlite3_close(db);
        return 1;
    }

    std::vector<Entry> entries;
    entries.reserve(chunk_limit);
    std::vector<fs::path> chunk_files;
    std::vector<uint64_t> doc_offsets;
    std::vector<uint32_t> doc_lengths;
    uint64_t total_tokens = 0;

    std::ofstream doc_data(out_path / "docstore_data.bin", std::ios::binary);

    uint32_t doc_id = 0;
    uint64_t row_count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* id_text = sqlite3_column_text(stmt, 0);
        const unsigned char* content_text = sqlite3_column_text(stmt, 1);
        const unsigned char* path_text = sqlite3_column_text(stmt, 2);

        std::string id = id_text ? reinterpret_cast<const char*>(id_text) : "";
        std::string content = content_text ? reinterpret_cast<const char*>(content_text) : "";
        std::string path = path_text ? reinterpret_cast<const char*>(path_text) : "";

        uint64_t offset = static_cast<uint64_t>(doc_data.tellp());
        doc_offsets.push_back(offset);
        write_varint(doc_data, id.size());
        doc_data.write(id.data(), static_cast<std::streamsize>(id.size()));
        write_varint(doc_data, path.size());
        doc_data.write(path.data(), static_cast<std::streamsize>(path.size()));

        std::unordered_map<std::string, uint32_t> counts;
        counts.reserve(128);
        uint32_t token_count = 0;
        tokenize_to_counts(content, counts, token_count);
        doc_lengths.push_back(token_count);
        total_tokens += token_count;

        for (const auto& kv : counts) {
            entries.push_back(Entry{kv.first, doc_id, kv.second});
            if (entries.size() >= chunk_limit) {
                fs::path chunk_file = tmp_path / ("chunk_" + std::to_string(chunk_files.size()) + ".bin");
                write_chunk(entries, chunk_file);
                chunk_files.push_back(chunk_file);
                entries.reserve(chunk_limit);
            }
        }

        ++doc_id;
        ++row_count;
        if (row_count % 5000 == 0) {
            std::cerr << "Indexed " << row_count << " docs...\n";
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (!entries.empty()) {
        fs::path chunk_file = tmp_path / ("chunk_" + std::to_string(chunk_files.size()) + ".bin");
        write_chunk(entries, chunk_file);
        chunk_files.push_back(chunk_file);
    }

    std::ofstream doc_offsets_out(out_path / "docstore_offsets.bin", std::ios::binary);
    for (uint64_t off : doc_offsets) {
        doc_offsets_out.write(reinterpret_cast<const char*>(&off), sizeof(off));
    }

    std::ofstream doc_len_out(out_path / "docstore_doclen.bin", std::ios::binary);
    for (uint32_t len : doc_lengths) {
        doc_len_out.write(reinterpret_cast<const char*>(&len), sizeof(len));
    }

    std::ofstream postings(out_path / "postings.bin", std::ios::binary);
    std::vector<LexEntry> lexicon;

    std::vector<std::unique_ptr<ChunkReader>> readers;
    readers.reserve(chunk_files.size());
    for (const auto& path : chunk_files) {
        readers.push_back(std::make_unique<ChunkReader>(path));
    }

    std::priority_queue<ChunkReader*, std::vector<ChunkReader*>, ReaderCmp> heap;
    for (auto& reader : readers) {
        if (reader->valid) {
            heap.push(reader.get());
        }
    }

    std::string current_term;
    uint64_t postings_offset = 0;
    uint32_t postings_count = 0;
    uint32_t prev_doc_id = 0;
    bool have_term = false;

    while (!heap.empty()) {
        ChunkReader* reader = heap.top();
        heap.pop();

        const Entry& e = reader->current;
        if (!have_term || e.term != current_term) {
            if (have_term) {
                lexicon.push_back(LexEntry{current_term, postings_offset, postings_count});
            }
            current_term = e.term;
            postings_offset = static_cast<uint64_t>(postings.tellp());
            postings_count = 0;
            prev_doc_id = 0;
            have_term = true;
        }

        uint32_t delta = e.doc_id - prev_doc_id;
        write_varint(postings, delta);
        write_varint(postings, e.tf);
        prev_doc_id = e.doc_id;
        ++postings_count;

        reader->advance();
        if (reader->valid) {
            heap.push(reader);
        }
    }

    if (have_term) {
        lexicon.push_back(LexEntry{current_term, postings_offset, postings_count});
    }

    std::ofstream lex_out(out_path / "lexicon.bin", std::ios::binary);
    for (const auto& lex : lexicon) {
        uint32_t len = static_cast<uint32_t>(lex.term.size());
        lex_out.write(reinterpret_cast<const char*>(&len), sizeof(len));
        lex_out.write(lex.term.data(), len);
        lex_out.write(reinterpret_cast<const char*>(&lex.postings_offset), sizeof(lex.postings_offset));
        lex_out.write(reinterpret_cast<const char*>(&lex.postings_count), sizeof(lex.postings_count));
    }

    double avg_doc_len = doc_lengths.empty()
                             ? 0.0
                             : static_cast<double>(total_tokens) / static_cast<double>(doc_lengths.size());

    std::ofstream meta(out_path / "index_meta.json");
    meta << "{\n";
    meta << "  \"doc_count\": " << doc_lengths.size() << ",\n";
    meta << "  \"avg_doc_len\": " << avg_doc_len << ",\n";
    meta << "  \"source_db\": \"" << db_path << "\"\n";
    meta << "}\n";

    std::cerr << "Index build complete. Docs: " << doc_lengths.size() << "\n";
    return 0;
}
