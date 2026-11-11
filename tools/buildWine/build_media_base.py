#!/usr/bin/env python3
"""Build the Wine 11 media base on native Linux/WSL, with pinned inputs.

Only the PCRE2 8-bit runtime, ASF demuxer, libav plugin and a minimal LGPL
FFmpeg decoder build are added. No Microsoft DLLs or game data are included.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tarfile
import zipfile

BASE_URL = 'https://boxedwine.org/v2/11/TinyCore15WineBase.zip'
BASE_SHA = '84b623a3d4fb303728915e871ed2af95ce2ba10203c68b2dd3e596534e61108a'
TC = 'http://tinycorelinux.net/15.x/x86/tcz/'
INPUTS = {
    'gst-plugins-good.tcz': (TC + 'gst-plugins-good.tcz', '2d295ce840428c813e776646e0805c8770bf53ba1d3ded7051ec2d5816ef191c'),
    'gst-plugins-good-1.16.2.tar.xz': ('https://gstreamer.freedesktop.org/src/gst-plugins-good/gst-plugins-good-1.16.2.tar.xz', '40bb3bafda25c0b739c8fc36e48380fccf61c4d3f83747e97ac3f9b0171b1319'),
    'pcre21042.tcz': (TC + 'pcre21042.tcz', 'd424efa5892915337ae9041f584674fbefd8be7eee00a8898bd8040e746a6593'),
    'gst-libav.tcz': (TC + 'gst-libav.tcz', 'fc0f3f60f95245ada5eb6a9e6d7ec036255bb451c9e467d48baf7d082e41c09f'),
    'gst-plugins-ugly.tcz': (TC + 'gst-plugins-ugly.tcz', '417acf878afa6f3e8c469f7c7d527c10cb171cdcc818f955fffe8073500cf8e5'),
    'ffmpeg-4.4.6.tar.xz': ('https://ffmpeg.org/releases/ffmpeg-4.4.6.tar.xz', '2290461f467c08ab801731ed412d8e724a5511d6c33173654bd9c1d2e25d0617'),
    'gst-libav-1.16.2.tar.xz': ('https://gstreamer.freedesktop.org/src/gst-libav/gst-libav-1.16.2.tar.xz', 'c724f612700c15a933c7356fbeabb0bb9571fb5538f8b1b54d4d2d94188deef2'),
    'gst-plugins-ugly-1.16.2.tar.xz': ('https://gstreamer.freedesktop.org/src/gst-plugins-ugly/gst-plugins-ugly-1.16.2.tar.xz', '5500415b865e8b62775d4742cbb9f37146a50caecfc0e7a6fc0160d3c560fbca'),
    'pcre2-10.42.tar.bz2': ('https://github.com/PCRE2Project/pcre2/releases/download/pcre2-10.42/pcre2-10.42.tar.bz2', '8d36cd8cb6ea2a4c2bb358ff6411b0c788633a2a45dabbf1aeb4b701d1b5e840'),
}
FFMPEG_ARGS = [
    '--prefix=/opt/boxedwine-media', '--cc=gcc -m32', '--arch=x86', '--cpu=pentium4',
    '--disable-autodetect', '--disable-everything', '--disable-programs', '--disable-doc',
    '--disable-debug', '--disable-network', '--enable-shared', '--disable-static', '--enable-pic',
    '--enable-avfilter', '--enable-swresample', '--enable-swscale',
    '--enable-filter=abuffer,abuffersink,buffer,buffersink',
    '--enable-decoder=wmv1,wmv2,wmv3,wmv3image,vc1,vc1image,wmav1,wmav2,wmapro,wmalossless,wmavoice',
    '--enable-parser=vc1', '--enable-demuxer=asf', '--enable-protocol=file',
    '--disable-avx', '--disable-avx2', '--disable-avx512', '--disable-fma3', '--disable-fma4',
]


def sha(path):
    return hashlib.file_digest(path.open('rb'), 'sha256').hexdigest()


def run(args, **kwargs):
    print(' '.join(map(str, args)), flush=True)
    subprocess.run(list(map(str, args)), check=True, **kwargs)


def fetch(path, url, expected):
    if not path.exists():
        run(['curl', '-fL', '--retry', '3', '-o', path, url])
    if sha(path) != expected:
        raise RuntimeError(f'Checksum mismatch: {path}')


def unpack(archive, dest):
    if not dest.exists():
        dest.mkdir(parents=True)
        with tarfile.open(archive) as source:
            source.extractall(dest, filter='data')


def put(root, name, data):
    target = root / name
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(data)


def package(base, root, output):
    if base.resolve() == output.resolve():
        raise RuntimeError('Base input and output archives must be different files')
    files = {p.relative_to(root).as_posix(): p for p in root.rglob('*') if p.is_file()}
    with zipfile.ZipFile(base) as old, zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as new:
        for info in old.infolist():
            if info.filename not in files:
                new.writestr(info, old.read(info), compresslevel=9)
        for name, path in sorted(files.items()):
            info = zipfile.ZipInfo(name, (2026, 11, 8, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = (0o100755 if '.so' in name else 0o100644) << 16
            new.writestr(info, path.read_bytes(), compresslevel=9)
    with zipfile.ZipFile(output) as check:
        if check.testzip():
            raise RuntimeError('Base ZIP CRC failure')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--work-dir', type=Path, required=True)
    parser.add_argument('--base', type=Path, required=True)
    parser.add_argument('--boxedwine', type=Path, required=True)
    parser.add_argument('--jobs', type=int, default=12)
    args = parser.parse_args()
    work = args.work_dir.resolve()
    work.mkdir(parents=True, exist_ok=True)
    downloads = work / 'downloads'
    downloads.mkdir(exist_ok=True)
    fetch(args.base, BASE_URL, BASE_SHA)
    for name, (url, expected) in INPUTS.items():
        fetch(downloads / name, url, expected)
    for name in ('pcre21042', 'gst-libav', 'gst-plugins-ugly', 'gst-plugins-good'):
        dest = work / name
        if not dest.exists():
            run(['unsquashfs', '-d', dest, downloads / (name + '.tcz')])
    sources = work / 'sources'
    for name in INPUTS:
        if '.tar.' in name:
            unpack(downloads / name, sources / name.split('.tar.')[0])
    ffmpeg = sources / 'ffmpeg-4.4.6/ffmpeg-4.4.6'
    install = work / 'ffmpeg-install'
    with (work / 'ffmpeg-build.log').open('w') as log:
        run(['./configure', *FFMPEG_ARGS], cwd=ffmpeg, stdout=log, stderr=subprocess.STDOUT)
        run(['make', f'-j{args.jobs}'], cwd=ffmpeg, stdout=log, stderr=subprocess.STDOUT)
        run(['make', 'install', f'DESTDIR={install}'], cwd=ffmpeg, stdout=log, stderr=subprocess.STDOUT)
    # Fresh overlay avoids retaining diagnostic overrides or ldconfig backup files.
    root = work / 'media-overlay'
    if root.exists():
        raise RuntimeError(f'Use a fresh overlay directory: {root}')
    root.mkdir()
    pcre = work / 'pcre21042/usr/local/lib/libpcre2-8.so.0.11.2'
    put(root, 'usr/local/lib/' + pcre.name, pcre.read_bytes())
    put(root, 'usr/local/lib/libpcre2-8.so.0.link', b'libpcre2-8.so.0.11.2')
    for package_name, plugin in (('gst-libav', 'libgstlibav.so'), ('gst-plugins-ugly', 'libgstasf.so'), ('gst-plugins-good', 'libgstdeinterlace.so'), ('gst-plugins-good', 'libgstvideofilter.so')):
        name = 'usr/local/lib/gstreamer-1.0/' + plugin
        put(root, name, (work / package_name / name).read_bytes())
    libraries = install / 'opt/boxedwine-media/lib'
    for stem in ('avcodec', 'avformat', 'avfilter', 'avutil', 'swresample', 'swscale'):
        library = next(p for p in libraries.glob('lib' + stem + '.so.*') if not p.is_symlink())
        put(root, 'usr/local/lib/' + library.name, library.read_bytes())
        soname = '.'.join(library.name.split('.')[:3])
        put(root, 'usr/local/lib/' + soname + '.link', library.name.encode())
    notices = 'usr/local/share/doc/boxedwine-media/'
    for dirname, license_name in (('pcre2-10.42', 'LICENCE'), ('gst-libav-1.16.2', 'COPYING'), ('gst-plugins-ugly-1.16.2', 'COPYING'), ('gst-plugins-good-1.16.2', 'COPYING'), ('ffmpeg-4.4.6', 'COPYING.LGPLv2.1')):
        put(root, notices + dirname + '-' + license_name, (sources / dirname / dirname / license_name).read_bytes())
    metadata = {'base_url': BASE_URL, 'base_sha256': BASE_SHA,
                'inputs': {n: {'url': u, 'sha256': h} for n, (u, h) in INPUTS.items()},
                'ffmpeg_configure': FFMPEG_ARGS,
                'contents': 'PCRE2 10.42 8-bit runtime; GStreamer 1.16.2 ASF/libav/deinterlace/videofilter plugins; FFmpeg 4.4.6 Windows Media decoders. No encoders.'}
    put(root, notices + 'sources.json', (json.dumps(metadata, indent=2) + '\n').encode())
    put(root, 'media-runtime.json', (json.dumps(metadata, indent=2) + '\n').encode())
    with zipfile.ZipFile(args.base) as base:
        instructions = base.read('recreateInstructions.txt')
    put(root, 'recreateInstructions.txt', instructions + b'\nMedia update: tools/buildWine/build_media_base.py; see media-runtime.json for pinned sources and build flags.\n')
    # The guest cache must include new /usr/local/lib SONAMEs. Host ldconfig is unsuitable.
    put(root, 'media-ldconfig.sh', b'/sbin/ldconfig\nprintf "%s" "$?" > /media-ldconfig.status\n')
    with (work / 'ldconfig.log').open('w') as log:
        result = subprocess.run(list(map(str, [args.boxedwine.resolve(), '-root', root, '-zip', args.base.resolve(), '-uid', '0', '/bin/sh', '/media-ldconfig.sh'])), stdout=log, stderr=subprocess.STDOUT, timeout=120)
    status = root / 'media-ldconfig.status'
    if result.returncode not in (0, 1) or not status.exists() or status.read_text() != '0':
        raise RuntimeError('Guest ldconfig failed; inspect ldconfig.log')
    status.unlink()
    (root / 'media-ldconfig.sh').unlink()
    cache = root / 'etc/ld.so.cache'
    if not cache.is_file() or b'libpcre2-8.so.0' not in cache.read_bytes():
        raise RuntimeError('Guest ldconfig did not cache PCRE2')
    (root / 'etc/ld.so.cache~').unlink(missing_ok=True)
    output = work / 'TinyCore15WineBase.zip'
    package(args.base, root, output)
    result = {'output': str(output), 'size': output.stat().st_size, 'sha256': sha(output), **metadata}
    (work / 'media-base-result.json').write_text(json.dumps(result, indent=2) + '\n')
    # Keep complete upstream source archives alongside the redistribution receipt.
    with zipfile.ZipFile(work / 'TinyCore15WineBase-media-sources.zip', 'w', zipfile.ZIP_STORED) as bundle:
        for name in INPUTS:
            if '.tar.' in name:
                bundle.write(downloads / name, name)
        bundle.write(Path(__file__), 'build_media_base.py')
        bundle.write(work / 'media-base-result.json', 'media-base-result.json')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
