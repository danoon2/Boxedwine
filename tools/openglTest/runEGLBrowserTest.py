#!/usr/bin/env python3
"""Run the Linux EGL/GLES probe with isolated Chrome and observed process exit."""

from datetime import datetime
import argparse
import json
import os
from pathlib import Path, PurePosixPath
import shlex
import struct
import sys
import zipfile

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPOSITORY_ROOT / "tools/wineTests"))
import wineGraphicsBrowser as browser  # noqa: E402
from auditGraphicsMatrix import diagnostics  # noqa: E402

BUILD_MODES = {"st": ("Release", "single-threaded-non-jit"),
    "mt": ("MultiThreaded", "multi-threaded-non-jit"),
    "st-jit": ("Jit", "single-threaded-jit"),
    "mt-jit": ("MultiThreadedJit", "multi-threaded-jit")}
PASS_MARKER = "PASS real ES pbuffer context, VBO draw, and texture sample"
RETURN_MARKER = "BOXEDWINE_EGL_PROBE_RETURNED:0"
SUITE = browser.GraphicsSuite("egl", "EGLRealESContextTest", ("real-es-context",),
    result_style="marshal", redirect_output=True, cleanup_wait_seconds=15,
    cleanup_marker=RETURN_MARKER)
LIBRARIES = ("libEGL.so.1", "libGLESv2.so.2", "libGL.so.1")


def validate_elf(executable, _suite=SUITE):
    with Path(executable).open('rb') as stream:
        header = stream.read(20)
    if (len(header) != 20 or header[:6] != b'\x7fELF\x01\x01'
            or struct.unpack_from('<H', header, 18)[0] != 3):
        raise browser.RunnerError('EGL probe must be a little-endian i386 ELF')


def create_app(executable, suite, destination, libraries):
    validate_elf(executable)
    destination = Path(destination)
    destination.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(destination, 'x', zipfile.ZIP_DEFLATED) as archive:
        archive.write(executable, suite.executable)
        for library in libraries:
            archive.write(library, PurePosixPath('lib', library.name).as_posix())
    return destination


def guest_command(use_filesystem_libraries):
    executable = '/home/username/.wine/dosdevices/c:/files/' + SUITE.executable
    # No Wine is launched. Preserve the ELF's actual exit status in the durable
    # log. The summary represents one aggregate executable regression.
    environment = ('unset LD_LIBRARY_PATH; ' if use_filesystem_libraries else
        'export LD_LIBRARY_PATH=/home/username/.wine/dosdevices/c:/files/lib; ')
    return (environment + shlex.quote(executable) + ' > /tmp/boxedwine-graphics-test.log 2>&1; '
        'probe_status=$?; '
        'echo BOXEDWINE_EGL_PROBE_STATUS:$probe_status >> /tmp/boxedwine-graphics-test.log; '
        'if [ "$probe_status" -eq 0 ]; then '
        "echo 'Summary: 1 passed, 0 failed, 0 skipped' >> /tmp/boxedwine-graphics-test.log; "
        "echo '" + RETURN_MARKER + "' >> /tmp/boxedwine-graphics-test.log; "
        'else echo "FAIL EGL process exited with status $probe_status" >> /tmp/boxedwine-graphics-test.log; '
        "echo 'Summary: 0 passed, 1 failed, 0 skipped' >> /tmp/boxedwine-graphics-test.log; fi; "
        "printf '\\r\\n'; cat /tmp/boxedwine-graphics-test.log; exit $probe_status")


def audit_result(result, manifest):
    artifacts = manifest['artifacts']
    payload = json.loads(Path(artifacts['browser_payload']).read_text(encoding='utf-8'))
    log = Path(artifacts['chrome_log']).read_text(encoding='utf-8', errors='replace')
    lines = browser.normalize_output(payload.get('output', '')).splitlines()
    problems = []
    if not result.passed or (result.tests, result.todo, result.failures, result.skipped) != (1, 0, 0, 0):
        problems.append(result.reason)
    if PASS_MARKER not in lines:
        problems.append('EGL probe did not emit its exact final pass marker')
    if any(line.lstrip().startswith('FAIL ') for line in lines):
        problems.append('EGL output contains a failed check')
    statuses = [line.partition(':')[2] for line in lines if line.startswith('BOXEDWINE_EGL_PROBE_STATUS:')]
    if not statuses or set(statuses) != {'0'}:
        problems.append('EGL process did not exit successfully')
    if RETURN_MARKER not in lines or payload.get('cleanupWaitSatisfied') is not True:
        problems.append('EGL process return and observation did not complete')
    if payload.get('kind') != 'complete' or payload.get('browserEvents') != []:
        problems.append('Browser payload is incomplete or has errors')
    if (not manifest.get('finished_at') or manifest.get('cleanup_wait_seconds') != 15
            or manifest.get('cleanup_marker') != RETURN_MARKER
            or manifest['browser'].get('timed_out') is not False
            or manifest['browser'].get('exited_early') is not False):
        problems.append('Browser did not complete the required exit observation')
    emitted = diagnostics(log)
    if not log.strip() or emitted:
        problems.append('Full browser stderr is empty or contains graphics/browser diagnostics')
    return dict(passed=not problems, problems=problems, diagnostics=emitted,
        scope='One aggregate EGL executable regression; no Wine assertion count or gameplay acceptance inferred.',
        launches_wine=False, exit_observation_seconds=15, result=manifest['result'],
        artifacts={name: dict(path=str(Path(artifacts[name]).resolve()),
            sha256=browser._sha256(Path(artifacts[name]))) for name in ('browser_payload', 'chrome_log')})


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-mode', choices=BUILD_MODES, default='st')
    parser.add_argument('--build-dir', type=Path, help='use an explicitly frozen Emscripten build')
    parser.add_argument('--build-commit', help='explicit full runtime commit label (default: unknown)')
    parser.add_argument('--build-source-dirty', choices=('true', 'false', 'unknown'), default='unknown')
    parser.add_argument('--filesystem', type=Path)
    parser.add_argument('--executable', type=Path, default=Path(__file__).resolve().parent / 'Win32/Release/EGLRealESContextTest')
    parser.add_argument('--use-filesystem-libraries', action='store_true', help='use installed GL/EGL/GLES libraries, with no app overrides')
    parser.add_argument('--library-dir', type=Path, help='bundled GL/EGL/GLES directory (default: lib beside executable)')
    parser.add_argument('--chrome', type=Path)
    parser.add_argument('--timeout', type=int, default=90)
    parser.add_argument('--output-dir', type=Path, default=REPOSITORY_ROOT / 'tmp/egl-validation')
    args = parser.parse_args(argv)
    if args.timeout <= 15:
        parser.error('--timeout must exceed the 15-second exit observation')
    if args.use_filesystem_libraries and args.library_dir:
        parser.error('--library-dir conflicts with --use-filesystem-libraries')
    filesystem = args.filesystem
    if filesystem is None:
        if not os.environ.get('APPDATA'):
            parser.error('--filesystem is required outside Windows')
        filesystem = Path(os.environ['APPDATA']) / 'Boxedwine/FileSystems2/TinyCore15Wine11.0.zip'
    build_name, mode = BUILD_MODES[args.build_mode]
    build = args.build_dir or REPOSITORY_ROOT / 'project/emscripten/Build' / build_name
    executable = args.executable.resolve()
    libraries = [] if args.use_filesystem_libraries else [
        (args.library_dir or executable.parent / 'lib') / name for name in LIBRARIES]
    try:
        browser.validate_web_build(build)
        validate_elf(executable)
        for path in [filesystem, *libraries]:
            if not path.is_file():
                raise browser.RunnerError('missing input: ' + str(path))
        run = args.output_dir.resolve() / ('browser-' + args.build_mode + '-' + datetime.now().strftime('%Y%m%d-%H%M%S-%f'))
        command = guest_command(args.use_filesystem_libraries)
        # Scoped adapters affect this runner process only. Ordinary PE/Wine
        # validation and the active conformance matrix are unchanged.
        original = (browser.validate_test_executable, browser.create_test_app_zip, browser.build_guest_test_command)
        browser.validate_test_executable = validate_elf
        browser.create_test_app_zip = lambda exe, suite, dest: create_app(exe, suite, dest, libraries)
        browser.build_guest_test_command = lambda *a, **k: command
        try:
            result, manifest = browser.run_browser_test(suite=SUITE, group=SUITE.groups[0],
                build_dir=build.resolve(), filesystem=filesystem.resolve(), test_executable=executable,
                chrome=browser.find_chrome(args.chrome), run_dir=run, timeout=args.timeout,
                headless=True, keep_browser_profile=True, mode=mode,
                build_commit=args.build_commit,
                build_source_dirty={'true': True, 'false': False, 'unknown': None}[args.build_source_dirty])
        finally:
            browser.validate_test_executable, browser.create_test_app_zip, browser.build_guest_test_command = original
        audit = audit_result(result, manifest)
        audit['library_source'] = 'filesystem' if args.use_filesystem_libraries else 'app override'
        audit['bundled_libraries'] = [dict(path=str(path.resolve()), sha256=browser._sha256(path)) for path in libraries]
        with (run / 'egl-audit.json').open('x', encoding='utf-8', newline='\n') as stream:
            json.dump(audit, stream, indent=2)
            stream.write('\n')
    except (browser.RunnerError, OSError, ValueError, KeyError) as error:
        print('ERROR: ' + str(error), file=sys.stderr)
        return 2
    print(json.dumps(audit, indent=2))
    print('Artifacts: ' + str(run))
    return 0 if audit['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
