import argparse
import sqlite3
import sys

def search_documents(query, db_file="ssot_parallel.db", limit=5, show_full=False):
    try:
        conn = sqlite3.connect(db_file)
        cursor = conn.cursor()

        if show_full:
            sql = "SELECT id, content FROM documents_fts WHERE documents_fts MATCH ? LIMIT ?;"
        else:
            # Snippets avoid pulling huge blobs of text into Python and speed up printing.
            sql = (
                "SELECT id, "
                "snippet(documents_fts, 5, '[', ']', '…', 20) AS snippet, "
                "bm25(documents_fts) AS score "
                "FROM documents_fts "
                "WHERE documents_fts MATCH ? "
                "ORDER BY score "
                "LIMIT ?;"
            )

        cursor.execute(sql, (query, limit))
        results = cursor.fetchall()

        if not results:
            print("No documents found for your query.")
            return

        print(f"Found {len(results)} documents (limited to {limit}):")
        for res in results:
            doc_id = res[0]
            print("-" * 20)
            print(f"Document ID: {doc_id}")
            if show_full:
                print(res[1])
            else:
                print(res[1])

        conn.close()

    except sqlite3.Error as e:
        print(f"Error searching database: {e}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Search SSOT FTS database.")
    parser.add_argument("query", nargs="+", help="FTS query string")
    parser.add_argument("--db", default="ssot_parallel.db", help="Path to SQLite DB")
    parser.add_argument("--limit", type=int, default=5, help="Max number of results")
    parser.add_argument(
        "--full",
        action="store_true",
        help="Print full content (slower, more output)",
    )
    args = parser.parse_args()

    search_query = " ".join(args.query)
    search_documents(search_query, db_file=args.db, limit=args.limit, show_full=args.full)
