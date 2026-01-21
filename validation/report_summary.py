import json
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Tuple

ROOT = Path(__file__).resolve().parents[1]
ARTIFACTS_DIR = ROOT / "artifacts" / "validation"
SUMMARY_PATH = ROOT / "validation" / "reports" / "summary.json"


def parse_timestamp(ts: str) -> datetime:
    try:
        return datetime.fromisoformat(ts)
    except Exception:
        return datetime.min


def load_artifacts() -> List[Dict]:
    results: List[Dict] = []
    for path in sorted(ARTIFACTS_DIR.glob("*.json")):
        try:
            with path.open("r", encoding="utf-8") as f:
                data = json.load(f)
                data["_path"] = str(path.relative_to(ROOT))
                results.append(data)
        except Exception as exc:
            print(f"[WARN] failed to load {path}: {exc}")
    return results


def latest_by_problem(results: List[Dict]) -> Dict[str, Dict]:
    latest: Dict[str, Tuple[datetime, Dict]] = {}
    for r in results:
        pid = r.get("problem_id", "unknown")
        ts = parse_timestamp(r.get("timestamp_utc", ""))
        if pid not in latest or ts > latest[pid][0]:
            latest[pid] = (ts, r)
    return {pid: rec for pid, (ts, rec) in latest.items()}


def summarize(results: List[Dict]) -> Dict:
    latest = latest_by_problem(results)
    verdict_counts: Dict[str, int] = {}
    for r in latest.values():
        verdict = r.get("verdict", "unknown")
        verdict_counts[verdict] = verdict_counts.get(verdict, 0) + 1
    return {
        "generated_at": datetime.utcnow().isoformat() + "Z",
        "artifact_count": len(results),
        "problems": len(latest),
        "verdict_counts": verdict_counts,
        "latest": latest,
    }


def main() -> int:
    results = load_artifacts()
    summary = summarize(results)
    SUMMARY_PATH.parent.mkdir(parents=True, exist_ok=True)
    with SUMMARY_PATH.open("w", encoding="utf-8") as f:
        json.dump(summary, f, indent=2)
    print(f"[INFO] wrote {SUMMARY_PATH.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
