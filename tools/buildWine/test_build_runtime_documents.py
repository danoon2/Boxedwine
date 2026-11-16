from pathlib import Path
import tempfile
import unittest
import zipfile

import build_filesystem
import build_media_base
import runtime_documents


class RuntimeDocumentsTests(unittest.TestCase):
    def test_base_and_full_packaging_move_records_without_changing_runtime(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source, overlay = root / 'base.zip', root / 'overlay'
            overlay.mkdir()
            entries = {
                'usr/local/share/doc/psvoodoo/source.tar.gz': b'original source',
                'usr/share/doc/License/gplv2.txt': b'license',
                'usr/local/share/licences/libogg/COPYING': b'notice',
                'build.txt': b'old build',
                'usr/local/lib/gstreamer-1.0/libgstavi.so': b'runtime plugin',
                'usr/local/share/gst-plugins-base/1.0/license-translations.dict': b'runtime data',
                'home/username/.wine/drive_c/windows/resources/themes/light/light.msstyles': b'theme',
                'home/username/.wine/drive_c/windows/temp/': b'',
            }
            with zipfile.ZipFile(source, 'w') as archive:
                for name, value in entries.items():
                    info = zipfile.ZipInfo(name)
                    info.external_attr = (0o40755 if name.endswith('/') else 0o100644) << 16
                    archive.writestr(info, value)
            (overlay / 'build.txt').write_bytes(b'new build')
            for package in (build_media_base.package, build_filesystem.combine_zip):
                output = root / 'runtime.zip'
                package(source, overlay, output, exclude=runtime_documents.is_documentation)
                runtime_documents.validate_runtime(output)
                with zipfile.ZipFile(source) as original, zipfile.ZipFile(output) as archive:
                    for name in entries:
                        if runtime_documents.is_documentation(name):
                            self.assertNotIn(name, archive.namelist())
                        else:
                            self.assertEqual(archive.read(name), entries[name])
                            self.assertEqual(archive.getinfo(name).external_attr, original.getinfo(name).external_attr)
            records = root / 'records.zip'
            runtime_documents.archive_documents(source, overlay, records)
            with zipfile.ZipFile(records) as archive:
                self.assertEqual(archive.read('build.txt'), b'new build')
                self.assertEqual(archive.read('usr/local/share/doc/psvoodoo/source.tar.gz'), b'original source')
                self.assertEqual(len(archive.namelist()), 4)
            with self.assertRaisesRegex(RuntimeError, 'records remain'):
                runtime_documents.validate_runtime(source)


if __name__ == '__main__':
    unittest.main()
