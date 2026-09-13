"""Run strict P8 copies and observed palette presentation in a JIT browser."""
import argparse
import hashlib
import json
from pathlib import Path
import re

from auditGraphicsMatrix import diagnostics
from auditP8Copy import audit, audit_display_frames
from auditSampleMask import recover_guest
import wineGraphicsBrowser as browser


def identity(path):
    raw = path.read_bytes()
    return dict(path=str(path.resolve()), bytes=len(raw), sha256=hashlib.sha256(raw).hexdigest())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--filesystem', type=Path, required=True)
    parser.add_argument('--build-dir', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--mode', choices=('single-threaded-jit', 'multi-threaded-jit'), default='single-threaded-jit')
    parser.add_argument('--presentation', choices=('default', 'immediate', 'batched'), default='default')
    parser.add_argument('--chrome', type=Path)
    parser.add_argument('--timeout', type=int, default=600)
    parser.add_argument('--headless', action='store_true')
    args = parser.parse_args()
    if args.timeout <= 15:
        parser.error('--timeout must exceed the 15-second cleanup observation')
    directory = Path(__file__).resolve().parent
    capture = directory / 'tests/webgl_p8_present_capture.js'
    script = capture.read_text(encoding='utf-8')
    inputs = [identity(p) for p in (Path(__file__), directory / 'auditP8Copy.py',
        directory / 'auditSampleMask.py', directory / 'auditGraphicsMatrix.py',
        directory / 'wineGraphicsBrowser.py', capture)]
    environment = ['WINETEST_DEBUG=1', 'WINETEST_MUTE_THRESHOLD=100000', 'WINEDEBUG=-all,err+all,warn+d3d']
    if args.presentation != 'default':
        environment.append('BOXEDWINE_WEBGL_FRONTBUFFER_PRESENT_MIN_RECTS=' + ('1' if args.presentation == 'immediate' else '0'))
    suite = browser.GraphicsSuite('p8-copy', args.executable.name, ('ddraw7',), redirect_output=True,
        exit_status_policy='wine', cleanup_wait_seconds=15, environment=tuple(environment))
    original_prelude = browser._worker_error_observer_script
    browser._worker_error_observer_script = lambda: script + '\n' + original_prelude()
    try:
        result, manifest = browser.run_browser_test(suite=suite, group='ddraw7',
            build_dir=args.build_dir.resolve(), filesystem=args.filesystem.resolve(),
            test_executable=args.executable.resolve(), chrome=args.chrome or browser.find_chrome(),
            run_dir=args.output.resolve(), timeout=args.timeout, headless=args.headless,
            keep_browser_profile=False, mode=args.mode)
    finally:
        browser._worker_error_observer_script = original_prelude
    out = args.output.resolve()
    payload = json.loads((out / 'browser-payload.json').read_text(encoding='utf-8'))
    chrome = (out / 'chrome.log').read_text(encoding='utf-8', errors='replace')
    capture_error = None
    try:
        guest, capture_method = recover_guest(payload, (out / 'chrome.log').read_bytes(), manifest['launch_url'], 'ddraw7')
    except ValueError as error:
        guest, capture_method, capture_error = '', None, str(error)
    (out / 'guest.log').write_bytes(guest.encode('utf-8'))
    pixels = audit(guest)
    frames = audit_display_frames(chrome)
    output = browser.normalize_output(payload['output'])
    kill = re.findall(r'^BOXEDWINE_WINESERVER_KILL_STATUS:(\d+)$', output, re.M)
    wait = re.findall(r'^BOXEDWINE_WINESERVER_WAIT_STATUS:(\d+)$', output, re.M)
    cleanup = (payload['kind'] == 'complete' and payload['cleanupWaitSatisfied'] is True
        and payload['browserEvents'] == [] and manifest['cleanup_wait_seconds'] >= 15
        and manifest['cleanup_marker'] in output.splitlines() and kill in (['0'], ['1']) and wait == ['0']
        and not manifest['browser']['timed_out'] and not manifest['browser']['exited_early'])
    errors = diagnostics(chrome)
    resource_errors = [v for v in guest.splitlines() if re.search(
        r':err:d3d_shader:|does not have any up to date location\.|Device released with resources still bound\.|Leftover resource |Context array not freed!', v)]
    unchanged = all(identity(Path(v['path'])) == v for v in inputs)
    passed = bool(result.passed and manifest['result']['exit_status'] == 0 and pixels['passed'] and frames['passed']
        and cleanup and unchanged and manifest['input_identity_verified'] and not manifest['input_identity_problems']
        and not errors and not resource_errors and capture_error is None)
    record = dict(passed=passed, mode=args.mode, presentation=args.presentation, audit=pixels, display=frames,
        cleanup=cleanup, kill_statuses=kill, wait_statuses=wait, diagnostics=errors, resource_errors=resource_errors,
        inputs=inputs, inputs_unchanged=unchanged, input_identity_verified=manifest['input_identity_verified'],
        capture_error=capture_error, capture_method=capture_method,
        guest_log=identity(out / 'guest.log'), chrome_log=identity(out / 'chrome.log'),
        manifest=identity(out / 'manifest.json'), payload=identity(out / 'browser-payload.json'))
    (out / 'p8-copy-audit.json').write_text(json.dumps(record, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(dict(passed=passed, mode=args.mode, presentation=args.presentation,
        counts=pixels['counts'], display_passed=frames['passed'], cleanup=cleanup)))
    return 0 if passed else 1


if __name__ == '__main__':
    raise SystemExit(main())
