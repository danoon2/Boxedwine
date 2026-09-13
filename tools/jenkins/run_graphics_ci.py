#!/usr/bin/env python3
"""Run and independently audit the full four-mode Wine graphics grid for CI.

The JSON configuration selects existing, immutable runtime directories and
reviewed Wine inputs. This command never builds, downloads, retries or publishes.
"""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import xml.etree.ElementTree as ET
import zipfile

REPO = Path(__file__).resolve().parents[2]
WINE_TOOLS = REPO / "tools/wineTests"
sys.path.insert(0, str(WINE_TOOLS))
import auditGraphicsCoverage as coverage
import auditGraphicsMatrix as artifacts
import runWineTests as wine
import wineGraphicsBrowser as browser


def identity(path: Path) -> dict:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return dict(path=str(path.resolve()), bytes=path.stat().st_size, sha256=digest.hexdigest())


def save(path: Path, data: dict) -> None:
    path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")


def prepare(config_path: Path, output: Path) -> tuple[dict, list[str], dict]:
    config_path = config_path.resolve()
    config = json.loads(config_path.read_text(encoding="utf-8"))
    if config.get("schema_version") != 1:
        raise ValueError("unsupported graphics CI configuration schema")
    if set(config["builds"]) != set(coverage.MODES):
        raise ValueError("exactly all four browser build modes are required")
    timeout = config.get("timeout_seconds", 1800)
    if type(timeout) is not int or timeout <= 15:
        raise ValueError("timeout_seconds must be an integer greater than 15")
    if type(config.get("headless", True)) is not bool:
        raise ValueError("headless must be a boolean")

    def resolve(value):
        return (config_path.parent / value).resolve()

    inputs = {name: identity(resolve(config[name])) for name in
              ("filesystem", "tests_archive", "baseline", "divergences")}
    for name in ("tests_archive", "baseline"):
        if inputs[name]["sha256"] != config[name + "_sha256"]:
            raise ValueError(name + " differs from the configured SHA-256")
    baseline = wine.load_graphics_baseline(Path(inputs["baseline"]["path"]))
    # The browser backend also knows standalone OpenGL probes. The full Wine
    # grid is defined by the unified Wine runner, as in the coverage auditor.
    suites = coverage.SUITES
    if set(baseline["suites"]) != set(suites) or any(
            set(baseline["suites"][name]) != set(suite.groups) for name, suite in suites.items()):
        raise ValueError("baseline must include every group in all five graphics suites")
    reference = baseline["reference_inputs"]
    for name, field in (("filesystem", "filesystem_sha256"),
                        ("divergences", "webgl_test_divergence_manifest_sha256")):
        if inputs[name]["sha256"] != reference[field]:
            raise ValueError(name + " differs from the reviewed baseline")
    # Read each member, checking its ZIP CRC and exact reviewed executable hash.
    with zipfile.ZipFile(inputs["tests_archive"]["path"]) as archive:
        for name, suite in suites.items():
            if archive.namelist().count(suite.executable) != 1:
                raise ValueError("missing or duplicated test executable: " + suite.executable)
            data = archive.read(suite.executable)
            if hashlib.sha256(data).hexdigest() != reference["test_executable_sha256"][name]:
                raise ValueError("test executable differs from baseline: " + suite.executable)
    builds = {mode: resolve(config["builds"][mode]) for mode in coverage.MODES}
    runtimes = {}
    for mode, folder in builds.items():
        browser.validate_web_build(folder)
        runtimes[mode] = {name: identity(folder / name) for name in browser.REQUIRED_WEB_FILES}
    tools = [Path(__file__), *[WINE_TOOLS / name for name in
        ("runGraphicsMatrix.py", "runWineTests.py", "wineGraphicsBrowser.py",
         "auditGraphicsMatrix.py", "auditGraphicsCoverage.py", "webglTestDivergences.py")]]
    pinned = dict(config=identity(config_path), inputs=inputs, runtimes=runtimes,
                  tools={str(path.relative_to(REPO)): identity(path) for path in tools})
    command = [sys.executable, "-u", str(WINE_TOOLS / "runGraphicsMatrix.py"),
               "--full", "--stop-on-failure", "--cleanup-wait-seconds", "15",
               "--timeout", str(timeout), "--output", str(output / "matrix")]
    for name, value in inputs.items():
        command.extend(["--" + name.replace("_", "-"), value["path"]])
    for mode, folder in builds.items():
        command.extend(["--build", mode + "=" + str(folder)])
    if config.get("headless", True):
        command.append("--headless")
    return pinned, command, builds


def verify_unchanged(pinned: dict) -> list[str]:
    files = [pinned["config"], *pinned["inputs"].values(), *pinned["tools"].values()]
    files.extend(item for runtime in pinned["runtimes"].values() for item in runtime.values())
    problems = []
    for expected in files:
        try:
            if identity(Path(expected["path"])) != expected:
                problems.append("input changed during execution: " + expected["path"])
        except OSError as error:
            problems.append("input unavailable after execution: " + str(error))
    return problems


def write_junit(path: Path, report: dict, audited: dict | None) -> None:
    # Missing coverage is a failure, not a skipped or silently omitted test.
    rows = {(row["mode"], row["suite"], row["group"]): row
            for row in (audited or {}).get("runs", [])}
    root = ET.Element("testsuite", name="boxedwine.full-wine-graphics")
    failures = 0
    for mode in coverage.MODES:
        for suite, definition in coverage.SUITES.items():
            for group in definition.groups:
                case = ET.SubElement(root, "testcase", classname=mode + "." + suite, name=group)
                row = rows.get((mode, suite, group))
                if not row or row.get("passed") is not True or row.get("reparsed_result", {}).get("exit_status") is None:
                    failures += 1
                    message = "; ".join(row.get("problems", [])) if row else "group did not complete"
                    ET.SubElement(case, "failure", message=message or "missing passing exit evidence").text = json.dumps(row)
    if not report["passed"]:
        case = ET.SubElement(root, "testcase", classname="boxedwine.ci", name="complete-matrix-gate")
        ET.SubElement(case, "failure", message="CI acceptance did not pass").text = json.dumps(report["problems"])
        failures += 1
    root.set("tests", str(len(root)))
    root.set("failures", str(failures))
    root.set("errors", "0")
    root.set("skipped", "0")
    ET.ElementTree(root).write(path, encoding="utf-8", xml_declaration=True)


def run(config_path: Path, output: Path, prepare_only: bool = False) -> int:
    output = output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    report = dict(schema_version=1, started_at=datetime.now(timezone.utc).isoformat(),
                  state="preparing", passed=False, problems=[])
    save(output / "status.json", report)
    audited = None
    pinned = None
    try:
        pinned, command, builds = prepare(config_path, output)
        save(output / "inputs.json", pinned)
        save(output / "command.json", command)
        if prepare_only:
            report["state"] = "prepared"
            # Preflight success does not establish graphics acceptance.
            return 0
        report["state"] = "running"
        save(output / "status.json", report)
        report["matrix_exit_code"] = subprocess.call(command, cwd=REPO)
        if report["matrix_exit_code"] != 0:
            report["problems"].append("matrix command returned " + str(report["matrix_exit_code"]))
        matrix_path = output / "matrix/matrix.json"
        # Attempt both audits, even after a command failure, retaining partial evidence.
        for name, audit in (
            ("artifact-audit", lambda: artifacts.audit_matrix(matrix_path)),
            ("coverage-audit", lambda: coverage.check_coverage(
                Path(pinned["inputs"]["baseline"]["path"]), [matrix_path], builds)),
        ):
            try:
                result = audit()
                save(output / (name + ".json"), result)
                if name == "coverage-audit":
                    audited = result
                if result["passed"] is not True:
                    report["problems"].append(name + " did not pass")
            except (OSError, ValueError, KeyError, TypeError, wine.RunnerError) as error:
                report["problems"].append(name + ": " + str(error))
        if not audited or not audited.get("runs") or any(
                row.get("reparsed_result", {}).get("exit_status") is None for row in audited["runs"]):
            report["problems"].append("every group must record its actual test exit status")
        report["problems"].extend(verify_unchanged(pinned))
        report["passed"] = not report["problems"]
        report["state"] = "finished"
        return 0 if report["passed"] else 1
    except (OSError, ValueError, KeyError, TypeError, zipfile.BadZipFile, wine.RunnerError, browser.RunnerError) as error:
        report["state"] = "failed"
        report["problems"].append(str(error))
        return 2
    finally:
        report["finished_at"] = datetime.now(timezone.utc).isoformat()
        save(output / "status.json", report)
        if not prepare_only:
            write_junit(output / "junit.xml", report, audited)
        print(json.dumps({key: report[key] for key in ("state", "passed", "problems")}), flush=True)


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--config", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path, help="new artifact directory")
    parser.add_argument("--prepare-only", action="store_true", help="validate inputs without launching Chrome")
    args = parser.parse_args(argv)
    try:
        return run(args.config, args.output, args.prepare_only)
    except OSError as error:
        parser.exit(2, str(error) + "\n")


if __name__ == "__main__":
    raise SystemExit(main())
