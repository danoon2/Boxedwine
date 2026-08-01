#!/usr/bin/env python3

from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import shlex
import shutil
import struct
import subprocess
import sys
import tempfile
from typing import Any, Mapping, Sequence
import zipfile


class ValidationError(RuntimeError):
    pass


SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parents[1]
DEFAULT_CONFIG = SCRIPT_DIR / "webgl_filesystems_v3.json"
SHA256_RE = re.compile(r"[0-9a-f]{64}")
PE_I386 = 0x014C
PE32_MAGIC = 0x010B
PLACEHOLDER_MARKERS = (
    b"Wine builtin DLL",
    b"Wine placeholder DLL",
    "Wine builtin DLL".encode("utf-16le"),
    "Wine placeholder DLL".encode("utf-16le"),
)


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with Path(path).open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _require_sha256(value: Any, label: str) -> str:
    if not isinstance(value, str) or not SHA256_RE.fullmatch(value):
        raise ValidationError(f"{label} must be a lowercase SHA-256 value")
    return value


def _require_positive_int(value: Any, label: str) -> int:
    if not isinstance(value, int) or isinstance(value, bool) or value <= 0:
        raise ValidationError(f"{label} must be a positive integer")
    return value


def _safe_archive_path(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value or "\\" in value or value.startswith("/"):
        raise ValidationError(f"{label} must be a safe forward-slash archive path")
    path = PurePosixPath(value)
    if any(part in {"", ".", ".."} for part in path.parts):
        raise ValidationError(f"{label} must be a safe forward-slash archive path")
    return value


def load_config(path: Path) -> dict[str, Any]:
    try:
        config = json.loads(Path(path).read_text(encoding="utf-8"))
    except OSError as error:
        raise ValidationError(f"could not read config {path}: {error}") from error
    except json.JSONDecodeError as error:
        raise ValidationError(f"could not parse config {path}: {error}") from error
    if not isinstance(config, dict) or config.get("schema_version") != 1:
        raise ValidationError("WebGL filesystem config must use schema_version 1")
    wine_version = config.get("wine_version")
    if not isinstance(wine_version, str) or not wine_version:
        raise ValidationError("wine_version must be a non-empty string")
    build_date = config.get("build_date")
    if not isinstance(build_date, str) or not re.fullmatch(r"\d{4}-\d{2}-\d{2}", build_date):
        raise ValidationError("build_date must use YYYY-MM-DD")
    _require_positive_int(config.get("source_date_epoch"), "source_date_epoch")
    commit = config.get("wine_source_commit")
    if not isinstance(commit, str) or not re.fullmatch(r"[0-9a-f]{40}", commit):
        raise ValidationError("wine_source_commit must be a full lowercase Git commit")
    patch_manifest = config.get("patch_manifest")
    if not isinstance(patch_manifest, dict):
        raise ValidationError("patch_manifest must be an object")
    _safe_archive_path(patch_manifest.get("path"), "patch_manifest.path")
    _require_sha256(patch_manifest.get("sha256"), "patch_manifest.sha256")
    _safe_archive_path(config.get("webgl_directory"), "webgl_directory")

    dlls = config.get("dlls")
    if not isinstance(dlls, dict) or not dlls:
        raise ValidationError("dlls must be a non-empty object")
    for name, spec in dlls.items():
        if not isinstance(name, str) or PurePosixPath(name).name != name or not name.endswith(".dll"):
            raise ValidationError(f"invalid DLL name in config: {name!r}")
        if not isinstance(spec, dict):
            raise ValidationError(f"DLL config for {name} must be an object")
        _require_positive_int(spec.get("size"), f"dlls.{name}.size")
        _require_sha256(spec.get("sha256"), f"dlls.{name}.sha256")
        imports = spec.get("required_imports")
        if not isinstance(imports, list) or not all(
            isinstance(item, str) and item == item.lower() and item.endswith(".dll")
            for item in imports
        ):
            raise ValidationError(f"dlls.{name}.required_imports must be lowercase DLL names")

    for key in ("gl_files", "gl_links", "ld_cache", "profiles"):
        if not isinstance(config.get(key), dict):
            raise ValidationError(f"{key} must be an object")
    for name, spec in config["gl_files"].items():
        _safe_archive_path(name, f"gl_files.{name}")
        if not isinstance(spec, dict):
            raise ValidationError(f"gl_files.{name} must be an object")
        _require_positive_int(spec.get("size"), f"gl_files.{name}.size")
        _require_sha256(spec.get("sha256"), f"gl_files.{name}.sha256")
    for name, target in config["gl_links"].items():
        _safe_archive_path(name, f"gl_links.{name}")
        if not isinstance(target, str) or not target or "/" in target or "\\" in target:
            raise ValidationError(f"gl_links.{name} must name a sibling target")
    for profile_name, profile in config["profiles"].items():
        if not isinstance(profile, dict):
            raise ValidationError(f"profiles.{profile_name} must be an object")
        filename = profile.get("filename")
        if not isinstance(filename, str) or Path(filename).name != filename:
            raise ValidationError(f"profiles.{profile_name}.filename must be a filename")
        filesystem_version = profile.get("filesystem_version")
        if not isinstance(filesystem_version, str) or not filesystem_version:
            raise ValidationError(
                f"profiles.{profile_name}.filesystem_version must be a non-empty string"
            )
        _require_positive_int(profile.get("size"), f"profiles.{profile_name}.size")
        _require_sha256(profile.get("sha256"), f"profiles.{profile_name}.sha256")
    return config


def verify_patch_series(config: Mapping[str, Any], repo_root: Path = REPO_ROOT) -> list[dict[str, str]]:
    patch_config = config["patch_manifest"]
    manifest_path = Path(repo_root) / patch_config["path"]
    actual_manifest_hash = sha256_file(manifest_path)
    if actual_manifest_hash != patch_config["sha256"]:
        raise ValidationError(
            f"patch manifest SHA-256 {actual_manifest_hash} does not match "
            f"{patch_config['sha256']}"
        )
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise ValidationError(f"could not load patch manifest {manifest_path}: {error}") from error
    if manifest.get("wine_source_commit") != config["wine_source_commit"]:
        raise ValidationError("patch manifest and filesystem config pin different Wine commits")
    production = manifest.get("production_patches")
    test_patch = manifest.get("test_patch")
    if not isinstance(production, list) or not isinstance(test_patch, dict):
        raise ValidationError("patch manifest has an invalid patch series")

    result: list[dict[str, str]] = []
    for item in [*production, test_patch]:
        if not isinstance(item, dict):
            raise ValidationError("patch manifest entries must be objects")
        relative = _safe_archive_path(item.get("path"), "patch path")
        expected_hash = _require_sha256(item.get("sha256"), f"{relative} sha256")
        patch_path = (Path(repo_root) / relative).resolve()
        try:
            patch_path.relative_to(Path(repo_root).resolve())
        except ValueError as error:
            raise ValidationError(f"patch escapes repository root: {relative}") from error
        actual_hash = sha256_file(patch_path)
        if actual_hash != expected_hash:
            raise ValidationError(
                f"{relative} SHA-256 {actual_hash} does not match {expected_hash}"
            )
        result.append(
            {
                "patch_id": str(item.get("patch_id", "")),
                "path": relative,
                "sha256": actual_hash,
            }
        )
    return result


def _read_u16(data: bytes, offset: int, label: str) -> int:
    if offset < 0 or offset + 2 > len(data):
        raise ValidationError(f"truncated PE while reading {label}")
    return struct.unpack_from("<H", data, offset)[0]


def _read_u32(data: bytes, offset: int, label: str) -> int:
    if offset < 0 or offset + 4 > len(data):
        raise ValidationError(f"truncated PE while reading {label}")
    return struct.unpack_from("<I", data, offset)[0]


def parse_pe32_imports(data: bytes, label: str) -> list[str]:
    if len(data) < 0x40 or data[:2] != b"MZ":
        raise ValidationError(f"{label} is not a PE file")
    pe_offset = _read_u32(data, 0x3C, "PE header offset")
    if pe_offset + 24 > len(data) or data[pe_offset : pe_offset + 4] != b"PE\0\0":
        raise ValidationError(f"{label} has no valid PE signature")
    machine = _read_u16(data, pe_offset + 4, "COFF machine")
    if machine != PE_I386:
        raise ValidationError(f"{label} machine is 0x{machine:04x}, expected PE32/i386 0x014c")
    section_count = _read_u16(data, pe_offset + 6, "section count")
    optional_size = _read_u16(data, pe_offset + 20, "optional-header size")
    optional_offset = pe_offset + 24
    if _read_u16(data, optional_offset, "optional-header magic") != PE32_MAGIC:
        raise ValidationError(f"{label} is not PE32")
    if optional_size < 112 or optional_offset + optional_size > len(data):
        raise ValidationError(f"{label} has a truncated PE32 optional header")

    import_rva = _read_u32(data, optional_offset + 104, "import-directory RVA")
    section_offset = optional_offset + optional_size
    sections: list[tuple[int, int, int, int]] = []
    for index in range(section_count):
        offset = section_offset + index * 40
        if offset + 40 > len(data):
            raise ValidationError(f"{label} has a truncated section table")
        virtual_size = _read_u32(data, offset + 8, "section virtual size")
        virtual_address = _read_u32(data, offset + 12, "section RVA")
        raw_size = _read_u32(data, offset + 16, "section raw size")
        raw_offset = _read_u32(data, offset + 20, "section raw offset")
        sections.append((virtual_address, virtual_size, raw_offset, raw_size))

    def rva_to_offset(rva: int) -> int:
        for virtual_address, virtual_size, raw_offset, raw_size in sections:
            span = max(virtual_size, raw_size)
            if virtual_address <= rva < virtual_address + span:
                delta = rva - virtual_address
                if delta >= raw_size or raw_offset + delta >= len(data):
                    break
                return raw_offset + delta
        raise ValidationError(f"{label} import RVA 0x{rva:x} is not backed by file data")

    if not import_rva:
        raise ValidationError(f"{label} has no PE import directory")
    descriptor_offset = rva_to_offset(import_rva)
    imports: list[str] = []
    for index in range(4096):
        offset = descriptor_offset + index * 20
        if offset + 20 > len(data):
            raise ValidationError(f"{label} has a truncated import descriptor")
        descriptor = data[offset : offset + 20]
        if descriptor == b"\0" * 20:
            break
        name_rva = _read_u32(data, offset + 12, "import name RVA")
        name_offset = rva_to_offset(name_rva)
        end = data.find(b"\0", name_offset, min(len(data), name_offset + 512))
        if end == -1:
            raise ValidationError(f"{label} has an unterminated import name")
        try:
            name = data[name_offset:end].decode("ascii").lower()
        except UnicodeDecodeError as error:
            raise ValidationError(f"{label} has a non-ASCII import name") from error
        imports.append(name)
    else:
        raise ValidationError(f"{label} has too many import descriptors")
    if not imports:
        raise ValidationError(f"{label} has an empty PE import table")
    return imports


def validate_dll_bytes(name: str, data: bytes, spec: Mapping[str, Any]) -> dict[str, Any]:
    expected_size = spec["size"]
    if len(data) != expected_size:
        raise ValidationError(f"{name} size {len(data)} does not match {expected_size}")
    actual_hash = sha256_bytes(data)
    if actual_hash != spec["sha256"]:
        raise ValidationError(
            f"{name} SHA-256 {actual_hash} does not match {spec['sha256']}"
        )
    for marker in PLACEHOLDER_MARKERS:
        if marker in data:
            raise ValidationError(f"{name} contains a Wine placeholder/builtin marker")
    imports = parse_pe32_imports(data, name)
    missing = sorted(set(spec["required_imports"]) - set(imports))
    if missing:
        raise ValidationError(f"{name} is missing required imports: {', '.join(missing)}")
    return {
        "size": len(data),
        "sha256": actual_hash,
        "machine": "PE32/i386",
        "imports": imports,
    }


def validate_dll_directory(dll_dir: Path, config: Mapping[str, Any]) -> dict[str, Any]:
    dll_dir = Path(dll_dir)
    if not dll_dir.is_dir():
        raise ValidationError(f"DLL directory does not exist: {dll_dir}")
    report: dict[str, Any] = {}
    for name, spec in config["dlls"].items():
        path = dll_dir / name
        try:
            data = path.read_bytes()
        except OSError as error:
            raise ValidationError(f"could not read required DLL {path}: {error}") from error
        report[name] = validate_dll_bytes(name, data, spec)
    return report


def _validate_zip_name(name: str) -> None:
    if "\\" in name or name.startswith("/"):
        raise ValidationError(f"unsafe ZIP entry name: {name}")
    path = PurePosixPath(name)
    if any(part in {"", ".", ".."} for part in path.parts):
        raise ValidationError(f"unsafe ZIP entry name: {name}")


def open_validated_zip(path: Path) -> tuple[zipfile.ZipFile, dict[str, zipfile.ZipInfo]]:
    path = Path(path)
    if not path.is_file() or not zipfile.is_zipfile(path):
        raise ValidationError(f"not a readable ZIP archive: {path}")
    archive = zipfile.ZipFile(path, "r")
    try:
        infos: dict[str, zipfile.ZipInfo] = {}
        for info in archive.infolist():
            _validate_zip_name(info.filename)
            if info.filename in infos:
                raise ValidationError(f"duplicate ZIP entry: {info.filename}")
            infos[info.filename] = info
        bad_crc = archive.testzip()
        if bad_crc is not None:
            raise ValidationError(f"CRC validation failed for ZIP entry: {bad_crc}")
    except Exception:
        archive.close()
        raise
    return archive, infos


def _require_zip_entry(
    archive: zipfile.ZipFile,
    infos: Mapping[str, zipfile.ZipInfo],
    name: str,
) -> bytes:
    if name not in infos:
        raise ValidationError(f"required ZIP entry is missing: {name}")
    return archive.read(name)


def _validate_sized_hash(name: str, data: bytes, spec: Mapping[str, Any]) -> dict[str, Any]:
    if len(data) != spec["size"]:
        raise ValidationError(f"{name} size {len(data)} does not match {spec['size']}")
    actual_hash = sha256_bytes(data)
    if actual_hash != spec["sha256"]:
        raise ValidationError(
            f"{name} SHA-256 {actual_hash} does not match {spec['sha256']}"
        )
    return {"size": len(data), "sha256": actual_hash}


def _runtime_target_exists(target: str, infos: Mapping[str, zipfile.ZipInfo]) -> bool:
    relative = target.lstrip("/")
    return relative in infos or f"{relative}.link" in infos


def validate_ld_cache(
    cache_data: bytes,
    expected: Mapping[str, str],
    infos: Mapping[str, zipfile.ZipInfo],
) -> dict[str, str]:
    if not expected:
        return {}
    executable = shutil.which("ldconfig")
    if executable is None and Path("/sbin/ldconfig").is_file():
        executable = "/sbin/ldconfig"
    if executable is None:
        raise ValidationError("ldconfig is required to validate etc/ld.so.cache")
    with tempfile.NamedTemporaryFile(prefix="boxedwine-ld-cache-", delete=False) as temporary:
        temporary.write(cache_data)
        cache_path = Path(temporary.name)
    try:
        completed = subprocess.run(
            [executable, "-p", "-C", str(cache_path)],
            check=False,
            capture_output=True,
            text=True,
        )
    finally:
        cache_path.unlink(missing_ok=True)
    if completed.returncode != 0:
        detail = completed.stderr.strip() or completed.stdout.strip()
        raise ValidationError(f"ldconfig could not parse etc/ld.so.cache: {detail}")
    mappings: dict[str, set[str]] = {}
    for line in completed.stdout.splitlines():
        match = re.match(r"\s*(\S+)\s+\(.*\)\s+=>\s+(\S+)\s*$", line)
        if match:
            mappings.setdefault(match.group(1), set()).add(match.group(2))
    report: dict[str, str] = {}
    for soname, target in expected.items():
        if target not in mappings.get(soname, set()):
            actual = ", ".join(sorted(mappings.get(soname, set()))) or "<missing>"
            raise ValidationError(
                f"ld.so.cache resolves {soname} to {actual}, expected {target}"
            )
        if not _runtime_target_exists(target, infos):
            raise ValidationError(
                f"ld.so.cache target for {soname} has no file or .link entry: {target}"
            )
        report[soname] = target
    return report


def validate_archive(
    path: Path,
    config: Mapping[str, Any],
    profile_name: str | None = None,
    *,
    validate_dlls: bool = True,
) -> dict[str, Any]:
    path = Path(path)
    profile = None
    if profile_name is not None:
        profile = config["profiles"].get(profile_name)
        if profile is None:
            raise ValidationError(f"unknown filesystem profile: {profile_name}")
        if path.name != profile["filename"]:
            raise ValidationError(
                f"profile {profile_name} expects filename {profile['filename']}, got {path.name}"
            )
        actual_size = path.stat().st_size
        if actual_size != profile["size"]:
            raise ValidationError(
                f"{path.name} size {actual_size} does not match profile size {profile['size']}"
            )
        actual_hash = sha256_file(path)
        if actual_hash != profile["sha256"]:
            raise ValidationError(
                f"{path.name} SHA-256 {actual_hash} does not match profile {profile['sha256']}"
            )
    else:
        actual_size = path.stat().st_size
        actual_hash = sha256_file(path)

    archive, infos = open_validated_zip(path)
    try:
        gl_report: dict[str, Any] = {}
        for name, spec in config["gl_files"].items():
            data = _require_zip_entry(archive, infos, name)
            gl_report[name] = _validate_sized_hash(name, data, spec)
        links_report: dict[str, str] = {}
        for name, target in config["gl_links"].items():
            data = _require_zip_entry(archive, infos, name)
            try:
                actual_target = data.decode("utf-8").strip()
            except UnicodeDecodeError as error:
                raise ValidationError(f"{name} is not a UTF-8 .link entry") from error
            if actual_target != target:
                raise ValidationError(f"{name} targets {actual_target!r}, expected {target!r}")
            resolved = str(PurePosixPath(name).parent / target)
            if resolved not in infos:
                raise ValidationError(f"{name} target is missing from archive: {resolved}")
            links_report[name] = target
        cache_data = _require_zip_entry(archive, infos, "etc/ld.so.cache")
        cache_report = validate_ld_cache(cache_data, config["ld_cache"], infos)

        webgl_prefix = f"{config['webgl_directory'].rstrip('/')}/"
        base_only = bool(profile and profile.get("base_only"))
        dll_report: dict[str, Any] = {}
        if base_only:
            unexpected = sorted(name for name in infos if name.startswith(webgl_prefix))
            if unexpected:
                raise ValidationError(
                    f"base filesystem contains Wine/WebGL payload: {unexpected[0]}"
                )
        elif validate_dlls:
            for name, spec in config["dlls"].items():
                entry = f"{webgl_prefix}{name}"
                data = _require_zip_entry(archive, infos, entry)
                dll_report[name] = validate_dll_bytes(name, data, spec)

        registry_report = None
        if profile and not base_only:
            registry_name = "home/username/.wine/user.reg"
            registry = _require_zip_entry(archive, infos, registry_name).decode(
                "utf-8", errors="replace"
            )
            direct_draw = profile.get("direct_draw_renderer")
            wine_renderer = profile.get("wine_renderer")
            if f'"DirectDrawRenderer"="{direct_draw}"' not in registry:
                raise ValidationError(
                    f"{registry_name} does not select DirectDrawRenderer={direct_draw}"
                )
            if f'"renderer"="{wine_renderer}"' not in registry:
                raise ValidationError(
                    f"{registry_name} does not select Wine renderer={wine_renderer}"
                )
            registry_report = {
                "DirectDrawRenderer": direct_draw,
                "renderer": wine_renderer,
            }
    finally:
        archive.close()

    return {
        "schema_version": 1,
        "manifest_id": config["manifest_id"],
        "validated_at": datetime.now(timezone.utc).isoformat(),
        "profile": profile_name,
        "filesystem_version": profile.get("filesystem_version") if profile else None,
        "wine_version": config["wine_version"],
        "build_date": config["build_date"],
        "source_date_epoch": config["source_date_epoch"],
        "archive": {
            "filename": path.name,
            "size": actual_size,
            "sha256": actual_hash,
            "entries": len(infos),
            "crc": "ok",
        },
        "wine_source_commit": config["wine_source_commit"],
        "patch_manifest": dict(config["patch_manifest"]),
        "dlls": dll_report,
        "gl_files": gl_report,
        "gl_links": links_report,
        "ld_cache": cache_report,
        "registry": registry_report,
        "result": "ok",
    }


def _zip_content_keys(path: Path) -> dict[str, tuple[int, int]]:
    archive, infos = open_validated_zip(path)
    try:
        return {
            name: (info.file_size, info.CRC)
            for name, info in infos.items()
        }
    finally:
        archive.close()


def validate_filesystem_set(
    normal: Path,
    gdi: Path,
    full: Path,
    base: Path,
    config: Mapping[str, Any],
) -> dict[str, Any]:
    reports = {
        "normal-v3": validate_archive(normal, config, "normal-v3"),
        "gdi-v3": validate_archive(gdi, config, "gdi-v3"),
        "full-v10": validate_archive(full, config, "full-v10"),
        "base-v10": validate_archive(base, config, "base-v10"),
    }
    if sha256_file(normal) != sha256_file(full):
        raise ValidationError("normal v3 and full v10 archives are not byte-identical")
    normal_entries = _zip_content_keys(normal)
    gdi_entries = _zip_content_keys(gdi)
    if normal_entries.keys() != gdi_entries.keys():
        raise ValidationError("normal and GDI v3 archives have different entry sets")
    differences = sorted(
        name for name in normal_entries if normal_entries[name] != gdi_entries[name]
    )
    expected_difference = ["home/username/.wine/user.reg"]
    if differences != expected_difference:
        raise ValidationError(
            "normal and GDI v3 content differences are "
            f"{differences}, expected {expected_difference}"
        )
    return {
        "schema_version": 1,
        "manifest_id": f"{config['manifest_id']}-set",
        "validated_at": datetime.now(timezone.utc).isoformat(),
        "wine_version": config["wine_version"],
        "build_date": config["build_date"],
        "source_date_epoch": config["source_date_epoch"],
        "wine_source_commit": config["wine_source_commit"],
        "patch_manifest": dict(config["patch_manifest"]),
        "archives": reports,
        "comparisons": {
            "normal_v3_equals_full_v10": True,
            "normal_vs_gdi_content_differences": differences,
            "base_has_no_wine_webgl_payload": True,
        },
        "result": "ok",
    }


def _atomic_write_json(path: Path, value: Mapping[str, Any]) -> None:
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    handle = tempfile.NamedTemporaryFile(
        mode="w",
        encoding="utf-8",
        prefix=f"{path.name}.",
        suffix=".tmp",
        dir=path.parent,
        delete=False,
    )
    temporary = Path(handle.name)
    try:
        with handle:
            json.dump(value, handle, indent=2)
            handle.write("\n")
        os.replace(temporary, path)
    except Exception:
        temporary.unlink(missing_ok=True)
        raise


def write_sidecar(archive: Path, report: Mapping[str, Any]) -> Path:
    sidecar = Path(f"{archive}.manifest.json")
    _atomic_write_json(sidecar, report)
    return sidecar


def _run(args: Sequence[object], *, cwd: Path | None = None) -> subprocess.CompletedProcess[str]:
    command = [str(item) for item in args]
    print(f"+ {shlex.join(command)}")
    return subprocess.run(command, cwd=cwd, check=True, text=True)


def package_dlls(
    input_zip: Path,
    dll_dir: Path,
    output_zip: Path,
    config: Mapping[str, Any],
) -> tuple[Path, dict[str, Any]]:
    input_zip = Path(input_zip).resolve()
    output_zip = Path(output_zip).resolve()
    if input_zip == output_zip:
        raise ValidationError("package output must differ from the input archive")
    if output_zip.exists():
        raise ValidationError(f"package output already exists: {output_zip}")
    sidecar = Path(f"{output_zip}.manifest.json")
    if sidecar.exists():
        raise ValidationError(f"package sidecar already exists: {sidecar}")
    if shutil.which("zip") is None:
        raise ValidationError("Info-ZIP 'zip' is required for metadata-preserving packaging")

    validate_archive(input_zip, config, validate_dlls=False)
    validate_dll_directory(dll_dir, config)
    output_zip.parent.mkdir(parents=True, exist_ok=True)
    temporary_handle = tempfile.NamedTemporaryFile(
        prefix=f"{output_zip.name}.",
        suffix=".part",
        dir=output_zip.parent,
        delete=False,
    )
    temporary_handle.close()
    temporary_zip = Path(temporary_handle.name)
    try:
        shutil.copyfile(input_zip, temporary_zip)
        with tempfile.TemporaryDirectory(prefix="boxedwine-webgl-package-") as stage_name:
            stage = Path(stage_name)
            webgl_dir = stage / config["webgl_directory"]
            webgl_dir.mkdir(parents=True)
            archive_names: list[str] = []
            fixed_time = 315532800
            for name in config["dlls"]:
                destination = webgl_dir / name
                shutil.copyfile(Path(dll_dir) / name, destination)
                os.utime(destination, (fixed_time, fixed_time))
                archive_names.append(
                    str(PurePosixPath(config["webgl_directory"]) / name)
                )
            _run(
                ["zip", "-q", "-X", "-9", str(temporary_zip), *archive_names],
                cwd=stage,
            )
        report = validate_archive(temporary_zip, config)
        os.replace(temporary_zip, output_zip)
        report["archive"]["filename"] = output_zip.name
        write_sidecar(output_zip, report)
        return output_zip, report
    except Exception:
        temporary_zip.unlink(missing_ok=True)
        raise


def prepare_and_build(
    work_dir: Path,
    wine_repository: str,
    jobs: int,
    config: Mapping[str, Any],
    patches: Sequence[Mapping[str, str]],
    *,
    prepare_only: bool,
) -> Path:
    work_dir = Path(work_dir).resolve()
    if work_dir.exists():
        raise ValidationError(f"build work directory already exists: {work_dir}")
    work_dir.parent.mkdir(parents=True, exist_ok=True)
    work_dir.mkdir()
    source_dir = work_dir / "wine-source"
    build_dir = work_dir / "wine-build"
    _run(["git", "clone", "--no-checkout", wine_repository, source_dir])
    _run(
        ["git", "-C", source_dir, "checkout", "--detach", config["wine_source_commit"]]
    )
    completed = subprocess.run(
        ["git", "-C", source_dir, "rev-parse", "HEAD"],
        check=True,
        capture_output=True,
        text=True,
    )
    if completed.stdout.strip() != config["wine_source_commit"]:
        raise ValidationError("prepared Wine checkout is not at the pinned commit")
    for patch in patches:
        patch_path = REPO_ROOT / patch["path"]
        _run(["git", "-C", source_dir, "apply", "--check", patch_path])
        _run(["git", "-C", source_dir, "apply", patch_path])
    _run(["git", "-C", source_dir, "diff", "--check"])

    prepared_report = {
        "schema_version": 1,
        "manifest_id": f"{config['manifest_id']}-prepared-source",
        "wine_version": config["wine_version"],
        "build_date": config["build_date"],
        "source_date_epoch": config["source_date_epoch"],
        "wine_source_commit": config["wine_source_commit"],
        "patches": list(patches),
        "source_directory": str(source_dir),
        "result": "ok",
    }
    _atomic_write_json(work_dir / "prepared-source-manifest.json", prepared_report)
    if prepare_only:
        return source_dir

    build_script = source_dir / "build-boxedwine-webgl-dlls.sh"
    if not build_script.is_file():
        raise ValidationError(f"patched Wine build script is missing: {build_script}")
    _run([build_script, "--build-dir", build_dir, "--jobs", jobs])
    dll_dir = build_dir / "boxedwine-webgl-dlls"
    dll_report = validate_dll_directory(dll_dir, config)
    build_report = {
        "schema_version": 1,
        "manifest_id": f"{config['manifest_id']}-dll-build",
        "built_at": datetime.now(timezone.utc).isoformat(),
        "wine_version": config["wine_version"],
        "build_date": config["build_date"],
        "source_date_epoch": config["source_date_epoch"],
        "wine_source_commit": config["wine_source_commit"],
        "patches": list(patches),
        "dlls": dll_report,
        "result": "ok",
    }
    _atomic_write_json(build_dir / "webgl-build-manifest.json", build_report)
    return dll_dir


def positive_int(value: str) -> int:
    try:
        parsed = int(value)
    except ValueError as error:
        raise argparse.ArgumentTypeError("must be a positive integer") from error
    if parsed <= 0:
        raise argparse.ArgumentTypeError("must be a positive integer")
    return parsed


def add_config_argument(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        "--config",
        type=Path,
        default=DEFAULT_CONFIG,
        help=f"validation/build manifest (default: {DEFAULT_CONFIG})",
    )


def create_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Build, package, and validate Wine 11 DirectX-to-WebGL filesystems."
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    verify = subparsers.add_parser("verify-patches", help="verify the pinned Wine patch series")
    add_config_argument(verify)

    build = subparsers.add_parser(
        "build",
        help="prepare a clean pinned Wine tree, apply the series, and build all WebGL DLLs",
    )
    add_config_argument(build)
    build.add_argument("--work-dir", type=Path, required=True)
    build.add_argument(
        "--wine-repository",
        default="https://github.com/wine-mirror/wine.git",
        help="Wine Git URL or local repository used for the clean clone",
    )
    build.add_argument("--jobs", type=positive_int, default=os.cpu_count() or 1)
    build.add_argument(
        "--prepare-only",
        action="store_true",
        help="stop after the clean checkout, patch application, and diff check",
    )

    validate_dlls_parser = subparsers.add_parser(
        "validate-dlls", help="validate a flat directory containing all eight WebGL DLLs"
    )
    add_config_argument(validate_dlls_parser)
    validate_dlls_parser.add_argument("--dll-dir", type=Path, required=True)
    validate_dlls_parser.add_argument("--report", type=Path)

    package = subparsers.add_parser(
        "package",
        help="copy a root ZIP, replace all eight WebGL DLLs, validate it, and write a sidecar",
    )
    add_config_argument(package)
    package.add_argument("--input", type=Path, required=True)
    package.add_argument("--dll-dir", type=Path, required=True)
    package.add_argument("--output", type=Path, required=True)

    validate = subparsers.add_parser("validate", help="validate one filesystem ZIP")
    add_config_argument(validate)
    validate.add_argument("--filesystem", type=Path, required=True)
    validate.add_argument("--profile", choices=("normal-v3", "gdi-v3", "full-v10", "base-v10"))
    validate.add_argument("--report", type=Path)
    validate.add_argument("--write-sidecar", action="store_true")

    validate_set = subparsers.add_parser(
        "validate-set",
        help="validate and compare normal v3, GDI v3, full v10, and base v10",
    )
    add_config_argument(validate_set)
    validate_set.add_argument("--normal", type=Path, required=True)
    validate_set.add_argument("--gdi", type=Path, required=True)
    validate_set.add_argument("--full", type=Path, required=True)
    validate_set.add_argument("--base", type=Path, required=True)
    validate_set.add_argument("--report", type=Path)
    validate_set.add_argument("--write-sidecars", action="store_true")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    arguments = create_parser().parse_args(argv)
    try:
        config = load_config(arguments.config)
        patches = verify_patch_series(config)
        if arguments.command == "verify-patches":
            print(
                f"{config['wine_source_commit']}: {len(patches)} ordered patches verified"
            )
        elif arguments.command == "build":
            output = prepare_and_build(
                arguments.work_dir,
                arguments.wine_repository,
                arguments.jobs,
                config,
                patches,
                prepare_only=arguments.prepare_only,
            )
            label = "Prepared source" if arguments.prepare_only else "Validated DLLs"
            print(f"{label}: {output}")
        elif arguments.command == "validate-dlls":
            report = {
                "schema_version": 1,
                "manifest_id": f"{config['manifest_id']}-dll-validation",
                "validated_at": datetime.now(timezone.utc).isoformat(),
                "wine_version": config["wine_version"],
                "build_date": config["build_date"],
                "source_date_epoch": config["source_date_epoch"],
                "wine_source_commit": config["wine_source_commit"],
                "patches": patches,
                "dlls": validate_dll_directory(arguments.dll_dir, config),
                "result": "ok",
            }
            if arguments.report:
                _atomic_write_json(arguments.report, report)
            print(f"Validated {len(report['dlls'])} PE32/i386 WebGL DLLs")
        elif arguments.command == "package":
            output, report = package_dlls(
                arguments.input,
                arguments.dll_dir,
                arguments.output,
                config,
            )
            print(
                f"Created {output} ({report['archive']['size']} bytes, "
                f"SHA-256 {report['archive']['sha256']})"
            )
        elif arguments.command == "validate":
            report = validate_archive(arguments.filesystem, config, arguments.profile)
            if arguments.report:
                _atomic_write_json(arguments.report, report)
            if arguments.write_sidecar:
                print(f"Wrote {write_sidecar(arguments.filesystem, report)}")
            print(
                f"{arguments.filesystem.name}: {report['archive']['entries']} entries, "
                f"CRC/PE/GL/cache validation passed"
            )
        elif arguments.command == "validate-set":
            report = validate_filesystem_set(
                arguments.normal,
                arguments.gdi,
                arguments.full,
                arguments.base,
                config,
            )
            if arguments.report:
                _atomic_write_json(arguments.report, report)
            if arguments.write_sidecars:
                paths = {
                    "normal-v3": arguments.normal,
                    "gdi-v3": arguments.gdi,
                    "full-v10": arguments.full,
                    "base-v10": arguments.base,
                }
                for profile_name, archive in paths.items():
                    write_sidecar(archive, report["archives"][profile_name])
            print(
                "Validated v3/v10 set: normal v3 == full v10; "
                "GDI differs only at user.reg; base has no WebGL DLL payload"
            )
        return 0
    except (OSError, subprocess.CalledProcessError, ValidationError) as error:
        print(f"error: {error}", file=sys.stderr)
        return error.returncode if isinstance(error, subprocess.CalledProcessError) else 1


if __name__ == "__main__":
    raise SystemExit(main())
