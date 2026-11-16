#!/usr/bin/env python3
"""Add AVI demuxing to the pinned v33 base, preserving its existing runtime."""
import argparse
import json
from pathlib import Path
import tarfile
import zipfile

import build_media_base as base
import runtime_documents

BASE_URL = 'https://boxedwine.org/v2/33/TinyCore15WineBase.zip'
BASE_SHA = 'ce55a695a5addbf8abab82d98e5c46d5787b517c570e6ac49deae1643cdf20bd'
PLUGIN = 'usr/local/lib/gstreamer-1.0/libgstavi.so'
PLUGIN_SHA = 'e9fa031bfdaba66055db8d9275c5e932ef70c192d4926ebdc9e059dcb556337b'
PACKAGE = 'gst-plugins-good.tcz'
SOURCE = 'gst-plugins-good-1.16.2.tar.xz'
DOCS = 'usr/local/share/doc/boxedwine-avi/'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--work-dir', type=Path, required=True)
    parser.add_argument('--base', type=Path, required=True)
    args = parser.parse_args()
    work = args.work_dir.resolve()
    work.mkdir(parents=True, exist_ok=True)
    downloads = work / 'downloads'
    downloads.mkdir(exist_ok=True)
    base.fetch(args.base, BASE_URL, BASE_SHA)
    for name in (PACKAGE, SOURCE):
        base.fetch(downloads / name, *base.INPUTS[name])
    extracted = work / 'gst-plugins-good'
    root = work / 'avi-overlay'
    if extracted.exists() or root.exists():
        raise RuntimeError('Use a fresh extraction and overlay directory')
    base.run(['unsquashfs', '-d', extracted, downloads / PACKAGE, PLUGIN])
    plugin = extracted / PLUGIN
    if base.sha(plugin) != PLUGIN_SHA:
        raise RuntimeError('AVI parser does not match the tested binary')
    root.mkdir()
    records = work / 'release-records'
    records.mkdir()
    base.put(root, PLUGIN, plugin.read_bytes())
    metadata = {
        'revision': '37', 'base_url': BASE_URL, 'base_sha256': BASE_SHA,
        'plugin': PLUGIN, 'plugin_sha256': PLUGIN_SHA,
        'inputs': {name: {'url': base.INPUTS[name][0], 'sha256': base.INPUTS[name][1]}
                   for name in (PACKAGE, SOURCE)},
        'contents': 'GStreamer 1.16.2 AVI parser; all existing v33 runtime files retained. No game files or proprietary codecs.',
    }
    with tarfile.open(downloads / SOURCE) as source:
        license_file = source.extractfile('gst-plugins-good-1.16.2/COPYING')
        if license_file is None:
            raise RuntimeError('GStreamer source archive is missing COPYING')
        base.put(records, DOCS + 'COPYING', license_file.read())
    base.put(records, DOCS + 'build_avi_base.py', Path(__file__).read_bytes())
    encoded = (json.dumps(metadata, indent=2) + '\n').encode()
    base.put(records, DOCS + 'sources.json', encoded)
    base.put(records, 'avi-runtime.json', encoded)
    with zipfile.ZipFile(args.base) as previous:
        instructions = previous.read('recreateInstructions.txt')
    base.put(records, 'recreateInstructions.txt', instructions +
             b'\nAVI playback update: tools/buildWine/build_avi_base.py; see avi-runtime.json for pinned inputs. Existing gamma, WAV and media runtime files are retained.\n')
    output = work / 'TinyCore15WineBase.zip'
    base.package(args.base, root, output, exclude=runtime_documents.is_documentation)
    runtime_documents.validate_runtime(output)
    documents = work / 'TinyCore15WineBase-records.zip'
    runtime_documents.archive_documents(args.base, records, documents)
    with zipfile.ZipFile(args.base) as previous, zipfile.ZipFile(output) as current:
        for entry in previous.infolist():
            if runtime_documents.is_documentation(entry.filename):
                continue
            updated = current.getinfo(entry.filename)
            if current.read(updated) != previous.read(entry) or updated.external_attr != entry.external_attr:
                raise RuntimeError(f'Existing base member changed: {entry.filename}')
    result = {'output': str(output), 'size': output.stat().st_size,
              'sha256': base.sha(output), **metadata}
    receipt = work / 'avi-base-result.json'
    receipt.write_text(json.dumps(result, indent=2) + '\n')
    with zipfile.ZipFile(work / 'TinyCore15WineBase-avi-sources.zip', 'w', zipfile.ZIP_STORED) as sources:
        sources.write(downloads / SOURCE, SOURCE)
        sources.write(Path(__file__), 'build_avi_base.py')
        sources.write(Path(base.__file__), 'build_media_base.py')
        sources.write(Path(runtime_documents.__file__), 'runtime_documents.py')
        sources.write(documents, documents.name)
        sources.write(receipt, receipt.name)
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
