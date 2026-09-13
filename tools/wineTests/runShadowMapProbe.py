#!/usr/bin/env python3
"""Check ShadowMap's stationary scene against an independent native capture.

The whole 640x480 frame and both shadow regions must match, followed by an
identical second frame and clean application/cleanup exit. This validates the
selected compiled-effect scene, not Wine's source-effect compiler.
"""
import argparse
from datetime import datetime, timezone
import json
from pathlib import Path
import queue
import subprocess
import time

from compareGameFrames import compare_frames, identity
from gamePerformancePhase import measure_phase, measurements_pass
from runMechWarriorProbe import Client, audit_exit
from runSdkInputProbe import check_inputs, scene_layout_ready


def check_scene(actual, reference):
    # Preserve the rules used by the original four-mode checkpoint and its
    # separately compiled missing-shadow negative control.
    return compare_frames(actual, reference, regions=[
        dict(name='shadow-wall', box=[0, 0, 640, 255], max_bad_fraction=0.05),
        dict(name='shadow-ground', box=[170, 270, 100, 110], max_bad_fraction=0.05)])


def wait_for_scene(action, output, reference, checkpoints, seconds, clock=time.monotonic):
    deadline, previous, settled = clock() + seconds, None, 0
    while clock() < deadline:
        state = action('state')
        if not isinstance(state, dict):
            raise ValueError('Malformed scene state')
        if state.get('lifecycle'):
            raise RuntimeError('Application exited before scene')
        if scene_layout_ready(state):
            name = f'scene-{len(checkpoints):02d}'
            action('native-capture', name=name)
            frame = output / (name + '-native.png')
            comparison = check_scene(frame, reference)
            checkpoints.append(comparison)
            current = identity(frame)['sha256']
            if (comparison['passed'] and comparison['actual_size'] == [640, 480]
                    and comparison['reference_size'] == [640, 480]
                    and comparison['actual_dominant_fraction'] < 0.98):
                settled = settled + 1 if current == previous else 1
                if settled >= 2:
                    return
            else:
                settled = 0
            previous = current
        else:
            # A hidden/replaced canvas interrupts a consecutive stable pair.
            previous, settled = None, 0
        action('wait', milliseconds=5000)
    raise RuntimeError('Stable native scene checkpoint timed out')


def run(args):
    config_bytes = args.config.read_bytes()
    config = json.loads(config_bytes)
    check_inputs(config)
    reference_before = identity(args.reference)
    output = Path(config['output'])
    if output.exists():
        raise ValueError('Capture output already exists: ' + str(output))
    command = [str(args.node), str(Path(__file__).with_name('captureGame.mjs')),
               str(args.config.resolve())]
    result = dict(started_at=datetime.now(timezone.utc).isoformat(), scope=__doc__,
                  command=command, runner=identity(Path(__file__)),
                  config=identity(args.config), reference=reference_before,
                  launch_path=config.get('launchPath'), launch_url=None,
                  passed=False, checkpoints=[], problems=[],
                  performance_requested=config.get('performance') is True,
                  performance={}, performance_acceptance=False)
    client = None
    try:
        client = Client(command)
        ready = client.receive()
        result['launch_url'] = ready.get('url')
        if ready.get('ready') is not True:
            raise RuntimeError('Capture driver did not become ready')
        wait_for_scene(client.action, output, args.reference, result['checkpoints'], args.startup_timeout)
        result['rendered_and_stable'] = True
        if result['performance_requested']:
            measure_phase(client.action, result['performance'], 'scene-steady')
        client.action('key', key='Escape')
        client.action('observe-exit', timeoutSeconds=60)
        result['exit_observed'] = True
    except (RuntimeError, ValueError, OSError, queue.Empty) as error:
        result['problems'].append(str(error) or type(error).__name__)
    finally:
        if client is not None:
            try:
                client.finish()
            except (RuntimeError, OSError, queue.Empty, subprocess.TimeoutExpired) as error:
                result['problems'].append('Driver teardown: ' + (str(error) or type(error).__name__))
    result['driver_started'] = client is not None
    result['driver_exit_code'] = client.process.returncode if client else None
    try:
        check_inputs(config)
        if identity(args.config) != result['config']:
            raise ValueError('Capture configuration changed during the run')
        if identity(args.reference) != reference_before:
            raise ValueError('Native reference changed during the run')
        capture = json.loads((output / 'capture.json').read_text())
        if not isinstance(capture, dict):
            raise ValueError('Malformed capture metadata')
        result['launch_url'] = capture.get('launchUrl', result['launch_url'])
        final = json.loads((output / 'final-state.json').read_text())
        result['full_chrome_diagnostics'] = audit_exit(capture, final,
            (output / 'chrome.log').read_text(errors='replace'),
            result['driver_exit_code'], result.get('exit_observed'))
    except (OSError, ValueError, KeyError) as error:
        result['problems'].append('Final audit: ' + str(error))
    result['input_lifecycle_passed'] = not result['problems']
    result['passed'] = result['input_lifecycle_passed'] and (
        not result['performance_requested'] or measurements_pass(result['performance'], ('scene-steady',)))
    output.mkdir(parents=True, exist_ok=True)
    with (output / 'shadowmap-config.json').open('xb') as stream:
        stream.write(config_bytes)
    for name, value in (
            ('shadowmap-driver.jsonl', ''.join(client.transcript) if client else ''),
            ('shadowmap-checkpoint.json', json.dumps(result, indent=2) + '\n')):
        with (output / name).open('x', encoding='utf-8') as stream:
            stream.write(value)
    print(('PASS ' if result['passed'] else 'FAIL ') + 'ShadowMap: ' + str(output), flush=True)
    return 0 if result['passed'] else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--config', type=Path, required=True)
    parser.add_argument('--node', type=Path, required=True)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--startup-timeout', type=float, default=600)
    args = parser.parse_args()
    if not 0 < args.startup_timeout <= 1800:
        parser.error('startup timeout must be between 0 and 1800 seconds')
    return run(args)


if __name__ == '__main__':
    raise SystemExit(main())
