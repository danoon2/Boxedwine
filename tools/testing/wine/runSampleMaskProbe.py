"""Run and audit the Wine 11 sample-mask probes in an isolated JIT browser session."""
import argparse
import hashlib
import json
from pathlib import Path
import re

from auditGraphicsMatrix import diagnostics
from auditSampleMask import audit_capabilities, audit_transitions, recover_guest
import wineGraphicsBrowser as browser


def identity(path):
    raw = path.read_bytes()
    return {"path": str(path.resolve()), "bytes": len(raw), "sha256": hashlib.sha256(raw).hexdigest()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--kind", choices=("transitions", "capabilities"), required=True)
    parser.add_argument("--extension", choices=("present", "absent"), default="present")
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--filesystem", type=Path, required=True)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--mode", choices=("single-threaded-jit", "multi-threaded-jit"), default="single-threaded-jit")
    parser.add_argument("--chrome", type=Path)
    parser.add_argument("--timeout", type=int, default=600)
    parser.add_argument("--headless", action="store_true")
    args = parser.parse_args()
    if args.extension == "absent" and args.kind != "capabilities":
        parser.error("--extension absent requires --kind capabilities")
    if args.timeout <= 15:
        parser.error("--timeout must exceed the 15-second cleanup observation")
    control = Path(__file__).resolve().parent / "tests/webgl_mask_extension_control.js"
    script = control.read_text(encoding="utf-8").replace("/*__DENIED__*/", "true" if args.extension == "absent" else "false")
    inputs = [identity(p) for p in (Path(__file__), Path(__file__).with_name("auditSampleMask.py"), control)]
    suite = browser.GraphicsSuite("sample-mask-" + args.kind, args.executable.name, ("visual",),
        redirect_output=True, exit_status_policy="wine", cleanup_wait_seconds=15,
        environment=("WINETEST_DEBUG=1", "WINETEST_MUTE_THRESHOLD=100000", "WINEDEBUG=-all,err+all,warn+d3d",
                     "MASK_EXTENSION_ABSENT=" + str(int(args.extension == "absent"))))
    # The harness serves this prelude before boxedwine.js in both page and worker realms.
    original_prelude = browser._worker_error_observer_script
    browser._worker_error_observer_script = lambda: script + "\n" + original_prelude()
    try:
        result, manifest = browser.run_browser_test(suite=suite, group="visual",
            build_dir=args.build_dir.resolve(), filesystem=args.filesystem.resolve(),
            test_executable=args.executable.resolve(), chrome=args.chrome or browser.find_chrome(),
            run_dir=args.output.resolve(), timeout=args.timeout, headless=args.headless,
            keep_browser_profile=False, mode=args.mode)
    finally:
        browser._worker_error_observer_script = original_prelude
    payload_path = Path(manifest["artifacts"]["browser_payload"])
    chrome_path = Path(manifest["artifacts"]["chrome_log"])
    payload = json.loads(payload_path.read_text(encoding="utf-8"))
    chrome = chrome_path.read_text(encoding="utf-8", errors="replace")
    capture_error = None
    try:
        guest, capture_method = recover_guest(payload, chrome_path.read_bytes(), manifest["launch_url"], "visual")
    except ValueError as error:
        guest, capture_method, capture_error = "", None, str(error)
    guest_path = args.output / "guest.log"
    guest_path.write_bytes(guest.encode("utf-8"))
    audit = audit_transitions("transitions", guest) if args.kind == "transitions" else audit_capabilities(args.extension, guest)
    expected_count = 1567 if args.kind == "transitions" else 58 if args.extension == "absent" else 84
    coverage = audit["counts"] == dict(tests=expected_count, todo=0, flaky=0, failures=0, skipped=0)
    output = browser.normalize_output(payload["output"])
    kill = re.findall(r"^BOXEDWINE_WINESERVER_KILL_STATUS:(\d+)$", output, re.M)
    wait = re.findall(r"^BOXEDWINE_WINESERVER_WAIT_STATUS:(\d+)$", output, re.M)
    cleanup = (payload["kind"] == "complete" and payload["cleanupWaitSatisfied"] is True
        and payload["browserEvents"] == [] and manifest["cleanup_marker"] in output.splitlines()
        and manifest["cleanup_wait_seconds"] >= 15 and kill in (["0"], ["1"]) and wait == ["0"]
        and not manifest["browser"]["timed_out"] and not manifest["browser"]["exited_early"])
    extensions = [json.loads(value) for value in re.findall(r"MASK_EXTENSION_CONTROL (\{[^\r\n]*?\})", chrome.replace('\\"', '"'))]
    extension_verified = bool(extensions) and all(row["denied"] == (args.extension == "absent")
        and row["available"] == (args.extension == "present") for row in extensions)
    errors = diagnostics(chrome)
    resource_errors = [line for line in guest.splitlines() if re.search(
        r"does not have any up to date location\.|Device released with resources still bound\.|Leftover resource |Context array not freed!", line)]
    shader_errors = [line for line in guest.splitlines() if ":err:d3d_shader:" in line]
    unchanged = all(identity(Path(row["path"])) == row for row in inputs)
    passed = (result.passed and manifest["result"]["exit_status"] == 0 and audit["passed"] and coverage
        and cleanup and extension_verified and unchanged and manifest["input_identity_verified"]
        and not manifest["input_identity_problems"] and not errors and not resource_errors
        and not shader_errors and capture_error is None)
    record = dict(passed=passed, kind=args.kind, mode=args.mode, extension=args.extension,
        audit=audit, coverage=coverage, cleanup=cleanup, kill_statuses=kill, wait_statuses=wait,
        extension_verified=extension_verified, extension_control=extensions, diagnostics=errors,
        resource_errors=resource_errors, shader_errors=shader_errors, inputs=inputs, inputs_unchanged=unchanged,
        capture_method=capture_method, capture_error=capture_error, guest_log=identity(guest_path),
        manifest=identity(args.output / "manifest.json"), payload=identity(payload_path), chrome_log=identity(chrome_path))
    (args.output / "sample-mask-audit.json").write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({key: record[key] for key in ("passed", "kind", "mode", "extension", "coverage", "cleanup", "extension_verified")}))
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
