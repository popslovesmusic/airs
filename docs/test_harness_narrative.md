# How We Built the Test Harness (Beginner Friendly)

1) **Picked the tools** – We use GoogleTest for assertions and CTest as the runner so everything is one ctest command away.
2) **Built tiny "step runners"** – Two small C++ programs feed canned JSONL commands into the engine CLIs and grab their stdout.
3) **Made it stable** – The runners now call the CLIs directly with CreateProcessW (no PowerShell). We strip out changing bits like timestamps and engine_ids before hashing so the results stay the same every run.
4) **Captured goldens** – For each engine family we ran the runners once and saved the repeatable hash in rtifacts/validation/<engine>/out.json.
5) **Hooked up the tests** – Each harness test runs its runner, compares the hash to the golden, and writes a small metrics JSON under metrics/<engine>/....
6) **Fail fast** – If a runner or CLI is missing, returns an error, or the hash is different, the test fails immediately.

## Running the suite
`powershell
cmake --build build --config Debug --target harness_tests
ctest -C Debug -L harness --output-on-failure
`

## Where things live
- Runners: uild/Debug/dase_step_runner.exe, uild/Debug/sid_step_runner.exe
- Fixtures: Simulation/tests/fixtures/inputs/*.jsonl
- Goldens: rtifacts/validation/<engine>/out.json
- Metrics: metrics/<engine>/...json

Feel free to open any of those files to see the flow. If you add a new engine, copy an existing fixture + harness test, run the runner once to record a new hash, and rerun the suite.
