#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Regression checks with small real Mach-O fixtures; executes no fixture code."""
import importlib.util
import plistlib
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.dont_write_bytecode = True
spec = importlib.util.spec_from_file_location("bundle_audit", Path(__file__).with_name("audit-native-bundle.py"))
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class BundleAuditTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix="boxedwine-bundle-audit-")
        cls.base = Path(cls.temp.name)
        cls.template = cls.base / "Template.app"
        cls.helper_relative = Path("Contents/Helpers/BoxedwineEngine.app")
        cls.frameworks_relative = Path("Contents/Frameworks")
        frameworks = cls.template / cls.frameworks_relative
        frameworks.mkdir(parents=True)
        cls.library_source = cls.base / "library.c"
        cls.library_source.write_text("int fixture(void) { return 0; }\n")
        main = cls.base / "main.c"
        main.write_text("extern int fixture(void); int main(void) { return fixture(); }\n")
        cls.compile_library(frameworks / "libFixture.dylib")
        for root, name, rpath in ((cls.template, "Launcher", "@executable_path/../Frameworks"),
                                  (cls.template / cls.helper_relative, "Runtime", "@executable_path/../../../../Frameworks")):
            (root / "Contents/MacOS").mkdir(parents=True)
            (root / "Contents/Info.plist").write_bytes(plistlib.dumps({
                "CFBundleExecutable": name, "CFBundleIdentifier": "org.boxedwine.audit." + name,
                "CFBundleVersion": "1", "CFBundleShortVersionString": "0.1.0", "LSMinimumSystemVersion": "13.0"}))
            module.run("/usr/bin/xcrun", "clang", "-target", "arm64-apple-macos13", str(main),
                       "-L" + str(frameworks), "-lFixture", "-Wl,-rpath," + rpath,
                       "-o", str(root / "Contents/MacOS" / name))
        resources = cls.template / "Contents/Resources"
        resources.mkdir()
        shutil.copy2(Path(__file__).resolve().parents[1] / "Resources/PrivacyInfo.xcprivacy", resources)
        (resources / "Boxedwine-LICENSE.txt").write_text("Test fixture only.\n")
        module.run(sys.executable, str(Path(__file__).with_name("prepare-third-party-notices.py")),
                   "--output", str(resources / "Licenses"))

    @classmethod
    def compile_library(cls, output, arch="arm64", minimum="13", install_name="@rpath/libFixture.dylib"):
        module.run("/usr/bin/xcrun", "clang", "-target", f"{arch}-apple-macos{minimum}",
                   "-dynamiclib", str(cls.library_source), "-Wl,-headerpad_max_install_names", "-Wl,-install_name," + install_name,
                   "-o", str(output))

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    def setUp(self):
        self.app = self.base / (self._testMethodName + " with spaces.app")
        shutil.copytree(self.template, self.app)
        self.library = self.app / self.frameworks_relative / "libFixture.dylib"

    def errors(self):
        return "\n".join(module.audit(self.app, signatures=False)["errors"])

    def test_valid_nested_graph_and_read_only(self):
        before = {str(p): module.digest(p) for p in self.app.rglob("*") if p.is_file()}
        report = module.audit(self.app, signatures=False)
        self.assertEqual(report["errors"], [])
        self.assertEqual(len(report["images"]), 3)
        self.assertEqual(before, {str(p): module.digest(p) for p in self.app.rglob("*") if p.is_file()})

    def test_release_requires_the_project_catalog_when_requested(self):
        report = module.audit(self.app, signatures=False, require_demo_catalog=True)
        self.assertIn("Pinned demo catalog is missing", "\n".join(report["errors"]))
        self.assertIsNone(report["demoCatalog"])

    def test_quarantine_on_resources_directories_and_links_is_rejected(self):
        resource = self.app / "Contents/Resources/packages.json"
        resource.write_text("{}\n")
        link = resource.with_name("package-link")
        link.symlink_to(resource.name)
        paths = [self.app, resource.parent, resource, link]
        for path in paths:
            module.run("/usr/bin/xattr", "-s", "-w", "com.apple.quarantine", "0081;0;BundleAudit;", str(path))
        report = module.audit(self.app, signatures=False, app_store=True)
        self.assertEqual(set(report["quarantinedPaths"]), {str(p.relative_to(self.app)) for p in paths})
        self.assertIn("App Store bundle contains com.apple.quarantine: Contents/Resources/packages.json",
                      "\n".join(report["errors"]))
        # The audit is read-only. Cleanup removes only this attribute and can be
        # repeated on an already clean app without changing resource contents.
        self.assertEqual(set(module.quarantined_paths(self.app)), set(paths))
        module.run("/usr/bin/xattr", "-w", "org.boxedwine.audit", "keep", str(resource))
        for _ in range(2):
            module.run("/usr/bin/xattr", "-r", "-s", "-d", "com.apple.quarantine", str(self.app))
            self.assertEqual(module.quarantined_paths(self.app), [])
        self.assertEqual(module.run("/usr/bin/xattr", "-p", "org.boxedwine.audit", str(resource)).strip(), b"keep")
        self.assertEqual(resource.read_text(), "{}\n")

    def test_missing_or_altered_third_party_notices_fail(self):
        notices = self.app / "Contents/Resources/Licenses/notices.json"
        notices.write_text('{"schemaVersion": 1, "components": []}')
        self.assertIn("Bundled third-party notices differ", self.errors())
        notices.unlink()
        self.assertIn("Third-party licenses:", self.errors())

    def test_missing_or_altered_privacy_manifest_fails(self):
        privacy = self.app / "Contents/Resources/PrivacyInfo.xcprivacy"
        privacy.write_bytes(plistlib.dumps({"NSPrivacyTracking": True}))
        self.assertIn("Bundled privacy manifest differs", self.errors())
        privacy.write_bytes(b"invalid plist")
        self.assertIn("Privacy manifest:", self.errors())
        privacy.unlink()
        self.assertIn("Privacy manifest:", self.errors())

    def test_an_incomplete_catalog_fails_even_a_local_audit(self):
        directory = self.app / "Contents/Resources/Demos"
        directory.mkdir()
        (directory / "catalog.xml").write_text("<XML />")
        self.assertIn("Catalog build receipt is missing", self.errors())

    def test_dependency_requires_newer_macos(self):
        self.compile_library(self.library, minimum="15")
        self.assertIn("needs macOS 15.0, app declares 13.0", self.errors())

    def test_unused_intel_only_library(self):
        self.compile_library(self.library.with_name("libUnused.dylib"), arch="x86_64")
        self.assertIn("libUnused.dylib lacks arm64", self.errors())

    def test_incremental_mesa_leftovers_fail_even_when_unused(self):
        leftover = self.library.with_name("libOSMesa.8.dylib")
        shutil.copy2(self.library, leftover)
        automation_leftover = self.app / "Contents/MacOS/libOSMesa.8.dylib"
        shutil.copy2(self.library, automation_leftover)
        self.assertIn("Retired Mesa dependency remains bundled", self.errors())
        # The shared cleanup works for a build directory containing spaces and
        # preserves unrelated libraries used by the native runtime.
        import os
        script = Path(__file__).resolve().parents[3] / "remove-mesa-libraries.sh"
        environment = dict(os.environ, TARGET_BUILD_DIR=str(self.app.parent),
                           FRAMEWORKS_FOLDER_PATH=str(Path(self.app.name) / self.frameworks_relative))
        subprocess.run(["/bin/sh", str(script)], env=environment, check=True)
        self.assertFalse(leftover.exists())
        self.assertFalse(automation_leftover.exists())
        self.assertTrue(self.library.exists())
        self.assertEqual(self.errors(), "")

    def test_missing_transitive_dependency(self):
        source = self.base / "dependent.c"
        source.write_text("extern int fixture(void); int dependent(void) { return fixture(); }\n")
        dependent = self.library.with_name("libDynamic.dylib")
        module.run("/usr/bin/xcrun", "clang", "-target", "arm64-apple-macos13", "-dynamiclib",
                   str(source), "-L" + str(self.library.parent), "-lFixture", "-o", str(dependent))
        module.run("/usr/bin/install_name_tool", "-change", "@rpath/libFixture.dylib", "@rpath/libMissing.dylib", str(dependent))
        self.assertIn("libDynamic.dylib (arm64): @rpath/libMissing.dylib", self.errors())

    def test_build_machine_rpath(self):
        module.run("/usr/bin/install_name_tool", "-add_rpath", "/opt/local/libexec/llvm-19/lib", str(self.library))
        self.assertIn("Nonportable rpath", self.errors())

    def test_escaping_bundle_link(self):
        (self.app / "Contents/Resources/escape").symlink_to(self.library_source)
        self.assertIn("Broken or escaping bundle link", self.errors())

    def test_release_contains_debug_image(self):
        shutil.copy2(self.library, self.library.with_name("__preview.dylib"))
        self.assertIn("Debug image in Release", self.errors())
        self.assertEqual(module.audit(self.app, "Debug", signatures=False)["errors"], [])

    def test_helper_version_mismatch(self):
        info_path = self.app / self.helper_relative / "Contents/Info.plist"
        info = plistlib.loads(info_path.read_bytes())
        info["CFBundleVersion"] = "2"
        info_path.write_bytes(plistlib.dumps(info))
        self.assertIn("Launcher/runtime build differs", self.errors())

    def test_unsigned_app_fails_signature_check(self):
        self.assertTrue(module.audit(self.app)["errors"])

    def test_obsolete_runtime_is_rejected(self):
        (self.app / "Contents/Helpers/BoxedwineRuntime.app").mkdir()
        self.assertIn("obsolete BoxedwineRuntime", self.errors())

    def test_development_and_packaged_entitlements_cannot_be_mixed(self):
        developer = {"com.apple.security.get-task-allow": True}
        launcher = {"com.apple.security.app-sandbox": True,
                    "com.apple.security.files.user-selected.read-write": True,
                    "com.apple.security.network.client": True, "com.apple.security.network.server": True}
        runtime = {"com.apple.security.app-sandbox": True, "com.apple.security.inherit": True,
                   "com.apple.security.cs.allow-jit": True}
        self.assertEqual(module.entitlement_errors("Debug", developer, developer), [])
        self.assertTrue(module.entitlement_errors("Debug", launcher, runtime))
        self.assertTrue(module.entitlement_errors("Debug", developer, {}))
        self.assertTrue(module.entitlement_errors("Debug", developer, dict(developer, **runtime)))
        for configuration in ("Sandbox", "Release"):
            self.assertEqual(module.entitlement_errors(configuration, launcher, runtime), [])
            self.assertTrue(module.entitlement_errors(configuration, developer, developer))
            self.assertTrue(module.entitlement_errors(configuration, launcher, dict(runtime, **developer)))
            self.assertTrue(module.entitlement_errors(configuration, dict(launcher, **developer), runtime))
            readonly = dict(launcher)
            readonly.pop("com.apple.security.files.user-selected.read-write")
            readonly["com.apple.security.files.user-selected.read-only"] = True
            self.assertTrue(module.entitlement_errors(configuration, readonly, runtime))
            self.assertTrue(module.entitlement_errors(configuration, dict(launcher, **{"com.apple.security.inherit": True}), runtime))

    def test_distribution_requires_developer_id_timestamp_runtime_and_matching_team(self):
        details = "CodeDirectory v=20500 flags=0x10000(runtime)\nAuthority=Developer ID Application: Test (TEAM123)\nTeamIdentifier=TEAM123\nTimestamp=Sep 13, 2026 at 12:00:00\n"
        self.assertEqual(module.distribution_errors(module.parse_signature(details), "TEAM123"), [])
        bad = [details.replace("Developer ID Application:", "Apple Development:"),
               details.replace("Timestamp=", "Signed Time="),
               details.replace("0x10000(runtime)", "0x0(none)"),
               details.replace("TeamIdentifier=TEAM123", "TeamIdentifier=OTHERTEAM"),
               details + "Signature=adhoc\n"]
        for value in bad:
            with self.subTest(value=value):
                self.assertTrue(module.distribution_errors(module.parse_signature(value), "TEAM123"))
        self.assertTrue(module.distribution_errors(module.parse_signature(details), None))

    def test_distribution_cannot_skip_signature_verification_or_use_debug(self):
        with self.assertRaises(ValueError):
            module.audit(self.app, signatures=False, distribution=True)
        with self.assertRaises(ValueError):
            module.audit(self.app, configuration="Debug", distribution=True)

    def test_valid_ad_hoc_bundle_is_rejected_for_distribution(self):
        native = Path(__file__).resolve().parents[1]
        module.run("/usr/bin/codesign", "--force", "--sign", "-", "--options", "runtime", str(self.library))
        for app, entitlement in ((self.app / self.helper_relative, "Boxedwine.entitlements"),
                                 (self.app, "BoxedwineUI.entitlements")):
            module.run("/usr/bin/codesign", "--force", "--sign", "-", "--options", "runtime",
                       "--entitlements", str(native / entitlement), str(app))
        self.assertEqual(module.audit(self.app)["errors"], [])
        report = module.audit(self.app, distribution=True)
        self.assertTrue(report["signaturesVerified"])
        self.assertIn("Developer ID Application signature is required", "\n".join(report["errors"]))
        self.assertEqual(len([image for image in report["images"] if "signature" in image]), 3)

    def test_report_cannot_change_input_bundle(self):
        result = subprocess.run(["/usr/bin/python3", str(Path(module.__file__)), str(self.app),
                                 "--skip-signatures", "--json", str(self.app / "report.json")], capture_output=True)
        self.assertEqual(result.returncode, 2)
        self.assertFalse((self.app / "report.json").exists())


if __name__ == "__main__":
    unittest.main()
