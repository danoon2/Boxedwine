#!/usr/bin/env python3
"""Check forced process exit with an i386 Linux ELF in an isolated browser run."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import shlex
import struct

import wineGraphicsBrowser as browser
from auditGraphicsMatrix import diagnostics


def validate_elf(executable: Path, suite: browser.GraphicsSuite) -> None:
    with Path(executable).open('rb') as stream:
        header = stream.read(20)
    if (len(header) != 20 or header[:6] != b'\x7fELF\x01\x01'
            or struct.unpack_from('<H', header, 18)[0] != 3):
        raise browser.RunnerError('process probe must be a little-endian i386 ELF')


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', choices=['kill', 'shared'], default='kill')
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--build-dir', type=Path, required=True)
    parser.add_argument('--filesystem', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--mode', choices=['single-threaded-non-jit', 'single-threaded-jit',
        'multi-threaded-non-jit', 'multi-threaded-jit'], required=True)
    parser.add_argument('--timeout', type=int, default=90)
    parser.add_argument('--headless', action='store_true')
    args = parser.parse_args()
    if args.timeout <= 15:
        parser.error('--timeout must exceed the 15-second exit observation')
    group = 'process-kill' if args.probe == 'kill' else 'process-shared-exit'
    expected_checks = 48 if args.probe == 'kill' else 18
    suite = browser.GraphicsSuite(group, 'process-kill-probe', (group,),
        redirect_output=True, cleanup_wait_seconds=15, cleanup_marker='BOXEDWINE_PROCESS_PROBE_RETURNED',
        exit_status_policy='zero')
    executable = '/home/username/.wine/dosdevices/c:/files/' + suite.executable
    command = (shlex.quote(executable) + ' > /tmp/boxedwine-graphics-test.log 2>&1; '
        "probe_status=$?; printf '\\r\\n'; cat /tmp/boxedwine-graphics-test.log; "
        "printf '\\nBOXEDWINE_TEST_EXIT:%s\\n' \"$probe_status\"; "
        'echo BOXEDWINE_PROCESS_PROBE_RETURNED; exit $probe_status')

    # Reuse input hashing, isolated Chrome, redirected logs and late-error
    # observation. These scoped adapters do not change the ordinary Wine runner
    # or relax its PE validation. This probe does not launch Wine.
    original_validate = browser.validate_test_executable
    original_command = browser.build_guest_test_command
    browser.validate_test_executable = validate_elf
    browser.build_guest_test_command = lambda *a, **k: command
    try:
        result, manifest = browser.run_browser_test(suite=suite, group=group,
            build_dir=args.build_dir, filesystem=args.filesystem, test_executable=args.executable,
            chrome=browser.find_chrome(), run_dir=args.output, timeout=args.timeout,
            headless=args.headless, keep_browser_profile=False, mode=args.mode)
    finally:
        browser.validate_test_executable = original_validate
        browser.build_guest_test_command = original_command
    log = (args.output/'chrome.log').read_text(encoding='utf-8', errors='replace')
    emitted = diagnostics(log)
    passed = (result.passed and result.tests == expected_checks and result.todo == 0
        and result.skipped == 0 and bool(log.strip()) and not emitted)
    audit = {'passed':passed, 'expected_checks':expected_checks, 'result':manifest['result'],
        'exit_observation_seconds':15, 'launches_wine':False, 'diagnostics':emitted}
    (args.output/'probe-audit.json').write_text(json.dumps(audit, indent=2)+'\n', encoding='utf-8')
    print(json.dumps(audit, indent=2))
    return 0 if passed else 1


if __name__ == '__main__':
    raise SystemExit(main())
