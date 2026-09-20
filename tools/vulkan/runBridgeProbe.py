#!/usr/bin/env python3
"""Run the bounded Vulkan probe natively or inside an isolated BoxedWine root."""
import argparse
import json
import os
from pathlib import Path
import re
import shutil
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "wineTests"))
from wineGraphicsNative import run_process, sha256


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--probe", type=Path, required=True)
    parser.add_argument("--boxedwine", type=Path)
    parser.add_argument("--filesystem", type=Path)
    parser.add_argument("--guest-vulkan", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--loops", type=int, default=4)
    parser.add_argument("--timeout", type=int, default=90)
    parser.add_argument("--host-env", action="append", default=[])
    args = parser.parse_args()
    if not 1 <= args.loops <= 100 or args.timeout < 1:
        parser.error("loops must be 1..100 and timeout must be positive")
    if args.boxedwine and (not args.filesystem or not args.guest_vulkan):
        parser.error("BoxedWine requires the filesystem and matching guest Vulkan shim")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    manifest = {"schema_version": 1, "passed": False, "inputs": {}, "reasons": [],
                "host_environment_overrides": args.host_env}
    try:
        for name in ("probe", "boxedwine", "filesystem", "guest_vulkan"):
            path = getattr(args, name)
            if path:
                manifest["inputs"][name] = {"path": str(path.resolve()), "sha256": sha256(path)}
        environment = os.environ.copy()
        for value in args.host_env:
            key, content = value.split("=", 1)
            environment[key] = content
        host_log = output / "host.log"
        if args.boxedwine:
            root = output / "root"
            guest = root / "home/username"
            guest.mkdir(parents=True)
            (root / "lib").mkdir()
            shutil.copy2(args.probe, guest / "bridge_probe")
            shutil.copy2(args.guest_vulkan, root / "lib/libvulkan.so.1")
            command = [str(args.boxedwine.resolve()), "-root", str(root), "-zip", str(args.filesystem.resolve()),
                       "-w", "/home/username", "/bin/sh", "-c",
                       f"/home/username/bridge_probe {args.loops} > /home/username/probe.log 2>&1; "
                       "rc=$?; echo BRIDGE_PROBE_EXIT:$rc >> /home/username/probe.log"]
            guest_log = guest / "probe.log"
        else:
            command = [str(args.probe.resolve()), str(args.loops)]
            guest_log = host_log
        manifest["command"] = command
        process = run_process(command, output, environment, args.timeout, host_log)
        manifest["process"] = process
        log = guest_log.read_text(errors="replace") if guest_log.exists() else ""
        host = host_log.read_text(errors="replace")
        expected = f"BRIDGE_PROBE_PASS:loops={args.loops} devices={args.loops * 2} submissions={args.loops * 20} readback_words={args.loops * 16544}"
        reasons = manifest["reasons"]
        if process["timed_out"]:
            reasons.append("probe timed out")
        if process["exit_code"] not in ((0, 1) if args.boxedwine else (0,)):
            reasons.append("unexpected host exit status")
        if log.splitlines().count(expected) != 1 or "BRIDGE_PROBE_FAIL:" in log:
            reasons.append("mandatory probe checks did not complete")
        if args.boxedwine and log.splitlines().count("BRIDGE_PROBE_EXIT:0") != 1:
            reasons.append("missing successful guest exit marker")
        errors = [line for line in (host + "\n" + log).splitlines()
                  if "Validation Error" in line or "Vulkan validation ERROR" in line]
        manifest["validation_errors"] = sorted(set(errors))
        if errors:
            reasons.append("host validation reported errors")
        if args.boxedwine and "BOXEDWINE_VULKAN_VALIDATION" in environment and "Vulkan host validation enabled" not in host:
            reasons.append("host validation was requested but not observed")
        manifest["gpu"] = re.findall(r"BRIDGE_PROBE_GPU:(.*)", log)
        manifest["passed"] = not reasons
    except (OSError, ValueError, RuntimeError) as error:
        manifest["reasons"].append(str(error))
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print(f"Vulkan bridge probe: {'PASS' if manifest['passed'] else 'FAIL'} {manifest['reasons']}")
    return 0 if manifest["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
