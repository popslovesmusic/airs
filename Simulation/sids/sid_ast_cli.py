from __future__ import annotations

import argparse
import json
import sys

from sid_ast_to_diagram import expr_to_diagram
from sid_parser import parse_expression, expr_to_dict


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description="Parse SID expressions into AST or diagram JSON."
    )
    parser.add_argument("expression", help="SID expression, e.g., P(Freedom)")
    parser.add_argument(
        "--mode",
        choices=["ast", "diagram"],
        default="diagram",
        help="Output AST or diagram JSON",
    )
    parser.add_argument("--diagram-id", default="d_expr", help="Diagram id")
    parser.add_argument("--compartment-id", help="Optional compartment id")
    args = parser.parse_args(argv)

    expr = parse_expression(args.expression)
    if args.mode == "ast":
        payload = expr_to_dict(expr)
    else:
        payload = expr_to_diagram(
            expr, diagram_id=args.diagram_id, compartment_id=args.compartment_id
        )
    print(json.dumps(payload, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
