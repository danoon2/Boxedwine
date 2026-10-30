#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team. GPL-2.0-or-later.
"""Exercise the packaged engine directly through Bash, without a launcher app.

Build Sandbox first. Supply a local Wine ZIP. On the first run, use --authorize
and choose the printed test folder in the engine's access dialog. Later runs
require remembered access and forbid permission prompts. All fixtures stay in tmp.
"""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import time
import uuid

repo = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--wine', type=Path, required=True)
parser.add_argument('--authorize', action='store_true')
parser.add_argument('--engine', type=Path, default=repo / 'project/mac-xcode/bin/native-build/Build/Products/Sandbox/Boxedwine.app/Contents/Helpers/BoxedwineEngine.app/Contents/MacOS/Boxedwine')
options = parser.parse_args()
output = repo / 'tmp/cli-sandbox-validation'
folder = output / 'host files 日本語'
folder.mkdir(parents=True, exist_ok=True)
wine = folder / 'wine.zip'
if not wine.exists():
    shutil.copyfile(options.wine.resolve(strict=True), wine)
(folder / 'mounted').mkdir(exist_ok=True)
(folder / 'mounted/input.txt').write_text('MOUNT_READ_OK\n')
(folder / 'invalid.zip').write_text('This is not a ZIP archive.\n')
unapproved = output / ('unapproved-' + str(uuid.uuid4()))
unapproved.mkdir()
(unapproved / 'input.txt').write_text('This folder has not been approved.\n')
print('Folder to approve:', folder, flush=True)


def execute(name, args, *, authorize=False, input_text=None):
    environment = dict(os.environ)
    environment.pop('BOXEDWINE_STDIN_CONTROL', None)
    if authorize:
        environment.pop('BOXEDWINE_NO_FILE_ACCESS_PROMPT', None)
    else:
        environment['BOXEDWINE_NO_FILE_ACCESS_PROMPT'] = '1'
    command = ['/bin/bash', '-c', 'exec "$@"', 'bash', str(options.engine.resolve()), *args]
    result = subprocess.run(command, cwd=folder, env=environment, text=True,
                            input=input_text,
                            stdin=None if input_text is not None else subprocess.DEVNULL,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            timeout=180 if authorize else 30)
    (output / (name + '.log')).write_text(result.stdout)
    if not authorize:
        assert 'choose a folder in the file-access dialog' not in result.stdout, result.stdout
    return result


# Every host path below is relative to Bash's working directory and contains
# spaces/Unicode in its parent. Guest arguments must never become host options.
arguments = ['-root', 'root-cli-test', '-zip', 'wine.zip', '-hideWindow',
             '-mount', 'mounted', '/home/username/host', '-log', 'engine.log',
             '/bin/sh', '-c', 'set -e; cat /home/username/host/input.txt; '
             'printf "MOUNT_WRITE_OK\\n" > /home/username/host/output.txt; '
             'printf "%s\\n" "$1"; printf "CLI_ARGUMENT_OK\\n"',
             'probe', 'literal $(no-shell) 日本語', '-log', str(unapproved / 'guest-argument.log')]
for name, authorize, input_text in [('cli-first', options.authorize, None), ('cli-cached', False, None),
                                     ('cli-piped-stdin', False, ''), ('cli-quit-stdin', False, 'quit\n')]:
    result = execute(name, arguments, authorize=authorize, input_text=input_text)
    assert result.returncode == 0 and '\nCLI_ARGUMENT_OK\n' in result.stdout, result.stdout
    assert '\nliteral $(no-shell) 日本語\n' in result.stdout, result.stdout
    assert (folder / 'mounted/output.txt').read_text() == 'MOUNT_WRITE_OK\n'
    assert 'CLI_ARGUMENT_OK' in (folder / 'engine.log').read_text()
    assert not (unapproved / 'guest-argument.log').exists()
    print('PASS', name, flush=True)

# Process-based launchers explicitly own stdin. Both their Stop command and
# their disappearance must end a running guest, without affecting ordinary CLI.
for mode in ['quit', 'eof']:
    name = 'cli-control-' + mode
    marker = folder / 'mounted' / (name + '.ready')
    marker.unlink(missing_ok=True)
    log = output / (name + '.log')
    environment = dict(os.environ, BOXEDWINE_STDIN_CONTROL='1', BOXEDWINE_NO_FILE_ACCESS_PROMPT='1')
    command = ['/bin/bash', '-c', 'exec "$@"', 'bash', str(options.engine.resolve()),
               '-root', 'root-cli-test', '-zip', 'wine.zip', '-hideWindow', '-nosound',
               '-mount', 'mounted', '/home/username/host', '/bin/sh', '-c',
               'test -z "$BOXEDWINE_STDIN_CONTROL" || exit 10; '
               f'echo ready > /home/username/host/{marker.name}; sleep 30']
    with log.open('w') as output_file:
        process = subprocess.Popen(command, cwd=folder, env=environment, text=True,
                                   stdin=subprocess.PIPE, stdout=output_file, stderr=subprocess.STDOUT)
        try:
            deadline = time.monotonic() + 15
            while not marker.exists() and process.poll() is None and time.monotonic() < deadline:
                time.sleep(.05)
            assert marker.exists() and process.poll() is None, log.read_text()
            if mode == 'quit':
                process.stdin.write('quit\n')
                process.stdin.flush()
            else:
                process.stdin.close()
            assert process.wait(timeout=5) == 0, log.read_text()
        finally:
            if process.poll() is None:
                process.kill()
                process.wait()
            process.stdin.close()
    assert 'choose a folder in the file-access dialog' not in log.read_text(), log.read_text()
    print('PASS', name, flush=True)

# Denied access and invalid resources must fail, without a dialog or a crash.
for name, args, expected in [
    ('cli-unapproved', ['-root', str(unapproved / 'root'), '-zip', 'wine.zip', '-hideWindow', '/bin/true'], 'cannot write to'),
    ('cli-missing-zip', ['-root', 'root-cli-test', '-zip', 'missing.zip', '-hideWindow', '/bin/true'], 'No such file'),
    ('cli-invalid-zip', ['-root', 'root-cli-test', '-zip', 'invalid.zip', '-hideWindow', '/bin/true'], 'Could not load zip file'),
    ('cli-invalid-mount', ['-root', 'root-cli-test', '-zip', 'wine.zip', '-mount', 'invalid.zip', '/home/username/invalid', '-hideWindow', '/bin/true'], 'Could not load zip file'),
]:
    result = execute(name, args)
    assert result.returncode == 1 and expected in result.stdout, result.stdout
    print('PASS', name, flush=True)
