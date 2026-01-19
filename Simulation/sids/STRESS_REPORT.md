# Stress Test Report

## Summary
Ran randomized validate → rewrite → re‑validate cycles on generated SID packages.

## Command
```
python sid_stress_cli.py --runs 200 --seed 1234 --report STRESS_REPORT.json
```

## Result
- Runs: 200
- Passes: 200
- Failures: 0

## Notes
The JSON report is stored in `STRESS_REPORT.json`.
