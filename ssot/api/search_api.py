#!/usr/bin/env python3
"""
SSOT Search API - Strict JSON Interface for Agent Use
Validates all input/output against JSON schemas
"""

import sys
import json
import sqlite3
import time
from pathlib import Path
from typing import Dict, List, Any

# Add parent to path for config import
sys.path.insert(0, str(Path(__file__).parent.parent))

try:
    import jsonschema
    HAS_VALIDATION = True
except ImportError:
    HAS_VALIDATION = False
    print("Warning: jsonschema not installed. Schema validation disabled.", file=sys.stderr)


class SSOTSearchAPI:
    """SSOT Search API with strict JSON I/O"""

    def __init__(self, config_path: Path = None):
        if config_path is None:
            config_path = Path(__file__).parent.parent / "config.json"

        # Load configuration
        with open(config_path, 'r') as f:
            self.config = json.load(f)['ssot']

        # Resolve paths relative to ssot directory
        self.ssot_root = Path(__file__).parent.parent
        self.db_path = self.ssot_root / self.config['data']['database_path']

        # Load schemas
        self.schemas = self._load_schemas()

        # Connect to database
        if not self.db_path.exists():
            raise FileNotFoundError(f"Database not found: {self.db_path}")

        self.conn = sqlite3.connect(str(self.db_path))

    def _load_schemas(self) -> Dict[str, Dict]:
        """Load JSON schemas"""
        schemas_dir = self.ssot_root / "api" / "schemas"
        schemas = {}

        schema_files = {
            'request': 'search_request.schema.json',
            'response': 'search_response.schema.json',
            'document': 'document.schema.json'
        }

        for name, filename in schema_files.items():
            schema_path = schemas_dir / filename
            if schema_path.exists():
                with open(schema_path, 'r') as f:
                    schemas[name] = json.load(f)

        return schemas

    def validate_request(self, request: Dict) -> None:
        """Validate request against schema"""
        if HAS_VALIDATION and 'request' in self.schemas:
            jsonschema.validate(instance=request, schema=self.schemas['request'])

    def validate_response(self, response: Dict) -> None:
        """Validate response against schema"""
        if HAS_VALIDATION and 'response' in self.schemas:
            jsonschema.validate(instance=response, schema=self.schemas['response'])

    def _sanitize_query(self, query_string: str) -> str:
        """Sanitize query string for FTS5"""
        import re
        sanitized = re.sub(r'[^\w\s\-]', ' ', query_string)
        sanitized = re.sub(r'\s+', ' ', sanitized)
        return sanitized.strip()

    def _build_fts_query(self, query_string: str, sensitivity: int) -> str:
        """Build FTS5 query based on sensitivity"""
        sanitized = self._sanitize_query(query_string)
        terms = sanitized.split()

        if not terms:
            return ""

        if sensitivity == 0:  # STRICT (AND)
            return " AND ".join(terms)
        elif sensitivity == 1:  # NORMAL
            if len(terms) == 1:
                return terms[0]
            else:
                core = " AND ".join(terms[:-1])
                return f"({core}) OR {terms[-1]}"
        elif sensitivity == 2:  # BROAD (OR)
            return " OR ".join(terms)
        elif sensitivity == 3:  # DIAGNOSTIC
            return " OR ".join(terms)
        else:
            return self._build_fts_query(query_string, 1)

    def search(self, request: Dict) -> Dict:
        """
        Execute search and return strict JSON response

        Args:
            request: Validated search request

        Returns:
            Validated search response
        """
        start_time = time.time()

        # Validate input
        self.validate_request(request)

        # Extract parameters
        query = request['query']
        mode = request.get('mode', 'full_text')
        sensitivity = request.get('sensitivity', 1)
        limit = request.get('limit', 20)
        filters = request.get('filters', {})

        # Build FTS query
        fts_query = self._build_fts_query(query, sensitivity)

        if not fts_query:
            response = {
                'status': 'error',
                'query': query,
                'results': [],
                'total_results': 0,
                'execution_time_ms': 0,
                'error': {
                    'code': 'EMPTY_QUERY',
                    'message': 'Query string is empty after sanitization'
                }
            }
            self.validate_response(response)
            return response

        # Execute search
        cursor = self.conn.cursor()

        try:
            # Build SQL query
            sql = """
                SELECT
                    d.id,
                    d.logic_class,
                    d.original_uri,
                    d.format,
                    snippet(documents_fts, 5, '<b>', '</b>', '...', 40) as snippet,
                    bm25(documents_fts) as rank
                FROM documents_fts
                JOIN documents_meta d ON documents_fts.id = d.id
                WHERE documents_fts MATCH ?
            """

            params = [fts_query]

            # Add filters
            if filters.get('logic_class'):
                sql += " AND d.logic_class = ?"
                params.append(filters['logic_class'])

            if filters.get('format'):
                sql += " AND d.format = ?"
                params.append(filters['format'])

            sql += " ORDER BY rank LIMIT ?"
            params.append(limit)

            cursor.execute(sql, params)
            rows = cursor.fetchall()

            # Format results
            results = []
            for row in rows:
                result = {
                    'doc_id': row[0],
                    'score': float(row[5]),
                    'snippet': row[4],
                    'metadata': {
                        'logic_class': row[1] if row[1] else "",
                        'format': row[3] if row[3] else "",
                        'original_uri': row[2] if row[2] else ""
                    }
                }
                results.append(result)

            # Build response
            execution_time = (time.time() - start_time) * 1000

            response = {
                'status': 'success',
                'query': query,
                'mode': mode,
                'sensitivity': sensitivity,
                'total_results': len(results),
                'execution_time_ms': round(execution_time, 2),
                'results': results
            }

        except sqlite3.Error as e:
            execution_time = (time.time() - start_time) * 1000
            response = {
                'status': 'error',
                'query': query,
                'results': [],
                'total_results': 0,
                'execution_time_ms': round(execution_time, 2),
                'error': {
                    'code': 'DATABASE_ERROR',
                    'message': str(e)
                }
            }

        # Validate output
        self.validate_response(response)
        return response

    def close(self):
        """Close database connection"""
        if self.conn:
            self.conn.close()


def main():
    """CLI interface for SSOT search API"""
    import argparse

    parser = argparse.ArgumentParser(
        description='SSOT Search API - Strict JSON I/O for agents',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Read from stdin
  echo '{"query":"test","mode":"full_text"}' | python search_api.py

  # From file
  python search_api.py < request.json

  # Direct parameters
  python search_api.py --query "test query" --mode full_text --limit 10
        """
    )

    parser.add_argument('--query', help='Search query string')
    parser.add_argument('--mode', choices=['keyword', 'full_text'], default='full_text')
    parser.add_argument('--sensitivity', type=int, choices=[0, 1, 2, 3], default=1)
    parser.add_argument('--limit', type=int, default=20)
    parser.add_argument('--json', action='store_true', help='Read JSON from stdin')

    args = parser.parse_args()

    # Build request
    if args.json or not args.query:
        # Read JSON from stdin
        try:
            request = json.load(sys.stdin)
        except json.JSONDecodeError as e:
            error_response = {
                'status': 'error',
                'query': '',
                'results': [],
                'total_results': 0,
                'execution_time_ms': 0,
                'error': {
                    'code': 'INVALID_JSON',
                    'message': f'Invalid JSON input: {str(e)}'
                }
            }
            print(json.dumps(error_response, indent=2))
            return 1
    else:
        # Build from arguments
        request = {
            'query': args.query,
            'mode': args.mode,
            'sensitivity': args.sensitivity,
            'limit': args.limit
        }

    # Execute search
    try:
        api = SSOTSearchAPI()
        response = api.search(request)
        api.close()

        # Output strict JSON
        print(json.dumps(response, indent=2))
        return 0 if response['status'] == 'success' else 1

    except Exception as e:
        error_response = {
            'status': 'error',
            'query': request.get('query', ''),
            'results': [],
            'total_results': 0,
            'execution_time_ms': 0,
            'error': {
                'code': 'INTERNAL_ERROR',
                'message': str(e)
            }
        }
        print(json.dumps(error_response, indent=2))
        return 1


if __name__ == '__main__':
    sys.exit(main())
