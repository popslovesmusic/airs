# SSOT C++ Index (AVX2)

This is a C++17 indexer/searcher that builds a fast, SIMD-friendly inverted index from `ssot_parallel.db`.
The SQLite DB remains the source of truth. The index is rebuildable.

Defaults:
- AND semantics across query terms
- ASCII lowercase only
- Output: `id` + `file_path` (no snippets)

## Build

```bash
cmake -S cpp_index -B cpp_index/build -DENABLE_AVX2=ON
cmake --build cpp_index/build --config Release
```

SQLite3 dev libraries are required to build the indexer. The searcher does not use SQLite.

## Index

```bash
cpp_index/build/Release/ssot_indexer --db ssot_parallel.db --out ssot_index_cpp --chunk 1000000
```

`--chunk` controls how many term entries are held in memory before flushing a sorted chunk to disk.
If you hit memory pressure, lower it (example: 200000).

## Search

Keyword mode (exact tokens, AND):

```bash
cpp_index/build/Release/ssot_searcher --index ssot_index_cpp --mode keyword --query "alpha beta" --limit 20
```

Full-text mode (BM25-lite, AND):

```bash
cpp_index/build/Release/ssot_searcher --index ssot_index_cpp --mode full --query "alpha beta" --limit 20
```

## Index Files

- `lexicon.bin`: term -> postings offset/count
- `postings.bin`: varint doc_id deltas + tf
- `docstore_data.bin`: varint-len strings (id, file_path)
- `docstore_offsets.bin`: offsets into `docstore_data.bin`
- `docstore_doclen.bin`: token counts per doc
- `index_meta.json`: doc count + avg doc length

## Notes

- Deletes/edits: rebuild the index (cheap with your update rates).
- Non-ASCII: tokenizer ignores unicode and only lowercases ASCII.
