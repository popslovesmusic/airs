# Engine Outputs for Validation Comparison

Optional: drop engine-produced metrics here so the validation harness can compare them against reference expectations. Use one JSON file per problem id:

- `diffusion_1d_fixed.json`: `{"mass_start": <float>, "mass_end": <float>}`
- `random_walk_to_diffusion.json`: `{"variance_slope": <float>}`
- `graph_flow_conservation.json`: `{"max_flow": <float>, "max_imbalance": <float>}`
- `lorenz_invariant_measure.json`: `{"bounded": <bool>, "max_radius": <float>}`

Notes:
- File names must match the `problem_id` with `.json` extension for the harness to pick them up.
- Keep values as numbers/bools; add other fields if useful (they will be included in observations but not compared unless checked in code).
- If no engine output is present, the harness uses only the Python reference and will not fail on missing engine data.
