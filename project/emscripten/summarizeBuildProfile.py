#!/usr/bin/env python3
"""Summarize retained EMPROFILE=1 blocks without double-counting nested time."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path


def union_seconds(intervals: list[tuple[float, float]]) -> float:
    """Elapsed time occupied by at least one interval, including parallel jobs."""
    total = 0.0
    end = None
    for start, stop in sorted(intervals):
        if end is None or start > end:
            total += stop - start
        elif stop > end:
            total += stop - end
        end = max(end, stop) if end is not None else stop
    return total


def read_blocks(events: list) -> tuple[list[dict], list[dict]]:
    if not isinstance(events, list) or not events:
        raise ValueError("empty or invalid profiler event list")
    processes = {}
    blocks = []
    for index, event in enumerate(events):
        if not isinstance(event, dict):
            raise ValueError(f"event {index}: expected an object")
        pid, worker = event.get("pid"), event.get("subprocessPid")
        if type(pid) is not int or type(worker) is not int:
            raise ValueError(f"event {index}: missing process identity")
        stamp = event.get("time")
        if type(stamp) not in (int, float) or not math.isfinite(stamp):
            raise ValueError(f"event {index}: invalid timestamp")
        key = (pid, worker)
        op = event.get("op")
        if op == "start":
            if key in processes:
                raise ValueError(f"event {index}: duplicate process start")
            processes[key] = dict(pid=pid, subprocess_pid=worker, start=stamp,
                last=stamp, stack=[], exit=None)
        process = processes.get(key)
        if process is None or process["exit"] is not None:
            raise ValueError(f"event {index}: event outside a recorded process lifetime")
        if stamp < process["last"]:
            raise ValueError(f"event {index}: clock moved backwards")
        process["last"] = stamp
        if op == "enterBlock":
            name = event.get("name")
            if not isinstance(name, str) or not name:
                raise ValueError(f"event {index}: invalid block name")
            process["stack"].append(dict(name=name, start=stamp,
                depth=len(process["stack"])))
        elif op == "exitBlock":
            if not process["stack"] or process["stack"][-1]["name"] != event.get("name"):
                raise ValueError(f"event {index}: mismatched block exit")
            block = process["stack"].pop()
            blocks.append(dict(block, end=stamp, pid=pid, subprocess_pid=worker))
        elif op == "exit":
            status = event.get("returncode")
            if type(status) is not int and status != "MISSING EXIT CODE":
                raise ValueError(f"event {index}: invalid process return code")
            if process["stack"]:
                raise ValueError(f"event {index}: process exited with open blocks")
            process.update(exit=stamp, returncode=status)
        elif op not in ("start", "spawn", "wait", "finish"):
            raise ValueError(f"event {index}: unknown profiler operation {op!r}")
        # Subprocess events are retained in the source trace. Their wrapper and
        # real-PID intervals can overlap, so they are not added to block timings.
    if any(process["exit"] is None for process in processes.values()):
        raise ValueError("unfinished profiler process")
    return blocks, [{key: value for key, value in process.items()
        if key not in ("stack", "last")} for process in processes.values()]


def summarize(directory: Path, build_exit_code: int) -> dict:
    report = dict(schema_version=1, build_exit_code=build_exit_code,
        complete=False, passed=False, files=[], errors=[], blocks={}, processes=[],
        scope="Emscripten EMPROFILE=1 block timings; inclusive nested times are not additive. "
            "Busy seconds merge overlapping intervals across jobs. Process seconds sum them. "
            "Neither is GPU time or runtime game performance; timestamps have millisecond precision.")
    all_blocks = []
    identities = set()
    for path in sorted(directory.glob("toolchain_profiler.pid_*.json")):
        data = path.read_bytes()
        record = dict(path=str(path), bytes=len(data), sha256=hashlib.sha256(data).hexdigest())
        report["files"].append(record)
        try:
            blocks, processes = read_blocks(json.loads(data))
            keys = {(item["pid"], item["subprocess_pid"]) for item in processes}
            if identities.intersection(keys):
                raise ValueError("process recorded in multiple files")
            identities.update(keys)
            report["processes"].extend(processes)
            all_blocks.extend(blocks)
        except (ValueError, TypeError, KeyError) as error:
            report["errors"].append(dict(path=str(path), error=str(error)))
    if not report["files"]:
        report["errors"].append(dict(error="no profiler traces found"))
    for name in sorted({block["name"] for block in all_blocks}):
        intervals = [(block["start"], block["end"]) for block in all_blocks if block["name"] == name]
        report["blocks"][name] = dict(calls=len(intervals),
            busy_seconds=round(union_seconds(intervals), 6),
            process_seconds=round(sum(end - start for start, end in intervals), 6))
    report["failed_profiled_processes"] = [item for item in report["processes"]
        if type(item["returncode"]) is int and item["returncode"] != 0]
    report["unrecorded_process_statuses"] = sum(item["returncode"] == "MISSING EXIT CODE"
        for item in report["processes"])
    report["complete"] = not report["errors"]
    # The upstream profiler may lack a return code even after normal atexit.
    # Require the separately observed make status; never infer success from time.
    report["passed"] = report["complete"] and build_exit_code == 0 and not report["failed_profiled_processes"]
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path)
    parser.add_argument("--build-exit-code", type=int, required=True,
        help="actual exit code captured from the completed build command")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    report = summarize(args.directory, args.build_exit_code)
    # Preserve earlier reports, including failures.
    with args.output.open("x", encoding="utf-8") as stream:
        json.dump(report, stream, indent=2, allow_nan=False)
        stream.write("\n")
    for name in ("compile inputs", "link", "binaryen", "wasm_opt"):
        if name in report["blocks"]:
            print(f"{name}: {report['blocks'][name]}")
    print(f"Profile {'PASS' if report['passed'] else 'FAIL'}: {args.output}")
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
