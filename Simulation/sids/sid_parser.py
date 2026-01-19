from __future__ import annotations

import re
from dataclasses import dataclass
from typing import List, Sequence

from sid_ast import Atom, Expr, OpExpr


OPERATORS = {"P", "S+", "S-", "O", "C", "T"}

# Operator arity constraints: (min_args, max_args) or None for any
OPERATOR_ARITY = {
    "P": (1, 1),      # P(DOF) - exactly 1 argument
    "S+": (1, None),  # S+(DOF, ...) - 1 or more
    "S-": (1, None),  # S-(DOF, ...) - 1 or more
    "O": (1, 1),      # O(expr) - exactly 1 argument
    "C": (2, 2),      # C(expr1, expr2) - exactly 2 arguments
    "T": (1, 1),      # T(expr) - exactly 1 argument
}


@dataclass(frozen=True)
class Token:
    kind: str
    value: str
    pos: int
    line: int = 1
    column: int = 1


TOKEN_RE = re.compile(
    r"""
    (?P<ws>\s+)
  | (?P<op>S\+|S-|P|O|C|T)
  | (?P<ident>[$]?[A-Za-z_][A-Za-z0-9_]*)  # $ prefix for variables (rewrite patterns)
                                            # Note: Unicode identifiers not supported (limitation)
                                            # To add: use \w instead of [A-Za-z0-9_]
  | (?P<lparen>\()
  | (?P<rparen>\))
  | (?P<comma>,)
  """,
    re.VERBOSE,
)


class ParseError(ValueError):
    pass


def tokenize(text: str) -> List[Token]:
    """Tokenize input text with line/column tracking."""
    tokens: List[Token] = []
    idx = 0
    line = 1
    column = 1

    while idx < len(text):
        match = TOKEN_RE.match(text, idx)
        if not match:
            raise ParseError(
                f"Unexpected character at line {line}, column {column}: {text[idx]!r}"
            )
        kind = match.lastgroup
        value = match.group(kind)

        if kind != "ws":
            tokens.append(Token(kind=kind, value=value, pos=idx, line=line, column=column))

        # Update line/column tracking
        for char in value:
            if char == '\n':
                line += 1
                column = 1
            else:
                column += 1

        idx = match.end()
    return tokens


class Parser:
    def __init__(self, tokens: Sequence[Token]) -> None:
        self.tokens = list(tokens)
        self.pos = 0

    def current(self) -> Token | None:
        if self.pos >= len(self.tokens):
            return None
        return self.tokens[self.pos]

    def consume(self, expected_kind: str | None = None) -> Token:
        token = self.current()
        if token is None:
            raise ParseError("Unexpected end of input")
        if expected_kind and token.kind != expected_kind:
            raise ParseError(
                f"Expected {expected_kind} at {token.pos}, got {token.kind}"
            )
        self.pos += 1
        return token

    def parse_expr(self) -> Expr:
        token = self.current()
        if token is None:
            raise ParseError("Empty expression")
        if token.kind == "op":
            # Validate operator before consuming
            if token.value not in OPERATORS:
                raise ParseError(
                    f"Unknown operator {token.value!r} at line {token.line}, column {token.column}"
                )
            op = self.consume("op").value
            if self.current() and self.current().kind == "lparen":
                self.consume("lparen")
                args = self.parse_expr_list()
                self.consume("rparen")
                # Validate arity
                self._validate_arity(op, args, token.pos)
                return OpExpr(op=op, args=args)
            # No parentheses - zero arguments
            self._validate_arity(op, [], token.pos)
            return OpExpr(op=op, args=[])
        if token.kind == "ident":
            # Bare atom (e.g., "Freedom") - interpreted as shorthand for P(Freedom)
            # This is converted to proper P operator form during diagram construction
            return Atom(name=self.consume("ident").value)
        raise ParseError(f"Unexpected token {token.kind} at {token.pos}")

    def _validate_arity(self, op: str, args: List[Expr], pos: int) -> None:
        """Validate operator arity constraints."""
        if op not in OPERATOR_ARITY:
            return  # No arity constraint defined

        min_args, max_args = OPERATOR_ARITY[op]
        num_args = len(args)

        if num_args < min_args:
            raise ParseError(
                f"Operator {op!r} requires at least {min_args} argument(s), "
                f"got {num_args} at position {pos}"
            )

        if max_args is not None and num_args > max_args:
            raise ParseError(
                f"Operator {op!r} accepts at most {max_args} argument(s), "
                f"got {num_args} at position {pos}"
            )

    def parse_expr_list(self) -> List[Expr]:
        exprs = [self.parse_expr()]
        while self.current() and self.current().kind == "comma":
            self.consume("comma")
            exprs.append(self.parse_expr())
        return exprs


def parse_expression(text: str) -> Expr:
    tokens = tokenize(text)
    parser = Parser(tokens)
    expr = parser.parse_expr()
    if parser.current() is not None:
        token = parser.current()
        raise ParseError(f"Unexpected trailing token {token.value!r} at {token.pos}")
    return expr


if __name__ == "__main__":
    import json
    import sys
    from sid_ast_utils import expr_to_dict

    if len(sys.argv) != 2:
        raise SystemExit("Usage: python sid_parser.py \"P(Freedom)\"")
    parsed = parse_expression(sys.argv[1])
    print(json.dumps(expr_to_dict(parsed), indent=2, sort_keys=True))
