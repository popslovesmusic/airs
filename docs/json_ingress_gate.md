# AIRS JSON Ingress Gate (Validator + Minifier)

This is the single entry point for any JSON command sent to `dase_cli.exe` (or future CLIs). It is intentionally small and strict: it **only** checks framing and produces a one‑line JSONL command; it does not guess or fix semantics.

## What it enforces
- Exactly one JSON value
- Top‑level must be an object
- `command` key must exist and be a non‑empty string
- No trailing junk after the JSON value
- One line only (JSONL framing)

If any of these fail, the gate exits non‑zero and prints a clear error. Nothing downstream should run.

## How to use
```bash
# Gate (validate then minify) stdin -> stdout
Get-Content cmd.json -Raw | python json_gate.py --mode gate > cmd.jsonl

# Validate an existing JSONL file
python json_gate.py --mode validate -i cmd.jsonl

# Minify pretty JSON into JSONL form
python json_gate.py --mode minify -i pretty.json -o cmd.jsonl
```

Exit codes: `0` on success, `1` on validation failure, `2` on I/O error.

## Why this exists
- Establishes a single ingress boundary (“AIRS Ingress Gate”)
- Prevents malformed JSONL from ever reaching the harness/engines
- Creates deterministic, auditable errors
- Keeps minification mechanical (no semantic changes)

## File locations
- Gate script: `json_gate.py`
- Original minify helper (for reference): `minify_json.py`
- Design notes: `jsonfix.txt`, `json_commands_guide.md`
