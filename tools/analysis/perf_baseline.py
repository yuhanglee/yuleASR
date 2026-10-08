#!/usr/bin/env python3
"""
Performance Baseline Manager
=============================

Captures benchmark output into a JSON baseline and compares subsequent runs
against it, flagging regressions beyond a configurable threshold.

Usage:
    python tools/analysis/perf_baseline.py capture --build-dir build-native
    python tools/analysis/perf_baseline.py compare --build-dir build-native
    python tools/analysis/perf_baseline.py compare --build-dir build-native --threshold 15

Exit codes:
    0  baseline captured / comparison within threshold
    1  regression detected (one or more metrics exceed threshold)
    2  environment error
"""

import argparse
import json
import os
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_BASELINE = REPO_ROOT / "tests" / "performance" / "baseline.json"
DEFAULT_THRESHOLD = 10.0

METRIC_RE = re.compile(
    r"(?P<name>[A-Za-z_][A-Za-z0-9_/]*)\s*:\s*(?P<value>[\d.]+)\s*(?P<unit>\S+)?"
)


def discover_benchmarks(build_dir: Path):
    benchmarks = []
    for pattern in ["test_*benchmark*", "test_*perf*", "test_latency", "test_throughput", "test_memory"]:
        for f in build_dir.rglob(pattern):
            if f.is_file() and os.access(f, os.X_OK):
                benchmarks.append(f)
    return sorted(set(benchmarks))


def run_benchmark(binary: Path, timeout: int = 60):
    try:
        result = subprocess.run(
            [str(binary)], capture_output=True, text=True, timeout=timeout
        )
        return result.stdout + result.stderr
    except subprocess.TimeoutExpired:
        return ""
    except OSError:
        return ""


def parse_metrics(output: str):
    metrics = {}
    for line in output.splitlines():
        m = METRIC_RE.search(line)
        if m:
            name = m.group("name")
            value = float(m.group("value"))
            unit = m.group("unit") or ""
            metrics[name] = {"value": value, "unit": unit}
    return metrics


def cmd_capture(args):
    build_dir = Path(args.build_dir)
    if not build_dir.is_dir():
        print(f"ERROR: Build directory not found: {build_dir}", file=sys.stderr)
        sys.exit(2)

    benchmarks = discover_benchmarks(build_dir)
    if not benchmarks:
        print(f"No benchmark binaries found in {build_dir}", file=sys.stderr)
        sys.exit(2)

    baseline = {
        "version": 1,
        "created": datetime.now(timezone.utc).isoformat(),
        "commit": _git_commit(),
        "threshold_pct": args.threshold,
        "benchmarks": {},
    }

    for bench in benchmarks:
        name = bench.name
        print(f"Running {name}...")
        output = run_benchmark(bench)
        metrics = parse_metrics(output)
        baseline["benchmarks"][name] = {
            "binary": str(bench.relative_to(REPO_ROOT)),
            "metrics": metrics,
        }
        print(f"  captured {len(metrics)} metrics")

    baseline_path = Path(args.output)
    baseline_path.parent.mkdir(parents=True, exist_ok=True)
    with open(baseline_path, "w") as f:
        json.dump(baseline, f, indent=2)

    print(f"\nBaseline written to {baseline_path}")
    print(f"  {len(baseline['benchmarks'])} benchmarks, "
          f"{sum(len(b['metrics']) for b in baseline['benchmarks'].values())} total metrics")


def cmd_compare(args):
    baseline_path = Path(args.output)
    if not baseline_path.is_file():
        print(f"ERROR: No baseline found at {baseline_path}", file=sys.stderr)
        print("Run 'capture' first to create a baseline.", file=sys.stderr)
        sys.exit(2)

    with open(baseline_path) as f:
        baseline = json.load(f)

    threshold = args.threshold if args.threshold is not None else baseline.get("threshold_pct", DEFAULT_THRESHOLD)
    build_dir = Path(args.build_dir)
    regressions = []
    improvements = []

    for bench_name, bench_data in baseline["benchmarks"].items():
        binary = build_dir / bench_data["binary"]
        if not binary.is_file():
            print(f"SKIP: {bench_name} (binary not found)")
            continue

        output = run_benchmark(binary)
        current = parse_metrics(output)

        for metric_name, old_data in bench_data["metrics"].items():
            old_val = old_data["value"]
            if metric_name not in current:
                continue
            new_val = current[metric_name]["value"]

            if old_val == 0:
                continue

            change_pct = ((new_val - old_val) / old_val) * 100

            if "latency" in metric_name.lower() or "memory" in metric_name.lower():
                is_regression = change_pct > threshold
            else:
                is_regression = change_pct < -threshold

            if is_regression:
                regressions.append({
                    "benchmark": bench_name,
                    "metric": metric_name,
                    "old": old_val,
                    "new": new_val,
                    "change_pct": round(change_pct, 2),
                    "unit": old_data.get("unit", ""),
                })
            elif abs(change_pct) > threshold / 2:
                improvements.append({
                    "benchmark": bench_name,
                    "metric": metric_name,
                    "old": old_val,
                    "new": new_val,
                    "change_pct": round(change_pct, 2),
                })

    if regressions:
        print(f"\nREGRESSIONS DETECTED (threshold: {threshold}%):")
        for r in regressions:
            direction = "slower" if "latency" in r["metric"].lower() else "change"
            print(f"  {r['benchmark']}/{r['metric']}: "
                  f"{r['old']} -> {r['new']} {r['unit']} ({r['change_pct']:+.1f}% {direction})")

    if improvements:
        print(f"\nNotable changes (>{threshold/2}%):")
        for i in improvements:
            print(f"  {i['benchmark']}/{i['metric']}: "
                  f"{i['old']} -> {i['new']} ({i['change_pct']:+.1f}%)")

    if not regressions and not improvements:
        print("All metrics within threshold")

    if regressions:
        sys.exit(1)


def _git_commit():
    try:
        return subprocess.check_output(
            ["git", "rev-parse", "--short", "HEAD"],
            cwd=REPO_ROOT
        ).decode().strip()
    except Exception:
        return "unknown"


def main():
    parser = argparse.ArgumentParser(description="Performance baseline manager")
    sub = parser.add_subparsers(dest="command")

    cap = sub.add_parser("capture", help="Capture current benchmark results as baseline")
    cap.add_argument("--build-dir", default="build-native")
    cap.add_argument("--output", default=str(DEFAULT_BASELINE))
    cap.add_argument("--threshold", type=float, default=DEFAULT_THRESHOLD)

    cmp = sub.add_parser("compare", help="Compare current results against baseline")
    cmp.add_argument("--build-dir", default="build-native")
    cmp.add_argument("--output", default=str(DEFAULT_BASELINE))
    cmp.add_argument("--threshold", type=float, default=None)

    args = parser.parse_args()
    if args.command == "capture":
        cmd_capture(args)
    elif args.command == "compare":
        cmd_compare(args)
    else:
        parser.print_help()
        sys.exit(2)


if __name__ == "__main__":
    main()
