from __future__ import annotations

import hashlib
import json
from pathlib import Path
import shutil
import struct
import sys
import tempfile
import unittest
import warnings
import zipfile

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR))

import webgl_filesystem


def make_pe32(imports: list[str]) -> bytes:
    data = bytearray(0x600)
    data[:2] = b"MZ"
    struct.pack_into("<I", data, 0x3C, 0x80)
    pe = 0x80
    data[pe : pe + 4] = b"PE\0\0"
    struct.pack_into("<H", data, pe + 4, webgl_filesystem.PE_I386)
    struct.pack_into("<H", data, pe + 6, 1)
    struct.pack_into("<H", data, pe + 20, 0xE0)
    optional = pe + 24
    struct.pack_into("<H", data, optional, webgl_filesystem.PE32_MAGIC)
    struct.pack_into("<I", data, optional + 92, 16)
    struct.pack_into("<II", data, optional + 104, 0x1000, (len(imports) + 1) * 20)
    section = optional + 0xE0
    data[section : section + 8] = b".rdata\0\0"
    struct.pack_into("<IIII", data, section + 8, 0x400, 0x1000, 0x400, 0x200)
    name_offset = 0x300
    for index, name in enumerate(imports):
        encoded = name.encode("ascii") + b"\0"
        data[name_offset : name_offset + len(encoded)] = encoded
        name_rva = 0x1000 + name_offset - 0x200
        struct.pack_into("<I", data, 0x200 + index * 20 + 12, name_rva)
        name_offset += len(encoded)
    return bytes(data)


def file_spec(data: bytes, imports: list[str]) -> dict:
    return {
        "size": len(data),
        "sha256": hashlib.sha256(data).hexdigest(),
        "required_imports": imports,
    }


def minimal_config(dll_data: bytes) -> dict:
    gl = b"GL"
    cache = b"cache"
    return {
        "schema_version": 1,
        "manifest_id": "test-webgl-filesystem",
        "wine_version": "11.0",
        "build_date": "2026-07-30",
        "source_date_epoch": 1768319767,
        "wine_source_commit": "a" * 40,
        "patch_manifest": {"path": "manifest.json", "sha256": "b" * 64},
        "webgl_directory": "home/username/.wine/drive_c/webgl",
        "dlls": {
            "d3d9.dll": file_spec(dll_data, ["wined3d.dll"]),
        },
        "gl_files": {
            "lib/libGL.so.1": {
                "size": len(gl),
                "sha256": hashlib.sha256(gl).hexdigest(),
            },
            "etc/ld.so.cache": {
                "size": len(cache),
                "sha256": hashlib.sha256(cache).hexdigest(),
            },
        },
        "gl_links": {"lib/libGL.so.link": "libGL.so.1"},
        "ld_cache": {},
        "profiles": {},
    }


def write_root(
    path: Path,
    config: dict,
    dll_data: bytes,
    *,
    renderer: str | None,
    include_dll: bool = True,
) -> None:
    registry = (
        "[Software\\\\Wine\\\\Direct3D]\n"
        f'"DirectDrawRenderer"="{renderer}"\n'
        '"renderer"="gl"\n'
    )
    with zipfile.ZipFile(path, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        archive.writestr("lib/libGL.so.1", b"GL")
        archive.writestr("lib/libGL.so.link", b"libGL.so.1")
        archive.writestr("etc/ld.so.cache", b"cache")
        if renderer is not None:
            archive.writestr("home/username/.wine/user.reg", registry)
        if include_dll:
            archive.writestr(
                "home/username/.wine/drive_c/webgl/d3d9.dll",
                dll_data,
            )


def set_profile(config: dict, name: str, path: Path, **extra) -> None:
    config["profiles"][name] = {
        "filename": path.name,
        "filesystem_version": "test",
        "size": path.stat().st_size,
        "sha256": webgl_filesystem.sha256_file(path),
        **extra,
    }


class PeValidationTests(unittest.TestCase):
    def test_accepts_pe32_with_required_import(self):
        data = make_pe32(["kernel32.dll", "wined3d.dll"])
        result = webgl_filesystem.validate_dll_bytes(
            "d3d9.dll", data, file_spec(data, ["wined3d.dll"])
        )

        self.assertEqual(result["machine"], "PE32/i386")
        self.assertIn("wined3d.dll", result["imports"])

    def test_rejects_wrong_machine(self):
        data = bytearray(make_pe32(["wined3d.dll"]))
        struct.pack_into("<H", data, 0x84, 0x8664)
        value = bytes(data)

        with self.assertRaisesRegex(webgl_filesystem.ValidationError, "expected PE32/i386"):
            webgl_filesystem.validate_dll_bytes(
                "d3d9.dll", value, file_spec(value, ["wined3d.dll"])
            )

    def test_rejects_placeholder_marker_even_when_hash_matches(self):
        data = make_pe32(["wined3d.dll"]) + b"Wine builtin DLL"

        with self.assertRaisesRegex(webgl_filesystem.ValidationError, "placeholder/builtin"):
            webgl_filesystem.validate_dll_bytes(
                "d3d9.dll", data, file_spec(data, ["wined3d.dll"])
            )

    def test_rejects_missing_required_import(self):
        data = make_pe32(["kernel32.dll"])

        with self.assertRaisesRegex(webgl_filesystem.ValidationError, "missing required imports"):
            webgl_filesystem.validate_dll_bytes(
                "d3d9.dll", data, file_spec(data, ["wined3d.dll"])
            )


class PatchPinTests(unittest.TestCase):
    def test_verifies_manifest_commit_order_and_patch_hashes(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "patches").mkdir()
            first = root / "patches" / "first.patch"
            second = root / "patches" / "tests.patch"
            first.write_bytes(b"first\n")
            second.write_bytes(b"second\n")
            manifest = {
                "wine_source_commit": "a" * 40,
                "production_patches": [
                    {
                        "patch_id": "first",
                        "path": "patches/first.patch",
                        "sha256": webgl_filesystem.sha256_file(first),
                    }
                ],
                "test_patch": {
                    "patch_id": "tests",
                    "path": "patches/tests.patch",
                    "sha256": webgl_filesystem.sha256_file(second),
                },
            }
            manifest_path = root / "manifest.json"
            manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
            config = {
                "wine_source_commit": "a" * 40,
                "patch_manifest": {
                    "path": "manifest.json",
                    "sha256": webgl_filesystem.sha256_file(manifest_path),
                },
            }

            patches = webgl_filesystem.verify_patch_series(config, root)

        self.assertEqual([item["patch_id"] for item in patches], ["first", "tests"])

    def test_rejects_changed_patch_manifest(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            manifest = root / "manifest.json"
            manifest.write_text("{}", encoding="utf-8")
            config = {
                "wine_source_commit": "a" * 40,
                "patch_manifest": {
                    "path": "manifest.json",
                    "sha256": "0" * 64,
                },
            }

            with self.assertRaisesRegex(webgl_filesystem.ValidationError, "patch manifest SHA"):
                webgl_filesystem.verify_patch_series(config, root)


class ZipValidationTests(unittest.TestCase):
    def test_validates_profile_and_webgl_payload(self):
        dll_data = make_pe32(["wined3d.dll"])
        config = minimal_config(dll_data)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / "normal.zip"
            write_root(root, config, dll_data, renderer="opengl")
            set_profile(
                config,
                "normal-v3",
                root,
                direct_draw_renderer="opengl",
                wine_renderer="gl",
            )

            report = webgl_filesystem.validate_archive(root, config, "normal-v3")

        self.assertEqual(report["result"], "ok")
        self.assertEqual(report["filesystem_version"], "test")
        self.assertEqual(report["wine_version"], "11.0")
        self.assertEqual(report["build_date"], "2026-07-30")
        self.assertEqual(report["dlls"]["d3d9.dll"]["machine"], "PE32/i386")

    def test_rejects_duplicate_entries(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / "duplicate.zip"
            with zipfile.ZipFile(root, "w") as archive:
                archive.writestr("same", b"one")
                with warnings.catch_warnings():
                    warnings.simplefilter("ignore", UserWarning)
                    archive.writestr("same", b"two")

            with self.assertRaisesRegex(webgl_filesystem.ValidationError, "duplicate ZIP entry"):
                archive, _ = webgl_filesystem.open_validated_zip(root)
                archive.close()

    def test_rejects_unsafe_entries(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / "unsafe.zip"
            with zipfile.ZipFile(root, "w") as archive:
                archive.writestr("../escape", b"x")

            with self.assertRaisesRegex(webgl_filesystem.ValidationError, "unsafe ZIP entry"):
                archive, _ = webgl_filesystem.open_validated_zip(root)
                archive.close()

    def test_validates_complete_v3_v10_set(self):
        dll_data = make_pe32(["wined3d.dll"])
        config = minimal_config(dll_data)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            normal = root / "normal.zip"
            gdi = root / "gdi.zip"
            full = root / "full.zip"
            base = root / "base.zip"
            write_root(normal, config, dll_data, renderer="opengl")
            shutil.copyfile(normal, full)
            write_root(gdi, config, dll_data, renderer="gdi")
            write_root(base, config, dll_data, renderer=None, include_dll=False)
            set_profile(
                config,
                "normal-v3",
                normal,
                direct_draw_renderer="opengl",
                wine_renderer="gl",
            )
            set_profile(
                config,
                "gdi-v3",
                gdi,
                direct_draw_renderer="gdi",
                wine_renderer="gl",
            )
            set_profile(
                config,
                "full-v10",
                full,
                direct_draw_renderer="opengl",
                wine_renderer="gl",
            )
            set_profile(config, "base-v10", base, base_only=True)

            report = webgl_filesystem.validate_filesystem_set(
                normal, gdi, full, base, config
            )

        self.assertTrue(report["comparisons"]["normal_v3_equals_full_v10"])
        self.assertEqual(
            report["comparisons"]["normal_vs_gdi_content_differences"],
            ["home/username/.wine/user.reg"],
        )

    @unittest.skipUnless(shutil.which("zip"), "Info-ZIP is not installed")
    def test_packages_to_new_output_and_writes_validated_sidecar(self):
        dll_data = make_pe32(["wined3d.dll"])
        config = minimal_config(dll_data)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            source = root / "source.zip"
            output = root / "output.zip"
            dll_dir = root / "dlls"
            dll_dir.mkdir()
            (dll_dir / "d3d9.dll").write_bytes(dll_data)
            write_root(source, config, dll_data, renderer="opengl")
            source_hash = webgl_filesystem.sha256_file(source)

            result, report = webgl_filesystem.package_dlls(
                source, dll_dir, output, config
            )

            self.assertEqual(result, output)
            self.assertEqual(report["result"], "ok")
            self.assertTrue(Path(f"{output}.manifest.json").is_file())
            self.assertEqual(
                webgl_filesystem.sha256_file(source),
                source_hash,
            )


if __name__ == "__main__":
    unittest.main()
