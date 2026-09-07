import json
import tempfile
import unittest
import zipfile
from pathlib import Path
from urllib.parse import parse_qs, urlsplit
from unittest import mock

import build_site


def create_zip(path, files=None):
    with zipfile.ZipFile(path, "w") as archive:
        for name, content in (files or {}).items():
            archive.writestr(name, content)


class DemoRootSelectionTests(unittest.TestCase):
    def test_v11_migrates_old_gdi_roots_and_leaves_other_demos_at_default(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            demo_source = Path(temp_dir)
            for name in ("TinyCore15Wine11.0.zip", "boxedwine.3.zip", "boxedwine.gdi.3.zip"):
                create_zip(demo_source / name)
            for name in ("gdi.zip", "normal.zip", "explicit.zip"):
                create_zip(demo_source / name, {"game.exe": b""})
            (demo_source / "demos.json").write_text(json.dumps({"demos": {
                "gdi.zip": {"exe": "game.exe", "root": "boxedwine.gdi.3.zip"},
                "normal.zip": {"exe": "game.exe", "root": "boxedwine.3.zip"},
                "explicit.zip": {"exe": "game.exe", "directDrawRenderer": "gdi"},
            }}))
            demos = {demo["zip"]: demo for demo in build_site.discover_demos(demo_source)}

        self.assertEqual({"gdi.zip", "normal.zip", "explicit.zip"}, set(demos))
        for name, expected in (("gdi.zip", "gdi"), ("normal.zip", None), ("explicit.zip", "gdi")):
            demo = demos[name]
            self.assertEqual("TinyCore15Wine11.0.zip", demo["root"])
            self.assertEqual(expected, demo["directDrawRenderer"])
            for url in (
                build_site.build_demo_launch_url("st", demo),
                build_site.demo_launch_url("branch", "1", "mt-jit", demo),
            ):
                query = parse_qs(urlsplit(url).query)
                if expected is None:
                    self.assertEqual(["game.exe"], query["p"])
                    self.assertNotIn("args", query)
                else:
                    self.assertEqual(["cmd"], query["p"])
                    self.assertIn(f'/v DirectDrawRenderer /t REG_SZ /d {expected} /f && "game.exe"', query["args"][0])

    def test_per_demo_root_is_used_and_root_zips_are_not_demos(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            site_dir = temp / "site"
            demo_source = temp / "demo-source"
            runner_source = temp / "runner"
            demo_source.mkdir()
            runner_source.mkdir()

            create_zip(demo_source / "boxedwine.3.zip")
            create_zip(demo_source / "boxedwine.gdi.3.zip")
            create_zip(demo_source / "boxedwine.2.zip")
            create_zip(demo_source / "boxedwine.gdi.2.zip")
            create_zip(demo_source / "ddraw.zip", {"ddraw.exe": b""})
            create_zip(demo_source / "d3d.zip", {"d3d.exe": b""})
            (demo_source / "demos.json").write_text(
                json.dumps(
                    {
                        "demos": {
                            "ddraw.zip": {
                                "exe": "ddraw.exe",
                                "root": "boxedwine.gdi.3.zip",
                            },
                            "d3d.zip": {"exe": "d3d.exe"},
                        }
                    }
                ),
                encoding="utf-8",
            )
            (runner_source / "boxedwine.html").write_text("", encoding="utf-8")

            build_site.update_demos(
                site_dir,
                "test/branch",
                "test__branch",
                "1",
                demo_source,
                [{"mode": "st", "label": "Single Threaded", "source": runner_source}],
                5,
            )

            build_dir = site_dir / "demos" / "build" / "test__branch" / "1"
            page = (build_dir / "index.html").read_text(encoding="utf-8")
            self.assertEqual(2, page.count('class="demo-card"'))
            self.assertIn("root=boxedwine.gdi.3.zip&amp;app=ddraw.zip", page)
            self.assertIn("root=boxedwine.3.zip&amp;app=d3d.zip", page)
            self.assertTrue((build_dir / "st" / "boxedwine.3.zip").exists())
            self.assertTrue((build_dir / "st" / "boxedwine.gdi.3.zip").exists())
            self.assertNotIn("Boxedwine.2", page)
            self.assertNotIn("Boxedwine.Gdi.2", page)

    def test_legacy_boxedwine_zip_remains_the_default_fallback(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            site_dir = temp / "site"
            demo_source = temp / "demo-source"
            runner_source = temp / "runner"
            demo_source.mkdir()
            runner_source.mkdir()

            create_zip(demo_source / "boxedwine.zip")
            create_zip(demo_source / "game.zip", {"game.exe": b""})
            (runner_source / "boxedwine.html").write_text("", encoding="utf-8")

            build_site.update_demos(
                site_dir,
                "legacy",
                "legacy",
                "1",
                demo_source,
                [{"mode": "st", "label": "Single Threaded", "source": runner_source}],
                5,
            )

            page = (
                site_dir / "demos" / "build" / "legacy" / "1" / "index.html"
            ).read_text(encoding="utf-8")
            self.assertIn("root=boxedwine.zip&amp;app=game.zip", page)


class DemoRootValidationTests(unittest.TestCase):
    def test_referenced_roots_use_exact_declared_profiles(self):
        config = {
            "profiles": {
                "normal-v3": {"filename": "boxedwine.3.zip"},
                "gdi-v3": {"filename": "boxedwine.gdi.3.zip"},
            }
        }
        with tempfile.TemporaryDirectory() as temp_dir:
            demo_source = Path(temp_dir)
            create_zip(demo_source / "boxedwine.3.zip")
            create_zip(demo_source / "boxedwine.gdi.3.zip")
            demos = [
                {"root": "boxedwine.3.zip"},
                {"root": "boxedwine.gdi.3.zip"},
            ]
            with (
                mock.patch.object(
                    build_site.webgl_filesystem,
                    "load_config",
                    return_value=config,
                ),
                mock.patch.object(
                    build_site.webgl_filesystem,
                    "verify_patch_series",
                ) as verify_patches,
                mock.patch.object(
                    build_site.webgl_filesystem,
                    "validate_archive",
                    side_effect=lambda path, _config, profile: {
                        "profile": profile,
                        "archive": {"filename": path.name},
                    },
                ) as validate_archive,
            ):
                reports = build_site.validate_demo_root_zips(
                    demo_source,
                    demos,
                    "roots.json",
                )

        verify_patches.assert_called_once_with(config)
        self.assertEqual(
            ["boxedwine.3.zip", "boxedwine.gdi.3.zip"],
            list(reports),
        )
        self.assertEqual(
            ["normal-v3", "gdi-v3"],
            [call.args[2] for call in validate_archive.call_args_list],
        )

    def test_missing_referenced_root_is_rejected(self):
        config = {
            "profiles": {
                "normal-v3": {"filename": "boxedwine.3.zip"},
            }
        }
        with tempfile.TemporaryDirectory() as temp_dir:
            with (
                mock.patch.object(
                    build_site.webgl_filesystem,
                    "load_config",
                    return_value=config,
                ),
                mock.patch.object(
                    build_site.webgl_filesystem,
                    "verify_patch_series",
                ),
                self.assertRaisesRegex(FileNotFoundError, "Demo root ZIP not found"),
            ):
                build_site.validate_demo_root_zips(
                    Path(temp_dir),
                    [{"root": "boxedwine.3.zip"}],
                    "roots.json",
                )

    def test_undeclared_referenced_root_is_rejected(self):
        config = {"profiles": {}}
        with tempfile.TemporaryDirectory() as temp_dir:
            demo_source = Path(temp_dir)
            create_zip(demo_source / "custom.zip")
            with (
                mock.patch.object(
                    build_site.webgl_filesystem,
                    "load_config",
                    return_value=config,
                ),
                mock.patch.object(
                    build_site.webgl_filesystem,
                    "verify_patch_series",
                ),
                self.assertRaisesRegex(
                    build_site.webgl_filesystem.ValidationError,
                    "not declared",
                ),
            ):
                build_site.validate_demo_root_zips(
                    demo_source,
                    [{"root": "custom.zip"}],
                    "roots.json",
                )

    def test_cli_validation_failure_does_not_create_site_outputs(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            site_dir = temp / "site"
            demo_source = temp / "demos"
            runner_source = temp / "runner"
            demo_source.mkdir()
            runner_source.mkdir()
            create_zip(demo_source / "boxedwine.3.zip")
            create_zip(demo_source / "game.zip", {"game.exe": b""})
            with (
                mock.patch.object(
                    build_site.webgl_filesystem,
                    "load_config",
                    side_effect=build_site.webgl_filesystem.ValidationError("stale root"),
                ),
                mock.patch(
                    "sys.argv",
                    [
                        "build_site.py",
                        "--site-dir",
                        str(site_dir),
                        "--demo-source",
                        str(demo_source),
                        "--single-threaded-dir",
                        str(runner_source),
                    ],
                ),
                self.assertRaisesRegex(SystemExit, "1"),
            ):
                build_site.main()

            self.assertFalse(site_dir.exists())


class DemoWindowsVersionTests(unittest.TestCase):
    def test_renderer_and_windows_version_share_one_launcher_and_preserve_args(self):
        demo = {
            "zip": "game.zip", "program": "Game Folder/game.exe",
            "windowsVersion": "win98", "directDrawRenderer": "gdi",
            "urlParams": [("args", "-setup"), ("bpp", "16")],
        }
        query = parse_qs(urlsplit(build_site.demo_launch_url("branch", "1", "mt", demo)).query)
        self.assertEqual(["cmd"], query["p"])
        self.assertEqual(["16"], query["bpp"])
        self.assertEqual(1, len(query["args"]))
        self.assertEqual(
            '/c reg add "HKCU\\Software\\Wine\\Direct3D" '
            '/v DirectDrawRenderer /t REG_SZ /d gdi /f && '
            'reg add "HKCU\\Software\\Wine\\AppDefaults\\game.exe" '
            '/v Version /t REG_SZ /d win98 /f && "Game Folder/game.exe" -setup',
            query["args"][0],
        )

    def test_renderer_can_wrap_an_existing_batch_file(self):
        program, params = build_site.apply_demo_settings("c:/c3.bat", [], direct_draw_renderer="gdi")
        self.assertEqual("cmd", program)
        self.assertTrue(dict(params)["args"].endswith('&& "c:/c3.bat"'))

    def test_invalid_renderer_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "Invalid DirectDraw renderer"):
            build_site.apply_demo_settings("game.exe", [], direct_draw_renderer="invalid")

    def test_windows_version_wraps_only_the_selected_demo(self):
        tomb_demo = {
            "zip": "TombRaider3.zip",
            "program": "tomb3.exe",
            "windowsVersion": "win98",
            "urlParams": [],
        }
        url = build_site.build_demo_launch_url("st", tomb_demo)

        self.assertIn("p=cmd", url)
        self.assertIn(
            "reg%20add%20%22HKCU%5CSoftware%5CWine%5CAppDefaults%5Ctomb3.exe%22",
            url,
        )
        self.assertIn("/d%20win98%20/f%20%26%26%20%22tomb3.exe%22", url)

    def test_windows_version_preserves_program_arguments(self):
        demo = {
            "zip": "game.zip",
            "program": "Game Folder/game.exe",
            "windowsVersion": "win95",
            "urlParams": [("args", "-setup"), ("bpp", "16")],
        }
        url = build_site.build_demo_launch_url("st", demo)

        self.assertIn("AppDefaults%5Cgame.exe", url)
        self.assertIn("%22Game%20Folder/game.exe%22%20-setup", url)
        self.assertIn("&bpp=16&", url)

    def test_invalid_windows_version_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "Invalid Wine Windows version"):
            build_site.build_demo_launch_url(
                "st",
                {
                    "zip": "game.zip",
                    "program": "game.exe",
                    "windowsVersion": "not-windows",
                    "urlParams": [],
                },
            )


if __name__ == "__main__":
    unittest.main()
