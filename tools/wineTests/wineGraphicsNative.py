#!/usr/bin/env python3
"""Native BoxedWine and Wine controls for pristine Wine graphics tests."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import signal
import subprocess
import time
import zipfile

import wineGraphicsBrowser as graphics

WINE_COMMIT = "db11d0fe6a169c457e23d007e20404643d067aa8"
GROUPS = {
    "graphics-probe": ("probe", "abandon"),
    "d3d11-probe": ("probe",),
    "vulkan-1": ("vulkan",), "d3d8": ("device", "stateblock", "visual"),
    "d3d9": ("d3d9ex", "device", "stateblock", "visual"),
    "d3d10": ("device", "effect"), "d3d10_1": ("d3d10_1",),
    "d3d11": ("d3d11",), "dxgi": ("dxgi",),
}
DXVK_DLLS = ("d3d8.dll", "d3d9.dll", "d3d10core.dll", "d3d11.dll", "dxgi.dll")


def sha256(path):
    with Path(path).open("rb") as src:
        return hashlib.file_digest(src, "sha256").hexdigest()


def extract_test(archive, suite, destination):
    name = f"{suite}_test.exe"
    with zipfile.ZipFile(archive) as src:
        manifest = json.loads(src.read("manifest.json"))
        if (manifest.get("wine_commit") != WINE_COMMIT or
                manifest.get("test_patches") != [] or manifest.get("architecture") != "i386"):
            raise ValueError("native graphics requires the pristine Wine 11.0 i386 bundle")
        image = src.read(name)
        if hashlib.sha256(image).hexdigest() != manifest["sha256"][name]:
            raise ValueError(f"test hash mismatch: {name}")
        graphics._validate_pe32_i386(image, name)
        destination.write_bytes(image)
    return manifest


def run_process(command, cwd, environment, timeout, log):
    """Retain output and terminate only this invocation's process tree on timeout."""
    options = {"start_new_session": True} if os.name != "nt" else {
        "creationflags": subprocess.CREATE_NEW_PROCESS_GROUP | subprocess.CREATE_NO_WINDOW}
    started = time.monotonic()
    with Path(log).open("wb") as output:
        process = subprocess.Popen(command, cwd=cwd, env=environment, stdout=output,
                                   stderr=subprocess.STDOUT, **options)
        timed_out = False
        try:
            process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            timed_out = True
            if os.name == "nt":
                subprocess.run(["taskkill", "/PID", str(process.pid), "/T", "/F"],
                               stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT, check=False)
                if process.poll() is None:
                    process.kill()
            else:
                os.killpg(process.pid, signal.SIGKILL)
            process.wait(timeout=30)
    return {"exit_code": process.returncode, "timed_out": timed_out,
            "seconds": time.monotonic() - started}


def environment_for(renderer):
    dlls = "d3d8,d3d9,d3d10core,d3d11,dxgi"
    environment = {"WINEARCH": "win32", "WINEDEBUG": "-all,+loaddll",
                   "WINEDLLOVERRIDES": f"mscoree,mshtml=;{dlls}=b;vulkan-1,winevulkan=b",
                   "DXVK_LOG_LEVEL": "info", "DXVK_LOG_PATH": "none"}
    if renderer == "dxvk":
        environment["WINEDLLOVERRIDES"] = f"mscoree,mshtml=;{dlls}=n;vulkan-1,winevulkan=b"
    elif renderer.startswith("wined3d-"):
        environment["WINE_D3D_CONFIG"] = "renderer=" + renderer.removeprefix("wined3d-")
    return environment


def backend_evidence(output, renderer):
    loaded = [line for line in output.splitlines()
              if re.search(r"^[0-9a-f]+:trace:loaddll:.* Loaded ", line, re.I)]
    if renderer == "dxvk":
        versions = sorted(set(re.findall(r"DXVK:\s+v?([\w.+-]+)", output)))
        native = any(re.search(r"(?:d3d8|d3d9|d3d11|dxgi)\.dll.*native", line, re.I)
                     for line in loaded)
        return {"verified": bool(versions) and native, "versions": versions,
                "loaded_graphics_dlls": [line for line in loaded if re.search(r"d3d|dxgi|vulkan", line, re.I)]}
    vulkan = any("winevulkan" in line.lower() for line in loaded)
    wined3d = any("wined3d" in line.lower() and "builtin" in line.lower() for line in loaded)
    opengl = any("opengl32" in line.lower() for line in loaded)
    verified = vulkan if renderer == "vulkan" else wined3d and (
        vulkan if renderer == "wined3d-vulkan" else opengl)
    return {"verified": verified, "loaded_graphics_dlls": [line for line in loaded
            if re.search(r"wined3d|opengl32|vulkan", line, re.I)]}


def assess(output, group, renderer, process, reference=None):
    output = graphics.normalize_output(output)
    counts = graphics._summary_for_group(group, output)
    failures = graphics._failure_records(output)
    skips = tuple(line.strip() for line in output.splitlines() if "Tests skipped:" in line)
    evidence = backend_evidence(output, renderer)
    reasons = []
    exits = re.findall(r"(?m)^BOXEDWINE_TEST_EXIT:(\d+)\s*$", output)
    if process["timed_out"]:
        reasons.append("test timed out")
    if counts is None:
        reasons.append("missing Wine summary")
    elif counts[0] == 0:
        reasons.append("no assertions executed")
    if len(exits) != 1 or (counts is not None and int(exits[0]) != min(counts[2], 255)):
        reasons.append("missing or inconsistent test exit status")
    if not re.search(r"(?m)^BOXEDWINE_WINESERVER_CLEANUP_OK\s*$", output):
        reasons.append("Wine cleanup did not complete")
    if process["exit_code"] not in process.get("expected_exit_codes", (0,)):
        reasons.append(f"runner exited {process['exit_code']}")
    if not evidence["verified"]:
        reasons.append("requested rendering backend was not observed")
    validation_errors = tuple(line.strip() for line in output.splitlines()
                              if "Validation Error" in line or "Vulkan validation ERROR" in line)
    if validation_errors:
        reasons.append("Vulkan validation reported errors")
    infrastructure_ok = not reasons
    signature = {"counts": list(counts) if counts else None,
                 "failure_records": list(failures), "skip_records": list(skips)}
    if reference is not None:
        if not reference.get("infrastructure_ok"):
            reasons.append("reference did not complete with a verified backend")
        if signature != reference["signature"]:
            reasons.append("counts or failure/skip identities differ from reference")
    elif counts is not None and (counts[2] or failures):
        reasons.append("Wine assertion failures (no accepted reference)")
    return {"passed": not reasons, "infrastructure_ok": infrastructure_ok,
            "reasons": reasons, "signature": signature,
            "backend": evidence, "validation_errors": list(validation_errors)}


def boxedwine_command(args, root, executable, group, guest_env):
    command = [str(args.boxedwine.resolve()), "-root", str(root), "-zip", str(args.filesystem.resolve()),
               "-w", "/home/username"]
    command += getattr(args, "boxedwine_arg", [])
    # Presentation tests require a real SDL surface: intentionally no -novideo.
    for key, value in guest_env.items():
        command += ["-env", f"{key}={value}"]
    script = (f"/bin/wine /home/username/{shlex.quote(executable.name)} {shlex.quote(group)}; "
              "rc=$?; echo BOXEDWINE_TEST_EXIT:$rc; "
              "/opt/wine/bin/wineserver -k; /opt/wine/bin/wineserver -w && "
              "echo BOXEDWINE_WINESERVER_CLEANUP_OK")
    return command + ["/bin/sh", "-c", "{ " + script + "; } > /home/username/guest.log 2>&1"]


def parse_arguments(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--runtime", choices=("boxedwine", "wine"), required=True)
    parser.add_argument("--renderer", choices=("vulkan", "wined3d-gl", "wined3d-vulkan", "dxvk"), required=True)
    parser.add_argument("--boxedwine", type=Path)
    parser.add_argument("--boxedwine-arg", action="append", default=[])
    parser.add_argument("--filesystem", type=Path)
    parser.add_argument("--wine-root", type=Path)
    parser.add_argument("--guest-vulkan", type=Path)
    parser.add_argument("--dxvk-dir", type=Path)
    parser.add_argument("--dxvk-option", action="append", default=[])
    binaries = parser.add_mutually_exclusive_group(required=True)
    binaries.add_argument("--tests-archive", type=Path)
    binaries.add_argument("--probe", type=Path)
    parser.add_argument("--suite", choices=tuple(GROUPS), required=True)
    parser.add_argument("--group", action="append")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--baseline", type=Path)
    parser.add_argument("--timeout", type=int, default=300)
    parser.add_argument("--host-env", action="append", default=[])
    args = parser.parse_args(argv)
    if args.timeout < 1:
        parser.error("timeout must be positive")
    if args.suite.endswith("-probe") != (args.probe is not None):
        parser.error("probe suites require --probe; Wine suites require --tests-archive")
    if args.runtime == "boxedwine" and (args.boxedwine is None or args.filesystem is None):
        parser.error("BoxedWine requires --boxedwine and --filesystem")
    if args.runtime == "wine" and (os.name == "nt" or args.wine_root is None):
        parser.error("native Wine requires Linux and --wine-root")
    if args.renderer == "dxvk" and args.dxvk_dir is None:
        parser.error("DXVK requires --dxvk-dir")
    if args.dxvk_option and args.renderer != "dxvk":
        parser.error("DXVK options require renderer=dxvk")
    if (args.suite == "vulkan-1") != (args.renderer == "vulkan"):
        parser.error("the Vulkan suite requires renderer=vulkan; D3D suites require a D3D renderer")
    args.group = args.group or list(GROUPS[args.suite])
    if any(group not in GROUPS[args.suite] for group in args.group) or len(set(args.group)) != len(args.group):
        parser.error("groups must be unique and belong to the selected suite")
    return args


def main(argv=None):
    args = parse_arguments(argv)
    run_dir = args.output.resolve()
    run_dir.mkdir(parents=True, exist_ok=False)
    manifest = {"schema_version": 1, "runtime": args.runtime, "renderer": args.renderer,
                "suite": args.suite, "inputs": {}, "results": {}, "complete": False}
    try:
        inputs = manifest["inputs"]
        if args.runtime == "boxedwine":
            inputs["boxedwine_arguments"] = args.boxedwine_arg
        if args.tests_archive:
            inputs["tests_archive_sha256"] = sha256(args.tests_archive)
        for name in ("boxedwine", "filesystem", "guest_vulkan"):
            path = getattr(args, name)
            if path is not None:
                inputs[name] = {"path": str(path.resolve()), "sha256": sha256(path)}
        executable = run_dir / f"{args.suite}_test.exe"
        if args.probe:
            image = args.probe.read_bytes()
            graphics._validate_pe32_i386(image, str(args.probe))
            executable.write_bytes(image)
            inputs["test_bundle"] = {"source": "local graphics probe", "architecture": "i386"}
        else:
            inputs["test_bundle"] = extract_test(args.tests_archive, args.suite, executable)
        inputs["test_sha256"] = sha256(executable)
        if args.dxvk_dir:
            inputs["dxvk"] = {name: sha256(args.dxvk_dir / name) for name in DXVK_DLLS}
            inputs["dxvk_options"] = args.dxvk_option
        reference = json.loads(args.baseline.read_text()) if args.baseline else None
        if reference and (not reference.get("complete") or reference["renderer"] != args.renderer or reference["suite"] != args.suite or
                          reference["inputs"]["test_sha256"] != inputs["test_sha256"] or
                          reference["inputs"].get("dxvk") != inputs.get("dxvk") or
                          reference["inputs"].get("dxvk_options", []) != inputs.get("dxvk_options", [])):
            raise ValueError("reference renderer, test binary, and DXVK inputs must match")
        host_env = os.environ.copy()
        for key in ("WINEPREFIX", "WINEARCH", "WINEDEBUG", "WINEDLLOVERRIDES", "WINE_D3D_CONFIG",
                    "DXVK_CONFIG_FILE", "DXVK_LOG_PATH", "DXVK_LOG_LEVEL"):
            host_env.pop(key, None)
        for entry in args.host_env:
            key, value = entry.split("=", 1)
            host_env[key] = value
        manifest["host_environment_overrides"] = args.host_env
        if args.runtime == "wine":
            wine = args.wine_root.resolve() / "loader/wine"
            wineserver = args.wine_root.resolve() / "server/wineserver"
            inputs["wine"] = {"path": str(wine), "sha256": sha256(wine),
                              "wineserver_sha256": sha256(wineserver)}
        for group in args.group:
            group_dir = run_dir / group
            group_dir.mkdir()
            guest_env = environment_for(args.renderer)
            if args.dxvk_option:
                guest_env["DXVK_CONFIG_FILE"] = r"C:\dxvk.conf"
            if args.runtime == "boxedwine":
                root = group_dir / "root"
                guest_dir = root / "home/username"
                guest_dir.mkdir(parents=True)
                shutil.copy2(executable, guest_dir / executable.name)
                prefix = guest_dir / ".wine"
                if args.guest_vulkan:
                    (root / "lib").mkdir()
                    shutil.copy2(args.guest_vulkan, root / "lib/libvulkan.so.1")
                command = boxedwine_command(args, root, executable, group, guest_env)
            else:
                prefix = group_dir / "prefix"
                guest_env["WINEPREFIX"] = str(prefix)
                host_env.update(environment_for("vulkan"), WINEPREFIX=str(prefix))
                bootstrap = run_process([str(wine), "wineboot", "-u"], group_dir, host_env,
                                        min(args.timeout, 120), group_dir / "wineboot.log")
                host_env.update(guest_env)
                if bootstrap["exit_code"] or bootstrap["timed_out"]:
                    run_process([str(wineserver), "-k"], group_dir, host_env, 30, group_dir / "cleanup.log")
                    raise RuntimeError(f"Wine prefix initialization failed: {group}")
                script = (f"{shlex.quote(str(wine))} {shlex.quote(str(executable))} {shlex.quote(group)}; "
                          "rc=$?; echo BOXEDWINE_TEST_EXIT:$rc; "
                          f"{shlex.quote(str(wineserver))} -k; {shlex.quote(str(wineserver))} -w && "
                          "echo BOXEDWINE_WINESERVER_CLEANUP_OK")
                command = ["/bin/sh", "-c", script]
            if args.dxvk_dir:
                system32 = prefix / "drive_c/windows/system32"
                system32.mkdir(parents=True, exist_ok=True)
                for name in DXVK_DLLS:
                    target = system32 / name
                    # wineboot may create a symlink into the Wine build tree.
                    if target.is_symlink():
                        target.unlink()
                    shutil.copy2(args.dxvk_dir / name, target)
                if args.dxvk_option:
                    (prefix / "drive_c/dxvk.conf").write_text("\n".join(args.dxvk_option) + "\n")
            log = group_dir / "output.log"
            process = run_process(command, group_dir, host_env, args.timeout, log)
            if args.runtime == "boxedwine":
                # Recorder-enabled builds return 1 on normal shutdown. Require the
                # independent guest summary, exit marker, and cleanup marker too.
                process["expected_exit_codes"] = [0, 1]
                log = guest_dir / "guest.log"
            elif process["timed_out"]:
                run_process([str(wineserver), "-k"], group_dir, host_env, 30, group_dir / "cleanup.log")
                run_process([str(wineserver), "-w"], group_dir, host_env, 30, group_dir / "cleanup-wait.log")
            output = log.read_text(errors="replace") if log.exists() else ""
            if args.runtime == "boxedwine":
                output += "\n" + (group_dir / "output.log").read_text(errors="replace")
            result = assess(output, group, args.renderer, process,
                            reference["results"][group] if reference else None)
            result.update(process=process, command=command, guest_environment=guest_env, log=str(log))
            manifest["results"][group] = result
            print(f"{args.suite}/{group}: {'PASS' if result['passed'] else 'FAIL'} {result['reasons']}", flush=True)
            (run_dir / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
        manifest["complete"] = True
    except (OSError, ValueError, KeyError, RuntimeError, subprocess.TimeoutExpired,
            graphics.RunnerError, zipfile.BadZipFile) as error:
        manifest["error"] = str(error)
        print(f"Native graphics error: {error}", flush=True)
    finally:
        (run_dir / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    return 0 if manifest["complete"] and all(r["passed"] for r in manifest["results"].values()) else 1


if __name__ == "__main__":
    raise SystemExit(main())
