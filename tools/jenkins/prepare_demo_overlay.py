#!/usr/bin/env python3
"""Create an application-only overlay from explicitly selected ZIP paths.

This never modifies the source or overwrites an existing output. Include a file
by its exact path, or a directory with a trailing slash. Keep the report with
the resulting archive; validate the game's startup before publishing it.
"""

import argparse
import hashlib
import json
from pathlib import Path
import stat
import zipfile


DRIVE = "home/username/.wine/drive_c/"


def valid_path(name):
    return (isinstance(name, str) and name.startswith(DRIVE) and name != DRIVE
            and "\\" not in name and ":" not in name
            and all(part not in (".", "..", "") for part in name.rstrip("/").split("/")))


def prepare_overlay(source, output, includes):
    if not includes or any(not valid_path(name) for name in includes):
        raise ValueError("select exact files or subdirectories within home/username/.wine/drive_c/")
    source, output = Path(source), Path(output)
    source_hash = hashlib.sha256(source.read_bytes()).hexdigest()
    with zipfile.ZipFile(source) as archive:
        names = archive.namelist()
        if len(names) != len(set(names)):
            raise ValueError("source ZIP contains duplicate paths")
        selected = [info for info in archive.infolist() if not info.is_dir() and any(
            info.filename.startswith(name) if name.endswith("/") else info.filename == name for name in includes)]
        for name in includes:
            if not any(info.filename.startswith(name) if name.endswith("/") else info.filename == name for info in selected):
                raise ValueError(f"selected path has no files: {name}")
        for info in selected:
            if (not valid_path(info.orig_filename) or info.orig_filename != info.filename
                    or info.filename.endswith((".link", ".deleted"))
                    or stat.S_ISLNK(info.external_attr >> 16)):
                raise ValueError(f"selected path is not a regular overlay file: {info.filename}")
        entries = []
        with zipfile.ZipFile(output, "x", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as target:
            for info in sorted(selected, key=lambda item: item.filename):
                data = archive.read(info)
                normalized = zipfile.ZipInfo(info.filename, date_time=(1980, 1, 1, 0, 0, 0))
                normalized.create_system = 3
                normalized.external_attr = (stat.S_IFREG | 0o644) << 16
                normalized.compress_type = zipfile.ZIP_DEFLATED
                target.writestr(normalized, data, compresslevel=9)
                entries.append(dict(path=info.filename, bytes=len(data), sha256=hashlib.sha256(data).hexdigest()))
    return dict(schema_version=1, scope="Packaging only; game startup is not validated.",
                source=dict(path=str(source.resolve()), sha256=source_hash), includes=includes,
                output=dict(path=str(output.resolve()), bytes=output.stat().st_size,
                            sha256=hashlib.sha256(output.read_bytes()).hexdigest()),
                retained=entries, omitted=sorted(set(names) - {info.filename for info in selected}))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--include", action="append", required=True)
    parser.add_argument("--report", type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        # Reserve the report first so an existing report cannot accompany new bytes.
        with args.report.open("x", encoding="utf-8", newline="\n") as stream:
            report = prepare_overlay(args.source, args.output, args.include)
            json.dump(report, stream, indent=2)
            stream.write("\n")
    except (OSError, ValueError, zipfile.BadZipFile) as error:
        parser.exit(2, f"overlay preparation failed: {error}\n")
    print(f"Prepared {len(report['retained'])} files: {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
