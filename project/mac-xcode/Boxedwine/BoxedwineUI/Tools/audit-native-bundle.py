#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Read-only checks of a built native preview; never signs, runs, or uploads it.

Uses Xcode's Mach-O tools, not third-party Python packages. Checks the declared
architectures, deployment floor, embedded load-command graph, and signatures.
System libraries are supplied by macOS's shared cache and are not opened here.
This is not an App Store validator or a complete audit of arbitrary dlopen calls.
"""

import argparse
import hashlib
import importlib.util
import json
import plistlib
import re
import subprocess
import sys
from pathlib import Path

sys.dont_write_bytecode = True


MACH_MAGICS = {bytes.fromhex(x) for x in (
    "cffaedfe", "cefaedfe", "feedfacf", "feedface", "cafebabe", "bebafeca", "cafebabf", "bfbafeca"
)}
LOAD_COMMANDS = {"LC_LOAD_DYLIB", "LC_LOAD_WEAK_DYLIB", "LC_REEXPORT_DYLIB",
                 "LC_LOAD_UPWARD_DYLIB", "LC_LAZY_LOAD_DYLIB"}
RETIRED_MESA_LIBRARIES = {
    "libOSMesa.8.dylib", "libglapi.0.dylib", "libLLVM.dylib", "libffi.8.dylib",
    "libedit.0.dylib", "libncurses.6.dylib", "libz.1.dylib", "libzstd.1.dylib",
    "libxml2.2.dylib", "liblzma.5.dylib", "libiconv.2.dylib", "libicuuc.76.dylib",
    "libicudata.76.dylib", "libc++.1.dylib", "libc++abi.1.dylib"
}


def run(*args):
    result = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode:
        raise ValueError(f"{args[0]} failed: {result.stderr.decode(errors='replace').strip()}")
    return result.stdout


def version(value):
    if not re.fullmatch(r"\d+(?:\.\d+){0,2}", str(value)):
        raise ValueError(f"Invalid macOS version: {value!r}")
    parts = tuple(int(x) for x in str(value).split("."))
    return parts + (0,) * (3 - len(parts))


def system_path(path):
    return path.startswith("/") and str(Path(path).resolve()).startswith(("/usr/lib/", "/System/Library/"))


def within(path, parent):
    return path == parent or parent in path.parents


def digest(path):
    sha = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            sha.update(chunk)
    return sha.hexdigest()


def load_commands(path, arch):
    output = run("/usr/bin/otool", "-arch", arch, "-l", str(path)).decode()
    result = {"dependencies": [], "rpaths": [], "minimumOS": None, "uuid": None}
    for block in re.split(r"Load command \d+\n", output)[1:]:
        cmd = re.search(r"^\s*cmd (\S+)", block, re.M)
        if not cmd:
            continue
        cmd = cmd[1]
        if cmd == "LC_UUID":
            match = re.search(r"^\s*uuid (\S+)", block, re.M)
            if match:
                result["uuid"] = match[1].upper()
        if cmd in LOAD_COMMANDS or cmd == "LC_RPATH":
            field = "path" if cmd == "LC_RPATH" else "name"
            match = re.search(r"^\s*" + field + r" (.+) \(offset \d+\)", block, re.M)
            if not match:
                raise ValueError(f"Cannot read {cmd} in {path}")
            result["rpaths" if cmd == "LC_RPATH" else "dependencies"].append(match[1])
        elif cmd in ("LC_BUILD_VERSION", "LC_VERSION_MIN_MACOSX"):
            if cmd == "LC_BUILD_VERSION" and not re.search(r"^\s*platform (?:1|MACOS)\s*$", block, re.M):
                raise ValueError(f"Non-macOS slice in {path} ({arch})")
            field = "minos" if cmd == "LC_BUILD_VERSION" else "version"
            match = re.search(r"^\s*" + field + r" (\S+)", block, re.M)
            if match:
                result["minimumOS"] = match[1]
    return result


def parse_signature(text):
    flags = re.search(r"flags=0x([0-9a-fA-F]+)", text)
    team = re.search(r"^TeamIdentifier=(.+)$", text, re.M)
    return {
        "adHocSigned": "Signature=adhoc" in text,
        "hardenedRuntime": bool(flags and (int(flags[1], 16) & 0x10000)),
        "teamIdentifier": team[1] if team and team[1] != "not set" else None,
        "developerID": bool(re.search(r"^Authority=Developer ID Application:", text, re.M)),
        "secureTimestamp": bool(re.search(r"^Timestamp=.+$", text, re.M)),
    }


def code_signature(path):
    result = subprocess.run(["/usr/bin/codesign", "-d", "--verbose=4", str(path)], capture_output=True)
    if result.returncode:
        raise ValueError(f"Cannot inspect code signature: {path}")
    return parse_signature(result.stderr.decode(errors="replace"))


def distribution_errors(signature, team):
    errors = []
    if signature["adHocSigned"] or not signature["developerID"]:
        errors.append("Developer ID Application signature is required")
    if not team or signature["teamIdentifier"] != team:
        errors.append("Code must share the launcher's signing team")
    if not signature["secureTimestamp"]:
        errors.append("Secure signing timestamp is missing")
    if not signature["hardenedRuntime"]:
        errors.append("Hardened Runtime is missing")
    return errors


def entitlement_errors(configuration, launcher, runtime):
    errors = []
    development = configuration == "Debug"
    for label, entitlements in (("Launcher", launcher), ("Boxedwine", runtime)):
        if development:
            if entitlements.get("com.apple.security.app-sandbox") or entitlements.get("com.apple.security.inherit"):
                errors.append(f"Development {label} must not enable sandboxing or inheritance")
            if entitlements.get("com.apple.security.get-task-allow") is not True:
                errors.append(f"Development {label} does not allow debugger attachment")
        else:
            if entitlements.get("com.apple.security.app-sandbox") is not True:
                errors.append(f"App Sandbox is missing: {label}")
            if entitlements.get("com.apple.security.get-task-allow"):
                errors.append(f"{configuration} permits debugger attachment: {label}")
    if not development:
        if launcher.get("com.apple.security.files.user-selected.read-write") is not True:
            errors.append("Launcher is missing read/write permission for user-selected files")
        if launcher.get("com.apple.security.inherit"):
            errors.append("Launcher must define its own sandbox, not inherit one")
        for direction in ("client", "server"):
            if launcher.get(f"com.apple.security.network.{direction}") is not True:
                errors.append("Launcher is missing network permission for guest apps: " + direction)
        if runtime.get("com.apple.security.inherit") is not True:
            errors.append("Runtime does not declare sandbox inheritance")
        sandbox_keys = {k for k in runtime if k.startswith("com.apple.security.") and not k.startswith("com.apple.security.cs.")}
        if sandbox_keys != {"com.apple.security.app-sandbox", "com.apple.security.inherit"}:
            errors.append("Runtime has sandbox keys beyond app-sandbox and inherit")
    return errors


def audit(app, configuration="Release", signatures=True, distribution=False, require_demo_catalog=False):
    if distribution and (not signatures or configuration != "Release"):
        raise ValueError("Distribution checks require Release and signature verification")
    app = app.resolve(strict=True)
    report = {"bundle": str(app), "configuration": configuration,
              "errors": [], "notes": [], "images": [], "applications": [],
              "signaturesVerified": False, "distribution": distribution}
    errors = report["errors"]
    images = {}

    def label(path):
        return str(path.relative_to(app)) if within(path, app) else str(path)

    def expand(value, loader, executable):
        if value == "@loader_path" or value.startswith("@loader_path/"):
            return Path(str(loader.parent) + value[len("@loader_path"):]).resolve()
        if value == "@executable_path" or value.startswith("@executable_path/"):
            return Path(str(executable.parent) + value[len("@executable_path"):]).resolve()
        if value.startswith("/"):
            return Path(value).resolve()
        return None

    runtime = app / "Contents/Helpers/BoxedwineEngine.app"
    for obsolete in ("BoxedwineRuntime.app", "Boxedwine.app"):
        if (app / "Contents/Helpers" / obsolete).exists():
            errors.append(f"The obsolete {obsolete} helper remains bundled")
    roots = []
    for bundle in (app, runtime):
        info = plistlib.loads((bundle / "Contents/Info.plist").read_bytes())
        executable_name = info.get("CFBundleExecutable", "")
        if not executable_name or Path(executable_name).name != executable_name:
            raise ValueError(f"Invalid executable in {bundle}")
        executable = (bundle / "Contents/MacOS" / executable_name).resolve(strict=True)
        if not within(executable, app):
            raise ValueError(f"Executable escapes bundle: {executable}")
        floor = info.get("LSMinimumSystemVersion")
        version(floor)
        roots.append(executable)
        report["applications"].append({"path": label(bundle), "identifier": info.get("CFBundleIdentifier"),
            "version": info.get("CFBundleShortVersionString"), "build": info.get("CFBundleVersion"),
            "minimumOS": floor})
    main_info, helper_info = report["applications"]
    for field in ("version", "build", "minimumOS"):
        if not main_info[field] or main_info[field] != helper_info[field]:
            errors.append(f"Launcher/runtime {field} differs or is missing")
    floor = version(main_info["minimumOS"])

    # Do not follow directory symlinks. Framework aliases are inventoried once,
    # and every alias (including resource links) must stay within the app.
    for path in sorted(app.rglob("*")):
        if path.name in RETIRED_MESA_LIBRARIES:
            errors.append(f"Retired Mesa dependency remains bundled: {label(path)}")
        if path.is_symlink():
            target = path.resolve()
            if not within(target, app) or not target.exists():
                errors.append(f"Broken or escaping bundle link: {label(path)}")
            continue
        if not path.is_file():
            continue
        with path.open("rb") as stream:
            if stream.read(4) not in MACH_MAGICS:
                continue
        archs = run("/usr/bin/lipo", "-archs", str(path)).decode().split()
        image = {"path": label(path), "sha256": digest(path), "architectures": archs,
                 "slices": {arch: load_commands(path, arch) for arch in archs}}
        images[path] = image
        report["images"].append(image)
        if configuration == "Release" and (path.name.endswith(".debug.dylib") or path.name == "__preview.dylib"):
            errors.append(f"Debug image in Release: {label(path)}")

    if any(root not in images for root in roots):
        raise ValueError("Launcher or runtime is not a Mach-O executable")
    archs = images[roots[0]]["architectures"]
    report["architectures"] = archs
    for path, image in images.items():
        for arch in archs:
            if arch not in image["slices"]:
                errors.append(f"{label(path)} lacks {arch}")
                continue
            minimum = image["slices"][arch]["minimumOS"]
            if not minimum:
                errors.append(f"{label(path)} ({arch}) has no macOS deployment version")
            elif version(minimum) > floor:
                errors.append(f"{label(path)} ({arch}) needs macOS {minimum}, app declares {main_info['minimumOS']}")

    visited = set()

    def walk(path, executable, arch, inherited=()):
        image = images.get(path)
        if not image or arch not in image["slices"]:
            return
        data = image["slices"][arch]
        own = []
        for rpath in data["rpaths"]:
            resolved = expand(rpath, path, executable)
            if resolved is None or not (within(resolved, app) or system_path(str(resolved))):
                errors.append(f"Nonportable rpath in {label(path)} ({arch}): {rpath}")
            else:
                own.append(resolved)
        search = tuple(dict.fromkeys([*own, *inherited]))
        key = (path, executable, arch, search)
        if key in visited:
            return
        visited.add(key)
        for dep in data["dependencies"]:
            if system_path(dep):
                continue
            if dep.startswith("@rpath/"):
                candidates = [(folder / dep[len("@rpath/"):]).resolve() for folder in search]
                # System Swift runtime images can exist only in the shared cache.
                resolved = next((p for p in candidates if p.exists() or system_path(str(p))), None)
            else:
                resolved = expand(dep, path, executable)
            if resolved is not None and system_path(str(resolved)):
                continue
            if resolved is None or not within(resolved, app) or resolved not in images:
                errors.append(f"Unresolved/external dependency in {label(path)} ({arch}): {dep}")
                continue
            walk(resolved, executable, arch, search)

    for arch in archs:
        for root in roots:
            walk(root, root, arch)
        # Check dynamic libraries in the helper's load context even when absent
        # from static load commands (Vulkan entry points are loaded through SDL).
        runtime_paths = tuple(expand(p, roots[1], roots[1]) for p in
                              images[roots[1]]["slices"].get(arch, {}).get("rpaths", []))
        runtime_paths = tuple(p for p in runtime_paths if p is not None)
        reached = {key[0] for key in visited if key[2] == arch}
        for path in images:
            if path not in reached:
                walk(path, roots[1], arch, runtime_paths)

    resources = app / "Contents/Resources"
    report["demoCatalog"] = None
    if require_demo_catalog or distribution or (resources / "Demos").exists():
        try:
            tool = Path(__file__).resolve().parents[5] / "tools/demo_catalog.py"
            spec = importlib.util.spec_from_file_location("demo_catalog", tool)
            catalog = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(catalog)
            report["demoCatalog"] = catalog.audit_staged(resources / "Demos", catalog.read_pin(catalog.DEFAULT_LOCK))
        except (OSError, ValueError) as error:
            errors.append("Demo catalog: " + str(error))
    else:
        report["notes"].append("No demo catalog included; connect and rebuild to include the pinned catalog.")
    if not (resources / "Boxedwine-LICENSE.txt").is_file():
        errors.append("Boxedwine license text is missing")
    try:
        spec = importlib.util.spec_from_file_location("third_party_notices", Path(__file__).with_name("prepare-third-party-notices.py"))
        notices = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(notices)
        report["thirdPartyLicenses"] = notices.audit(resources / "Licenses")
        records = json.loads(notices.SOURCES.read_text())
        # Signing changes whole-file hashes but preserves Mach-O UUIDs. Original
        # build inputs are hash-checked during preparation; verify here that the
        # re-signed libraries still identify those reviewed builds.
        for relative, expected in records["bundleUUIDs"].items():
            path = (app / relative).resolve()
            if path in images:
                actual = {arch: info["uuid"] for arch, info in images[path]["slices"].items()}
                if actual != expected:
                    errors.append("Native library differs from source record: " + relative)
    except (OSError, ValueError, KeyError) as error:
        errors.append("Third-party licenses: " + str(error))
    wine = resources / "WindowsSupport/wine.zip"
    report["includedWine"] = {"bytes": wine.stat().st_size, "sha256": digest(wine)} if wine.is_file() else None
    if not wine.is_file():
        report["notes"].append("No included Wine package; the UI downloads Wine on demand after disclosing its size.")

    if signatures:
        try:
            run("/usr/bin/codesign", "--verify", "--deep", "--strict", str(app))
            for bundle, entry in zip((app, runtime), report["applications"]):
                entitlements = plistlib.loads(run("/usr/bin/codesign", "-d", "--entitlements", ":-", str(bundle)))
                entry["entitlements"] = entitlements
                entry.update(code_signature(bundle))
                if not entry["hardenedRuntime"]:
                    errors.append(f"Hardened Runtime is missing: {label(bundle)}")
            errors.extend(entitlement_errors(configuration, main_info["entitlements"], helper_info["entitlements"]))
            if distribution:
                # A valid outer signature alone does not establish that every
                # embedded binary was re-signed with Developer ID and a timestamp.
                for path, entry in images.items():
                    signature = code_signature(path)
                    entry["signature"] = signature
                    errors.extend(f"{problem}: {label(path)}" for problem in
                                  distribution_errors(signature, main_info["teamIdentifier"]))
            if (main_info["adHocSigned"] and main_info["hardenedRuntime"] and
                    not main_info["entitlements"].get("com.apple.security.cs.disable-library-validation") and
                    any(dep.endswith(".debug.dylib") for data in images[roots[0]]["slices"].values()
                        for dep in data["dependencies"])):
                errors.append("Ad-hoc hardened launcher loads a Swift debug dylib without a shared Team ID; set ENABLE_DEBUG_DYLIB=NO")
            report["signaturesVerified"] = True
        except ValueError as error:
            errors.append(str(error))
    else:
        report["notes"].append("Signature checks explicitly skipped.")
    report["notes"].append("This checks preview packaging, not App Store eligibility, licensing, privacy declarations, or runtime compatibility.")
    if configuration == "Debug":
        report["notes"].append("Development build: debugger attachment enabled; sandbox behavior requires the Sandbox or Release configuration.")
    if any(info.get("adHocSigned") for info in report["applications"]):
        report["notes"].append("Ad-hoc signed for local development; distribution signing/provisioning is outstanding.")
    report["errors"] = list(dict.fromkeys(errors))
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("app", type=Path)
    parser.add_argument("--configuration", choices=("Debug", "Sandbox", "Release"), default="Release")
    parser.add_argument("--skip-signatures", action="store_true", help="inspect an unsigned development fixture")
    parser.add_argument("--distribution", action="store_true", help="require Developer ID, matching signing teams, and secure timestamps for every embedded image")
    parser.add_argument("--require-demo-catalog", action="store_true", help="require the exact project catalog in this build")
    parser.add_argument("--json", type=Path, help="write the complete inventory to this file outside the app")
    args = parser.parse_args()
    if args.distribution and (args.skip_signatures or args.configuration != "Release"):
        parser.error("--distribution requires Release and cannot skip signatures")
    if args.json and within(args.json.resolve(), args.app.resolve()):
        parser.error("The report must be outside the app; this audit never edits its input bundle")
    try:
        report = audit(args.app, args.configuration, not args.skip_signatures, args.distribution, args.require_demo_catalog)
    except (OSError, ValueError, plistlib.InvalidFileException) as error:
        print(f"error: Native bundle audit failed: {error}", file=sys.stderr)
        return 1
    if args.json:
        args.json.write_text(json.dumps(report, indent=2) + "\n")
    print(f"Native bundle audit: {len(report['images'])} Mach-O images, {', '.join(report['architectures'])}, macOS {report['applications'][0]['minimumOS']}+")
    for error in report["errors"]:
        print(f"error: {error}", file=sys.stderr)
    for note in report["notes"]:
        print(f"note: {note}")
    if report["errors"]:
        print(f"FAILED: {len(report['errors'])} packaging issue(s).")
        return 1
    print("Preview packaging checks passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
