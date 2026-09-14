#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Version changes must propagate without losing UI bundle declarations."""

import importlib.util
from pathlib import Path
import plistlib
import sys
import tempfile
import unittest


sys.dont_write_bytecode = True
spec = importlib.util.spec_from_file_location("version_plist", Path(__file__).with_name("prepare-version-plist.py"))
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class VersionPlistTests(unittest.TestCase):
    def test_both_targets_follow_header_changes_and_preserve_template(self):
        with tempfile.TemporaryDirectory() as folder:
            base = Path(folder)
            header, template = base / "boxedwine.h", base / "Info.plist"
            declarations = {"UTExportedTypeDeclarations": [{"UTTypeIdentifier": "org.boxedwine.app-backup"}]}
            template.write_bytes(plistlib.dumps(declarations))
            original = template.read_bytes()
            for version in ("26.1.0", "27.2.3"):
                header.write_text('#define BOXEDWINE_VERSION_STR "26R2"\r\n#define BOXEDWINE_VERSION_DISPLAY "' + version + '"\r\n')
                for target, source in (("UI", template), ("Runtime", None)):
                    output = base / target / "Versioned-Info.plist"
                    module.prepare(header, output, source)
                    result = plistlib.loads(output.read_bytes())
                    self.assertEqual(result["CFBundleShortVersionString"], version)
                    self.assertEqual(result["CFBundleVersion"], "$(CURRENT_PROJECT_VERSION)")
                    if source:
                        self.assertEqual(result["UTExportedTypeDeclarations"], declarations["UTExportedTypeDeclarations"])
                self.assertEqual(template.read_bytes(), original)

    def test_missing_ambiguous_or_invalid_version_fails(self):
        valid = '#define BOXEDWINE_VERSION_DISPLAY "26.1.0"\n'
        for source in ("", valid + valid, valid.replace('"26.1.0"', '"26R2"'), valid.replace('"26.1.0"', '"26.1"'),
                       valid.replace('"26.1.0"', '26.1.0'), valid + '#define BOXEDWINE_VERSION_DISPLAY "unknown"\n'):
            with self.subTest(source=source), self.assertRaises(ValueError):
                module.display_version(source)

    def test_invalid_header_leaves_existing_output_untouched(self):
        with tempfile.TemporaryDirectory() as folder:
            header, output = Path(folder) / "boxedwine.h", Path(folder) / "Info.plist"
            header.write_text("missing version")
            output.write_bytes(b"previous build")
            with self.assertRaises(ValueError):
                module.prepare(header, output)
            self.assertEqual(output.read_bytes(), b"previous build")


if __name__ == "__main__":
    unittest.main()
