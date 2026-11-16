#!/usr/bin/env python3
# Copyright (C) 2026 The BoxedWine Team. SPDX-License-Identifier: GPL-2.0-or-later
"""Exercise disc autorun through the actual engine/Wine; all writes use a fresh test root.

Compile cdrom_install_probe.c with MinGW first, then pass --probe, --engine,
--wine (the pinned filesystem ZIP), and a new --work-dir.
"""
import argparse
from pathlib import Path
import struct
import subprocess
import time

SECTOR = 2048


def both(value):
    return struct.pack('<I', value) + struct.pack('>I', value)


def record(lba, size, name, directory=False):
    data = bytearray((33 + len(name) + 1) & ~1)
    data[0] = len(data)
    data[2:10] = both(lba)
    data[10:18] = both(size)
    data[18:25] = bytes([126, 10, 24, 12, 0, 0, 0])
    data[25] = 2 if directory else 0
    data[28] = data[31] = 1
    data[32] = len(name)
    data[33:33 + len(name)] = name
    return data


def iso(probe, autorun):
    payloads = [(b'MY SETUP.EXE;1', probe)]
    if autorun is not None:
        payloads.append((b'AuToRuN.InF;1', autorun))
    root = record(20, SECTOR, b'\0', True) + record(20, SECTOR, b'\1', True)
    lba = 21
    extents = []
    for name, data in payloads:
        root += record(lba, len(data), name)
        extents.append((lba, data))
        lba += (len(data) + SECTOR - 1) // SECTOR
    image = bytearray(lba * SECTOR)
    pvd = memoryview(image)[16 * SECTOR:17 * SECTOR]
    pvd[:7] = b'\x01CD001\x01'
    pvd[40:72] = b'BOXEDWINE INSTALL TEST'.ljust(32)
    pvd[80:88] = both(lba)
    pvd[128:132] = b'\x00\x08\x08\x00'
    pvd[156:190] = record(20, SECTOR, b'\0', True)
    image[17 * SECTOR:17 * SECTOR + 7] = b'\xffCD001\x01'
    image[20 * SECTOR:20 * SECTOR + len(root)] = root
    for lba, data in extents:
        image[lba * SECTOR:lba * SECTOR + len(data)] = data
    return image


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('probe', 'engine', 'wine', 'work-dir'):
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args()
    args.work_dir.mkdir(parents=True, exist_ok=False)
    cases = [
        ('autorun', b'[other]\nopen=missing.exe\n[AutoRun]\nopen="my setup.exe" /language "English UK"\n', True),
        ('cue', b'[autorun]\nshellexecute="my setup.exe" /language "English UK"\n', True),
        ('utf16', '[autorun]\nopen="my setup.exe" /language "English UK"\n'.encode('utf-16'), True),
        ('explorer', None, False),
        ('missing-target', b'[autorun]\nopen=missing.exe\n', False),
    ]
    for name, entry, runs in cases:
        folder = args.work_dir / name
        folder.mkdir()
        root = folder / 'root'
        root.mkdir()
        image = folder / 'Install disc.iso'
        image.write_bytes(iso(args.probe.read_bytes(), entry))
        if name == 'cue':
            image.rename(folder / 'Track 1.bin')
            image = folder / 'Install disc.cue'
            image.write_text('FILE "Track 1.bin" BINARY\n TRACK 01 MODE1/2048\n INDEX 01 00:00:00\n')
        log = folder / 'engine.log'
        marker = root / 'home/username/.wine/drive_c/disc-install-result.txt'
        command = [str(args.engine.resolve()), '-root', str(root.resolve()), '-zip', str(args.wine.resolve()),
                   '-log', str(log.resolve()), '-novideo', '-nosound', '-cdrom', str(image.resolve()), 'e', '-cdromInstall']
        with (folder / 'stdout.log').open('wb') as out:
            process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=out, stderr=subprocess.STDOUT,
                                       creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
            try:
                deadline = time.monotonic() + 100
                while time.monotonic() < deadline:
                    if runs and marker.exists():
                        text = marker.read_text(encoding='utf-16-le')
                        assert '/language "English UK"' in text and text.splitlines()[-1].lower() == 'e:\\', text
                        process.wait(timeout=30)
                        assert process.returncode == 0, process.returncode
                        break
                    if not runs and log.exists() and 'Launching "/bin/wine" "explorer" "e:\\"' in log.read_text(errors='replace'):
                        assert not marker.exists()
                        break
                    if process.poll() is not None:
                        raise AssertionError(f'{name}: engine exited {process.returncode}; see {log}')
                    time.sleep(.1)
                else:
                    raise TimeoutError(f'{name}: see {log}')
            finally:
                if process.poll() is None:
                    try:
                        process.stdin.write(b'quit\n'); process.stdin.flush()
                        process.wait(timeout=10)
                    except (BrokenPipeError, subprocess.TimeoutExpired):
                        process.kill(); process.wait()
                process.stdin.close()
        print(f'PASS {name}: {"autorun launched with arguments and disc working directory" if runs else "Explorer fallback selected"}', flush=True)


if __name__ == '__main__':
    main()
