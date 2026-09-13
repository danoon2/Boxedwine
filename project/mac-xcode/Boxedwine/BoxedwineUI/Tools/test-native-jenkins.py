#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Exercise the actual Mac Jenkins shell block with local tool substitutes.

Never builds guest code, accesses a signing key, notarizes, or publishes. ditto
packages a text fixture; the test substitutes only the external tool boundaries.
"""
import json
import os
import re
import subprocess
import tempfile
import unittest
import zipfile
from pathlib import Path


class NativeJenkinsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        repo = Path(__file__).resolve().parents[5]
        stage = (repo / "Jenkinsfile").read_text().split("stage ('Build Mac (ARMv8)')", 1)[1]
        cls.script = re.search(r"sh '''(.*?)'''", stage, re.S)[1]

    def run_pipeline(self, scenario="success", identity="Developer ID Application: Test"):
        with tempfile.TemporaryDirectory(prefix="boxedwine-jenkins-test-") as directory:
            root = Path(directory)
            fake_bin = root / "tools"
            fake_bin.mkdir()
            tool = '''#!/usr/bin/python3
import json, os, sys
from pathlib import Path
name = Path(sys.argv[0]).name
args = sys.argv[1:]
with open(os.environ["NATIVE_TEST_TRACE"], "a") as log:
    log.write(json.dumps([name] + args) + "\\n")
scenario = os.environ["NATIVE_TEST_SCENARIO"]
if name == "xcrun" and args[:2] == ["notarytool", "submit"]:
    if scenario == "notary-command": sys.exit(17)
    if scenario == "notary-malformed": print("not JSON")
    else: print(json.dumps({"status": "Invalid" if scenario == "notary-invalid" else "Accepted"}))
if name == "xcrun" and args[:2] == ["stapler", "staple"] and scenario == "staple": sys.exit(18)
if name == "xcrun" and args[:2] == ["stapler", "validate"] and scenario == "stapler-validate": sys.exit(19)
if name == "codesign" and scenario == "signature-verify": sys.exit(20)
'''
            for name in ("xcrun", "codesign"):
                path = fake_bin / name
                path.write_text(tool)
                path.chmod(0o755)
            (root / "buildRelease.sh").write_text('''#!/bin/sh
set -eu
echo '["build"]' >> "$NATIVE_TEST_TRACE"
[ "$NATIVE_TEST_SCENARIO" != build ] || exit 15
mkdir -p bin/Boxedwine.app/Contents/MacOS
echo "Text fixture, never executed" > bin/Boxedwine.app/Contents/MacOS/fixture
''')
            (root / "signNative.sh").write_text('''#!/bin/sh
set -eu
echo '["sign"]' >> "$NATIVE_TEST_TRACE"
[ "$NATIVE_TEST_SCENARIO" != sign ] || exit 16
''')
            for path in (root / "Deploy/Mac/Boxedwine.zip", root / "bin/Boxedwine.zip"):
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text("Stale artifact")
            # The pipeline uses an absolute codesign path. Substitute that tool
            # only; run the rest of the extracted Jenkins shell unchanged.
            script = self.script.replace("/usr/bin/codesign", str(fake_bin / "codesign"))
            shell = root / "stage.sh"
            shell.write_text(script)
            subprocess.run(["/bin/bash", "-n", str(shell)], check=True)
            trace = root / "trace.jsonl"
            environment = dict(os.environ, PATH=str(fake_bin) + ":/usr/bin:/bin",
                               BOXEDWINE_SIGN_NAME=identity, BOXEDWINE_KEYCHAIN_PROFILE="test-only",
                               NATIVE_TEST_TRACE=str(trace), NATIVE_TEST_SCENARIO=scenario)
            result = subprocess.run(["/bin/bash", str(shell)], cwd=root, env=environment, capture_output=True, text=True)
            calls = [json.loads(line) for line in trace.read_text().splitlines()] if trace.exists() else []
            artifact = root / "Deploy/Mac/Boxedwine.zip"
            if scenario == "success" and identity != "-":
                self.assertEqual(result.returncode, 0, result.stderr)
                with zipfile.ZipFile(artifact) as archive:
                    self.assertIn("Boxedwine.app/Contents/MacOS/fixture", archive.namelist())
                self.assertEqual(calls[:2], [["build"], ["sign"]])
                self.assertEqual([call[:3] for call in calls[2:]], [
                    ["xcrun", "notarytool", "submit"], ["xcrun", "stapler", "staple"],
                    ["xcrun", "stapler", "validate"], ["codesign", "--verify", "--deep"]])
                self.assertFalse((root / "bin/BoxedwineUpload.zip").exists())
            else:
                self.assertNotEqual(result.returncode, 0)
                if identity != "-": self.assertFalse(artifact.exists(), result.stdout)
            return calls

    def test_packages_only_after_accepted_notarization_and_validation(self):
        self.run_pipeline()

    def test_build_or_sign_failure_prevents_upload(self):
        for scenario in ("build", "sign"):
            with self.subTest(scenario=scenario):
                calls = self.run_pipeline(scenario)
                self.assertFalse(any(call[0] == "xcrun" for call in calls))

    def test_notarization_failure_prevents_stapling_or_delivery(self):
        for scenario in ("notary-command", "notary-invalid", "notary-malformed"):
            with self.subTest(scenario=scenario):
                calls = self.run_pipeline(scenario)
                self.assertFalse(any(call[:2] == ["xcrun", "stapler"] for call in calls))

    def test_stapling_or_signature_failure_prevents_delivery(self):
        for scenario in ("staple", "stapler-validate", "signature-verify"):
            with self.subTest(scenario=scenario):
                self.run_pipeline(scenario)

    def test_jenkins_rejects_ad_hoc_identity_before_building(self):
        self.assertEqual(self.run_pipeline(identity="-"), [])


if __name__ == "__main__":
    unittest.main()
