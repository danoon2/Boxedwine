#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Pack, cache and stage the shared native-UI demo catalog (Python 3 standard library).

Only an exact size/SHA-256 match to resources/demo-catalog.lock.json is usable.
The cache contains immutable ZIPs, not a last-known-good fallback across versions.
"""

import argparse
import hashlib
import io
import json
import os
from pathlib import Path
import re
import shutil
import stat
import subprocess
import sys
import tempfile
import urllib.parse
import urllib.request
import xml.etree.ElementTree as ET
import zipfile


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_LOCK = ROOT / "resources/demo-catalog.lock.json"
MAX_ZIP = 32 * 1024 * 1024
MAX_EXPANDED = 64 * 1024 * 1024
MAX_FILE = 1024 * 1024
MAX_FILES = 4096
RECEIPT = "catalog-package.json"
PNG = b"\x89PNG\r\n\x1a\n"


def digest(data):
    return hashlib.sha256(data).hexdigest()


def json_bytes(value):
    return (json.dumps(value, indent=2, sort_keys=True) + "\n").encode()


def valid_url(url):
    parts = urllib.parse.urlsplit(url)
    if (parts.scheme != "https" or parts.hostname not in ("boxedwine.org", "www.boxedwine.org")
            or parts.username is not None or parts.password is not None or parts.port not in (None, 443)
            or parts.fragment or parts.query or not parts.path.endswith(".zip")):
        raise ValueError("Catalog URL must be an HTTPS ZIP on boxedwine.org, without credentials or query parameters")
    return url


def validate_pin(pin):
    if not isinstance(pin, dict) or set(pin) != {"formatVersion", "version", "url", "bytes", "sha256"}:
        raise ValueError("Invalid demo catalog lock fields")
    if type(pin["formatVersion"]) is not int or pin["formatVersion"] != 1:
        raise ValueError("Unsupported demo catalog lock format")
    if not isinstance(pin["version"], str) or not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9._-]{0,127}", pin["version"]):
        raise ValueError("Invalid catalog version")
    if type(pin["bytes"]) is not int or not 0 < pin["bytes"] <= MAX_ZIP:
        raise ValueError("Invalid catalog ZIP size")
    if not isinstance(pin["sha256"], str) or not re.fullmatch(r"[a-f0-9]{64}", pin["sha256"]):
        raise ValueError("Invalid catalog SHA-256")
    if not isinstance(pin["url"], str):
        raise ValueError("Invalid catalog URL")
    valid_url(pin["url"])
    return pin


def read_pin(path):
    return validate_pin(json.loads(path.read_bytes()))


def default_cache():
    override = os.environ.get("BOXEDWINE_CATALOG_CACHE")
    if override:
        return Path(override).expanduser().resolve()
    if sys.platform == "darwin":
        base = Path.home() / "Library/Caches"
    elif sys.platform == "win32":
        base = Path(os.environ.get("LOCALAPPDATA", Path.home() / "AppData/Local"))
    else:
        base = Path(os.environ.get("XDG_CACHE_HOME", Path.home() / ".cache"))
    return base / "BoxedwineBuild/demo-catalogs"


def safe_name(name):
    # Flat layout also works on case-insensitive filesystems. Never extract paths,
    # Windows drive names, special files, or platform metadata from a ZIP.
    return (name == "catalog.xml" or bool(re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9._-]*\.png", name))) and len(name) <= 200


def validate_files(files, version):
    if not files or len(files) > MAX_FILES or "catalog.xml" not in files:
        raise ValueError("Catalog ZIP must contain catalog.xml and its PNG icons")
    if len({name.casefold() for name in files}) != len(files):
        raise ValueError("Case-insensitive filename collision")
    if any(not safe_name(name) or not 0 < len(data) <= MAX_FILE for name, data in files.items()):
        raise ValueError("Unsupported filename or file size in catalog")
    if sum(map(len, files.values())) > MAX_EXPANDED:
        raise ValueError("Expanded catalog exceeds size limit")
    xml = files["catalog.xml"].decode("utf-8")
    if "<!DOCTYPE" in xml.upper() or "<!ENTITY" in xml.upper():
        raise ValueError("Catalog XML cannot contain DTDs or entities")
    try:
        root = ET.fromstring(xml)
    except ET.ParseError as error:
        raise ValueError(f"Invalid catalog XML: {error}") from error
    if root.tag != "XML" or root.get("release") != version or not re.fullmatch(r"[1-9][0-9]*", root.get("schemaVersion", "")):
        raise ValueError("Catalog release must match the pin and declare a schema version")
    demos = list(root)
    if not demos or any(item.tag != "Demo" for item in demos):
        raise ValueError("Catalog must contain Demo entries")
    ids, icons = set(), set()
    for demo in demos:
        fields = [child.tag for child in demo]
        identifier = demo.findtext("ID", "")
        if len(set(fields)) != len(fields) or not identifier or identifier in ids:
            raise ValueError("Missing/duplicate demo ID or duplicate recipe fields")
        ids.add(identifier)
        icon = demo.findtext("Icon", "")
        if icon:
            if not safe_name(icon) or not icon.endswith(".png") or icon not in files or not files[icon].startswith(PNG):
                raise ValueError("Missing or invalid catalog PNG: " + icon)
            icons.add(icon)
    if set(files) != {"catalog.xml", *icons}:
        raise ValueError("Catalog ZIP has unreferenced files")
    return {"version": version, "schemaVersion": int(root.get("schemaVersion")), "demos": len(demos), "icons": len(icons)}


def unpack(data, pin):
    if len(data) != pin["bytes"] or digest(data) != pin["sha256"]:
        raise ValueError("Catalog ZIP does not match the pinned size and SHA-256")
    files = {}
    with zipfile.ZipFile(io.BytesIO(data)) as archive:
        entries = archive.infolist()
        if len(entries) > MAX_FILES or sum(item.file_size for item in entries) > MAX_EXPANDED:
            raise ValueError("Catalog ZIP exceeds extraction limits")
        for item in entries:
            mode = item.external_attr >> 16
            if (not safe_name(item.filename) or item.orig_filename != item.filename or item.filename in files or item.is_dir()
                    or (stat.S_IFMT(mode) not in (0, stat.S_IFREG)) or item.flag_bits & 1
                    or item.compress_type not in (zipfile.ZIP_STORED, zipfile.ZIP_DEFLATED)
                    or not 0 < item.file_size <= MAX_FILE):
                raise ValueError("Unsafe or unsupported catalog ZIP entry: " + item.filename)
            # zipfile verifies each entry's CRC while reading; extraction never
            # delegates path handling or follows links in the archive.
            files[item.filename] = archive.read(item)
    validate_files(files, pin["version"])
    return files


def read_zip(path, pin):
    with path.open("rb") as stream:
        data = stream.read(pin["bytes"] + 1)
    return data, unpack(data, pin)


def atomic_write(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, name = tempfile.mkstemp(prefix=".catalog-", dir=path.parent)
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(data)
        os.replace(name, path)
    finally:
        Path(name).unlink(missing_ok=True)


class CatalogRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, request, fp, code, msg, headers, url):
        valid_url(url)
        return super().redirect_request(request, fp, code, msg, headers, url)


def transfer(url, destination, maximum):
    opener = urllib.request.build_opener(CatalogRedirect())
    request = urllib.request.Request(valid_url(url), headers={"User-Agent": "Boxedwine-Catalog-Build/1"})
    with opener.open(request, timeout=10) as response, destination.open("wb") as output:
        total = 0
        while True:
            chunk = response.read(min(65536, maximum + 1 - total))
            if not chunk:
                break
            total += len(chunk)
            if total > maximum:
                raise ValueError("Catalog download exceeds pinned size")
            output.write(chunk)


def download(pin, destination):
    # Bound the entire transfer, including DNS and repeated socket reads. Killing
    # this child on timeout closes the file before the parent removes it.
    result = subprocess.run([sys.executable, str(Path(__file__).resolve()), "_download",
                             pin["url"], str(destination), str(pin["bytes"])],
                            capture_output=True, text=True, timeout=20)
    if result.returncode:
        raise ValueError(result.stderr.strip() or "Catalog download failed")


def resolve(pin, cache, offline=False):
    cached = cache / (pin["sha256"] + ".zip")
    if cached.is_file():
        try:
            return read_zip(cached, pin)[1]
        except (OSError, ValueError, zipfile.BadZipFile, ET.ParseError) as error:
            print(f"warning: Ignoring invalid cached catalog: {error}", file=sys.stderr)
    if offline:
        raise ValueError("No verified copy of the pinned catalog is cached (offline mode)")
    cache.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix=".download-", dir=cache) as temporary:
        pending = Path(temporary) / "catalog.zip"
        download(pin, pending)
        data, files = read_zip(pending, pin)
        atomic_write(cached, data)
    return files


def remove_output(output):
    # --output is an owned build-resource directory, never an authoring source.
    if output.is_symlink() or output.is_file():
        output.unlink()
    elif output.exists():
        shutil.rmtree(output)


def stage(pin, cache, output, required=False, offline=False):
    output = output.absolute()
    if output == Path(output.anchor) or output == ROOT or output in cache.absolute().parents or output == cache.absolute():
        raise ValueError("Catalog output must be a dedicated build-resource directory")
    # Remove stale resources before attempting a new version, even if the build
    # is offline. An earlier branch's catalog must never slip into this build.
    remove_output(output)
    try:
        files = resolve(pin, cache, offline)
    except (OSError, ValueError, zipfile.BadZipFile, ET.ParseError, subprocess.TimeoutExpired) as error:
        message = f"Demo catalog {pin['version']} is unavailable: {error}"
        if required:
            raise ValueError(message) from error
        print(f"warning: {message}. Building without demos; connect and rebuild to include them.", file=sys.stderr)
        return False
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix=".catalog-stage-", dir=output.parent) as temporary:
        staged = Path(temporary) / "Demos"
        staged.mkdir()
        for name, data in files.items():
            (staged / name).write_bytes(data)
        (staged / RECEIPT).write_bytes(json_bytes({"pin": pin, "files": {name: digest(data) for name, data in files.items()}}))
        staged.rename(output)
    print(f"Demo catalog {pin['version']}: {len(files) - 1} icons, verified and staged from {pin['sha256'][:12]}")
    return True


def audit_staged(directory, pin):
    if directory.is_symlink() or not directory.is_dir():
        raise ValueError("Pinned demo catalog is missing from the app")
    receipt = directory / RECEIPT
    if receipt.is_symlink() or not receipt.is_file() or receipt.stat().st_size > MAX_FILE:
        raise ValueError("Catalog build receipt is missing or invalid")
    record = json.loads(receipt.read_bytes())
    if not isinstance(record, dict) or record.get("pin") != pin:
        raise ValueError("Bundled catalog does not match the project's pin")
    files = {}
    for path in directory.iterdir():
        if path.name == RECEIPT:
            continue
        if path.is_symlink() or not path.is_file() or not safe_name(path.name) or path.stat().st_size > MAX_FILE:
            raise ValueError("Unexpected bundled catalog entry")
        files[path.name] = path.read_bytes()
        if len(files) > MAX_FILES or sum(map(len, files.values())) > MAX_EXPANDED:
            raise ValueError("Bundled catalog exceeds size limits")
    if record.get("files") != {name: digest(data) for name, data in files.items()}:
        raise ValueError("Bundled catalog content differs from its build receipt")
    return {**validate_files(files, pin["version"]), "sha256": pin["sha256"], "bytes": pin["bytes"]}


def pack(source, version, url, output, lock):
    files = {}
    for path in source.iterdir():
        if path.name == RECEIPT:
            continue  # A staged package can be copied and used as authoring input.
        if path.is_symlink() or not path.is_file() or not safe_name(path.name) or path.stat().st_size > MAX_FILE:
            raise ValueError("Authoring directory must contain only catalog.xml and referenced PNGs")
        files[path.name] = path.read_bytes()
    validate_files(files, version)
    stream = io.BytesIO()
    with zipfile.ZipFile(stream, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for name, data in sorted(files.items()):
            entry = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            entry.create_system = 3
            entry.external_attr = (stat.S_IFREG | 0o644) << 16
            entry.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(entry, data)
    data = stream.getvalue()
    pin = validate_pin({"formatVersion": 1, "version": version, "url": url, "bytes": len(data), "sha256": digest(data)})
    unpack(data, pin)
    atomic_write(output, data)
    atomic_write(lock, json_bytes(pin))
    print(f"Packed {version}: {len(data)} bytes, SHA-256 {pin['sha256']}\nUpload {output} to {url}\nPin written to {lock}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    package = commands.add_parser("pack", help="create a release ZIP and its project pin")
    for name in ("source", "output", "lock"):
        package.add_argument("--" + name, type=Path, required=True)
    package.add_argument("--version", required=True, help="must match XML release")
    package.add_argument("--url", required=True)
    for name in ("stage", "import", "audit"):
        command = commands.add_parser(name)
        command.add_argument("--lock", type=Path, default=DEFAULT_LOCK)
        if name != "audit":
            command.add_argument("--cache", type=Path, default=default_cache())
        if name == "import":
            command.add_argument("--zip", type=Path, required=True)
        else:
            command.add_argument("--output", type=Path, required=True, help="dedicated catalog resource directory (stage replaces it)")
        if name == "stage":
            command.add_argument("--required", action="store_true", help="fail if the pinned catalog is unavailable")
            command.add_argument("--offline", action="store_true", help="use only an exact verified cache hit")
    child = commands.add_parser("_download", help=argparse.SUPPRESS)
    child.add_argument("url")
    child.add_argument("destination", type=Path)
    child.add_argument("maximum", type=int)
    args = parser.parse_args()
    try:
        if args.command == "_download":
            transfer(args.url, args.destination, args.maximum)
        elif args.command == "pack":
            pack(args.source, args.version, args.url, args.output, args.lock)
        else:
            pin = read_pin(args.lock)
            if args.command == "import":
                data, _ = read_zip(args.zip, pin)
                atomic_write(args.cache / (pin["sha256"] + ".zip"), data)
                print("Cached verified catalog " + pin["version"])
            elif args.command == "stage":
                stage(pin, args.cache, args.output, args.required, args.offline)
            else:
                print(json.dumps(audit_staged(args.output, pin), indent=2))
    except (OSError, ValueError, zipfile.BadZipFile, ET.ParseError, subprocess.TimeoutExpired) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
