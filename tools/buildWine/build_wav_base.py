#!/usr/bin/env python3
"""Add WAV playback support to the pinned v31 base, retaining its gamma bridge."""
import argparse
import json
from pathlib import Path
import subprocess
import tarfile
import zipfile

import build_media_base as base

BASE_URL = 'https://boxedwine.org/v2/31/TinyCore15WineBase.zip'
BASE_SHA = 'e64a97dffa5962f3278ac43f3160382636ca95585dcf13d3f39fb63236d11fe7'
PLUGIN = 'usr/local/lib/gstreamer-1.0/libgstwavparse.so'
PACKAGE = 'gst-plugins-good.tcz'
SOURCE = 'gst-plugins-good-1.16.2.tar.xz'
DOCS = 'usr/local/share/doc/boxedwine-wavparse/'


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
        url, expected = base.INPUTS[name]
        base.fetch(downloads / name, url, expected)
    extracted = work / 'gst-plugins-good'
    if extracted.exists():
        raise RuntimeError(f'Use a fresh extraction directory: {extracted}')
    base.run(['unsquashfs', '-d', extracted, downloads / PACKAGE, PLUGIN])
    root = work / 'wav-overlay'
    if root.exists():
        raise RuntimeError(f'Use a fresh overlay directory: {root}')
    root.mkdir()
    plugin = extracted / PLUGIN
    base.put(root, PLUGIN, plugin.read_bytes())
    metadata = {
        'revision': '33', 'base_url': BASE_URL, 'base_sha256': BASE_SHA,
        'plugin': PLUGIN, 'plugin_sha256': base.sha(plugin),
        'inputs': {name: {'url': base.INPUTS[name][0], 'sha256': base.INPUTS[name][1]}
                   for name in (PACKAGE, SOURCE)},
        'contents': 'GStreamer 1.16.2 WAV parser; all existing v31 runtime files retained.',
    }
    with tarfile.open(downloads / SOURCE) as source:
        license_file = source.extractfile('gst-plugins-good-1.16.2/COPYING')
        if license_file is None:
            raise RuntimeError('GStreamer source archive is missing COPYING')
        base.put(root, DOCS + 'COPYING', license_file.read())
    base.put(root, DOCS + 'build_wav_base.py', Path(__file__).read_bytes())
    encoded = (json.dumps(metadata, indent=2) + '\n').encode()
    base.put(root, DOCS + 'sources.json', encoded)
    base.put(root, 'wav-runtime.json', encoded)
    with zipfile.ZipFile(args.base) as previous:
        instructions = previous.read('recreateInstructions.txt')
    base.put(root, 'recreateInstructions.txt', instructions +
             b'\nWAV playback update: tools/buildWine/build_wav_base.py; see wav-runtime.json for pinned inputs. Existing gamma and media runtime files are retained.\n')
    output = work / 'TinyCore15WineBase.zip'
    base.package(args.base, root, output)
    # Verify every parent member, including Unix permissions and empty directories.
    with zipfile.ZipFile(args.base) as previous, zipfile.ZipFile(output) as current:
        for entry in previous.infolist():
            if entry.filename == 'recreateInstructions.txt':
                continue
            updated = current.getinfo(entry.filename)
            if current.read(updated) != previous.read(entry) or updated.external_attr != entry.external_attr:
                raise RuntimeError(f'Existing base member changed: {entry.filename}')
    result = {'output': str(output), 'size': output.stat().st_size,
              'sha256': base.sha(output), **metadata}
    receipt = work / 'wav-base-result.json'
    receipt.write_text(json.dumps(result, indent=2) + '\n')
    with zipfile.ZipFile(work / 'TinyCore15WineBase-wavparse-sources.zip', 'w', zipfile.ZIP_STORED) as sources:
        sources.write(downloads / SOURCE, SOURCE)
        sources.write(Path(__file__), 'build_wav_base.py')
        sources.write(Path(base.__file__), 'build_media_base.py')
        sources.write(receipt, receipt.name)
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
