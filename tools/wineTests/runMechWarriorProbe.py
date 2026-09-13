#!/usr/bin/env python3
"""Drive the local MechWarrior 3 demo through Instant Action and clean exit.

UI templates gate input transitions. They do not validate the 3D scene against
an independent native reference. Game files and template images stay local.
"""
import argparse
from datetime import datetime, timezone
import json
import math
import os
from pathlib import Path
import queue
import signal
import subprocess
import threading
import time

from PIL import Image

from auditGraphicsMatrix import diagnostics
from compareGameFrames import identity
from prepareGameCapture import prepare


# These compositor coordinates belong to the retained 1280x960 Chrome runs.
# Boxes are left/top/right/bottom. Foreground samples ignore animated menus.
TEMPLATES = {
    'main': ((95, 283, 284, 300), 'yellow'),
    'pilot': ((55, 27, 250, 48), 'yellow'),
    'setup': ((100, 30, 350, 52), 'yellow'),
    'launch': ((752, 575, 850, 638), 'green'),
    'results': ((311, 14, 483, 34), 'white'),
    'promo': ((341, 133, 651, 200), 'white'),
    'hud': ((723, 40, 845, 128), 'green'),
    'pause': ((99, 71, 193, 91), 'yellow'),
}


def foreground(rgb, color):
    r, g, b = rgb
    if color == 'yellow':
        return r > 110 and g > 70 and b < 75
    if color == 'white':
        return r > 185 and g > 185 and b > 185
    return r < 65 and g > 150 and b < 85


def verify_pins(entries):
    for expected in entries:
        actual = identity(Path(expected['path']))
        if any(actual[key] != expected[key] for key in ('bytes', 'sha256')):
            raise ValueError('Changed input: ' + expected['path'])


def rgb_pixels(image):
    data = image.convert('RGB').tobytes()
    return list(zip(data[0::3], data[1::3], data[2::3]))


class Templates:
    def __init__(self, manifest):
        data = json.loads(manifest.read_text(encoding='utf-8'))
        if data.get('schema_version') != 1 or set(data.get('templates', {})) != set(TEMPLATES):
            raise ValueError('Exactly the eight named UI templates are required')
        self.pins, self.samples = [], {}
        for name, entry in data['templates'].items():
            path = (manifest.parent / entry['path']).resolve()
            pin = dict(entry, path=str(path))
            verify_pins([pin])
            self.pins.append(pin)
            box, color = TEMPLATES[name]
            with Image.open(path) as image:
                if image.size != (886, 664):
                    raise ValueError('UI template must be an unscaled 886x664 compositor capture: ' + name)
                pixels = rgb_pixels(image.crop(box))
            samples = [(index, pixel) for index, pixel in enumerate(pixels) if foreground(pixel, color)]
            if len(samples) <= 40:
                raise ValueError('UI template has too few foreground samples: ' + name)
            self.samples[name] = samples

    def match(self, name, path):
        with Image.open(path) as image:
            if image.size != (886, 664):
                return 0.0
            actual = rgb_pixels(image.crop(TEMPLATES[name][0]))
        samples = self.samples[name]
        matched = sum(max(abs(a - b) for a, b in zip(actual[index], pixel)) < 55
                      for index, pixel in samples)
        return matched / len(samples)


class Scenario:
    """One controller owns every relative mouse event and remembered position."""
    def __init__(self, action, output, templates, clock=time.monotonic, performance=False):
        self.action, self.output, self.templates, self.clock = action, output, templates, clock
        self.mouse, self.sequence, self.observations = (0.0, 0.0), 0, []
        self.performance, self.measurement = performance, None

    def wait(self, milliseconds):
        self.action('wait', milliseconds=milliseconds)

    def capture(self, name):
        return self.action('capture', name=name)

    def until(self, name, seconds):
        self.sequence += 1
        deadline, attempt = self.clock() + seconds, 0
        while self.clock() < deadline:
            label = f'{name}-{self.sequence}-wait-{attempt:03}'
            attempt += 1
            state = self.capture(label)
            if state.get('lifecycle'):
                raise RuntimeError('Application exited before ' + name)
            score = self.templates.match(name, self.output / (label + '-canvas.png'))
            self.observations.append(dict(state=name, capture=label, foreground_match=score))
            if score > .88:
                print('Observed ' + name, flush=True)
                return
            self.wait(2000)
        raise RuntimeError('Menu was not observed: ' + name)

    def move(self, x, y):
        self.action('move', x=x, y=y)
        self.mouse = x, y
        self.wait(400)

    def move_steps(self, x, y):
        start = self.mouse
        steps = max(1, int(max(abs(x - start[0]), abs(y - start[1])) / .15) + 1)
        for step in range(1, steps + 1):
            self.move(start[0] + (x - start[0]) * step / steps,
                      start[1] + (y - start[1]) * step / steps)

    def button(self):
        self.action('mouse-button', down=True)
        self.wait(1500)
        self.action('mouse-button', down=False)
        self.wait(1200)

    def run(self):
        self.until('main', 300)
        self.action('pointer-lock', enabled=True)
        self.move(.5, .5)
        for x, y in [(.34, .33), (.18, .16), (.005, .068)]:
            self.move(x, y)
        self.capture('instant-hover'); self.button(); self.until('pilot', 45)
        self.action('focus')
        for key in ['b', 'w', 'Enter']:
            self.action('key', key=key, holdMs=300)
            self.wait(500)
        for x, y in [(.165, .155), (.32, .242)]:
            self.move(x, y)
        self.capture('pilot-accept-hover'); self.button(); self.until('setup', 45)
        self.button(); self.until('launch', 600)
        self.move(.492, .436); self.capture('launch-hover'); self.button()
        self.until('hud', 180)
        self.capture('mission-ready'); self.action('focus')
        if self.performance:
            self.wait(5000); self.wait(5000)
            self.measurement = self.action('measure-performance', name='mission-steady', milliseconds=30000)
            self.capture('mission-performance-ended')
        self.action('key', key='ArrowUp', holdMs=3000)
        self.wait(2000); self.capture('mission-moved')
        self.action('key', key='Escape', holdMs=1500)
        self.until('pause', 120); self.capture('mission-paused')
        self.action('key', key='Escape', holdMs=1500)
        self.until('hud', 120); self.move(.532, .436)
        self.wait(2000); self.capture('mission-resumed-aimed')
        self.action('key', key='Escape', holdMs=1500); self.until('pause', 120)
        self.move_steps(.300, .276); self.capture('mission-quit-hover'); self.button()
        self.until('results', 180); self.capture('mission-results')
        self.move_steps(.168, .767); self.capture('results-list-hover'); self.button()
        self.until('setup', 180); self.capture('instant-menu-returned')
        self.action('key', key='Escape', holdMs=200); self.until('main', 60)
        self.capture('main-returned')
        self.move_steps(.677, .747); self.capture('main-quit-hover'); self.button()
        self.until('promo', 60); self.capture('promo-screen')
        self.move(.561, .695); self.capture('final-quit-hover'); self.button()
        self.action('observe-exit', timeoutSeconds=45)
        self.capture('exited')


class Client:
    def __init__(self, command):
        self.process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT, text=True, encoding='utf-8', bufsize=1,
            creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0),
            start_new_session=os.name != 'nt')
        self.messages, self.transcript, self.closed = queue.Queue(), [], False
        self.reader = threading.Thread(target=self.read_stdout, daemon=True)
        self.reader.start()

    def read_stdout(self):
        for line in self.process.stdout:
            self.transcript.append(line)
            try:
                self.messages.put(json.loads(line))
            except ValueError:
                self.messages.put({'error': 'Non-JSON driver output: ' + line})
        self.messages.put({'error': 'Capture driver stdout closed'})

    def receive(self, timeout=75):
        reply = self.messages.get(timeout=timeout)
        if not isinstance(reply, dict):
            raise RuntimeError('Malformed capture driver response')
        if reply.get('error'):
            raise RuntimeError(reply['error'])
        return reply

    def action(self, kind, **values):
        if self.closed:
            raise RuntimeError('Capture driver already closed')
        command = dict(type=kind, **values)
        self.process.stdin.write(json.dumps(command) + '\n')
        self.process.stdin.flush()
        reply = self.receive(values.get('timeoutSeconds', 45) + 30 if kind == 'observe-exit' else 75)
        if kind == 'close':
            if reply.get('closed') != 'requested':
                raise RuntimeError('Capture driver did not acknowledge close')
            self.closed = True
        elif reply.get('action') != command or 'result' not in reply:
            raise RuntimeError('Mismatched capture action response')
        return reply.get('result', reply)

    def finish(self):
        try:
            if not self.closed and self.process.poll() is None:
                self.action('close')
            self.process.stdin.close()
            self.process.wait(timeout=30)
        finally:
            if self.process.poll() is None:
                if hasattr(subprocess, 'CREATE_NO_WINDOW'):
                    subprocess.run(['taskkill', '/PID', str(self.process.pid), '/T', '/F'],
                        capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
                else:
                    os.killpg(self.process.pid, signal.SIGTERM)
                self.process.wait(timeout=30)
            self.process.stdin.close()
            self.reader.join(timeout=5)
            self.process.stdout.close()


def audit_exit(capture, final_state, chrome_log, driver_code, observed):
    if not isinstance(capture, dict) or not isinstance(final_state, dict):
        raise ValueError('Malformed lifecycle evidence')
    duration = capture.get('cleanupObservationMilliseconds')
    lifecycle = final_state.get('lifecycle')
    if (driver_code != 0 or observed is not True or capture.get('errors') != []
            or not all(capture.get(k) is True for k in ('applicationExitObserved', 'cleanupObserved', 'cleanupWaitSatisfied'))
            or type(duration) not in (int, float) or not math.isfinite(duration) or duration < 15000
            or not isinstance(lifecycle, str)
            or lifecycle.splitlines() != ['BW_GAME_EXIT:0', 'BW_GAME_CLEANUP:0']):
        raise ValueError('Incomplete application, cleanup or driver lifecycle')
    findings = diagnostics(chrome_log)
    if not chrome_log.strip() or findings:
        raise ValueError('Full Chrome log is empty or contains graphics/browser failures: ' + str(findings))
    return findings


def validate_config(config):
    if Path(config['output']).exists():
        raise ValueError('Capture output already exists')
    mode = config['buildIdentity']['mode']
    source = config['buildIdentity']['source']
    checked = prepare(config, mode=mode, commit=source['commit'], source_dirty=source['dirty'])
    if (checked['buildIdentity'] != config['buildIdentity'] or checked['buildId'] != config['buildId']
            or checked['applicationIdentity'] != config['applicationIdentity']):
        raise ValueError('Prepared capture identity changed')
    if (checked['applicationIdentity']['sha256'] != '5c720ba778fa76fccf3896a5611da6c080e7d855f42074c7d12b4c6962d289c2'
            or checked['applicationIdentity']['bytes'] != 31505519):
        raise ValueError('This sequence requires the pinned MechWarrior 3 demo package')
    if '--window-size=1280,960' not in config.get('chromeArgs', []):
        raise ValueError('UI templates require a 1280x960 Chrome window')


def run(args):
    config_bytes = args.config.read_bytes()
    config = json.loads(config_bytes)
    validate_config(config)
    templates = Templates(args.templates.resolve())
    output = Path(config['output'])
    command = [str(args.node), str(Path(__file__).with_name('captureGame.mjs')), str(args.config.resolve())]
    result = dict(started_at=datetime.now(timezone.utc).isoformat(), scope=__doc__,
        input_lifecycle_passed=False, game_acceptance=False, problems=[], command=command,
        config=identity(args.config), templates=identity(args.templates), references=templates.pins,
        runner=identity(Path(__file__)), expected_build_id=config['buildId'],
        launch_path=config.get('launchPath'), launch_url=None)
    client = scenario = None
    try:
        client = Client(command)
        scenario = Scenario(client.action, output, templates, performance=config.get('performance') is True)
        ready = client.receive(90)
        result['launch_url'] = ready.get('url')
        if ready.get('ready') is not True:
            raise RuntimeError('Capture driver did not become ready')
        scenario.run()
        result['exit_observed'] = True
    except (RuntimeError, ValueError, OSError, queue.Empty) as error:
        result['problems'].append(str(error) or 'Driver response timeout')
    finally:
        if client is not None:
            try:
                client.finish()
            except (RuntimeError, OSError, queue.Empty, subprocess.TimeoutExpired) as error:
                result['problems'].append('Driver teardown: ' + str(error))
    result['driver_started'] = client is not None
    result['driver_exit_code'] = client.process.returncode if client is not None else None
    result['observations'] = scenario.observations if scenario is not None else []
    try:
        verify_pins(config['inputs'] + templates.pins + [result['config'], result['templates'], result['runner']])
        capture = json.loads((output / 'capture.json').read_text())
        if not isinstance(capture, dict):
            raise ValueError('Malformed capture metadata')
        result['launch_url'] = capture.get('launchUrl', result['launch_url'])
        state = json.loads((output / 'final-state.json').read_text())
        log = (output / 'chrome.log').read_text(errors='replace')
        result['full_chrome_diagnostics'] = audit_exit(capture, state, log,
            result['driver_exit_code'], result.get('exit_observed'))
        result['capture'] = identity(output / 'capture.json')
    except (OSError, ValueError, KeyError) as error:
        result['problems'].append(str(error))
    result['input_lifecycle_passed'] = not result['problems']
    result['performance_requested'] = config.get('performance') is True
    result['performance'] = scenario.measurement if scenario is not None else None
    result['passed'] = result['input_lifecycle_passed'] and (not result['performance_requested']
        or isinstance(result['performance'], dict) and result['performance'].get('measurementValid') is True)
    result['finished_at'] = datetime.now(timezone.utc).isoformat()
    output.mkdir(parents=True, exist_ok=True)
    with (output / 'mech-input-config.json').open('xb') as stream:
        stream.write(config_bytes)
    for name, value in [('mech-driver.jsonl', ''.join(client.transcript) if client is not None else ''),
                        ('mech-probe.json', json.dumps(result, indent=2) + '\n')]:
        with (output / name).open('x', encoding='utf-8') as stream:
            stream.write(value)
    print(('PASS' if result['passed'] else 'FAIL') + ' MechWarrior input/lifecycle and requested measurements: ' + str(output))
    return 0 if result['passed'] else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--config', type=Path, required=True, help='Prepared captureGame configuration')
    parser.add_argument('--templates', type=Path, required=True, help='Local pinned UI template manifest')
    parser.add_argument('--node', type=Path, required=True)
    return run(parser.parse_args())


if __name__ == '__main__':
    raise SystemExit(main())
