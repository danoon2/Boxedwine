"""Plain unified production patches must participate in policy checks."""
from pathlib import Path
import json
import sys
import tempfile
import unittest

sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import webglTestDivergences as policy


class UnifiedPatchInventoryTests(unittest.TestCase):
    def parse(self,text):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/'change.patch'
            path.write_text(text,encoding='utf-8')
            return policy.modified_patch_files(path)

    def test_plain_unified_and_hunk_contents(self):
        text='--- a/dlls/example/source.c\n+++ b/dlls/example/source.c\n@@ -1,2 +1,2 @@\n--- a/not-a-file.c\n+++ b/not-a-file.c\n kept\n'
        self.assertEqual(self.parse(text),{'dlls/example/source.c'})

    def test_added_and_deleted_test_files(self):
        text='--- /dev/null\n+++ b/dlls/example/tests/new.c\n@@ -0,0 +1 @@\n+line\n--- a/dlls/example/tests/old.c\n+++ /dev/null\n@@ -1 +0,0 @@\n-line\n'
        self.assertEqual(self.parse(text),{'dlls/example/tests/new.c','dlls/example/tests/old.c'})

    def test_git_header_still_supported(self):
        self.assertEqual(self.parse('diff --git a/file.c b/file.c\n--- a/file.c\n+++ b/file.c\n@@ -1 +1 @@\n-before\n+after\n'),{'file.c'})

    def test_plain_unified_production_patch_cannot_change_tests(self):
        with tempfile.TemporaryDirectory() as directory:
            directory=Path(directory)
            production=directory/'production.patch'; tests=directory/'tests.patch'
            production.write_text('--- a/dlls/example/tests/test.c\n+++ b/dlls/example/tests/test.c\n@@ -1 +1 @@\n-check\n+skip\n')
            tests.write_text('')
            manifest=dict(schema_version=2,production_patches=[dict(patch_id='production',category='test',path=production.name,sha256=policy.sha256(production))],
                test_patch=dict(patch_id='tests',path=tests.name,sha256=policy.sha256(tests)),modified_test_files=[],classified_rules=[])
            path=directory/'manifest.json'; path.write_text(json.dumps(manifest))
            with self.assertRaisesRegex(policy.DivergenceError,'contains test files'):
                policy.load_and_validate(path,[production],tests)

    def test_v30_restores_only_two_skips(self):
        folder=Path(__file__).resolve().parents[1]
        old=policy.load_and_validate(folder/'webgl-test-divergences-v29.json')
        new=policy.load_and_validate(folder/'webgl-test-divergences-v30.json')
        self.assertEqual(new['_series_counts']['production_patches'],40)
        self.assertEqual(new['_policy_counts']['skip_calls'],old['_policy_counts']['skip_calls']-2)
        self.assertEqual(new['_policy_counts']['todo_calls'],old['_policy_counts']['todo_calls'])

    def test_current_ffp_patch_has_three_files(self):
        patch=Path(__file__).resolve().parents[3]/'tools/d3dToWebGL/webgl-ffp-failure-recovery-against-wine-11.0.patch'
        self.assertEqual(policy.modified_patch_files(patch),{
            'dlls/wined3d/shader.c','dlls/wined3d/cs.c','dlls/wined3d/wined3d_private.h'})


if __name__ == '__main__':
    unittest.main()
