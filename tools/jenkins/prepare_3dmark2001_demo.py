#!/usr/bin/env python3
"""Package an installed 3DMark2001 SE as a C: drive overlay for the demos."""

import argparse
import hashlib
import json
from pathlib import Path
import stat
import zipfile


DRIVE = "home/username/.wine/drive_c/"
APP = "Program Files/MadOnion.com/3DMark2001 SE/"
REQUIRED = ("3DMark2001SE.exe", "data.ras", "textures.ras", "customizable.ras",
            "e2mfc.dll", "e2driver/e2_d3d8_driver_mfc.dll", "mfc42.dll")
LAUNCHER = r'''@echo off
cd /d "C:\Program Files\MadOnion.com\3DMark2001 SE"
3DMark2001SE.exe %*
'''


def prepare(root, output):
    root, output = Path(root).resolve(), Path(output)
    app = root / DRIVE / APP
    for name in REQUIRED:
        path = app / name
        if not path.is_file() or not path.stat().st_size:
            raise ValueError(f"incomplete installed 3DMark2001 SE: missing or empty {name}")
    files, omitted = {}, []
    for path in sorted(app.rglob("*")):
        relative = path.relative_to(app)
        if path.is_symlink():
            raise ValueError(f"unexpected link in installed application: {relative}")
        if not path.is_file():
            continue
        name = relative.as_posix()
        if any(part in (".", "..") or "\\" in part or ":" in part for part in relative.parts):
            raise ValueError(f"invalid installed application path: {name}")
        if name.lower().endswith((".log", ".3dp", ".3dr", ".link", ".deleted")):
            omitted.append(name)
            continue
        files[DRIVE + APP + name] = path
    generated = {DRIVE + "3dmark2001se.bat": LAUNCHER.replace("\n", "\r\n").encode("ascii")}
    retained = []
    with zipfile.ZipFile(output, "x", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for name in sorted(files.keys() | generated.keys()):
            data = files[name].read_bytes() if name in files else generated[name]
            info = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            info.create_system = 3
            info.external_attr = (stat.S_IFREG | 0o644) << 16
            info.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(info, data, compresslevel=9)
            retained.append(dict(path=name, bytes=len(data), sha256=hashlib.sha256(data).hexdigest(),
                                 origin="installed" if name in files else "generated"))
    return dict(schema_version=1, installed_root=str(root),
                scope="Installed application overlay; startup and rendering must be validated separately.",
                output=dict(path=str(output.resolve()), bytes=output.stat().st_size,
                            sha256=hashlib.sha256(output.read_bytes()).hexdigest()),
                retained=retained, omitted_app_files=omitted,
                excluded="Wine/Linux libraries, global registry hives, installer caches, and benchmark results")


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("installed_root", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--report", type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        with args.report.open("x", encoding="utf-8", newline="\n") as stream:
            report = prepare(args.installed_root, args.output)
            json.dump(report, stream, indent=2)
            stream.write("\n")
    except (OSError, ValueError) as error:
        parser.exit(2, f"3DMark overlay preparation failed: {error}\n")
    print(f"Prepared {len(report['retained'])} files: {args.output} ({report['output']['bytes']} bytes)")


if __name__ == "__main__":
    main()
