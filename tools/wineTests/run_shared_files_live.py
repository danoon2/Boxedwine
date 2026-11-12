#!/usr/bin/env python3
"""Run shared_files_live.c in an isolated native Boxedwine/Wine environment."""
import argparse
import os
from pathlib import Path
import queue
import re
import shutil
import subprocess
import tempfile
import threading
import time


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--engine', type=Path, required=True)
    parser.add_argument('--wine', type=Path, required=True)
    parser.add_argument('--probe', type=Path, required=True)
    args = parser.parse_args()
    run = Path(tempfile.mkdtemp(prefix='boxedwine-shared-live-'))
    shared = run / 'root/home/username/Shared'
    (shared / 'Nested').mkdir(parents=True)
    shutil.copyfile(args.probe, shared / 'probe.exe')
    (shared / 'Nested/changed.txt').write_text('before')
    (shared / 'old.txt').write_text('rename')
    (shared / 'removed.txt').write_text('remove')
    command = [str(args.engine.resolve()), '-root', str(run / 'root'), '-zip', str(args.wine.resolve()),
               '-shared_folder', str(shared), '-nosound', '-w', '/home/username/Shared',
               '/bin/wine', '/home/username/Shared/probe.exe']
    process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               text=True, encoding='utf-8', errors='replace',
                               env=dict(os.environ, BOXEDWINE_STDIN_CONTROL='1'))
    lines = queue.Queue()
    def drain():
        for line in process.stdout:
            lines.put(line)
        lines.put(None)
    threading.Thread(target=drain, daemon=True).start()
    log, ready, passed = [], False, False
    deadline = time.monotonic() + 100
    try:
        while time.monotonic() < deadline and not passed:
            try:
                line = lines.get(timeout=.2)
            except queue.Empty:
                continue
            if line is None:
                break
            log.append(line)
            plain = re.sub(r'\x1B\[[0-?]*[ -/]*[@-~]', '', line)
            if 'SHARED_READY' in plain and not ready:
                ready = True
                time.sleep(.65)
                (shared / 'patch.txt').write_text('copied')
                (shared / 'Nested/changed.txt').write_text('updated')
                (shared / 'old.txt').rename(shared / 'renamed.txt')
                (shared / 'removed.txt').unlink()
                print('Guest ready; applied host additions, modification, rename and deletion.', flush=True)
            passed = 'SHARED_PASS' in plain
        if not passed:
            raise RuntimeError('Live Shared Files probe failed or timed out.')
        assert (shared / 'guest-output.txt').read_text() == 'from Wine'
        print('PASS Y: live additions, nested modifications, rename, deletion, Wine notifications and guest writes.')
    finally:
        (run / 'engine.log').write_text(''.join(log))
        if process.poll() is None:
            try:
                process.stdin.write('quit\n')
                process.stdin.flush()
                process.wait(timeout=5)
            except (OSError, subprocess.TimeoutExpired):
                process.kill()
                process.wait()
        print('Run log:', run / 'engine.log', flush=True)


if __name__ == '__main__':
    main()
