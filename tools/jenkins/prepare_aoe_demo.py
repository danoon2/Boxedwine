#!/usr/bin/env python3
"""Package the public Age of Empires trial with its installation registry entries."""

import argparse
import hashlib
import json
from pathlib import Path
import stat
import zipfile


SOURCE_URL = "https://www.boxedwine.org/boxedwine/apps/aoe.zip"
SOURCE_SHA256 = "0311e6451cf419c7afa48ef0e93954c69bee9173a27ff85a541bed62b81b859f"
DRIVE = "home/username/.wine/drive_c/"
APP = "Program Files/Microsoft Games/Age of Empires Trial/"
REGISTRY = r'''Windows Registry Editor Version 5.00

[HKEY_LOCAL_MACHINE\Software\Microsoft\Games\AOE Trial\1.00]
"InstallationDirectory"="C:\\Program Files\\Microsoft Games\\Age of Empires Trial"
"CDPath"="C:\\Program Files\\Microsoft Games\\Age of Empires Trial\\"
"InstallType"="0"
"ProductName"="Microsoft Age of Empires Trial Version"
"VersionType"="TrialVersion"
"Custom Mouse"=dword:00000000

[HKEY_LOCAL_MACHINE\Software\Microsoft\DirectPlay\Applications\Age of Empires Trial]
"CommandLine"="lobby"
"CurrentDirectory"="C:\\Program Files\\Microsoft Games\\Age of Empires Trial"
"File"="Empires.Exe"
"Guid"="{3422E981-1A89-11D1-B093-00A024C74776}"
"Path"="C:\\Program Files\\Microsoft Games\\Age of Empires Trial"
'''
LAUNCHER = r'''@echo off
regedit /S C:\aoe-demo.reg
cd /d "C:\Program Files\Microsoft Games\Age of Empires Trial"
empires.exe
'''


def prepare(source, output):
    source, output = Path(source), Path(output)
    source_hash = hashlib.sha256(source.read_bytes()).hexdigest()
    if source_hash != SOURCE_SHA256:
        raise ValueError("source must be the pinned public Age of Empires trial ZIP")
    with zipfile.ZipFile(source) as archive:
        files = {DRIVE + APP + info.filename: archive.read(info)
                 for info in archive.infolist() if not info.is_dir()}
    game_file_count = len(files)
    files[DRIVE + "aoe-demo.reg"] = REGISTRY.replace("\n", "\r\n").encode("ascii")
    files[DRIVE + "aoe.bat"] = LAUNCHER.replace("\n", "\r\n").encode("ascii")
    with zipfile.ZipFile(output, "x", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as target:
        for name, data in sorted(files.items()):
            info = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            info.create_system = 3
            info.external_attr = (stat.S_IFREG | 0o644) << 16
            info.compress_type = zipfile.ZIP_DEFLATED
            target.writestr(info, data, compresslevel=9)
    return dict(schema_version=1,
                source=dict(url=SOURCE_URL, sha256=source_hash),
                output=dict(path=str(output.resolve()), bytes=output.stat().st_size,
                            sha256=hashlib.sha256(output.read_bytes()).hexdigest()),
                game_files=game_file_count, generated=[DRIVE + "aoe-demo.reg", DRIVE + "aoe.bat"],
                scope="Public trial files plus game-specific registry entries and launcher; no user registry or saves.")


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--report", type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        with args.report.open("x", encoding="utf-8", newline="\n") as stream:
            report = prepare(args.source, args.output)
            json.dump(report, stream, indent=2)
            stream.write("\n")
    except (OSError, ValueError, zipfile.BadZipFile) as error:
        parser.exit(2, f"Age of Empires overlay preparation failed: {error}\n")
    print(f"Prepared {report['game_files']} game files and launcher: {args.output}")


if __name__ == "__main__":
    main()
