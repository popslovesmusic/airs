from __future__ import annotations

from dataclasses import dataclass
from typing import List, Union


@dataclass(frozen=True)
class Atom:
    name: str


@dataclass(frozen=True)
class OpExpr:
    op: str
    args: List["Expr"]


Expr = Union[Atom, OpExpr]
