# SSOT - Single Source of Truth

Knowledge management system with multi-backend search capabilities.

## Components

- **Data**: Authoritative database (2.3GB)
  - `ssot_parallel.db` - Main document database with FTS5
  - `rmap_graph.gpickle` - Relational graph
  - `ssot_index/` - Whoosh search index

- **API**: JSON-based search API
  - Strict JSON schemas in `api/schemas/`
  - Search modes: keyword, full_text
  - Sensitivity levels: 0-3

- **Tools**: Utilities for search and inspection
  - `search.py` - Command-line search tool
  - `inspector.py` - Data inspector

- **C++ Index**: High-performance AVX2 indexer/searcher
  - Build with CMake in `cpp_index/`

## Configuration

Edit `config.json` to customize:
- Search backends and priorities
- API settings
- Security parameters
- Audit logging

## Usage (Agent-Oriented)

### Search via JSON API:
```json
{
  "query": "your search terms",
  "mode": "full_text",
  "sensitivity": 1,
  "limit": 20
}
```

### Response Format:
```json
{
  "status": "success",
  "query": "your search terms",
  "total_results": 42,
  "execution_time_ms": 12.5,
  "results": [
    {
      "doc_id": "abc123...",
      "score": 3.14,
      "snippet": "...matching text...",
      "metadata": {...}
    }
  ]
}
```

See `api/schemas/` for complete JSON schemas.
