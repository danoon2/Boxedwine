import difflib
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import indexWebglTestChanges as changes


class TestChangeIndex(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.name = 'dlls/d3d9/tests/device.c'
        self.old = self.root / 'old' / self.name
        self.new = self.root / 'new' / self.name
        self.old.parent.mkdir(parents=True)
        self.new.parent.mkdir(parents=True)
        self.manifest = self.root / 'manifest.json'
        self.manifest.write_text('{}\n')
        self.diff = self.root / 'tests.patch'
        self.policy = dict(_test_patch_path=str(self.diff), modified_test_files=[self.name],
                           wine_source_commit='test-fixture')
        self.loader = patch.object(changes, 'load_and_validate', return_value=self.policy)
        self.loader.start()
        self.addCleanup(self.loader.stop)
        prefix = 'static void test_cursor(void)\n{\n    int success = query();\n'
        suffix = '}\n' + '/* untouched */\n' * 20 + 'int untouched = 1;\n'
        self.prepare(prefix + '    ok(success, "cursor");\n' + suffix,
                     prefix + '    if (!success)\n        skip("cursor");\n' + suffix)

    def prepare(self, old, new):
        self.old.write_text(old)
        self.new.write_text(new)
        diff = ''.join(difflib.unified_diff(old.splitlines(True), new.splitlines(True),
                                         fromfile='a/' + self.name, tofile='b/' + self.name))
        self.diff.write_text('diff --git a/' + self.name + ' b/' + self.name + '\n' + diff)

    def index(self):
        return changes.index_changes(self.manifest, self.root / 'old', self.root / 'new')

    def test_removed_assertion_and_unguarded_skip_are_visible(self):
        report = self.index()
        self.assertFalse(report['runtime_acceptance'])
        rows = report['hunks'][0]['changes']
        self.assertEqual([(r['operation'], r['text'].strip()) for r in rows],
                         [('-', 'ok(success, "cursor");'), ('+', 'if (!success)'), ('+', 'skip("cursor");')])
        self.assertTrue(all(r['function_hint'] == 'test_cursor' for r in rows))
        self.assertEqual(rows[0]['source_line'], 4)

    def test_changed_upstream_assertion_is_rejected(self):
        self.old.write_text(self.old.read_text().replace('ok(success,', 'ok(1,'))
        with self.assertRaisesRegex(ValueError, 'Upstream source differs'):
            self.index()

    def test_changed_patched_predicate_is_rejected(self):
        self.new.write_text(self.new.read_text().replace('if (!success)', 'if (success)'))
        with self.assertRaisesRegex(ValueError, 'Patched source differs'):
            self.index()

    def test_unindexed_source_change_is_rejected(self):
        self.new.write_text(self.new.read_text().replace('untouched = 1', 'untouched = 0'))
        with self.assertRaisesRegex(ValueError, 'outside verified patch hunks'):
            self.index()

    def test_truncated_hunk_is_rejected(self):
        lines = self.diff.read_text().splitlines(True)
        self.diff.write_text(''.join(lines[:-1]))
        with self.assertRaisesRegex(ValueError, 'Hunk old count differs'):
            self.index()

    def test_wrong_destination_coordinate_is_rejected(self):
        text = self.diff.read_text().replace('@@ -1,7 +1,8 @@', '@@ -1,7 +2,8 @@')
        self.assertNotEqual(text, self.diff.read_text())
        self.diff.write_text(text)
        with self.assertRaisesRegex(ValueError, 'Patched source differs|New hunk position differs'):
            self.index()

    def test_missing_manifest_file_is_rejected(self):
        self.policy['modified_test_files'].append('dlls/ddraw/tests/ddraw7.c')
        with self.assertRaisesRegex(ValueError, 'file set differs'):
            self.index()

    def test_repeated_file_section_is_rejected(self):
        text = self.diff.read_text()
        self.diff.write_text(text + text)
        with self.assertRaisesRegex(ValueError, 'repeated patch file'):
            self.index()

    def test_insert_into_empty_source(self):
        self.prepare('', 'int added = 1;\n')
        self.assertEqual(self.index()['counts']['added_lines'], 1)

    def test_remove_entire_source(self):
        self.prepare('int removed = 1;\n', '')
        self.assertEqual(self.index()['counts']['removed_lines'], 1)


if __name__ == '__main__':
    unittest.main()
