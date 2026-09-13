"""Validate timing accounting and reject incomplete/failed profiling evidence."""

import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from summarizeBuildProfile import read_blocks, summarize, union_seconds


def trace(pid=10, offset=0):
    def event(op, stamp, **fields):
        return dict(pid=pid, subprocessPid=pid, op=op, time=stamp + offset, **fields)
    return [event("start", 0), event("enterBlock", 0, name="run"),
        event("enterBlock", 1, name="link"), event("exitBlock", 3, name="link"),
        event("enterBlock", 3, name="binaryen"), event("enterBlock", 4, name="wasm_opt"),
        event("exitBlock", 8, name="wasm_opt"), event("exitBlock", 9, name="binaryen"),
        event("exitBlock", 10, name="run"), event("exit", 10, returncode=0)]


class BuildProfileTests(unittest.TestCase):
    def report(self, entries, code=0):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for index, events in enumerate(entries):
                (root / f"toolchain_profiler.pid_{index}.json").write_text(json.dumps(events))
            return summarize(root, code)

    def test_nested_optimizer_time_is_not_added_to_parent(self):
        report = self.report([trace()])
        self.assertTrue(report["passed"])
        self.assertEqual(10, report["blocks"]["run"]["busy_seconds"])
        self.assertEqual(6, report["blocks"]["binaryen"]["busy_seconds"])
        self.assertEqual(4, report["blocks"]["wasm_opt"]["busy_seconds"])
        self.assertEqual(2, report["blocks"]["link"]["busy_seconds"])

    def test_parallel_blocks_report_busy_and_process_time_separately(self):
        report = self.report([trace(), trace(11, 2)])
        self.assertTrue(report["passed"])
        self.assertEqual(dict(calls=2, busy_seconds=6, process_seconds=8),
            report["blocks"]["wasm_opt"])
        self.assertEqual(12, report["blocks"]["run"]["busy_seconds"])

    def test_interval_union_includes_disjoint_nested_equal_and_touching(self):
        self.assertEqual(7, union_seconds([(1, 4), (2, 3), (1, 4), (4, 6), (8, 10)]))
        self.assertEqual(0, union_seconds([]))

    def test_missing_code_requires_successful_outer_build(self):
        events = trace()
        events[-1]["returncode"] = "MISSING EXIT CODE"
        success = self.report([events])
        self.assertTrue(success["passed"])
        self.assertEqual(1, success["unrecorded_process_statuses"])
        self.assertFalse(self.report([events], code=2)["passed"])

    def test_profiled_failure_is_not_overridden_by_successful_outer_build(self):
        events = trace()
        events[-1]["returncode"] = 7
        report = self.report([events])
        self.assertFalse(report["passed"])
        self.assertEqual(7, report["failed_profiled_processes"][0]["returncode"])

    def test_empty_truncated_and_duplicate_traces_do_not_pass(self):
        for entries in ([], [[]], [trace()[:-1]], [trace(), trace()]):
            with self.subTest(entries=entries):
                self.assertFalse(self.report(entries)["complete"])

    def test_malformed_events_do_not_produce_timings(self):
        for index, fields in ((3, {"name": "wasm_opt"}), (3, {"time": -1}),
                (3, {"time": float("nan")}), (3, {"time": float("inf")}),
                (3, {"pid": "10"}), (3, {"op": "unknown"}), (9, {"returncode": None}),
                (9, {"returncode": True}), (5, {"name": ""})):
            with self.subTest(fields=fields):
                events = copy.deepcopy(trace())
                events[index].update(fields)
                report = self.report([events])
                self.assertFalse(report["complete"])
                self.assertEqual({}, report["blocks"])

    def test_open_blocks_and_events_after_exit_are_rejected(self):
        events = trace()
        for changed in (events[:8] + events[9:], events + [events[2]], events[1:], [events[0]] + events):
            with self.subTest(events=changed):
                with self.assertRaises(ValueError):
                    read_blocks(changed)

    def test_spawn_wrapper_events_are_not_double_counted(self):
        events = trace()
        events[3:3] = [dict(pid=10, subprocessPid=10, op=op, time=stamp, targetPid=target)
            for op, stamp, target in (("spawn", 1, -1), ("spawn", 1, 22),
                ("wait", 2, 22), ("finish", 3, 22), ("finish", 3, -1))]
        self.assertEqual(self.report([trace()])["blocks"], self.report([events])["blocks"])

    def test_cli_retains_incomplete_evidence_and_refuses_overwrite(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path = root / "toolchain_profiler.pid_1.json"
            path.write_text('[{"pid":10')
            output = root / "report.json"
            command = [sys.executable, str(Path(__file__).with_name("summarizeBuildProfile.py")),
                str(root), "--build-exit-code", "0", "--output", str(output)]
            child = subprocess.run(command, capture_output=True)
            self.assertEqual(1, child.returncode)
            report = json.loads(output.read_text())
            self.assertFalse(report["complete"])
            self.assertEqual(path.stat().st_size, report["files"][0]["bytes"])
            original = output.read_bytes()
            self.assertNotEqual(0, subprocess.run(command, capture_output=True).returncode)
            self.assertEqual(original, output.read_bytes())


if __name__ == "__main__":
    unittest.main()
