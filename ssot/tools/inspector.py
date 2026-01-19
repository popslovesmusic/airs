import sqlite3

def inspect_database(db_file):
    try:
        conn = sqlite3.connect(db_file)
        cursor = conn.cursor()
        print(f"Inspecting {db_file}...")
        
        cursor.execute("SELECT name FROM sqlite_master WHERE type='table';")
        tables = cursor.fetchall()
        
        for table in tables:
            table_name = table[0]
            print(f"\nSchema of {table_name}:")
            cursor.execute(f"PRAGMA table_info({table_name});")
            columns = cursor.fetchall()
            column_names = [col[1] for col in columns]
            print(column_names)

        conn.close()
    except sqlite3.Error as e:
        print(f"Error inspecting database {db_file}: {e}")

if __name__ == "__main__":
    inspect_database("ssot_parallel.db")