#!/usr/bin/env python3
"""Package the installed SimTower demo and its WinG/sound files as a C: overlay."""

import argparse
import hashlib
import json
from pathlib import Path
import stat
import zipfile


SOURCE_SHA256 = "409b3c1bb7614216a2708c687f9664f75539107419ba0b5c5654d70e7d410bb2"
DRIVE = "home/username/.wine/drive_c/"
# Original files from towerdmo.zip, installed to C:\SIMTDEMO by its setup.
# Pin every file so a full game or a user's replacement DLL cannot enter the demo.
FILES = {
    "SIMTDEMO/INFO.EXE": "e9173fb13035f81eddd17c3dd2159b526a4cb2aefc6f77a479f10c087fef8911",
    "SIMTDEMO/SIMTDEMO.EXE": "f940630ddc9a341ec65d0bd0e02ea4dc7c14e5dd8f67072ee78ff4d699dc797b",
    "SIMTDEMO/UNIVBE.EXE": "4338e5e2f4d42e0a7b77aceddb4faf12aa9604b169787fe0e10ec6f019037453",
    "windows/system/DVA.386": "19f9c5a1d8dd6a93758741b82578c126263b2892e2b219d6f92ef17f0848bfa7",
    "windows/system/WAVMIX16.DLL": "1e40eee4846f65343bcfd18ba1b0644b7828666829fbcbeecbc8db8622b63985",
    "windows/system/WING.DLL": "4d21333b4997beecb667181bee4c1081b32be8f1d54ce0e71eeee173fecd3f52",
    "windows/system/WING32.DLL": "bb1f552e2525e784b61d2fe0ca23f3402adec05aa5f92f4c1dfbea3966a84cbb",
    "windows/system/WINGDE.DLL": "e3fdd4ee8cb81c4a88371c84364d9aabe90ea0b4286a0a7705929b4189a54fb5",
    "windows/system/WINGDIB.DRV": "26e2296e247eb7065a90eca42ee82bdb2df8d67d65b9f8f269ae6977223abf81",
    "windows/system/WINGPAL.WND": "d845868b8dc3551b2b9a3a1c3cd21ec9b7148ef79eed30c82cba83a1d09547a7",
    "windows/WAVEMIX.INI": "871891fa499f2e045c4f6d83f7f791f96681a4e2ff0f9079d47b4ec01f8c89ef",
}
LAUNCHER = b"@echo off\r\ncd /d C:\\SIMTDEMO\r\nSIMTDEMO.EXE\r\n"


def sha(data):
    return hashlib.sha256(data).hexdigest()


def prepare(root, source, output):
    root, source, output = Path(root).resolve(), Path(source), Path(output)
    if sha(source.read_bytes()) != SOURCE_SHA256:
        raise ValueError("source must be the pinned towerdmo.zip demo installer")
    files = {}
    for name, digest in FILES.items():
        path = root / DRIVE / name
        if not path.resolve().is_relative_to(root) or path.is_symlink() or not path.is_file():
            raise ValueError(f"missing or linked installed demo file: {name}")
        data = path.read_bytes()
        if sha(data) != digest:
            raise ValueError(f"installed demo file does not match towerdmo.zip: {name}")
        files[DRIVE + name] = data
    files[DRIVE + "simtower-demo.bat"] = LAUNCHER
    retained = []
    with zipfile.ZipFile(output, "x", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for name, data in sorted(files.items()):
            info = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            info.create_system = 3
            info.external_attr = (stat.S_IFREG | 0o644) << 16
            info.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(info, data, compresslevel=9)
            retained.append(dict(path=name, bytes=len(data), sha256=sha(data)))
    return dict(schema_version=1, source=dict(archive="towerdmo.zip", sha256=SOURCE_SHA256),
                output=dict(path=str(output.resolve()), bytes=output.stat().st_size, sha256=sha(output.read_bytes())),
                files=retained,
                excluded="Global Wine registries, installer caches, shortcuts, saved towers and user settings")


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("installed_root", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--report", type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        with args.report.open("x", encoding="utf-8", newline="\n") as report:
            result = prepare(args.installed_root, args.source, args.output)
            json.dump(result, report, indent=2)
            report.write("\n")
    except (OSError, ValueError) as error:
        parser.exit(2, f"SimTower demo preparation failed: {error}\n")
    print(f"Prepared {len(result['files'])} demo files: {args.output}")


if __name__ == "__main__":
    main()
