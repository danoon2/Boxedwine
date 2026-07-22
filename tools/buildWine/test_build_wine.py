import contextlib
import io
import json
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path
from unittest import mock

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR))

import build_wine


class VersionParsingTests(unittest.TestCase):
    def test_parse_wine_tag_requires_wine_prefix(self):
        tag = build_wine.parse_wine_tag("wine-11.0")

        self.assertEqual(tag.tag, "wine-11.0")
        self.assertEqual(tag.version, "11.0")
        self.assertEqual(tag.components, (11, 0))

        with self.assertRaisesRegex(build_wine.BuildError, "Expected a Wine tag like wine-11.0"):
            build_wine.parse_wine_tag("11.0")

    def test_version_components_sort_numeric_parts(self):
        self.assertGreater(build_wine.parse_version("9.10"), build_wine.parse_version("9.9"))
        self.assertLess(build_wine.parse_version("10.0"), build_wine.parse_version("10.1"))

    def test_version_range_is_inclusive_and_allows_open_bounds(self):
        version = build_wine.parse_version("9.13")

        self.assertTrue(build_wine.version_in_range(version, {"min": "9.13", "max": "9.99"}))
        self.assertTrue(build_wine.version_in_range(version, {"max": "9.13"}))
        self.assertTrue(build_wine.version_in_range(version, {"min": "9.13"}))
        self.assertTrue(build_wine.version_in_range(version, {}))
        self.assertFalse(build_wine.version_in_range(version, {"min": "9.14"}))
        self.assertFalse(build_wine.version_in_range(version, {"max": "9.12"}))
        self.assertTrue(build_wine.version_in_range(build_wine.parse_version("9.13"), {"min": "9.13.0"}))
        self.assertTrue(build_wine.version_in_range(build_wine.parse_version("9.13.0"), {"max": "9.13"}))
        self.assertFalse(build_wine.version_in_range(build_wine.parse_version("9.13.1"), {"max": "9.13"}))


class ConfigOperationTests(unittest.TestCase):
    def test_select_operations_preserves_json_order(self):
        config = {
            "operations": [
                {"when": {"min": "6.23"}, "apply_patch": "always-after-623.patch", "reason": "first"},
                {"when": {"min": "9.13", "max": "9.99"}, "revert": "abc123", "reason": "second"},
                {"when": {"min": "10.0"}, "cherry_pick": "def456", "reason": "not for 9.13"},
                {"apply_patch": "all-tags.patch", "reason": "third"},
            ]
        }

        selected = build_wine.select_operations(config, build_wine.parse_version("9.13"))

        self.assertEqual([op.kind for op in selected], ["apply_patch", "revert", "apply_patch"])
        self.assertEqual([op.value for op in selected], ["always-after-623.patch", "abc123", "all-tags.patch"])
        self.assertEqual([op.reason for op in selected], ["first", "second", "third"])

    def test_operation_requires_one_supported_action(self):
        with self.assertRaisesRegex(build_wine.BuildError, "exactly one operation"):
            build_wine.operation_from_config({"apply_patch": "a.patch", "revert": "abc123"})

        with self.assertRaisesRegex(build_wine.BuildError, "exactly one operation"):
            build_wine.operation_from_config({"when": {"min": "1.0"}})

    def test_select_operations_rejects_malformed_config_shapes(self):
        version = build_wine.parse_version("9.13")

        with self.assertRaisesRegex(build_wine.BuildError, "Config must be an object"):
            build_wine.select_operations([], version)

        with self.assertRaisesRegex(build_wine.BuildError, "operations must be a list"):
            build_wine.select_operations({"operations": "bad"}, version)

        with self.assertRaisesRegex(build_wine.BuildError, "operation entries must be objects"):
            build_wine.select_operations({"operations": [42]}, version)

    def test_when_bounds_reject_unknown_keys_and_non_string_values(self):
        version = build_wine.parse_version("9.13")

        with self.assertRaisesRegex(build_wine.BuildError, "Unsupported when keys"):
            build_wine.select_operations(
                {"operations": [{"when": {"minimum": "9.0"}, "apply_patch": "x.patch"}]},
                version,
            )

        with self.assertRaisesRegex(build_wine.BuildError, "when min must be a string"):
            build_wine.select_operations(
                {"operations": [{"when": {"min": 9}, "apply_patch": "x.patch"}]},
                version,
            )

    def test_when_bounds_reject_impossible_ranges(self):
        with self.assertRaisesRegex(build_wine.BuildError, "when min must be <= max"):
            build_wine.select_operations(
                {"operations": [{"when": {"min": "11.0", "max": "10.0"}, "apply_patch": "x.patch"}]},
                build_wine.parse_version("11.0"),
            )

    def test_operation_rejects_unknown_keys(self):
        with self.assertRaisesRegex(build_wine.BuildError, "Unsupported operation keys"):
            build_wine.operation_from_config({"apply_patch": "a.patch", "reasn": "typo"})

    def test_apply_patch_requires_safe_relative_path(self):
        for patch_path in ("../bad.patch", "/tmp/bad.patch", "bad\\path.patch", "bad//path.patch"):
            with self.subTest(patch_path=patch_path):
                with self.assertRaisesRegex(build_wine.BuildError, "patch path must be a safe relative path"):
                    build_wine.select_operations(
                        {"operations": [{"apply_patch": patch_path}]},
                        build_wine.parse_version("11.0"),
                    )

    def test_select_operations_validates_unmatched_operations(self):
        version = build_wine.parse_version("9.13")

        with self.assertRaisesRegex(build_wine.BuildError, "Unsupported operation keys"):
            build_wine.select_operations(
                {"operations": [{"when": {"min": "10.0"}, "apply_patch": "x.patch", "reasn": "typo"}]},
                version,
            )

    def test_load_config_reads_json_file(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "wine_builds.json"
            path.write_text(json.dumps({"operations": []}), encoding="utf-8")

            self.assertEqual(build_wine.load_config(path), {"operations": []})


class ConfigLogAuditTests(unittest.TestCase):
    def test_default_config_disables_mingw_pe_builds(self):
        config = build_wine.load_config(SCRIPT_DIR / "wine_builds.json")

        configure = build_wine.configure_command(config)
        self.assertIn("--without-mingw", configure)
        env = build_wine.configure_environment(config, {})
        self.assertEqual(env["LD"], "ld -m elf_i386")
        self.assertEqual(env["TARGETFLAGS"], "-m32")

    def test_default_config_disables_pcap_without_requiring_it(self):
        config = build_wine.load_config(SCRIPT_DIR / "wine_builds.json")

        self.assertIn("--without-pcap", build_wine.configure_command(config))
        self.assertNotIn(
            "checking for pcap_init in -lpcap",
            [check.checking for check in build_wine.required_config_checks(config)],
        )

    def test_default_config_disables_gphoto_without_requiring_it(self):
        config = build_wine.load_config(SCRIPT_DIR / "wine_builds.json")
        check_names = [check.checking for check in build_wine.required_config_checks(config)]

        self.assertIn("--without-gphoto", build_wine.configure_command(config))
        self.assertNotIn("checking for gp_camera_new in -lgphoto2", check_names)
        self.assertNotIn("checking for gp_port_info_list_new in -lgphoto2_port", check_names)

    def test_default_config_disables_v4l2_without_requiring_it(self):
        config = build_wine.load_config(SCRIPT_DIR / "wine_builds.json")

        self.assertIn("--without-v4l2", build_wine.configure_command(config))
        self.assertNotIn(
            "checking for -lv4l2",
            [check.checking for check in build_wine.required_config_checks(config)],
        )

    def test_default_config_disables_enterprise_and_translation_extras_without_requiring_them(self):
        config = build_wine.load_config(SCRIPT_DIR / "wine_builds.json")
        configure = build_wine.configure_command(config)
        check_names = [check.checking for check in build_wine.required_config_checks(config)]

        self.assertIn("--without-capi", configure)
        self.assertIn("--without-krb5", configure)
        self.assertIn("--without-gssapi", configure)
        self.assertIn("--without-gettextpo", configure)
        self.assertNotIn("checking for -lodbc", check_names)
        self.assertNotIn("checking for capi20_register in -lcapi20", check_names)
        self.assertNotIn("checking for -lkrb5", check_names)
        self.assertNotIn("checking for -lgssapi_krb5", check_names)
        self.assertNotIn("checking for gettext-po.h", check_names)

    def test_validate_config_log_accepts_exact_and_regex_results(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            config_log = Path(temp_dir) / "config.log"
            config_log.write_text(
                "\n".join(
                    [
                        "configure:10: checking for -lGL",
                        "configure:11: result: libGL.so.1",
                        "configure:12: checking for wayland-scanner",
                        "configure:13: result: /usr/bin/wayland-scanner",
                    ]
                ),
                encoding="utf-8",
            )
            config = {
                "required_config_checks": [
                    {"checking": "checking for -lGL", "result": "libGL.so.1"},
                    {"checking": "checking for wayland-scanner", "result_regex": r".*/wayland-scanner"},
                ]
            }

            build_wine.validate_config_log(config, config_log)

    def test_validate_config_log_reports_missing_and_changed_results_with_apt_hint(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            config_log = Path(temp_dir) / "config.log"
            config_log.write_text(
                "\n".join(
                    [
                        "configure:10: checking for -lgnutls",
                        "configure:11: result: not found",
                    ]
                ),
                encoding="utf-8",
            )
            config = {
                "required_config_checks": [
                    {
                        "checking": "checking for -lgnutls",
                        "result": "libgnutls.so.30",
                        "apt": ["libgnutls28-dev:i386"],
                    },
                    {
                        "checking": "checking for -lSDL2",
                        "result": "libSDL2-2.0.so.0",
                        "apt": ["libsdl2-dev:i386"],
                    },
                ]
            }

            with self.assertRaises(build_wine.BuildError) as context:
                build_wine.validate_config_log(config, config_log)

            message = str(context.exception)
            self.assertIn("checking for -lgnutls", message)
            self.assertIn("expected libgnutls.so.30, got 'not found'", message)
            self.assertIn("checking for -lSDL2", message)
            self.assertIn("configure did not record this check", message)
            self.assertIn("sudo apt install libgnutls28-dev:i386 libsdl2-dev:i386", message)

    def test_validate_config_rejects_malformed_required_config_checks(self):
        with self.assertRaisesRegex(build_wine.BuildError, "required_config_checks must be a list"):
            build_wine.required_config_checks({"required_config_checks": "bad"})

        with self.assertRaisesRegex(build_wine.BuildError, "exactly one of result or result_regex"):
            build_wine.required_config_checks(
                {
                    "required_config_checks": [
                        {"checking": "checking for -lGL", "result": "libGL.so.1", "result_regex": ".*"}
                    ]
                }
            )

        with self.assertRaisesRegex(build_wine.BuildError, "apt must be a list"):
            build_wine.required_config_checks(
                {
                    "required_config_checks": [
                        {"checking": "checking for -lGL", "result": "libGL.so.1", "apt": "libgl-dev:i386"}
                    ]
                }
            )


class EnvironmentCheckTests(unittest.TestCase):
    def _checker(self, **overrides):
        values = dict(
            os_release={"ID": "debian", "ID_LIKE": ""},
            is_linux=True,
            is_wsl=False,
            work_dir=Path("/home/boxedwine"),
            command_exists=lambda name: True,
            gcc_m32_works=lambda: True,
            gxx_m32_works=lambda: True,
            x11_m32_works=lambda: True,
            freetype_m32_works=lambda: True,
            opengl_m32_works=lambda: True,
            egl_m32_works=lambda: True,
            vulkan_m32_works=lambda: True,
        )
        values.update(overrides)
        return build_wine.EnvironmentChecker(**values)

    def test_debian_environment_passes_when_tools_and_m32_work(self):
        checker = self._checker()

        checker.check()

    def test_non_debian_environment_reports_package_scope(self):
        checker = self._checker(
            os_release={"ID": "fedora", "ID_LIKE": ""},
        )

        with self.assertRaisesRegex(build_wine.BuildError, "Debian or Ubuntu"):
            checker.check()

    def test_wsl_windows_mount_checkout_is_rejected(self):
        checker = self._checker(
            os_release={"ID": "ubuntu", "ID_LIKE": "debian"},
            is_wsl=True,
            work_dir=Path("/mnt/c/Boxedwine/tools/buildWine"),
        )

        with self.assertRaises(build_wine.BuildError) as context:
            checker.check()

        message = str(context.exception)
        self.assertIn("WSL-native Linux filesystem", message)
        self.assertIn("/mnt/c/Boxedwine", message)

    def test_non_debian_environment_does_not_report_apt_hint(self):
        checker = self._checker(
            os_release={"ID": "fedora", "ID_LIKE": ""},
            command_exists=lambda name: False,
            gcc_m32_works=lambda: False,
            gxx_m32_works=lambda: False,
            x11_m32_works=lambda: False,
            freetype_m32_works=lambda: False,
            opengl_m32_works=lambda: False,
            egl_m32_works=lambda: False,
            vulkan_m32_works=lambda: False,
        )

        with self.assertRaises(build_wine.BuildError) as context:
            checker.check()

        message = str(context.exception)
        self.assertIn("Debian or Ubuntu", message)
        self.assertNotIn("Missing required commands", message)
        self.assertNotIn("sudo apt install", message)

    def test_debian_like_derivative_is_supported(self):
        checker = self._checker(
            os_release={"ID": "linuxmint", "ID_LIKE": "ubuntu debian"},
        )

        checker.check()

    def test_missing_tools_and_m32_failure_include_apt_hint(self):
        missing = {"git", "fontforge"}
        checker = self._checker(
            os_release={"ID": "ubuntu", "ID_LIKE": "debian"},
            is_wsl=True,
            command_exists=lambda name: name not in missing,
            gcc_m32_works=lambda: False,
            gxx_m32_works=lambda: False,
            x11_m32_works=lambda: False,
        )

        with self.assertRaises(build_wine.BuildError) as context:
            checker.check()

        message = str(context.exception)
        self.assertIn("Missing required commands: fontforge, git", message)
        self.assertIn("gcc -m32 could not compile and link a tiny program", message)
        self.assertIn("sudo apt install", message)
        apt_line = next(line for line in message.splitlines() if line.startswith("Install build dependencies with:"))
        self.assertIn(" git", f" {apt_line} ")
        self.assertIn("gcc-multilib", message)

    def test_missing_gxx_reports_apt_hint(self):
        missing = {"g++"}
        checker = self._checker(
            os_release={"ID": "ubuntu", "ID_LIKE": "debian"},
            is_wsl=True,
            command_exists=lambda name: name not in missing,
            gxx_m32_works=lambda: False,
        )

        with self.assertRaises(build_wine.BuildError) as context:
            checker.check()

        message = str(context.exception)
        self.assertIn("Missing required commands: g++", message)
        self.assertIn("g++-multilib", message)

    def test_missing_mingw_strip_reports_apt_hint(self):
        missing = {build_wine.MINGW_STRIP}
        checker = self._checker(
            os_release={"ID": "ubuntu", "ID_LIKE": "debian"},
            is_wsl=True,
            command_exists=lambda name: name not in missing,
        )

        with self.assertRaises(build_wine.BuildError) as context:
            checker.check()

        message = str(context.exception)
        self.assertIn(f"Missing required commands: {build_wine.MINGW_STRIP}", message)
        self.assertIn("binutils-mingw-w64-i686", message)

    def test_mingw_strip_is_not_required_when_mingw_pe_builds_are_disabled(self):
        missing = {build_wine.MINGW_STRIP}
        checker = self._checker(
            os_release={"ID": "ubuntu", "ID_LIKE": "debian"},
            is_wsl=True,
            command_exists=lambda name: name not in missing,
            mingw_required=False,
        )

        checker.check()

    def test_gxx_m32_failure_reports_apt_hint(self):
        checker = self._checker(
            os_release={"ID": "ubuntu", "ID_LIKE": "debian"},
            is_wsl=True,
            gxx_m32_works=lambda: False,
        )

        with self.assertRaises(build_wine.BuildError) as context:
            checker.check()

        message = str(context.exception)
        self.assertIn("g++ -m32 could not compile and link a tiny program", message)
        self.assertIn("g++-multilib", message)

    def test_x11_m32_failure_reports_i386_dev_hint(self):
        checker = self._checker(
            os_release={"ID": "ubuntu", "ID_LIKE": "debian"},
            is_wsl=True,
            x11_m32_works=lambda: False,
        )

        with self.assertRaises(build_wine.BuildError) as context:
            checker.check()

        message = str(context.exception)
        self.assertIn("gcc -m32 could not compile and link a tiny X11 program", message)
        self.assertIn("libx11-dev:i386", message)
        self.assertIn("sudo dpkg --add-architecture i386", message)

    def test_graphics_and_freetype_failures_report_i386_dev_hints(self):
        checker = self._checker(
            os_release={"ID": "ubuntu", "ID_LIKE": "debian"},
            is_wsl=True,
            freetype_m32_works=lambda: False,
            opengl_m32_works=lambda: False,
            egl_m32_works=lambda: False,
            vulkan_m32_works=lambda: False,
        )

        with self.assertRaises(build_wine.BuildError) as context:
            checker.check()

        message = str(context.exception)
        self.assertIn("gcc -m32 could not compile and link a tiny FreeType program", message)
        self.assertIn("gcc -m32 could not compile and link a tiny OpenGL program", message)
        self.assertIn("gcc -m32 could not compile and link a tiny EGL program", message)
        self.assertIn("gcc -m32 could not compile and link a tiny Vulkan program", message)
        self.assertIn("libfreetype-dev:i386", message)
        self.assertIn("libgl-dev:i386", message)
        self.assertIn("libegl-dev:i386", message)
        self.assertIn("libvulkan-dev:i386", message)

    def test_read_os_release_strips_single_and_double_quotes(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "os-release"
            path.write_text("ID='ubuntu'\nID_LIKE=\"debian\"\n", encoding="utf-8")

            self.assertEqual(build_wine.read_os_release(path), {"ID": "ubuntu", "ID_LIKE": "debian"})

    def test_default_freetype_probe_uses_i386_pkg_config_metadata(self):
        calls = []

        def fake_run(args, **kwargs):
            calls.append((list(args), kwargs))
            if args[:3] == ["pkg-config", "--cflags", "--libs"]:
                return build_wine.subprocess.CompletedProcess(args, 0, stdout="-I/usr/include/freetype2 -lfreetype\n", stderr="")
            return build_wine.subprocess.CompletedProcess(args, 0, stdout="", stderr="")

        with mock.patch.object(build_wine.subprocess, "run", side_effect=fake_run):
            self.assertTrue(build_wine.default_freetype_m32_works())

        self.assertEqual(calls[0][1]["env"]["PKG_CONFIG_LIBDIR"], build_wine.DEFAULT_I386_PKG_CONFIG_LIBDIR)


class CommandPlanningTests(unittest.TestCase):
    def test_configure_environment_uses_config_flags(self):
        config = {
            "build": {
                "cc": "gcc -m32",
                "cxx": "g++ -m32",
                "cflags": "-DNDEBUG -O2",
                "ld": "ld -m elf_i386",
                "targetflags": "-m32",
                "ldflags": "-s",
                "configure_args": ["--prefix=/opt/wine", "--disable-tests"],
            }
        }

        env = build_wine.configure_environment(config, {"PATH": "/usr/bin"})

        self.assertEqual(env["PATH"], "/usr/bin")
        self.assertEqual(env["CC"], "gcc -m32")
        self.assertEqual(env["CXX"], "g++ -m32")
        self.assertEqual(env["CFLAGS"], "-DNDEBUG -O2")
        self.assertEqual(env["LD"], "ld -m elf_i386")
        self.assertEqual(env["TARGETFLAGS"], "-m32")
        self.assertEqual(env["LDFLAGS"], "-s")
        self.assertEqual(env["PKG_CONFIG_LIBDIR"], build_wine.DEFAULT_I386_PKG_CONFIG_LIBDIR)

    def test_configure_command_uses_config_args(self):
        config = {
            "build": {
                "configure_args": ["--prefix=/opt/wine", "--disable-tests"],
            }
        }

        self.assertEqual(
            build_wine.configure_command(config),
            ["./configure", "--prefix=/opt/wine", "--disable-tests"],
        )

    def test_configure_helpers_reject_malformed_build_config(self):
        with self.assertRaisesRegex(build_wine.BuildError, "build config must be an object"):
            build_wine.configure_command({"build": "bad"})

        with self.assertRaisesRegex(build_wine.BuildError, "configure_args must be a list of strings"):
            build_wine.configure_command({"build": {"configure_args": "--disable-tests"}})

        with self.assertRaisesRegex(build_wine.BuildError, "cflags must be a string"):
            build_wine.configure_environment({"build": {"cflags": ["-O2"]}}, {})

        with self.assertRaisesRegex(build_wine.BuildError, "cc must be a string"):
            build_wine.configure_environment({"build": {"cc": ["gcc", "-m32"]}}, {})

        with self.assertRaisesRegex(build_wine.BuildError, "cxx must be a string"):
            build_wine.configure_environment({"build": {"cxx": ["g++", "-m32"]}}, {})

        with self.assertRaisesRegex(build_wine.BuildError, "targetflags must be a string"):
            build_wine.configure_environment({"build": {"targetflags": ["-m32"]}}, {})

    def test_configure_command_rejects_prefix_mismatch(self):
        config = {
            "build": {
                "prefix": "/opt/wine",
                "configure_args": ["--prefix=/usr/local"],
            }
        }

        with self.assertRaisesRegex(build_wine.BuildError, "build prefix must match configure_args"):
            build_wine.configure_command(config)

    def test_command_runner_prints_and_runs_command(self):
        runner = build_wine.CommandRunner()
        stdout = io.StringIO()
        env = {"CFLAGS": "-O2"}
        cwd = Path("/tmp/wine")

        with mock.patch.object(build_wine.subprocess, "run") as mocked_run:
            with contextlib.redirect_stdout(stdout):
                runner.run(["git", "apply", "patch with spaces.patch"], cwd=cwd, env=env)

        self.assertIn("+ git apply 'patch with spaces.patch'", stdout.getvalue())
        mocked_run.assert_called_once_with(
            ["git", "apply", "patch with spaces.patch"],
            cwd=cwd,
            env=env,
            check=True,
        )

    def test_command_runner_dry_run_does_not_execute(self):
        runner = build_wine.CommandRunner(dry_run=True)

        with mock.patch.object(build_wine.subprocess, "run") as mocked_run:
            runner.run(["make"])

        mocked_run.assert_not_called()

    def test_git_operation_command_mapping(self):
        patches = Path("/repo/tools/buildwine/patches")

        self.assertEqual(
            build_wine.operation_command(build_wine.Operation("apply_patch", "fix.patch", ""), patches),
            ["git", "apply", "/repo/tools/buildwine/patches/fix.patch"],
        )
        self.assertEqual(
            build_wine.operation_command(build_wine.Operation("cherry_pick", "abc123", ""), patches),
            ["git", "cherry-pick", "abc123", "--no-commit"],
        )
        self.assertEqual(
            build_wine.operation_command(build_wine.Operation("revert", "def456", ""), patches),
            ["git", "revert", "-n", "def456"],
        )


class StagingTests(unittest.TestCase):
    def test_prepare_staging_root_cleans_at_start(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            stage = Path(temp_dir) / "tmp_install"
            old_file = stage / "old.txt"
            old_file.parent.mkdir()
            old_file.write_text("old", encoding="utf-8")

            build_wine.prepare_staging_root(stage)

            self.assertTrue(stage.is_dir())
            self.assertFalse(old_file.exists())

    def test_prepare_staging_root_rejects_unexpected_directory_name(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            unsafe = Path(temp_dir) / "not_tmp_install"
            unsafe.mkdir()
            marker = unsafe / "marker.txt"
            marker.write_text("keep", encoding="utf-8")

            with self.assertRaisesRegex(build_wine.BuildError, "Refusing to clean staging path"):
                build_wine.prepare_staging_root(unsafe)

            self.assertTrue(marker.exists())

    def test_prepare_oss_headers_stages_sys_soundcard_and_cleans_at_start(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            source = root / "oss" / "soundcard.h"
            source.parent.mkdir()
            source.write_text("/* oss */\n", encoding="utf-8")
            oss_root = root / "tmp_oss"
            old_file = oss_root / "include" / "old.h"
            old_file.parent.mkdir(parents=True)
            old_file.write_text("old", encoding="utf-8")

            prepared = build_wine.prepare_oss_headers(source, oss_root)

            self.assertEqual(prepared, oss_root)
            self.assertEqual((oss_root / "include" / "sys" / "soundcard.h").read_text(encoding="utf-8"), "/* oss */\n")
            self.assertFalse(old_file.exists())
            self.assertEqual(source.read_text(encoding="utf-8"), "/* oss */\n")

    def test_prepare_oss_headers_rejects_unexpected_directory_name(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            source = root / "oss" / "soundcard.h"
            source.parent.mkdir()
            source.write_text("/* oss */\n", encoding="utf-8")
            unsafe = root / "oss-out"
            unsafe.mkdir()
            marker = unsafe / "marker.txt"
            marker.write_text("keep", encoding="utf-8")

            with self.assertRaisesRegex(build_wine.BuildError, "Refusing to clean OSS header path"):
                build_wine.prepare_oss_headers(source, unsafe)

            self.assertTrue(marker.exists())

    def test_regenerate_wine_fonts_runs_fontforge_from_fonts_directory(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            wine_dir = root / "wine-git"
            fonts_dir = wine_dir / "fonts"
            fonts_dir.mkdir(parents=True)
            (fonts_dir / "genttf.ff").write_text("Generate($2, \"ttf\", 0)\n", encoding="utf-8")
            (fonts_dir / "courier.sfd").write_text("SplineFontDB\n", encoding="utf-8")
            (fonts_dir / "symbol.sfd").write_text("SplineFontDB\n", encoding="utf-8")
            runner = FakeRunner()

            commands = build_wine.regenerate_wine_fonts(wine_dir, runner)

            self.assertEqual(
                commands,
                [
                    ["fontforge", "-script", "genttf.ff", "courier.sfd", "courier.ttf"],
                    ["fontforge", "-script", "genttf.ff", "symbol.sfd", "symbol.ttf"],
                ],
            )
            self.assertEqual([cwd for _args, cwd, _env in runner.commands], [fonts_dir, fonts_dir])

    def test_regenerate_wine_fonts_is_noop_when_wine_font_script_is_missing(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            wine_dir = Path(temp_dir) / "wine-git"
            wine_dir.mkdir()
            runner = FakeRunner()

            commands = build_wine.regenerate_wine_fonts(wine_dir, runner)

            self.assertEqual(commands, [])
            self.assertEqual(runner.commands, [])

    def test_write_metadata_files(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            stage = Path(temp_dir)

            build_wine.write_metadata(stage, build_wine.parse_wine_tag("wine-11.0"), ["git checkout wine-11.0"])

            self.assertEqual((stage / "wineVersion.txt").read_text(encoding="utf-8"), "11.0")
            self.assertEqual((stage / "name.txt").read_text(encoding="utf-8"), "Wine 11.0")
            self.assertFalse((stage / "version.txt").exists())
            self.assertEqual((stage / "build.txt").read_text(encoding="utf-8"), "git checkout wine-11.0\n")
            self.assertTrue((stage / "home" / "username").is_dir())

    def test_prune_modern_wine_layout(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            stage = Path(temp_dir)
            wine_dir = stage / "opt" / "wine" / "lib" / "wine" / "i386-unix"
            wine_dir.mkdir(parents=True)
            (wine_dir / "wine").write_text("loader", encoding="utf-8")
            (wine_dir / "winemenubuilder.exe.so").write_text("remove", encoding="utf-8")
            (wine_dir / "libwine.so.1").write_text("remove", encoding="utf-8")

            build_wine.prune_packaged_wine(stage)

            self.assertTrue((wine_dir / "wine").exists())
            self.assertFalse((wine_dir / "winemenubuilder.exe.so").exists())
            self.assertFalse((wine_dir / "libwine.so.1").exists())
            self.assertEqual((wine_dir / "libwine.so.1.link").read_text(encoding="utf-8"), "libwine.so.1.0")

    def test_prune_modern_wine_11_layout_without_libwine(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            stage = Path(temp_dir)
            wine_dir = stage / "opt" / "wine" / "lib" / "wine" / "i386-unix"
            wine_dir.mkdir(parents=True)
            (wine_dir / "wine").write_text("loader", encoding="utf-8")
            (wine_dir / "winemenubuilder.exe.so").write_text("remove", encoding="utf-8")

            build_wine.prune_packaged_wine(stage)

            self.assertTrue((wine_dir / "wine").exists())
            self.assertFalse((wine_dir / "winemenubuilder.exe.so").exists())
            self.assertFalse((wine_dir / "libwine.so.1.link").exists())

    def test_prune_modern_layout_requires_loader(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            stage = Path(temp_dir)
            wine_dir = stage / "opt" / "wine" / "lib" / "wine" / "i386-unix"
            wine_dir.mkdir(parents=True)

            with self.assertRaisesRegex(build_wine.BuildError, "Missing required Wine file"):
                build_wine.prune_packaged_wine(stage)

    def test_prune_old_wine_layout(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            stage = Path(temp_dir)
            old_wine_dir = stage / "opt" / "wine" / "lib" / "wine"
            old_lib_dir = stage / "opt" / "wine" / "lib"
            old_wine_dir.mkdir(parents=True)
            (old_wine_dir / "winemenubuilder.exe.so").write_text("remove", encoding="utf-8")
            (old_lib_dir / "libwine.so").write_text("remove", encoding="utf-8")
            (old_lib_dir / "libwine.so.1").write_text("remove", encoding="utf-8")

            build_wine.prune_packaged_wine(stage)

            self.assertFalse((old_wine_dir / "winemenubuilder.exe.so").exists())
            self.assertFalse((old_lib_dir / "libwine.so").exists())
            self.assertFalse((old_lib_dir / "libwine.so.1").exists())
            self.assertEqual((old_lib_dir / "libwine.so.link").read_text(encoding="utf-8"), "libwine.so.1.0")
            self.assertEqual((old_lib_dir / "libwine.so.1.link").read_text(encoding="utf-8"), "libwine.so.1.0")

    def test_prune_old_layout_requires_expected_files(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            stage = Path(temp_dir)
            (stage / "opt" / "wine" / "lib" / "wine").mkdir(parents=True)

            with self.assertRaisesRegex(build_wine.BuildError, "Missing required Wine file"):
                build_wine.prune_packaged_wine(stage)


class ZipPackagingTests(unittest.TestCase):
    def test_package_zip_copies_base_and_uses_forward_slash_entries(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            base_zip = root / "TinyCore15WineBase.zip"
            output_zip = root / "Wine-11.0.zip"
            stage = root / "tmp_install"
            (stage / "opt" / "wine").mkdir(parents=True)
            (stage / "opt" / "wine" / "wineVersion.txt").write_text("inside", encoding="utf-8")
            (stage / "home" / "username").mkdir(parents=True)

            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("bin/sh", "base")

            build_wine.package_zip(base_zip, stage, output_zip)

            with zipfile.ZipFile(output_zip, "r") as zf:
                names = set(zf.namelist())
                self.assertIn("bin/sh", names)
                self.assertIn("opt/wine/wineVersion.txt", names)
                self.assertIn("home/username/", names)
                self.assertEqual(zf.getinfo("home/username/").compress_type, zipfile.ZIP_DEFLATED)
                self.assertNotIn("opt\\wine\\wineVersion.txt", names)

    def test_package_zip_replaces_generated_metadata_entries_but_preserves_base_changes_and_version(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            base_zip = root / "TinyCore15WineBase.zip"
            output_zip = root / "Wine-11.0.zip"
            stage = root / "tmp_install"
            stage.mkdir()
            (stage / "name.txt").write_text("Wine 11.0", encoding="utf-8")
            (stage / "version.txt").write_text("repo version", encoding="utf-8")
            (stage / "changes.txt").write_text("repo changes", encoding="utf-8")

            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("name.txt", "Tiny Core Linux 15.0 + Wine Base")
                zf.writestr("version.txt", "base version")
                zf.writestr("changes.txt", "base changes")
                zf.writestr("bin/sh", "base")

            build_wine.package_zip(base_zip, stage, output_zip)

            with zipfile.ZipFile(output_zip, "r") as zf:
                names = zf.namelist()
                self.assertEqual(names.count("name.txt"), 1)
                self.assertEqual(names.count("version.txt"), 1)
                self.assertEqual(names.count("changes.txt"), 1)
                self.assertEqual(zf.read("name.txt"), b"Wine 11.0")
                self.assertEqual(zf.read("version.txt"), b"base version")
                self.assertEqual(zf.read("changes.txt"), b"base changes")
                self.assertEqual(zf.read("bin/sh"), b"base")
            self.assertFalse((stage / "version.txt").exists())
            self.assertFalse((stage / "changes.txt").exists())

    def test_package_zip_temp_name_does_not_delete_base_zip(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            output_zip = root / "Wine-11.0.zip"
            base_zip = root / f".{output_zip.name}.tmp"
            stage = root / "tmp_install"
            (stage / "opt" / "wine").mkdir(parents=True)
            (stage / "opt" / "wine" / "wineVersion.txt").write_text("inside", encoding="utf-8")
            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("bin/sh", "base")

            build_wine.package_zip(base_zip, stage, output_zip)

            self.assertTrue(base_zip.exists())
            with zipfile.ZipFile(output_zip, "r") as zf:
                self.assertIn("bin/sh", zf.namelist())
                self.assertIn("opt/wine/wineVersion.txt", zf.namelist())

    def test_package_zip_rejects_same_base_and_output(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            base_zip = root / "TinyCore15WineBase.zip"
            stage = root / "tmp_install"
            stage.mkdir()
            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("bin/sh", "base")

            with self.assertRaisesRegex(build_wine.BuildError, "base and output zip must be different"):
                build_wine.package_zip(base_zip, stage, base_zip)

            self.assertTrue(base_zip.exists())

    def test_package_zip_rejects_output_inside_stage(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            base_zip = root / "TinyCore15WineBase.zip"
            stage = root / "tmp_install"
            stage.mkdir()
            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("bin/sh", "base")

            with self.assertRaisesRegex(build_wine.BuildError, "output zip must not be inside staging"):
                build_wine.package_zip(base_zip, stage, stage / "Wine-11.0.zip")

    def test_package_zip_rejects_duplicate_file_entries_but_skips_duplicate_directories(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            base_zip = root / "TinyCore15WineBase.zip"
            output_zip = root / "Wine-11.0.zip"
            stage = root / "tmp_install"
            (stage / "home" / "username").mkdir(parents=True)
            (stage / "bin").mkdir()
            (stage / "bin" / "sh").write_text("stage", encoding="utf-8")
            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("home/username/", b"")
                zf.writestr("bin/sh", "base")

            with self.assertRaisesRegex(build_wine.BuildError, "Duplicate zip entry: bin/sh"):
                build_wine.package_zip(base_zip, stage, output_zip)

        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            base_zip = root / "TinyCore15WineBase.zip"
            output_zip = root / "Wine-11.0.zip"
            stage = root / "tmp_install"
            (stage / "home" / "username").mkdir(parents=True)
            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("home/username/", b"")

            build_wine.package_zip(base_zip, stage, output_zip)
            with zipfile.ZipFile(output_zip, "r") as zf:
                self.assertEqual(zf.namelist().count("home/username/"), 1)

    def test_package_zip_preserves_existing_output_when_append_fails(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            base_zip = root / "TinyCore15WineBase.zip"
            output_zip = root / "Wine-11.0.zip"
            stage = root / "tmp_install"
            (stage / "bin").mkdir(parents=True)
            (stage / "bin" / "sh").write_text("stage", encoding="utf-8")
            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("bin/sh", "base")
            with zipfile.ZipFile(output_zip, "w") as zf:
                zf.writestr("old.txt", "old")

            with self.assertRaisesRegex(build_wine.BuildError, "Duplicate zip entry: bin/sh"):
                build_wine.package_zip(base_zip, stage, output_zip)

            with zipfile.ZipFile(output_zip, "r") as zf:
                self.assertEqual(zf.read("old.txt"), b"old")


class BaseFilesystemTests(unittest.TestCase):
    def _config(self):
        return {
            "base_filesystem": {
                "url": "https://example.invalid/TinyCore15WineBase.zip",
                "filename": "TinyCore15WineBase.zip",
            }
        }

    def test_ensure_base_filesystem_rejects_corrupt_existing_zip(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            base_zip = root / "TinyCore15WineBase.zip"
            base_zip.write_text("not a zip", encoding="utf-8")

            with self.assertRaisesRegex(build_wine.BuildError, "Base filesystem is not a valid zip"):
                build_wine.ensure_base_filesystem(self._config(), root)

    def test_ensure_base_filesystem_downloads_to_temp_and_rejects_corrupt_download(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            base_zip = root / "TinyCore15WineBase.zip"

            def fake_download(_url, target):
                Path(target).write_text("not a zip", encoding="utf-8")

            with mock.patch.object(build_wine, "urlretrieve", side_effect=fake_download):
                with self.assertRaisesRegex(build_wine.BuildError, "Downloaded base filesystem is not a valid zip"):
                    build_wine.ensure_base_filesystem(self._config(), root)

            self.assertFalse(base_zip.exists())
            self.assertEqual(list(root.glob("*.tmp")), [])

    def test_ensure_base_filesystem_redownloads_existing_zip_when_config_size_differs(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            base_zip = root / "TinyCore15WineBase.zip"
            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("version.txt", "old")
            new_zip = io.BytesIO()
            with zipfile.ZipFile(new_zip, "w") as zf:
                zf.writestr("version.txt", "new base filesystem")
            config = self._config()
            config["base_filesystem"]["size"] = len(new_zip.getvalue())

            def fake_download(_url, target):
                Path(target).write_bytes(new_zip.getvalue())

            with mock.patch.object(build_wine, "urlretrieve", side_effect=fake_download):
                self.assertEqual(build_wine.ensure_base_filesystem(config, root), base_zip)

            with zipfile.ZipFile(base_zip, "r") as zf:
                self.assertEqual(zf.read("version.txt"), b"new base filesystem")

    def test_ensure_base_filesystem_redownloads_existing_zip_when_remote_size_differs(self):
        class FakeHeadResponse:
            def __init__(self, size):
                self.headers = {"Content-Length": str(size)}

            def __enter__(self):
                return self

            def __exit__(self, _exc_type, _exc, _traceback):
                return False

        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            base_zip = root / "TinyCore15WineBase.zip"
            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("version.txt", "old")
            new_zip = io.BytesIO()
            with zipfile.ZipFile(new_zip, "w") as zf:
                zf.writestr("version.txt", "new base filesystem")
            remote_size = len(new_zip.getvalue())

            def fake_download(_url, target):
                Path(target).write_bytes(new_zip.getvalue())

            with mock.patch("urllib.request.urlopen", return_value=FakeHeadResponse(remote_size)):
                with mock.patch.object(build_wine, "urlretrieve", side_effect=fake_download):
                    self.assertEqual(build_wine.ensure_base_filesystem(self._config(), root), base_zip)

            with zipfile.ZipFile(base_zip, "r") as zf:
                self.assertEqual(zf.read("version.txt"), b"new base filesystem")


class FakeRunner:
    def __init__(
        self,
        install_pe_binaries=False,
        initialize_wine_home=False,
        fail_wineboot_after_init=False,
        assert_clean_wine_home_zip=False,
    ):
        self.commands = []
        self.install_pe_binaries = install_pe_binaries
        self.initialize_wine_home = initialize_wine_home
        self.fail_wineboot_after_init = fail_wineboot_after_init
        self.assert_clean_wine_home_zip = assert_clean_wine_home_zip

    def run(self, args, cwd=None, env=None):
        args = list(args)
        self.commands.append((args, cwd, env))
        if args and args[0] == "./configure" and cwd is not None:
            (Path(cwd) / "config.log").write_text(
                "configure:1: checking for test feature\nconfigure:2: result: yes\n",
                encoding="utf-8",
            )
        if args[:2] == ["make", "install"]:
            destdir_arg = next(arg for arg in args if str(arg).startswith("DESTDIR="))
            stage = Path(str(destdir_arg).split("=", 1)[1])
            modern_dir = stage / "opt" / "wine" / "lib" / "wine" / "i386-unix"
            modern_dir.mkdir(parents=True)
            (modern_dir / "wine").write_text("fake loader", encoding="utf-8")
            (modern_dir / "libwine.so.1").write_text("fake wine", encoding="utf-8")
            if self.install_pe_binaries:
                windows_dir = stage / "opt" / "wine" / "lib" / "wine" / "i386-windows"
                windows_dir.mkdir(parents=True)
                (windows_dir / "kernel32.dll").write_text("fake pe dll", encoding="utf-8")
                (windows_dir / "notepad.exe").write_text("fake pe exe", encoding="utf-8")
                (windows_dir / "libkernel32.a").write_text("fake import lib", encoding="utf-8")
        if args and Path(str(args[0])).name == "boxedwine" and "-root" in args and self.initialize_wine_home:
            root_arg = args[args.index("-root") + 1]
            wine_home = Path(str(root_arg)) / "home" / "username" / ".wine"
            wine_home.mkdir(parents=True, exist_ok=True)
            if "/opt/wine/bin/wineboot" in args:
                if self.assert_clean_wine_home_zip:
                    zip_path = Path(str(args[args.index("-zip") + 1]))
                    with zipfile.ZipFile(zip_path, "r") as zf:
                        if any(name.startswith("home/username/.wine/") for name in zf.namelist()):
                            raise AssertionError("wineboot zip should not contain an existing .wine tree")
                (wine_home / "system.reg").write_text("fake registry", encoding="utf-8")
                (wine_home / "user.reg").write_text("WINE REGISTRY Version 2\n", encoding="utf-8")
                drive_c = wine_home / "drive_c"
                drive_c.mkdir()
                (drive_c / "users.txt").write_text("fake users", encoding="utf-8")
            if "/opt/wine/bin/wineboot" in args and self.fail_wineboot_after_init:
                raise build_wine.subprocess.CalledProcessError(1, args)


class OrchestrationTests(unittest.TestCase):
    def test_apply_window_manager_registry_updates_existing_user_reg_section(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            wine_home = Path(temp_dir) / ".wine"
            wine_home.mkdir()
            user_reg = wine_home / "user.reg"
            user_reg.write_text(
                "\n".join(
                    [
                        "WINE REGISTRY Version 2",
                        "",
                        "[Software\\\\Wine\\\\X11 Driver] 123456",
                        "#time=1",
                        '"Decorated"="Y"',
                        '"Other"="kept"',
                        "",
                        "[Software\\\\Other]",
                        '"Value"="kept"',
                    ]
                )
                + "\n",
                encoding="utf-8",
            )

            build_wine.apply_window_manager_registry(wine_home)

            text = user_reg.read_text(encoding="utf-8")
            self.assertIn('"Decorated"="N"', text)
            self.assertIn('"Managed"="N"', text)
            self.assertIn('"Other"="kept"', text)
            self.assertIn("[Software\\\\Other]", text)
            self.assertNotIn('"Decorated"="Y"', text)

    def test_build_wine_sequences_checkout_operations_build_install_and_package(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            wine_dir = root / "wine-git"
            wine_dir.mkdir()
            base_zip = root / "TinyCore15WineBase.zip"
            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("bin/sh", "base")
            patches_dir = root / "patches"
            patches_dir.mkdir()
            (patches_dir / "fix.patch").write_text("patch", encoding="utf-8")
            config = {
                "base_filesystem": {
                    "url": "https://example.invalid/base.zip",
                    "filename": "TinyCore15WineBase.zip",
                },
                "wine": {
                    "repo": "https://example.invalid/wine.git",
                    "source_dir": "wine-git",
                },
                "build": {
                    "configure_args": ["--prefix=/opt/wine", "--disable-tests"],
                    "cc": "gcc -m32",
                    "cxx": "g++ -m32",
                    "cflags": "-DNDEBUG",
                    "ldflags": "-s",
                },
                "operations": [
                    {"when": {"min": "11.0"}, "apply_patch": "fix.patch", "reason": "test patch"}
                ],
            }
            linux_dir = root / "project" / "linux"
            boxedwine = linux_dir / "Build" / "Release" / "boxedwine"
            boxedwine.parent.mkdir(parents=True)
            boxedwine.write_text("fake boxedwine", encoding="utf-8")
            runner = FakeRunner(install_pe_binaries=True, initialize_wine_home=True)

            output_zip = build_wine.build_wine(
                "wine-11.0",
                root,
                config,
                runner,
                jobs=3,
                patches_dir=patches_dir,
                check_environment=False,
            )

            command_texts = [build_wine.command_text(command) for command, _cwd, _env in runner.commands]
            self.assertEqual(output_zip, root / "Wine-11.0.zip")
            self.assertEqual(
                command_texts,
                [
                    "git fetch --tags --force",
                    "git reset --hard",
                    "git clean -d -f -x",
                    "git checkout wine-11.0",
                    f"git apply {patches_dir / 'fix.patch'}",
                    "./configure --prefix=/opt/wine --disable-tests",
                    "make -j3",
                    f"make install DESTDIR={root / 'tmp_install'}",
                    "i686-w64-mingw32-strip -s opt/wine/lib/wine/i386-windows/kernel32.dll opt/wine/lib/wine/i386-windows/notepad.exe",
                    "make release",
                    f"{boxedwine} -root {root / 'tmp_root'} -zip {root / 'Wine-11.0.zip'} -env WINEDLLOVERRIDES=mscoree,mshtml= /opt/wine/bin/wineboot -u",
                ],
            )
            self.assertTrue(all(cwd == wine_dir for _args, cwd, _env in runner.commands[:8]))
            self.assertEqual(runner.commands[8][1], root / "tmp_install")
            self.assertEqual(runner.commands[9][1], linux_dir)
            self.assertIsNone(runner.commands[10][1])
            configure_env = runner.commands[5][2]
            self.assertEqual(configure_env["CC"], "gcc -m32")
            self.assertEqual(configure_env["CXX"], "g++ -m32")
            self.assertEqual(configure_env["CFLAGS"], "-DNDEBUG")
            self.assertEqual(configure_env["LDFLAGS"], "-s")
            self.assertTrue((root / "tmp_install" / "build.txt").exists())
            build_txt = (root / "tmp_install" / "build.txt").read_text(encoding="utf-8")
            self.assertIn("Requested tag: wine-11.0", build_txt)
            self.assertIn("Wine version: 11.0", build_txt)
            self.assertIn("Base filesystem: TinyCore15WineBase.zip", build_txt)
            self.assertIn("Output zip: Wine-11.0.zip", build_txt)
            self.assertTrue(output_zip.exists())
            with zipfile.ZipFile(output_zip, "r") as zf:
                self.assertEqual(zf.read("home/username/.wine/system.reg"), b"fake registry")
                user_reg = zf.read("home/username/.wine/user.reg").decode("utf-8")
                self.assertIn('"Decorated"="N"', user_reg)
                self.assertIn('"Managed"="N"', user_reg)
                self.assertEqual(zf.read("home/username/.wine/drive_c/users.txt"), b"fake users")

    def test_build_wine_skips_pe_strip_when_mingw_pe_builds_are_disabled(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            (root / "wine-git").mkdir()
            with zipfile.ZipFile(root / "TinyCore15WineBase.zip", "w") as zf:
                zf.writestr("bin/sh", "base")
            patches_dir = root / "patches"
            patches_dir.mkdir()
            linux_dir = root / "project" / "linux"
            boxedwine = linux_dir / "Build" / "Release" / "boxedwine"
            boxedwine.parent.mkdir(parents=True)
            boxedwine.write_text("fake boxedwine", encoding="utf-8")
            config = {
                "base_filesystem": {
                    "url": "https://example.invalid/base.zip",
                    "filename": "TinyCore15WineBase.zip",
                },
                "wine": {
                    "repo": "https://example.invalid/wine.git",
                    "source_dir": "wine-git",
                },
                "build": {
                    "configure_args": ["--without-mingw", "--prefix=/opt/wine", "--disable-tests"],
                },
                "operations": [],
            }
            runner = FakeRunner(install_pe_binaries=True, initialize_wine_home=True)

            build_wine.build_wine(
                "wine-11.0",
                root,
                config,
                runner,
                jobs=3,
                patches_dir=patches_dir,
                check_environment=False,
            )

            command_texts = [build_wine.command_text(command) for command, _cwd, _env in runner.commands]
            self.assertNotIn(
                "i686-w64-mingw32-strip -s opt/wine/lib/wine/i386-windows/kernel32.dll opt/wine/lib/wine/i386-windows/notepad.exe",
                command_texts,
            )

    def test_strip_packaged_pe_binaries_strips_installed_dlls_and_exes(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            stage = Path(temp_dir) / "tmp_install"
            pe_dir = stage / "opt" / "wine" / "lib" / "wine" / "i386-windows"
            pe_dir.mkdir(parents=True)
            (pe_dir / "a.dll").write_text("dll", encoding="utf-8")
            (pe_dir / "b.exe").write_text("exe", encoding="utf-8")
            (pe_dir / "c.a").write_text("import lib", encoding="utf-8")
            runner = FakeRunner()

            commands = build_wine.strip_packaged_pe_binaries(stage, runner)

            self.assertEqual(
                commands,
                [["i686-w64-mingw32-strip", "-s", "opt/wine/lib/wine/i386-windows/a.dll", "opt/wine/lib/wine/i386-windows/b.exe"]],
            )
            self.assertEqual(runner.commands, [(commands[0], stage, None)])

    def test_initialize_wine_home_runs_boxedwine_wineboot_and_merges_wine_home_into_zip(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            work_dir = Path(temp_dir) / "tools" / "buildWine"
            work_dir.mkdir(parents=True)
            output_zip = work_dir / "Wine-11.0.zip"
            with zipfile.ZipFile(output_zip, "w") as zf:
                zf.writestr("bin/sh", "base")
            old_root = work_dir / "tmp_root"
            old_root.mkdir()
            (old_root / "old.txt").write_text("old", encoding="utf-8")
            linux_dir = Path(temp_dir) / "project" / "linux"
            boxedwine = linux_dir / "Build" / "Release" / "boxedwine"
            boxedwine.parent.mkdir(parents=True)
            boxedwine.write_text("fake boxedwine", encoding="utf-8")
            runner = FakeRunner(initialize_wine_home=True)

            commands = build_wine.initialize_wine_home(work_dir, output_zip, runner)

            self.assertFalse((old_root / "old.txt").exists())
            self.assertEqual(
                commands,
                [
                    ["make", "release"],
                    [
                        str(boxedwine),
                        "-root",
                        str(work_dir / "tmp_root"),
                        "-zip",
                        str(output_zip),
                        "-env",
                        "WINEDLLOVERRIDES=mscoree,mshtml=",
                        "/opt/wine/bin/wineboot",
                        "-u",
                    ],
                ],
            )
            self.assertEqual(runner.commands[0], (commands[0], linux_dir, None))
            self.assertEqual(runner.commands[1], (commands[1], None, None))
            reg_file = work_dir / "tmp_root" / "home" / "username" / "boxedwine-window-manager.reg"
            self.assertIn('"Decorated"="N"', reg_file.read_text(encoding="ascii"))
            self.assertIn('"Managed"="N"', reg_file.read_text(encoding="ascii"))
            with zipfile.ZipFile(output_zip, "r") as zf:
                self.assertEqual(zf.read("bin/sh"), b"base")
                self.assertEqual(zf.read("home/username/.wine/system.reg"), b"fake registry")
                user_reg = zf.read("home/username/.wine/user.reg").decode("utf-8")
                self.assertIn('"Decorated"="N"', user_reg)
                self.assertIn('"Managed"="N"', user_reg)
                self.assertEqual(zf.read("home/username/.wine/drive_c/users.txt"), b"fake users")

    def test_initialize_wine_home_accepts_wineboot_exit_1_when_wine_home_exists(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            work_dir = Path(temp_dir) / "tools" / "buildWine"
            work_dir.mkdir(parents=True)
            output_zip = work_dir / "Wine-11.0.zip"
            with zipfile.ZipFile(output_zip, "w") as zf:
                zf.writestr("bin/sh", "base")
            linux_dir = Path(temp_dir) / "project" / "linux"
            boxedwine = linux_dir / "Build" / "Release" / "boxedwine"
            boxedwine.parent.mkdir(parents=True)
            boxedwine.write_text("fake boxedwine", encoding="utf-8")
            runner = FakeRunner(initialize_wine_home=True, fail_wineboot_after_init=True)

            build_wine.initialize_wine_home(work_dir, output_zip, runner)

            with zipfile.ZipFile(output_zip, "r") as zf:
                self.assertEqual(zf.read("home/username/.wine/system.reg"), b"fake registry")

    def test_initialize_wine_home_removes_existing_wine_home_from_zip_before_wineboot(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            work_dir = Path(temp_dir) / "tools" / "buildWine"
            work_dir.mkdir(parents=True)
            output_zip = work_dir / "Wine-11.0.zip"
            with zipfile.ZipFile(output_zip, "w") as zf:
                zf.writestr("bin/sh", "base")
                zf.writestr("home/username/.wine/system.reg", "old registry")
            linux_dir = Path(temp_dir) / "project" / "linux"
            boxedwine = linux_dir / "Build" / "Release" / "boxedwine"
            boxedwine.parent.mkdir(parents=True)
            boxedwine.write_text("fake boxedwine", encoding="utf-8")
            runner = FakeRunner(initialize_wine_home=True, assert_clean_wine_home_zip=True)

            build_wine.initialize_wine_home(work_dir, output_zip, runner)

            with zipfile.ZipFile(output_zip, "r") as zf:
                self.assertEqual(zf.read("home/username/.wine/system.reg"), b"fake registry")

    def test_build_wine_audits_config_log_after_configure_before_make(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            wine_dir = root / "wine-git"
            wine_dir.mkdir()
            base_zip = root / "TinyCore15WineBase.zip"
            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("bin/sh", "base")
            patches_dir = root / "patches"
            patches_dir.mkdir()
            config = {
                "base_filesystem": {"url": "https://example.invalid/base.zip", "filename": "TinyCore15WineBase.zip"},
                "wine": {"repo": "https://example.invalid/wine.git", "source_dir": "wine-git"},
                "build": {"configure_args": ["--prefix=/opt/wine"]},
                "operations": [],
                "required_config_checks": [
                    {"checking": "checking for test feature", "result": "enabled", "apt": ["test-dev:i386"]}
                ],
            }
            runner = FakeRunner()

            with self.assertRaisesRegex(build_wine.BuildError, "checking for test feature"):
                build_wine.build_wine(
                    "wine-11.0",
                    root,
                    config,
                    runner,
                    jobs=1,
                    patches_dir=patches_dir,
                    check_environment=False,
                    initialize_home=False,
                )

            command_texts = [build_wine.command_text(command) for command, _cwd, _env in runner.commands]
            self.assertEqual(
                command_texts,
                [
                    "git fetch --tags --force",
                    "git reset --hard",
                    "git clean -d -f -x",
                    "git checkout wine-11.0",
                    "./configure --prefix=/opt/wine",
                ],
            )

    def test_build_wine_regenerates_fonts_after_operations_before_configure(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            wine_dir = root / "wine-git"
            fonts_dir = wine_dir / "fonts"
            fonts_dir.mkdir(parents=True)
            (fonts_dir / "genttf.ff").write_text("Generate($2, \"ttf\", 0)\n", encoding="utf-8")
            (fonts_dir / "courier.sfd").write_text("SplineFontDB\n", encoding="utf-8")
            base_zip = root / "TinyCore15WineBase.zip"
            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("bin/sh", "base")
            patches_dir = root / "patches"
            patches_dir.mkdir()
            config = {
                "base_filesystem": {"url": "https://example.invalid/base.zip", "filename": "TinyCore15WineBase.zip"},
                "wine": {"repo": "https://example.invalid/wine.git", "source_dir": "wine-git"},
                "build": {"configure_args": ["--prefix=/opt/wine"]},
                "operations": [],
            }
            runner = FakeRunner()

            build_wine.build_wine(
                "wine-11.0",
                root,
                config,
                runner,
                jobs=1,
                patches_dir=patches_dir,
                check_environment=False,
                initialize_home=False,
            )

            command_texts = [build_wine.command_text(command) for command, _cwd, _env in runner.commands]
            self.assertEqual(
                command_texts,
                [
                    "git fetch --tags --force",
                    "git reset --hard",
                    "git clean -d -f -x",
                    "git checkout wine-11.0",
                    "fontforge -script genttf.ff courier.sfd courier.ttf",
                    "./configure --prefix=/opt/wine",
                    "make -j1",
                    f"make install DESTDIR={root / 'tmp_install'}",
                ],
            )
            self.assertEqual(runner.commands[4][1], fonts_dir)
            build_txt = (root / "tmp_install" / "build.txt").read_text(encoding="utf-8")
            self.assertIn("fontforge -script genttf.ff courier.sfd courier.ttf", build_txt)

    def test_build_wine_stages_oss_header_and_sets_osslibdir_for_with_oss(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            wine_dir = root / "wine-git"
            wine_dir.mkdir()
            (root / "oss").mkdir()
            (root / "oss" / "soundcard.h").write_text("/* oss */\n", encoding="utf-8")
            base_zip = root / "TinyCore15WineBase.zip"
            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("bin/sh", "base")
            patches_dir = root / "patches"
            patches_dir.mkdir()
            config = {
                "base_filesystem": {"url": "https://example.invalid/base.zip", "filename": "TinyCore15WineBase.zip"},
                "wine": {"repo": "https://example.invalid/wine.git", "source_dir": "wine-git"},
                "build": {
                    "configure_args": ["--prefix=/opt/wine", "--with-oss"],
                    "cc": "gcc -m32",
                },
                "operations": [],
            }
            runner = FakeRunner()

            build_wine.build_wine(
                "wine-11.0",
                root,
                config,
                runner,
                jobs=1,
                patches_dir=patches_dir,
                check_environment=False,
                initialize_home=False,
            )

            oss_root = root / "tmp_oss"
            self.assertEqual((oss_root / "include" / "sys" / "soundcard.h").read_text(encoding="utf-8"), "/* oss */\n")
            configure_env = runner.commands[4][2]
            self.assertEqual(configure_env["OSSLIBDIR"], str(oss_root))
            build_txt = (root / "tmp_install" / "build.txt").read_text(encoding="utf-8")
            self.assertIn(f"OSSLIBDIR={oss_root}", build_txt)

    def test_build_wine_rejects_missing_oss_header_before_cleanup_when_oss_is_required(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            stage = root / "tmp_install"
            stage.mkdir()
            marker = stage / "keep.txt"
            marker.write_text("keep", encoding="utf-8")
            base_zip = root / "TinyCore15WineBase.zip"
            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("bin/sh", "base")
            config = {
                "base_filesystem": {"url": "https://example.invalid/base.zip", "filename": "TinyCore15WineBase.zip"},
                "wine": {"repo": "https://example.invalid/wine.git", "source_dir": "wine-git"},
                "build": {"configure_args": ["--prefix=/opt/wine", "--with-oss"]},
                "operations": [],
            }
            runner = FakeRunner()

            with self.assertRaisesRegex(build_wine.BuildError, "Missing OSS soundcard.h"):
                build_wine.build_wine(
                    "wine-11.0",
                    root,
                    config,
                    runner,
                    jobs=1,
                    patches_dir=root / "patches",
                    check_environment=False,
                    initialize_home=False,
                )

            self.assertEqual(runner.commands, [])
            self.assertTrue(marker.exists())

    def test_build_wine_rejects_unsafe_source_dir_before_git_clean(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            stage = root / "tmp_install"
            stage.mkdir()
            marker = stage / "keep.txt"
            marker.write_text("keep", encoding="utf-8")
            output_zip = root / "Wine-11.0.zip"
            output_zip.write_text("old", encoding="utf-8")
            base_zip = root / "TinyCore15WineBase.zip"
            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("bin/sh", "base")
            config = {
                "base_filesystem": {"url": "https://example.invalid/base.zip", "filename": "TinyCore15WineBase.zip"},
                "wine": {"repo": "https://example.invalid/wine.git", "source_dir": "."},
                "build": {"configure_args": []},
                "operations": [],
            }
            runner = FakeRunner()

            with self.assertRaisesRegex(
                build_wine.BuildError,
                "wine source_dir must be a safe relative directory name",
            ):
                build_wine.build_wine(
                    "wine-11.0",
                    root,
                    config,
                    runner,
                    jobs=1,
                    patches_dir=root / "patches",
                    check_environment=False,
                    initialize_home=False,
                )

            self.assertEqual(runner.commands, [])
            self.assertTrue(marker.exists())
            self.assertEqual(output_zip.read_text(encoding="utf-8"), "old")

    def test_build_wine_rejects_unsafe_base_filename(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            stage = root / "tmp_install"
            stage.mkdir()
            marker = stage / "keep.txt"
            marker.write_text("keep", encoding="utf-8")
            config = {
                "base_filesystem": {"url": "https://example.invalid/base.zip", "filename": "../TinyCore15WineBase.zip"},
                "wine": {"repo": "https://example.invalid/wine.git", "source_dir": "wine-git"},
                "build": {"configure_args": []},
                "operations": [],
            }
            runner = FakeRunner()

            with mock.patch.object(build_wine, "urlretrieve", side_effect=AssertionError("urlretrieve should not be called")):
                with self.assertRaisesRegex(build_wine.BuildError, "base filename must be a safe relative filename"):
                    build_wine.build_wine(
                        "wine-11.0",
                        root,
                        config,
                        runner,
                        jobs=1,
                        patches_dir=root / "patches",
                        check_environment=False,
                        initialize_home=False,
                    )

            self.assertEqual(runner.commands, [])
            self.assertTrue(marker.exists())

    def test_build_wine_rejects_missing_selected_patch_before_cleanup(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            stage = root / "tmp_install"
            stage.mkdir()
            marker = stage / "keep.txt"
            marker.write_text("keep", encoding="utf-8")
            base_zip = root / "TinyCore15WineBase.zip"
            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("bin/sh", "base")
            (root / "wine-git").mkdir()
            patches_dir = root / "patches"
            patches_dir.mkdir()
            config = {
                "base_filesystem": {"url": "https://example.invalid/base.zip", "filename": "TinyCore15WineBase.zip"},
                "wine": {"repo": "https://example.invalid/wine.git", "source_dir": "wine-git"},
                "build": {"configure_args": ["--prefix=/opt/wine"]},
                "operations": [{"when": {"min": "11.0"}, "apply_patch": "missing.patch"}],
            }
            runner = FakeRunner()

            with self.assertRaisesRegex(build_wine.BuildError, "Missing patch file"):
                build_wine.build_wine(
                    "wine-11.0",
                    root,
                    config,
                    runner,
                    jobs=1,
                    patches_dir=patches_dir,
                    check_environment=False,
                    initialize_home=False,
                )

            self.assertEqual(runner.commands, [])
            self.assertTrue(marker.exists())

    def test_build_wine_uses_normalized_copy_for_crlf_patches(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            wine_dir = root / "wine-git"
            wine_dir.mkdir()
            base_zip = root / "TinyCore15WineBase.zip"
            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("bin/sh", "base")
            patches_dir = root / "patches"
            patches_dir.mkdir()
            patch_file = patches_dir / "fix.patch"
            patch_file.write_bytes(b"diff --git a/a b/a\r\n--- a/a\r\n+++ b/a\r\n")
            config = {
                "base_filesystem": {"url": "https://example.invalid/base.zip", "filename": "TinyCore15WineBase.zip"},
                "wine": {"repo": "https://example.invalid/wine.git", "source_dir": "wine-git"},
                "build": {"configure_args": ["--prefix=/opt/wine"]},
                "operations": [{"when": {"min": "11.0"}, "apply_patch": "fix.patch"}],
            }
            runner = FakeRunner()

            build_wine.build_wine(
                "wine-11.0",
                root,
                config,
                runner,
                jobs=1,
                patches_dir=patches_dir,
                check_environment=False,
                initialize_home=False,
            )

            normalized_patch = root / "tmp_patches" / "fix.patch"
            apply_command = runner.commands[4][0]
            self.assertEqual(apply_command, ["git", "apply", str(normalized_patch)])
            self.assertEqual(normalized_patch.read_bytes(), b"diff --git a/a b/a\n--- a/a\n+++ b/a\n")
            self.assertIn(b"\r\n", patch_file.read_bytes())

    def test_build_wine_rejects_malformed_required_config_before_cleanup(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            stage = root / "tmp_install"
            stage.mkdir()
            marker = stage / "keep.txt"
            marker.write_text("keep", encoding="utf-8")
            config = {
                "base_filesystem": "bad",
                "wine": {"repo": "https://example.invalid/wine.git", "source_dir": "wine-git"},
                "build": {"configure_args": []},
                "operations": [],
            }
            runner = FakeRunner()

            with self.assertRaisesRegex(build_wine.BuildError, "base_filesystem config must be an object"):
                build_wine.build_wine(
                    "wine-11.0",
                    root,
                    config,
                    runner,
                    jobs=1,
                    patches_dir=root / "patches",
                    check_environment=False,
                    initialize_home=False,
                )

            self.assertEqual(runner.commands, [])
            self.assertTrue(marker.exists())

    def test_build_wine_rejects_malformed_operations_before_cleanup(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            stage = root / "tmp_install"
            stage.mkdir()
            marker = stage / "keep.txt"
            marker.write_text("keep", encoding="utf-8")
            base_zip = root / "TinyCore15WineBase.zip"
            with zipfile.ZipFile(base_zip, "w") as zf:
                zf.writestr("bin/sh", "base")
            (root / "wine-git").mkdir()
            config = {
                "base_filesystem": {"url": "https://example.invalid/base.zip", "filename": "TinyCore15WineBase.zip"},
                "wine": {"repo": "https://example.invalid/wine.git", "source_dir": "wine-git"},
                "build": {"configure_args": ["--prefix=/opt/wine"]},
                "operations": [{"when": {"min": "11.0"}, "apply_patch": "x.patch", "reasn": "typo"}],
            }
            runner = FakeRunner()

            with self.assertRaisesRegex(build_wine.BuildError, "Unsupported operation keys"):
                build_wine.build_wine(
                    "wine-11.0",
                    root,
                    config,
                    runner,
                    jobs=1,
                    patches_dir=root / "patches",
                    check_environment=False,
                    initialize_home=False,
                )

            self.assertEqual(runner.commands, [])
            self.assertTrue(marker.exists())


class CliTests(unittest.TestCase):
    def test_main_rejects_bare_version(self):
        stderr = io.StringIO()
        with contextlib.redirect_stderr(stderr):
            with self.assertRaises(SystemExit) as context:
                build_wine.main(["11.0"])

        self.assertEqual(context.exception.code, 1)
        self.assertIn("Expected a Wine tag like wine-11.0", stderr.getvalue())

    def test_main_loads_default_config_and_invokes_build(self):
        with mock.patch.object(build_wine, "build_wine", return_value=Path("/tmp/Wine-11.0.zip")) as mocked:
            with mock.patch.object(build_wine, "load_config", return_value={"operations": []}):
                stdout = io.StringIO()
                with contextlib.redirect_stdout(stdout):
                    result = build_wine.main(["wine-11.0", "--jobs", "2", "--skip-env-check"])

        self.assertEqual(result, 0)
        self.assertIn("Created /tmp/Wine-11.0.zip", stdout.getvalue())
        self.assertEqual(mocked.call_args.args[0], "wine-11.0")
        self.assertEqual(mocked.call_args.kwargs["jobs"], 2)
        self.assertFalse(mocked.call_args.kwargs["check_environment"])

    def test_main_rejects_non_positive_jobs(self):
        for value in ("0", "-1"):
            with self.subTest(value=value):
                stdout = io.StringIO()
                stderr = io.StringIO()
                with contextlib.redirect_stdout(stdout):
                    with contextlib.redirect_stderr(stderr):
                        with mock.patch.object(build_wine, "build_wine", return_value=Path("/tmp/Wine-11.0.zip")):
                            with mock.patch.object(build_wine, "load_config", return_value={"operations": []}):
                                with self.assertRaises(SystemExit) as context:
                                    build_wine.main(["wine-11.0", "--jobs", value])

                self.assertEqual(context.exception.code, 2)
                self.assertEqual(stdout.getvalue(), "")
                self.assertIn("jobs must be a positive integer", stderr.getvalue())

    def test_main_reports_called_process_errors(self):
        error = build_wine.subprocess.CalledProcessError(7, ["make", "-j2"])
        with mock.patch.object(build_wine, "build_wine", side_effect=error):
            with mock.patch.object(build_wine, "load_config", return_value={"operations": []}):
                stderr = io.StringIO()
                with contextlib.redirect_stderr(stderr):
                    with self.assertRaises(SystemExit) as context:
                        build_wine.main(["wine-11.0", "--skip-env-check"])

        self.assertEqual(context.exception.code, 7)
        self.assertIn("Command failed with exit code 7: make -j2", stderr.getvalue())

    def test_main_reports_missing_config_without_traceback(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            missing_config = Path(temp_dir) / "missing.json"
            stderr = io.StringIO()

            with contextlib.redirect_stderr(stderr):
                with self.assertRaises(SystemExit) as context:
                    build_wine.main(["wine-11.0", "--config", str(missing_config), "--skip-env-check"])

        self.assertEqual(context.exception.code, 1)
        self.assertIn("Could not read config", stderr.getvalue())
        self.assertIn("missing.json", stderr.getvalue())

    def test_main_reports_invalid_json_config_without_traceback(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            config = Path(temp_dir) / "wine_builds.json"
            config.write_text("{", encoding="utf-8")
            stderr = io.StringIO()

            with contextlib.redirect_stderr(stderr):
                with self.assertRaises(SystemExit) as context:
                    build_wine.main(["wine-11.0", "--config", str(config), "--skip-env-check"])

        self.assertEqual(context.exception.code, 1)
        self.assertIn("Could not parse config", stderr.getvalue())
        self.assertIn("wine_builds.json", stderr.getvalue())


if __name__ == "__main__":
    unittest.main()
