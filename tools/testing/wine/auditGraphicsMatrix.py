#!/usr/bin/env python3
"""Audit completed matrix artifacts for browser diagnostics and observed cleanup.

This reads retained logs without launching a browser or changing the source
matrix. An in-progress matrix can be inspected but cannot pass this gate.
"""

from __future__ import annotations

import argparse
from collections import Counter
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re

from wineGraphicsBrowser import normalize_output, parse_test_exit_status


def read_artifact(path: Path, artifacts: dict, name: str) -> bytes:
    data = path.read_bytes()
    artifacts[name] = {"path": str(path.resolve()), "bytes": len(data),
                       "sha256": hashlib.sha256(data).hexdigest()}
    return data


def diagnostics(log: str) -> list[dict]:
    """Keep source line numbers; normalize only the ephemeral ANGLE context ID."""
    found = []
    for number, line in enumerate(log.splitlines(), 1):
        console = re.search(r':CONSOLE:\d+\]\s*"(.*)', line)
        message = console.group(1) if console else line.strip()
        if console:
            message = re.sub(r'", source:.*$', '', message)
        category = None
        if re.match(r'(?:WebGL(?:2)?:|\[\.WebGL-|GL_INVALID_|GL_OUT_OF_MEMORY)', message):
            category = "graphics"
        elif message.startswith('Could not restore guest GL context '):
            category = "graphics"
        elif re.match(r'(?:Uncaught\b|Unhandled [Pp]romise|worker sent an error!)', message):
            category = "browser_exception"
        elif re.search(r'(?:GPU process exited unexpectedly|GPU process launch failed|'
                       r'The GPU process has crashed|GPU process isn.t usable)', message):
            category = "gpu_process"
        if category:
            found.append({"line": number, "category": category, "message": message,
                          "signature": re.sub(r'\[\.WebGL-[^]]+\]', '[.WebGL-context]', message)})
    return found


def audit_run(run: dict, minimum_cleanup_seconds: int) -> dict:
    row = {key: run.get(key) for key in ("mode", "suite", "group")}
    row.update(assertions_passed=run.get("assertions_passed", run.get("passed")) is True,
               results=run.get("results", []), artifacts={}, problems=[], diagnostics=[])
    problems = row["problems"]
    if not row["assertions_passed"] or run.get("exit_code") != 0:
        problems.append("matrix result did not pass")
    try:
        outer = json.loads(read_artifact(Path(run["manifest"]), row["artifacts"], "manifest"))
        artifact = outer["graphics_artifacts"][run["group"]]
        manifest = json.loads(read_artifact(Path(artifact["browser_manifest"]),
                                           row["artifacts"], "browser_manifest"))
        if not manifest.get("finished_at"):
            problems.append("browser manifest has no completion timestamp")
        for key in ("suite", "group"):
            if manifest["result"].get(key) != run[key]:
                problems.append(f"browser {key} does not match matrix")
        if manifest.get("mode") != run["mode"]:
            problems.append("browser mode does not match matrix")
        browser = manifest["browser"]
        if browser.get("timed_out") is not False or browser.get("exited_early") is not False:
            problems.append("browser timed out, exited early, or lacks completion flags")
        files = manifest["artifacts"]
        payload = json.loads(read_artifact(Path(files["browser_payload"]), row["artifacts"], "payload"))
        log = read_artifact(Path(files["chrome_log"]), row["artifacts"], "chrome_log").decode('utf-8', errors='replace')
        row["diagnostics"] = diagnostics(log)
        if not log.strip():
            problems.append("browser stderr is empty")
        if row["diagnostics"]:
            problems.append("browser stderr contains graphics or browser diagnostics")
        row["browser_events"] = payload.get("browserEvents")
        if row["browser_events"] != []:
            problems.append("browser events are present or missing")
        marker = manifest.get("cleanup_marker", "BOXEDWINE_WINESERVER_CLEANUP_OK")
        output = normalize_output(payload.get("output", ""))
        policy = manifest.get("exit_status_policy")
        status, exit_problem = parse_test_exit_status(output, policy, manifest["result"].get("failures"))
        row["test_exit"] = {"policy": policy, "status": status, "recorded": policy is not None}
        if exit_problem:
            problems.append(exit_problem)
        if policy is not None and manifest["result"].get("exit_status") != status:
            problems.append("recorded test exit status differs from guest output")
        has_marker = bool(marker) and any(line.strip() == marker for line in output.splitlines())
        seconds = manifest.get("cleanup_wait_seconds")
        cleanup = {"marker": marker, "standalone_marker_observed": has_marker,
                   "wait_satisfied": payload.get("cleanupWaitSatisfied") is True,
                   "wait_seconds": seconds, "payload_complete": payload.get("kind") == "complete"}
        row["cleanup"] = cleanup
        if not has_marker or not cleanup["wait_satisfied"] or not cleanup["payload_complete"]:
            problems.append("standalone cleanup and its observation did not complete")
        if not isinstance(seconds, (int, float)) or seconds < minimum_cleanup_seconds:
            problems.append(f"cleanup observation is shorter than {minimum_cleanup_seconds} seconds")
    except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
        problems.append(f"missing or invalid artifact: {error}")
    row["passed"] = not problems
    return row


def audit_matrix(path: Path, minimum_cleanup_seconds: int = 15) -> dict:
    artifacts = {}
    read_artifact(Path(__file__), artifacts, "auditor")
    matrix = json.loads(read_artifact(path, artifacts, "matrix"))
    rows = [audit_run(run, minimum_cleanup_seconds) for run in matrix["runs"]]
    complete = matrix.get("complete") is True
    counts = Counter(item["signature"] for row in rows for item in row["diagnostics"])
    return {"schema_version": 1, "audited_at": datetime.now(timezone.utc).isoformat(),
            "scope": "Completed rows only; full retained Chrome stderr and cleanup artifacts. No browser launched.",
            "artifacts": artifacts, "minimum_cleanup_seconds": minimum_cleanup_seconds,
            "matrix_complete": complete, "matrix_passed": matrix.get("passed") is True,
            "completed_runs": len(rows), "diagnostic_counts": dict(sorted(counts.items())),
            "passed": complete and matrix.get("passed") is True and bool(rows) and all(row["passed"] for row in rows),
            "runs": rows}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("matrix", type=Path)
    parser.add_argument("--output", type=Path, required=True, help="new report path; existing evidence is never overwritten")
    parser.add_argument("--minimum-cleanup-seconds", type=int, default=15)
    args = parser.parse_args(argv)
    if args.minimum_cleanup_seconds < 0:
        parser.error("--minimum-cleanup-seconds must be nonnegative")
    try:
        report = audit_matrix(args.matrix, args.minimum_cleanup_seconds)
        with args.output.open("x", encoding="utf-8", newline="\n") as stream:
            json.dump(report, stream, indent=2)
            stream.write("\n")
    except (OSError, ValueError, KeyError, TypeError) as error:
        parser.exit(2, f"audit failed: {error}\n")
    print(f"{'PASS' if report['passed'] else 'NOT PASSED'}: {report['completed_runs']} completed runs, "
          f"{sum(report['diagnostic_counts'].values())} diagnostics, matrix complete={report['matrix_complete']}")
    for message, count in report["diagnostic_counts"].items():
        print(f"  {count}: {message}")
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
