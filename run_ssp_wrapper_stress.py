import json
import subprocess
import uuid
from pathlib import Path

# Paths
BASE = Path(__file__).parent
DASE = BASE / "Simulation" / "bin" / "dase_cli.exe"


def run_session(cmds):
    payload = "\n".join(json.dumps(c) for c in cmds) + "\n"
    proc = subprocess.run([str(DASE)], input=payload, text=True, capture_output=True)
    responses = [json.loads(line) for line in proc.stdout.splitlines() if line.strip()]
    return responses, proc


def summarize(responses):
    motions_applied = 0
    motions_refused = 0
    invariants = []
    for r in responses:
        if r.get("command") == "sid_wrapper_metrics" and r.get("status") == "success":
            res = r["result"]
            last = res.get("last_motion", {})
            if last.get("applied"):
                motions_applied += 1
            elif last:
                motions_refused += 1
            invariants.append(
                {
                    "I": res.get("I_mass"),
                    "N": res.get("N_mass"),
                    "U": res.get("U_mass"),
                    "is_conserved": res.get("is_conserved"),
                    "last_motion": last,
                }
            )
    return motions_applied, motions_refused, invariants


def main():
    cmds = [
        {"command": "create_engine", "params": {"engine_type": "sid_ssp", "num_nodes": 8, "capacity": 1.0, "alpha": 0.1}},
    ]
    # Motion exhaustion: six semantic_motion commits with epsilon=0.05
    for _ in range(6):
        cmds.append(
            {
                "command": "run_mission",
                "params": {
                    "engine_id": "engine_001",
                    "num_steps": 1,
                    "motion_metadata": {"mode": "semantic_motion", "epsilon": 0.05},
                    "auto_apply_wrapper_motion": True,
                },
            }
        )
        cmds.append({"command": "sid_wrapper_metrics", "params": {"engine_id": "engine_001"}})
    # Symbolic-only commit
    cmds.append(
        {
            "command": "run_mission",
            "params": {
                "engine_id": "engine_001",
                "num_steps": 1,
                "motion_metadata": {"mode": "symbolic_only"},
                "auto_apply_wrapper_motion": True,
            },
        }
    )
    cmds.append({"command": "sid_wrapper_metrics", "params": {"engine_id": "engine_001"}})
    # Adversarial epsilons
    for eps in [0.0, -0.1, 1.5]:
        cmds.append(
            {
                "command": "run_mission",
                "params": {
                    "engine_id": "engine_001",
                    "num_steps": 1,
                    "motion_metadata": {"mode": "semantic_motion", "epsilon": eps},
                    "auto_apply_wrapper_motion": True,
                },
            }
        )
        cmds.append({"command": "sid_wrapper_metrics", "params": {"engine_id": "engine_001"}})
    cmds.append({"command": "destroy_engine", "params": {"engine_id": "engine_001"}})

    responses, proc = run_session(cmds)
    Path("ssp_wrapper_session_out.jsonl").write_text("\n".join(json.dumps(r) for r in responses))

    motions_applied, motions_refused, invariants = summarize(responses)
    ts = str(uuid.uuid4())

    Path("ssp_wrapper_stress_results.json").write_text(
        json.dumps(
            {
                "timestamp": ts,
                "results": [
                    {"track": "session", "motions_applied": motions_applied, "motions_refused": motions_refused}
                ],
            },
            indent=2,
        )
    )
    Path("ssp_wrapper_invariant_report.json").write_text(
        json.dumps({"timestamp": ts, "invariants": invariants}, indent=2)
    )
    Path("ssp_wrapper_event_counters.json").write_text(
        json.dumps(
            {
                "timestamp": ts,
                "events": [{"track": "session", "motions": motions_applied, "refused": motions_refused}],
            },
            indent=2,
        )
    )

    print(f"Session complete (returncode={proc.returncode})")
    print(f"motions_applied={motions_applied}, motions_refused={motions_refused}")


if __name__ == "__main__":
    main()
