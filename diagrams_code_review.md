DIAGRAMS CODE REVIEW

Scope
- Review target: D:\airs\diagrams
- Integration context: D:\airs\Simulation is the canonical project; diagrams tooling should produce compatible SID package artifacts.

Findings (ordered by severity)

1) sid_ast_cli import error prevents CLI execution
- File: D:\airs\diagrams\sid_ast_cli.py:8
- Issue: `expr_to_dict` is imported from `sid_parser`, but it is defined in `sid_ast_utils`. Running the CLI fails at import time, blocking AST/diagram export for integration with Simulation workflows.
- Impact: The CLI cannot generate AST output, which breaks any pipeline that relies on it to feed diagram JSON into other tools.

2) Rewrite replacement creates nodes with inputs but no edges
- File: D:\airs\diagrams\sid_rewrite.py:218-239
- Issue: `ensure_node` creates wrapper nodes with `inputs` set but does not add corresponding edges. Downstream tooling uses edges for CSI validation and pattern matching, so the graph becomes inconsistent.
- Impact: Rewrites can produce diagrams that appear valid by `inputs` but are missing edges, reducing accuracy of further rewrites and validations.

3) Expression matching allows extra node inputs
- File: D:\airs\diagrams\sid_rewrite.py:341-350
- Issue: `match_expr` accepts nodes whose input count exceeds the pattern (only checks `len(input_ids) < len(expr.args)`). This can match nodes with extra inputs and then rewrite them, potentially dropping those inputs.
- Impact: Over-permissive rewrites can discard data and create incorrect diagrams.

4) Iterative rewrite loop is only a single pass
- File: D:\airs\diagrams\sid_rewrite.py:548-602
- Issue: The function claims to apply rewrites iteratively, but it only walks each rule once. `max_iterations` currently caps rules, not iterations over the diagram until convergence.
- Impact: Partial rewrites may leave the diagram in a non-normalized state compared to the intended behavior.

5) Stress test uses non-standard operator tokens
- File: D:\airs\diagrams\sid_stress_cli.py:12
- Issue: OPS includes `S_PLUS` and `S_MINUS`, which are not recognized elsewhere (parser expects `S+` / `S-`).
- Impact: Stress tests exercise invalid operators, reducing confidence in test coverage for real-world inputs.

Integration Notes
- Ensure all CLI tools produce JSON that matches the operator set used by the parser and any Simulation-side consumers.
- Rewrites should preserve edge structure so diagram validation and CSI enforcement in integrated pipelines remain accurate.

End of report.
