#!/usr/bin/env python3
"""Check PostProcess's visible Blur button against native scene references.

This is an input/effect regression, not full SDK image acceptance. Whole-frame
comparisons are retained separately, including Wine/native font differences.
"""
import argparse
from datetime import datetime, timezone
import json
from pathlib import Path
import queue
import subprocess
import threading
import time

from auditGraphicsMatrix import diagnostics
from compareGameFrames import compare_frames, identity
from gamePerformancePhase import measure_phase, measurements_pass


def check_inputs(config):
    for expected in config['inputs']:
        actual = identity(Path(expected['path']))
        if any(actual[key] != expected[key] for key in ('bytes', 'sha256')):
            raise ValueError('Changed input: ' + expected['path'])


def check_scene(actual, reference, previous=None):
    rule = {'name': 'dwarf-and-hallway', 'box': [175, 90, 225, 310],
            'max_bad_fraction': 0.01}
    if previous is not None:
        rule['min_progress_fraction'] = 0.10
    result = compare_frames(actual, reference, regions=[rule], previous=previous)
    # Full-frame failure remains in result['passed']; never silently relax it.
    result['input_probe_scene_passed'] = (
        result['actual_size'] == [640, 480] == result['reference_size']
        and len(result['regions']) == 1 and result['regions'][0]['passed']
        and result['actual_dominant_fraction'] < 0.98
        and result['reference_dominant_fraction'] < 0.98)
    return result


def scene_layout_ready(state):
    # Capability probes resize GL canvases before a frame has been presented.
    # Wait for the presentation layout as well; nativeClip still checks the
    # actual geometry immediately before and after each lossless capture.
    visible = [c for c in state.get('canvases', [])
               if c.get('display') != 'none' and c.get('visibility') == 'visible']
    return (state.get('frameCanvasSize') == [640, 480] and bool(visible)
            and [visible[-1]['width'], visible[-1]['height']] == [640, 480])


def check_ui(actual, reference):
    # Same-font desktop Wine control, independent of the browser compositor.
    # Keep the Windows-native scene checks and failed full images separately.
    return compare_frames(actual, reference, max_bad_fraction=0.001, regions=[
        {'name': 'effect-manager', 'box': [415, 0, 225, 480],
         'max_bad_fraction': 0.001}])


def run(args):
    config_bytes = args.config.read_bytes()
    config = json.loads(config_bytes)
    check_inputs(config)
    refs = [identity(args.native_base), identity(args.native_blur)]
    ui_refs = ([identity(args.desktop_ui_base), identity(args.desktop_ui_blur)]
               if args.desktop_ui_base else [])
    output = Path(config['output'])
    if output.exists():
        raise ValueError('Capture output already exists: ' + str(output))
    driver = Path(__file__).with_name('captureGame.mjs')
    command = [str(args.node), str(driver), str(args.config.resolve())]
    result = {'started_at': datetime.now(timezone.utc).isoformat(), 'command': command,
              'scope': __doc__, 'passed': False, 'sdk_acceptance': False,
              'references': refs, 'runner': identity(Path(__file__)),
              'checkpoints': [], 'problems': [], 'ui_references': ui_refs,
              'ui_checkpoints': [], 'config': identity(args.config),
              'launch_path': config.get('launchPath'), 'launch_url': None,
              'performance_requested': config.get('performance') is True,
              'performance': {}, 'performance_acceptance': False}
    messages, transcript, stderr = queue.Queue(), [], []
    process = reader = error_reader = None

    def read_stdout():
        for line in process.stdout:
            transcript.append(line)
            try:
                messages.put(json.loads(line))
            except ValueError:
                messages.put({'error': 'Non-JSON driver output: ' + line})
        messages.put({'error': 'Driver stdout closed before expected response'})

    def receive(timeout):
        message = messages.get(timeout=timeout)
        if not isinstance(message, dict):
            raise RuntimeError('Malformed capture driver response')
        if message.get('error'):
            raise RuntimeError(message['error'])
        return message

    def action(kind, timeout=30, **values):
        process.stdin.write(json.dumps(dict(type=kind, **values)) + '\n')
        process.stdin.flush()
        reply = receive(timeout)
        if kind == 'close':
            if reply.get('closed') != 'requested':
                raise RuntimeError('Driver did not acknowledge close')
        elif reply.get('action') != dict(type=kind, **values) or 'result' not in reply:
            raise RuntimeError('Mismatched driver action response')
        return reply.get('result', reply)

    def stable_ui(label, reference):
        # Click the blank scene to clear the control's hover and keyboard focus,
        # matching the explicit input used for the desktop Wine reference.
        # A fixed wall-clock delay can capture a fade on the interpreter modes.
        action('click', x=10 / 640, y=300 / 480, holdMs=100)
        deadline, previous, attempt = time.monotonic() + 120, None, 0
        while time.monotonic() < deadline:
            if action('wait', milliseconds=1000).get('lifecycle'):
                raise RuntimeError('Application exited before the stable ' + label + ' UI checkpoint')
            name = label + '-ui-' + str(attempt)
            action('native-capture', name=name)
            frame = output / (name + '-native.png')
            comparison = check_ui(frame, reference)
            stability = (compare_frames(frame, previous, tolerance=0, max_bad_fraction=0)
                         if previous else None)
            result['ui_checkpoints'].append(dict(state=label, reference=comparison,
                                                 stability=stability))
            if comparison['passed'] and stability and stability['passed']:
                return frame
            previous, attempt = frame, attempt + 1
        raise RuntimeError(label + ' UI did not reach a stable matching frame')

    try:
        process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE, text=True, encoding='utf-8',
                                   creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        reader = threading.Thread(target=read_stdout, daemon=True)
        reader.start()
        error_reader = threading.Thread(target=lambda: stderr.extend(process.stderr), daemon=True)
        error_reader.start()
        ready = receive(60)
        result['launch_url'] = ready.get('url')
        if ready.get('ready') is not True:
            raise RuntimeError('Driver did not become ready')
        deadline, attempt, base = time.monotonic() + args.startup_timeout, 0, None
        while time.monotonic() < deadline:
            state = action('state')
            if state.get('lifecycle'):
                raise RuntimeError('Application exited before the scene checkpoint')
            if scene_layout_ready(state):
                name = 'base-ready-' + str(attempt)
                action('native-capture', name=name)
                base = output / (name + '-native.png')
                comparison = check_scene(base, args.native_base)
                result['checkpoints'].append(comparison)
                attempt += 1
                if comparison['input_probe_scene_passed']:
                    break
            action('wait', milliseconds=5000)
        else:
            raise RuntimeError('Native base scene did not appear before startup timeout')
        if ui_refs:
            base = stable_ui('base', args.desktop_ui_base)
        if result['performance_requested']:
            measure_phase(lambda kind, **values: action(kind, timeout=75, **values),
                          result['performance'], 'base-steady')
        print('Base scene ready; clicking visible Blur button.', flush=True)
        action('click', x=470 / 640, y=428 / 480, holdMs=300)
        action('wait', milliseconds=3000)
        action('native-capture', name='blur')
        comparison = check_scene(output / 'blur-native.png', args.native_blur, base)
        result['checkpoints'].append(comparison)
        if not comparison['input_probe_scene_passed']:
            raise RuntimeError('Visible Blur click did not produce the native scene/effect')
        if ui_refs:
            stable_ui('blur', args.desktop_ui_blur)
        if result['performance_requested']:
            measure_phase(lambda kind, **values: action(kind, timeout=75, **values),
                          result['performance'], 'blur-steady')
        action('key', key='Escape')
        action('observe-exit', timeout=90, timeoutSeconds=60)
        result['exit_observed'] = True
    except (RuntimeError, ValueError, OSError, queue.Empty) as error:
        result['problems'].append(str(error) or 'Driver response timeout')
    finally:
        if process is not None and process.poll() is None:
            try:
                action('close')
                process.wait(timeout=30)
            except (RuntimeError, OSError, queue.Empty, subprocess.TimeoutExpired) as error:
                result['problems'].append('Driver teardown: ' + str(error))
                if process.poll() is None:
                    if hasattr(subprocess, 'CREATE_NO_WINDOW'):
                        subprocess.run(['taskkill', '/PID', str(process.pid), '/T', '/F'],
                                       capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
                    else:
                        process.terminate()
                    process.wait(timeout=30)
        if reader is not None and reader.ident is not None:
            reader.join(timeout=5)
        if error_reader is not None and error_reader.ident is not None:
            error_reader.join(timeout=5)
    result['driver_started'] = process is not None
    result['driver_exit_code'] = process.returncode if process is not None else None
    try:
        check_inputs(config)
        if identity(args.config) != result['config']:
            raise ValueError('Capture configuration changed during the run')
        if refs != [identity(args.native_base), identity(args.native_blur)]:
            raise ValueError('Native references changed during the run')
        if ui_refs and ui_refs != [identity(args.desktop_ui_base), identity(args.desktop_ui_blur)]:
            raise ValueError('Desktop Wine UI references changed during the run')
        capture = json.loads((output / 'capture.json').read_text())
        if not isinstance(capture, dict):
            raise ValueError('Malformed capture metadata')
        result['launch_url'] = capture.get('launchUrl', result['launch_url'])
        if (result['driver_exit_code'] != 0 or capture['errors'] or not result.get('exit_observed')
                or not all(capture[k] for k in ('applicationExitObserved', 'cleanupObserved', 'cleanupWaitSatisfied'))
                or capture.get('cleanupObservationMilliseconds', 0) < 15000):
            raise ValueError('Application/cleanup/driver lifecycle did not pass')
        log = (output / 'chrome.log').read_text(errors='replace')
        result['full_chrome_diagnostics'] = diagnostics(log)
        if not log.strip() or result['full_chrome_diagnostics']:
            raise ValueError('Full Chrome log is empty or contains graphics/browser failures')
    except (OSError, ValueError, KeyError) as error:
        result['problems'].append(str(error))
    result['input_lifecycle_passed'] = not result['problems']
    result['stable_desktop_wine_ui_passed'] = bool(ui_refs) and result['input_lifecycle_passed']
    result['passed'] = result['input_lifecycle_passed'] and (
        not result['performance_requested'] or measurements_pass(
            result['performance'], ('base-steady', 'blur-steady')))
    output.mkdir(parents=True, exist_ok=True)
    with (output / 'sdk-input-config.json').open('xb') as stream:
        stream.write(config_bytes)
    for name, data in (('sdk-input-driver.jsonl', ''.join(transcript)),
                       ('sdk-input-driver.stderr', ''.join(stderr)),
                       ('sdk-input-probe.json', json.dumps(result, indent=2) + '\n')):
        with (output / name).open('x', encoding='utf-8') as stream:
            stream.write(data)
    print(('PASS' if result['passed'] else 'FAIL') + ' PostProcess input/effect: ' + str(output), flush=True)
    return 0 if result['passed'] else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--config', type=Path, required=True)
    parser.add_argument('--node', type=Path, required=True)
    parser.add_argument('--native-base', type=Path, required=True)
    parser.add_argument('--native-blur', type=Path, required=True)
    parser.add_argument('--desktop-ui-base', type=Path)
    parser.add_argument('--desktop-ui-blur', type=Path)
    parser.add_argument('--startup-timeout', type=float, default=300)
    args = parser.parse_args()
    if not 0 < args.startup_timeout <= 1800:
        parser.error('startup timeout must be between 0 and 1800 seconds')
    if bool(args.desktop_ui_base) != bool(args.desktop_ui_blur):
        parser.error('desktop UI references must be supplied as a pair')
    return run(args)


if __name__ == '__main__':
    raise SystemExit(main())
