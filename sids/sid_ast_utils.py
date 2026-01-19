"""
AST utility functions (addresses Minor Issue #2).
Separates AST manipulation from parsing logic.
"""
from __future__ import annotations

from sid_ast import Atom, Expr, OpExpr


def expr_to_dict(expr: Expr) -> dict:
    """Convert AST expression to dictionary representation."""
    if isinstance(expr, Atom):
        return {"type": "atom", "name": expr.name}
    return {"type": "op", "op": expr.op, "args": [expr_to_dict(a) for a in expr.args]}


def dict_to_expr(data: dict) -> Expr:
    """Convert dictionary representation back to AST expression."""
    if data.get("type") == "atom":
        return Atom(name=data["name"])
    elif data.get("type") == "op":
        args = [dict_to_expr(a) for a in data.get("args", [])]
        return OpExpr(op=data["op"], args=args)
    else:
        raise ValueError(f"Unknown expression type: {data.get('type')}")
