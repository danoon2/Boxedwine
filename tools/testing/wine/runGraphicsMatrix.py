#!/usr/bin/env python3
"""Run an exact Wine graphics baseline against explicitly selected browser builds."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
from pathlib import Path
import subprocess
import sys

import wineGraphicsBrowser as graphics
from auditGraphicsMatrix import audit_run

MODES = ("single-threaded-non-jit", "multi-threaded-non-jit",
         "single-threaded-jit", "multi-threaded-jit")


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def runtime_candidate(baseline: dict, mode: str, wasm_hash: str) -> dict:
    """Change runtime identity only; preserve all reviewed result expectations."""
    result = copy.deepcopy(baseline)
    result["baseline_id"] = f"{baseline['baseline_id']}-{mode}-runtime-candidate"
    result["reference_inputs"]["boxedwine_wasm_sha256"] = wasm_hash
    result["runtime_candidate"] = {
        "mode": mode, "source_baseline_id": baseline["baseline_id"],
        "source_wasm_sha256": baseline["reference_inputs"]["boxedwine_wasm_sha256"],
        "policy": "Runtime identity refreshed explicitly; assertion, TODO, failure, skip and failure-location expectations unchanged.",
    }
    return result


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", action="append", required=True, metavar="MODE=DIR")
    parser.add_argument("--filesystem", type=Path, required=True)
    parser.add_argument("--tests-archive", type=Path, required=True)
    parser.add_argument("--baseline", type=Path, required=True)
    parser.add_argument("--divergences", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--timeout", type=int, default=1200)
    parser.add_argument("--cleanup-wait-seconds", type=int, default=15,
                        help="require each group's cleanup, then observe for late errors (default: 15)")
    parser.add_argument("--full", action="store_true", help="include the large visual groups")
    parser.add_argument("--headless", action="store_true")
    parser.add_argument("--stop-on-failure", action="store_true",
                        help="stop after a failed group or artifact audit; preserve the incomplete matrix")
    args = parser.parse_args(argv)
    if args.timeout <= 0:
        parser.error("--timeout must be positive")
    if not 0 <= args.cleanup_wait_seconds < args.timeout:
        parser.error("cleanup observation must be nonnegative and shorter than the timeout")
    builds = {}
    for value in args.build:
        mode, separator, folder = value.partition("=")
        if not separator or mode not in MODES or not folder or mode in builds:
            parser.error(f"invalid or duplicate build: {value}")
        builds[mode] = Path(folder).resolve()
        graphics.validate_web_build(builds[mode])
    baseline = json.loads(args.baseline.read_text(encoding="utf-8"))
    args.output.mkdir(parents=True, exist_ok=False)
    report = {
        "schema_version": 1, "complete": False, "passed": False,
        "selection": "full" if args.full else "fast",
        "cleanup_wait_seconds": args.cleanup_wait_seconds,
        "stop_on_failure": args.stop_on_failure,
        "source_baseline": {"path": str(args.baseline.resolve()), "sha256": sha256(args.baseline)},
        "filesystem": {"path": str(args.filesystem.resolve()), "sha256": sha256(args.filesystem)},
        "tests_archive": {"path": str(args.tests_archive.resolve()), "sha256": sha256(args.tests_archive)},
        "runs": [],
    }
    report_path = args.output / "matrix.json"

    def save():
        report_path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

    save()
    for mode, folder in builds.items():
        candidate_path = args.output / f"baseline-{mode}.json"
        candidate_path.write_text(json.dumps(runtime_candidate(baseline, mode,
            sha256(folder / "boxedwine.wasm")), indent=2) + "\n", encoding="utf-8")
        for suite, groups in baseline["suites"].items():
            for group in groups:
                if not args.full and group == "visual":
                    continue
                label = f"{mode}-{suite}-{group}"
                log_path = args.output / f"{label}.log"
                cache = args.output / label
                flag = "d3dx9" if suite == "d3dx9_43" else suite
                command = [sys.executable, "-u", str(Path(__file__).with_name("runWineTests.py")),
                    f"--{flag}-group", group, "--graphics-mode", mode,
                    "--graphics-build-dir", str(folder),
                    "--graphics-filesystem", str(args.filesystem.resolve()),
                    "--graphics-tests-archive", str(args.tests_archive.resolve()),
                    "--graphics-baseline", str(candidate_path.resolve()),
                    "--webgl-test-divergences", str(args.divergences.resolve()),
                    "--cache-dir", str(cache.resolve()), "--graphics-timeout", str(args.timeout),
                    "--graphics-cleanup-wait-seconds", str(args.cleanup_wait_seconds)]
                if args.headless:
                    command.append("--graphics-headless")
                print(f"START {label}", flush=True)
                with log_path.open("w", encoding="utf-8") as log:
                    completed = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT)
                manifests = sorted(cache.glob("runs/*/manifest.json"))
                row = {"mode": mode, "suite": suite, "group": group,
                       "exit_code": completed.returncode, "log": str(log_path.resolve()),
                       "passed": False}
                if manifests:
                    manifest = json.loads(manifests[-1].read_text(encoding="utf-8"))
                    row.update(manifest=str(manifests[-1].resolve()), results=manifest.get("results", []))
                    row["passed"] = completed.returncode == 0 and len(row["results"]) == 1 and row["results"][0]["passed"]
                row["assertions_passed"] = row["passed"]
                row["artifact_audit"] = audit_run(row, max(15, args.cleanup_wait_seconds))
                row["passed"] = row["assertions_passed"] and row["artifact_audit"]["passed"]
                report["runs"].append(row)
                save()
                print(f"{'PASS' if row['passed'] else 'FAIL'} {label}", flush=True)
                if not row["passed"] and args.stop_on_failure:
                    report["stopped_after_failure"] = label
                    save()
                    return 1
    report["complete"] = True
    report["passed"] = bool(report["runs"]) and all(row["passed"] for row in report["runs"])
    save()
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
