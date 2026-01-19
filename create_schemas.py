#!/usr/bin/env python3
"""
Create JSON schema files for AIRS components
Ensures strict JSON contracts for agent communication
"""

import json
from pathlib import Path

def create_ssot_schemas(root):
    """Create SSOT API JSON schemas"""
    schemas_dir = root / "ssot" / "api" / "schemas"

    # Search Request Schema
    search_request = {
        "$schema": "http://json-schema.org/draft-07/schema#",
        "$id": "https://airs.local/schemas/ssot/search_request.json",
        "title": "SSOT Search Request",
        "description": "Request schema for SSOT search API",
        "type": "object",
        "required": ["query", "mode"],
        "properties": {
            "query": {
                "type": "string",
                "description": "Search query string",
                "minLength": 1,
                "maxLength": 1000
            },
            "mode": {
                "type": "string",
                "enum": ["keyword", "full_text"],
                "description": "Search mode: keyword (exact/AND) or full_text (BM25)"
            },
            "sensitivity": {
                "type": "integer",
                "minimum": 0,
                "maximum": 3,
                "default": 1,
                "description": "Search sensitivity (0=STRICT, 1=NORMAL, 2=BROAD, 3=DIAGNOSTIC)"
            },
            "limit": {
                "type": "integer",
                "minimum": 1,
                "maximum": 500,
                "default": 20,
                "description": "Maximum results to return"
            },
            "filters": {
                "type": "object",
                "properties": {
                    "logic_class": {
                        "type": "string",
                        "description": "Filter by logic class"
                    },
                    "format": {
                        "type": "string",
                        "description": "Filter by document format"
                    }
                },
                "additionalProperties": False
            }
        },
        "additionalProperties": False
    }

    # Search Response Schema
    search_response = {
        "$schema": "http://json-schema.org/draft-07/schema#",
        "$id": "https://airs.local/schemas/ssot/search_response.json",
        "title": "SSOT Search Response",
        "description": "Response schema for SSOT search API",
        "type": "object",
        "required": ["status", "query", "results"],
        "properties": {
            "status": {
                "type": "string",
                "enum": ["success", "error"],
                "description": "Request status"
            },
            "query": {
                "type": "string",
                "description": "Original query string"
            },
            "mode": {
                "type": "string",
                "enum": ["keyword", "full_text"]
            },
            "sensitivity": {
                "type": "integer",
                "minimum": 0,
                "maximum": 3
            },
            "total_results": {
                "type": "integer",
                "minimum": 0,
                "description": "Total number of results found"
            },
            "execution_time_ms": {
                "type": "number",
                "minimum": 0,
                "description": "Query execution time in milliseconds"
            },
            "results": {
                "type": "array",
                "items": {
                    "$ref": "#/definitions/search_result"
                }
            },
            "error": {
                "type": "object",
                "properties": {
                    "code": {"type": "string"},
                    "message": {"type": "string"}
                }
            }
        },
        "definitions": {
            "search_result": {
                "type": "object",
                "required": ["doc_id", "score"],
                "properties": {
                    "doc_id": {
                        "type": "string",
                        "description": "Document ID (hash)"
                    },
                    "score": {
                        "type": "number",
                        "description": "Relevance score (BM25 or match score)"
                    },
                    "snippet": {
                        "type": "string",
                        "description": "Snippet with highlighted matches"
                    },
                    "metadata": {
                        "type": "object",
                        "properties": {
                            "logic_class": {"type": "string"},
                            "format": {"type": "string"},
                            "original_uri": {"type": "string"}
                        }
                    }
                }
            }
        },
        "additionalProperties": False
    }

    # Document Schema
    document_schema = {
        "$schema": "http://json-schema.org/draft-07/schema#",
        "$id": "https://airs.local/schemas/ssot/document.json",
        "title": "SSOT Document",
        "description": "Schema for SSOT document objects",
        "type": "object",
        "required": ["doc_id", "content"],
        "properties": {
            "doc_id": {
                "type": "string",
                "description": "Document ID (hash)"
            },
            "content": {
                "type": "string",
                "description": "Full document content"
            },
            "metadata": {
                "type": "object",
                "properties": {
                    "logic_class": {"type": "string"},
                    "format": {"type": "string"},
                    "original_uri": {"type": "string"},
                    "created_at": {"type": "string", "format": "date-time"}
                }
            }
        },
        "additionalProperties": False
    }

    # Write schemas
    (schemas_dir / "search_request.schema.json").write_text(
        json.dumps(search_request, indent=2), encoding='utf-8'
    )
    (schemas_dir / "search_response.schema.json").write_text(
        json.dumps(search_response, indent=2), encoding='utf-8'
    )
    (schemas_dir / "document.schema.json").write_text(
        json.dumps(document_schema, indent=2), encoding='utf-8'
    )

    print("[OK] Created SSOT API schemas")

def create_simulation_schemas(root):
    """Create Simulation CLI JSON schemas"""
    schemas_dir = root / "sim" / "cli" / "schemas"

    # Command Schema
    command_schema = {
        "$schema": "http://json-schema.org/draft-07/schema#",
        "$id": "https://airs.local/schemas/sim/command.json",
        "title": "Simulation Command",
        "description": "Command schema for simulation CLI",
        "type": "object",
        "required": ["command"],
        "properties": {
            "command": {
                "type": "string",
                "enum": [
                    "list_engines",
                    "describe_engine",
                    "create_engine",
                    "evolve",
                    "get_state",
                    "snapshot",
                    "analyze",
                    "reset"
                ],
                "description": "Command to execute"
            },
            "params": {
                "type": "object",
                "description": "Command-specific parameters",
                "additionalProperties": True
            }
        },
        "additionalProperties": False
    }

    # Response Schema
    response_schema = {
        "$schema": "http://json-schema.org/draft-07/schema#",
        "$id": "https://airs.local/schemas/sim/response.json",
        "title": "Simulation Response",
        "description": "Response schema for simulation CLI",
        "type": "object",
        "required": ["command", "status"],
        "properties": {
            "command": {
                "type": "string",
                "description": "Command that was executed"
            },
            "status": {
                "type": "string",
                "enum": ["success", "error"],
                "description": "Execution status"
            },
            "execution_time_ms": {
                "type": "number",
                "minimum": 0,
                "description": "Command execution time in milliseconds"
            },
            "result": {
                "type": "object",
                "description": "Command-specific result",
                "additionalProperties": True
            },
            "error": {
                "type": "object",
                "properties": {
                    "code": {"type": "string"},
                    "message": {"type": "string"},
                    "details": {"type": "object"}
                }
            }
        },
        "additionalProperties": False
    }

    # State Export Schema
    state_schema = {
        "$schema": "http://json-schema.org/draft-07/schema#",
        "$id": "https://airs.local/schemas/sim/state.json",
        "title": "Simulation State",
        "description": "Schema for exported simulation state",
        "type": "object",
        "required": ["engine_type", "timestamp", "state"],
        "properties": {
            "engine_type": {
                "type": "string",
                "description": "Engine type (e.g., igsoa_complex)"
            },
            "timestamp": {
                "type": "number",
                "description": "Simulation timestamp"
            },
            "timestep": {
                "type": "integer",
                "description": "Timestep number"
            },
            "state": {
                "type": "object",
                "properties": {
                    "nodes": {
                        "type": "array",
                        "items": {
                            "type": "object",
                            "properties": {
                                "id": {"type": "integer"},
                                "phi_real": {"type": "number"},
                                "phi_imag": {"type": "number"},
                                "position": {
                                    "type": "array",
                                    "items": {"type": "number"}
                                }
                            }
                        }
                    },
                    "energy": {"type": "number"},
                    "mass": {"type": "number"}
                },
                "additionalProperties": True
            },
            "metadata": {
                "type": "object",
                "properties": {
                    "num_nodes": {"type": "integer"},
                    "R_c": {"type": "number"},
                    "parameters": {"type": "object"}
                }
            }
        },
        "additionalProperties": False
    }

    # Write schemas
    (schemas_dir / "command.schema.json").write_text(
        json.dumps(command_schema, indent=2), encoding='utf-8'
    )
    (schemas_dir / "response.schema.json").write_text(
        json.dumps(response_schema, indent=2), encoding='utf-8'
    )
    (schemas_dir / "state.schema.json").write_text(
        json.dumps(state_schema, indent=2), encoding='utf-8'
    )

    print("[OK] Created Simulation CLI schemas")

def create_integration_schemas(root):
    """Create integration workflow schemas"""
    schemas_dir = root / "integration" / "schemas"

    # Pipeline Schema
    pipeline_schema = {
        "$schema": "http://json-schema.org/draft-07/schema#",
        "$id": "https://airs.local/schemas/integration/pipeline.json",
        "title": "Integration Pipeline",
        "description": "Schema for cross-component workflow pipelines",
        "type": "object",
        "required": ["pipeline_id", "steps"],
        "properties": {
            "pipeline_id": {
                "type": "string",
                "description": "Unique pipeline identifier"
            },
            "description": {
                "type": "string",
                "description": "Pipeline description"
            },
            "steps": {
                "type": "array",
                "items": {
                    "$ref": "#/definitions/pipeline_step"
                },
                "minItems": 1
            }
        },
        "definitions": {
            "pipeline_step": {
                "type": "object",
                "required": ["step_id", "component", "action"],
                "properties": {
                    "step_id": {
                        "type": "string",
                        "description": "Step identifier"
                    },
                    "component": {
                        "type": "string",
                        "enum": ["ssot", "simulation"],
                        "description": "Component to execute"
                    },
                    "action": {
                        "type": "string",
                        "description": "Action to perform"
                    },
                    "input": {
                        "type": "object",
                        "description": "Input data or reference to previous step"
                    },
                    "output_path": {
                        "type": "string",
                        "description": "Where to save output"
                    }
                }
            }
        }
    }

    # Write schema
    (schemas_dir / "pipeline.schema.json").write_text(
        json.dumps(pipeline_schema, indent=2), encoding='utf-8'
    )

    print("[OK] Created Integration schemas")

def main():
    root = Path("D:/airs")

    print("Creating JSON schemas for AIRS components...")
    print("=" * 60)

    create_ssot_schemas(root)
    create_simulation_schemas(root)
    create_integration_schemas(root)

    print("=" * 60)
    print("All schemas created successfully!")
    print()
    print("Schemas created:")
    print("  - ssot/api/schemas/search_request.schema.json")
    print("  - ssot/api/schemas/search_response.schema.json")
    print("  - ssot/api/schemas/document.schema.json")
    print("  - sim/cli/schemas/command.schema.json")
    print("  - sim/cli/schemas/response.schema.json")
    print("  - sim/cli/schemas/state.schema.json")
    print("  - integration/schemas/pipeline.schema.json")

if __name__ == "__main__":
    main()
