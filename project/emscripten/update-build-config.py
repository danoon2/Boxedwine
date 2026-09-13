#!/usr/bin/env python3
"""Update a make dependency only when compiler or build settings change."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile


COMPILE_KEYS = (
    "CC", "CXX", "AS", "CPPFLAGS", "CFLAGS", "CXXFLAGS", "ASFLAGS",
    "COMMON_INCLUDES", "SOFTFLOAT_INCLUDES", "WASM_EXCEPTION_CPPFLAGS",
)
LINK_KEYS = ("CC", "LDFLAGS", "OBJECTS", "TARGET")
COMPILER_ENVIRONMENT_KEYS = ("EMCC_CFLAGS", "SOURCE_DATE_EPOCH")


def compiler_identity(command: str) -> dict:
    arguments = shlex.split(command)
    if not arguments:
        raise ValueError("missing compiler command")
    executable = shutil.which(arguments[0])
    if not executable:
        raise ValueError(f"compiler not found: {arguments[0]}")
    result = subprocess.run(arguments + ["--version"], check=True,
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    return {"command": arguments, "path": str(Path(executable).resolve()),
        "version": result.stdout.strip()}


def configuration(kind: str, environment: dict) -> dict:
    keys = COMPILE_KEYS if kind == "compile" else LINK_KEYS
    settings = {key: environment.get("BW_BUILD_" + key, "") for key in keys}
    compilers = ("CC", "CXX") if kind == "compile" else ("CC",)
    return {"schema_version": 1, "kind": kind, "settings": settings,
        # Emscripten injects EMCC_CFLAGS after its --version early return.
        # SOURCE_DATE_EPOCH also changes compiler-generated timestamp bytes.
        # Neither change is visible in make flags or compiler --version output.
        "environment": {key: environment.get(key) for key in COMPILER_ENVIRONMENT_KEYS},
        "compilers": {key: compiler_identity(settings[key]) for key in compilers},
        "helper_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}


def outputs_missing(value: dict) -> bool:
    if value["kind"] != "link":
        return False
    target = Path(value["settings"]["TARGET"])
    outputs = [target]
    if target.suffix in (".html", ".js"):
        outputs.extend((target.with_suffix(".js"), target.with_suffix(".wasm")))
    return any(not path.is_file() for path in outputs)


def update(path: Path, value: dict, *, force: bool = False) -> bool:
    data = (json.dumps(value, sort_keys=True, indent=2) + "\n").encode("utf-8")
    if not force and path.exists() and path.read_bytes() == data:
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=path.parent, prefix=path.name + ".",
                delete=False) as stream:
            temporary = Path(stream.name)
            stream.write(data)
        os.replace(temporary, path)
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)
    return True


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("kind", choices=("compile", "link"))
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    try:
        value = configuration(args.kind, os.environ)
        missing = outputs_missing(value)
        changed = update(args.output, value, force=missing)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        parser.exit(1, f"build configuration failed: {error}\n")
    if changed:
        reason = "outputs missing" if missing else "configuration changed"
        print(f"Build dependency updated: {args.kind}, {reason} ({args.output})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
