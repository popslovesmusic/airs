import argparse
import json
import math
import random
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, List, Tuple

ROOT = Path(__file__).resolve().parents[1]
CATALOG_PATH = ROOT / "validation" / "catalog.json"
SCHEMA_PATH = ROOT / "validation" / "validation.schema.json"
ARTIFACTS_DIR = ROOT / "artifacts" / "validation"
ENGINE_OUTPUTS_DIR = ROOT / "validation" / "engine_outputs"


def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat()


def load_catalog() -> List[Dict[str, Any]]:
    with CATALOG_PATH.open("r", encoding="utf-8") as f:
        data = json.load(f)
    return data.get("scenarios", [])


def write_result(result: Dict[str, Any]) -> Path:
    ARTIFACTS_DIR.mkdir(parents=True, exist_ok=True)
    fname = f"{result['problem_id']}_{result['timestamp_utc'].replace(':', '').replace('-', '')}.json"
    path = ARTIFACTS_DIR / fname
    with path.open("w", encoding="utf-8") as f:
        json.dump(result, f, indent=2)
    return path


def stub_result(problem_id: str, known_invariants: List[str]) -> Dict[str, Any]:
    # Placeholder runner: logs that implementation is pending.
    return {
        "problem_id": problem_id,
        "timestamp_utc": utc_now(),
        "runner": "validation/run_validation.py",
        "known_invariants": known_invariants,
        "expected_behavior": {},
        "run_config": {},
        "observations": {"note": "runner not implemented"},
        "verdict": "not_implemented",
        "notes": "Fill in scenario-specific runner to compute metrics and verdict."
    }


def load_engine_observation(problem_id: str) -> Dict[str, Any]:
    """Load optional engine-produced metrics from validation/engine_outputs/<problem_id>.json."""
    path = ENGINE_OUTPUTS_DIR / f"{problem_id}.json"
    if not path.exists():
        return {}
    try:
        with path.open("r", encoding="utf-8") as f:
            return json.load(f)
    except Exception as exc:
        print(f"[WARN] failed to load engine output for {problem_id}: {exc}", file=sys.stderr)
        return {}


def run_diffusion_1d() -> Dict[str, Any]:
    # Reference explicit FTCS diffusion on [0, 1] with fixed boundaries.
    n = 101
    dx = 1.0 / (n - 1)
    alpha = 0.1  # diffusion coefficient
    dt = 0.25 * dx * dx / alpha  # stability criterion
    steps = 400

    u = [0.0] * n
    u[n // 2] = 1.0  # initial spike at center

    def step(u_prev: List[float]) -> List[float]:
        u_next = u_prev.copy()
        coeff = alpha * dt / (dx * dx)
        for i in range(1, n - 1):
            u_next[i] = u_prev[i] + coeff * (u_prev[i - 1] - 2 * u_prev[i] + u_prev[i + 1])
        u_next[0] = 0.0
        u_next[-1] = 0.0
        return u_next

    mass0 = sum(u) * dx
    for _ in range(steps):
        u = step(u)
    mass1 = sum(u) * dx
    drift_rate = abs(mass1 - mass0) / max(1, steps)

    # Reference expectation: smooth bell curve; we approximate error vs. itself (no engine yet)
    l1_error = 0.0

    verdict = "passes_within_tolerance" if drift_rate < 1e-5 else "fail"

    engine_obs = load_engine_observation("diffusion_1d_fixed")
    engine_verdict = None
    if engine_obs:
        # Prefer normalized metrics if provided.
        emass_start = engine_obs.get("mass_start_norm", engine_obs.get("mass_start", mass0))
        emass_end = engine_obs.get("mass_end_norm", engine_obs.get("mass_end", engine_obs.get("mass", mass1)))
        edrift = abs(emass_end - emass_start)
        drift_tol = 1e-3  # allow small numerical differences for normalized values
        engine_ok = edrift < drift_tol
        engine_verdict = "passes_within_tolerance" if engine_ok else "fail"
        verdict = engine_verdict if engine_verdict == "fail" else verdict

    return {
        "problem_id": "diffusion_1d_fixed",
        "timestamp_utc": utc_now(),
        "runner": "validation/run_validation.py",
        "known_invariants": ["mass_conservation"],
        "expected_behavior": {
            "long_term": "smooths to zero with fixed boundaries",
            "mass_conservation": True,
            "error_scaling": "O(dx^2) expected for FTCS"
        },
        "run_config": {"n": n, "dx": dx, "dt": dt, "steps": steps, "alpha": alpha},
        "observations": {
            "mass_start": mass0,
            "mass_end": mass1,
            "drift_rate": drift_rate,
            "l1_error": l1_error,
            "engine": engine_obs or None
        },
        "verdict": verdict,
        "notes": "Pure reference FTCS; integrate engine output when available."
    }


def run_random_walk() -> Dict[str, Any]:
    random.seed(42)
    walkers = 5000
    steps = 200
    positions = [0] * walkers
    variances: List[float] = []

    for t in range(1, steps + 1):
        for i in range(walkers):
            positions[i] += 1 if random.random() < 0.5 else -1
        mean = sum(positions) / walkers
        var = sum((p - mean) ** 2 for p in positions) / walkers
        variances.append(var)

    # Fit variance ~ slope * t
    xs = list(range(1, steps + 1))
    x_mean = sum(xs) / steps
    y_mean = sum(variances) / steps
    num = sum((x - x_mean) * (y - y_mean) for x, y in zip(xs, variances))
    den = sum((x - x_mean) ** 2 for x in xs)
    slope = num / den if den != 0 else 0.0

    expected_slope = 1.0  # unit step variance for unbiased walk
    rel_error = abs(slope - expected_slope) / expected_slope
    verdict = "passes_within_tolerance" if rel_error < 0.05 else "fail"

    engine_obs = load_engine_observation("random_walk_to_diffusion")
    engine_verdict = None
    if engine_obs:
        e_slope = engine_obs.get("variance_slope", slope)
        e_rel_error = abs(e_slope - expected_slope) / expected_slope
        engine_verdict = "passes_within_tolerance" if e_rel_error < 0.05 else "fail"
        verdict = engine_verdict if engine_verdict == "fail" else verdict

    return {
        "problem_id": "random_walk_to_diffusion",
        "timestamp_utc": utc_now(),
        "runner": "validation/run_validation.py",
        "known_invariants": ["variance_growth_rate"],
        "expected_behavior": {"variance_growth_rate": "linear", "slope_expected": expected_slope},
        "run_config": {"walkers": walkers, "steps": steps, "seed": 42},
        "observations": {
            "variance_slope": slope,
            "relative_error": rel_error,
            "variance_at_steps": {
                "1": variances[0],
                "50": variances[49],
                "100": variances[99],
                "200": variances[-1]
            },
            "engine": engine_obs or None
        },
        "verdict": verdict,
        "notes": "Pure random walk reference; compare engine diffusion once integrated."
    }


def run_graph_flow() -> Dict[str, Any]:
    # Small directed graph max-flow with known result.
    # Graph from CLRS example: expected max flow = 23.
    capacities = {
        ("s", "u"): 10, ("s", "v"): 10,
        ("u", "v"): 2, ("u", "t"): 8,
        ("v", "u"): 0, ("v", "t"): 10
    }
    nodes = {"s", "u", "v", "t"}
    source, sink = "s", "t"

    def neighbors(n: str):
        return [b for (a, b) in capacities if a == n] + [a for (a, b) in capacities if b == n]

    def residual_graph(flow: Dict[Tuple[str, str], int]) -> Dict[Tuple[str, str], int]:
        res = {}
        for (a, b), cap in capacities.items():
            f = flow.get((a, b), 0)
            res[(a, b)] = cap - f
            res[(b, a)] = flow.get((b, a), 0)
        return res

    def bfs(res) -> Dict[str, Tuple[str, int]]:
        parent: Dict[str, Tuple[str, int]] = {}
        queue = [(source, math.inf)]
        seen = {source}
        while queue:
            cur, flow_cap = queue.pop(0)
            for nb in neighbors(cur):
                cap = res.get((cur, nb), 0)
                if cap > 0 and nb not in seen:
                    seen.add(nb)
                    parent[nb] = (cur, cap)
                    new_flow = min(flow_cap, cap)
                    if nb == sink:
                        return {"parent": parent, "bottleneck": new_flow}
                    queue.append((nb, new_flow))
        return {}

    flow: Dict[Tuple[str, str], int] = {}
    max_flow = 0
    while True:
        res = residual_graph(flow)
        path_info = bfs(res)
        if not path_info:
            break
        bottleneck = path_info["bottleneck"]
        parent = path_info["parent"]
        max_flow += bottleneck
        v = sink
        while v != source:
            u = parent[v][0]
            flow[(u, v)] = flow.get((u, v), 0) + bottleneck
            flow[(v, u)] = flow.get((v, u), 0) - bottleneck
            v = u

    # Check conservation at internal nodes
    def net_flow(node: str) -> float:
        inflow = sum(flow.get((u, node), 0) for u in nodes if (u, node) in capacities or (u, node) in flow)
        outflow = sum(flow.get((node, v), 0) for v in nodes if (node, v) in capacities or (node, v) in flow)
        return inflow - outflow

    imbalance = {n: net_flow(n) for n in nodes if n not in (source, sink)}
    max_imbalance = max(abs(v) for v in imbalance.values()) if imbalance else 0.0

    expected_flow = 18  # for this graph variant, expected max flow is 18 (10+8)
    flow_error = abs(max_flow - expected_flow)
    verdict = "passes_within_tolerance" if flow_error == 0 and max_imbalance < 1e-9 else "fail"

    engine_obs = load_engine_observation("graph_flow_conservation")
    if engine_obs:
        e_flow = engine_obs.get("max_flow", max_flow)
        e_imbalance = engine_obs.get("max_imbalance", max_imbalance)
        engine_ok = (abs(e_flow - expected_flow) == 0) and (e_imbalance < 1e-6)
        if not engine_ok:
            verdict = "fail"

    return {
        "problem_id": "graph_flow_conservation",
        "timestamp_utc": utc_now(),
        "runner": "validation/run_validation.py",
        "known_invariants": ["flow_conservation"],
        "expected_behavior": {
            "max_flow": expected_flow,
            "conservation": "net flow zero at internal nodes"
        },
        "run_config": {"graph": "clrs_s_u_v_t", "expected_max_flow": expected_flow},
        "observations": {
            "max_flow": max_flow,
            "flow_error": flow_error,
            "imbalance": imbalance,
            "max_imbalance": max_imbalance,
            "engine": engine_obs or None
        },
        "verdict": verdict,
        "notes": "Pure Python Edmonds-Karp on a small graph; replace with engine-backed flow if available."
    }


def run_lorenz() -> Dict[str, Any]:
    # Lorenz system boundedness / attractor sanity check (RK4 integrator).
    sigma, beta, rho = 10.0, 8.0 / 3.0, 28.0
    dt = 0.01
    steps = 10000
    x, y, z = 1.0, 1.0, 1.0

    def deriv(x: float, y: float, z: float) -> Tuple[float, float, float]:
        dx = sigma * (y - x)
        dy = x * (rho - z) - y
        dz = x * y - beta * z
        return dx, dy, dz

    def rk4(x: float, y: float, z: float, dt: float) -> Tuple[float, float, float]:
        k1x, k1y, k1z = deriv(x, y, z)
        k2x, k2y, k2z = deriv(x + 0.5 * dt * k1x, y + 0.5 * dt * k1y, z + 0.5 * dt * k1z)
        k3x, k3y, k3z = deriv(x + 0.5 * dt * k2x, y + 0.5 * dt * k2y, z + 0.5 * dt * k2z)
        k4x, k4y, k4z = deriv(x + dt * k3x, y + dt * k3y, z + dt * k3z)
        x_next = x + (dt / 6.0) * (k1x + 2 * k2x + 2 * k3x + k4x)
        y_next = y + (dt / 6.0) * (k1y + 2 * k2y + 2 * k3y + k4y)
        z_next = z + (dt / 6.0) * (k1z + 2 * k2z + 2 * k3z + k4z)
        return x_next, y_next, z_next

    radii: List[float] = []
    for _ in range(steps):
        x, y, z = rk4(x, y, z, dt)
        radii.append(math.sqrt(x * x + y * y + z * z))

    max_radius = max(radii) if radii else 0.0
    min_radius = min(radii) if radii else 0.0
    bounded = max_radius < 100.0
    verdict = "passes_within_tolerance" if bounded else "fail"

    engine_obs = load_engine_observation("lorenz_invariant_measure")
    if engine_obs:
        e_bounded = engine_obs.get("bounded", True)
        e_max_r = engine_obs.get("max_radius", max_radius)
        engine_ok = e_bounded and e_max_r < 100.0
        if not engine_ok:
            verdict = "fail"

    return {
        "problem_id": "lorenz_invariant_measure",
        "timestamp_utc": utc_now(),
        "runner": "validation/run_validation.py",
        "known_invariants": ["bounded_state"],
        "expected_behavior": {
            "bounded_state": True,
            "attractor_shape": "butterfly",
            "rho": rho,
            "sigma": sigma,
            "beta": beta
        },
        "run_config": {"dt": dt, "steps": steps, "initial": [1.0, 1.0, 1.0]},
        "observations": {
            "max_radius": max_radius,
            "min_radius": min_radius,
            "final_state": [x, y, z],
            "bounded": bounded,
            "engine": engine_obs or None
        },
        "verdict": verdict,
        "notes": "RK4 Lorenz reference; integrate engine-backed chaos checks if available."
    }


RUNNERS = {
    "diffusion_1d_fixed": run_diffusion_1d,
    "random_walk_to_diffusion": run_random_walk,
    "graph_flow_conservation": run_graph_flow,
    "lorenz_invariant_measure": run_lorenz,
}


def run_selected(selected: List[str]) -> List[Path]:
    paths: List[Path] = []
    for pid in selected:
        runner = RUNNERS.get(pid)
        if runner is None:
            print(f"[WARN] No runner for scenario '{pid}'", file=sys.stderr)
            continue
        result = runner()
        out_path = write_result(result)
        print(f"[INFO] wrote {out_path.relative_to(ROOT)}")
        paths.append(out_path)
    return paths


def main() -> int:
    parser = argparse.ArgumentParser(description="Run AIRS validation scenarios.")
    parser.add_argument("--problem_id", help="Scenario id to run (see validation/catalog.json)")
    parser.add_argument("--all", action="store_true", help="Run all scenarios in catalog")
    args = parser.parse_args()

    catalog = load_catalog()
    catalog_ids = [c["id"] for c in catalog]

    if args.all:
        selected = catalog_ids
    elif args.problem_id:
        if args.problem_id not in catalog_ids:
            print(f"[ERROR] problem_id '{args.problem_id}' not in catalog: {catalog_ids}", file=sys.stderr)
            return 1
        selected = [args.problem_id]
    else:
        print("[ERROR] Provide --problem_id <id> or --all", file=sys.stderr)
        return 1

    run_selected(selected)
    return 0


if __name__ == "__main__":
    sys.exit(main())
