#!/usr/bin/env python3
"""Audit the cursor probe's GL/GDI compositor pixels, lifecycle and full log."""
import argparse
import json
from pathlib import Path

from PIL import Image
from auditGraphicsMatrix import diagnostics
from compareGameFrames import identity
from runSdkInputProbe import check_inputs


def check_pixels(path, mode):
    with Image.open(path) as image:
        image = image.convert('RGB')
        expected_size = (640, 480) if mode == 'GL' else (1024, 768)
        problems = []
        if image.size != expected_size:
            return dict(passed=False, image=identity(path), problems=['Wrong presentation size'], size=list(image.size))
        # The probe requests these colors directly through Clear / FillRect.
        # GDI's Win32 client origin is (84,90); exclude the text at client y=20.
        # The browser canvas has a one-pixel border and rounded CSS corners;
        # neither is output from the D3D clear. Exclude those four edge pixels.
        box, color = ((4, 4, 636, 476), (15, 80, 120)) if mode == 'GL' else ((84, 140, 724, 570), (80, 30, 110))
        def pixels_of(image):
            return image.get_flattened_data() if hasattr(image, 'get_flattened_data') else image.getdata()
        pixels = list(pixels_of(image.crop(box)))
        bad = sum(pixel != color for pixel in pixels)
        if bad:
            problems.append(f'{bad} incorrect client pixels of {len(pixels)}')
        # Child positions must not overwrite the desktop above/left of HWND.
        background_bad = 0
        if mode == 'GDI':
            for strip in ((4, 30, 80, 570), (80, 30, 724, 60)):
                background_bad += sum(pixel != (58, 110, 165) for pixel in pixels_of(image.crop(strip)))
            if background_bad:
                problems.append(f'{background_bad} overwritten desktop pixels')
        return dict(passed=not problems, image=identity(path), size=list(image.size),
                    client_box=list(box), expected_rgb=list(color), bad_pixels=bad,
                    checked_pixels=len(pixels), background_bad_pixels=background_bad, problems=problems)


def audit(folder):
    config = json.loads((folder / 'config.json').read_text())
    capture = json.loads((folder / 'capture.json').read_text())
    runner = json.loads((folder / 'cursor-result.json').read_text())
    check_inputs(config)
    assert runner['passed'] and runner['cursor']['passed'] and runner['driverExit'] == 0
    assert not capture['errors']
    assert all(capture[key] for key in ('applicationExitObserved', 'cleanupObserved', 'cleanupWaitSatisfied'))
    assert capture['cleanupObservationMilliseconds'] >= 15000
    chrome = (folder / 'chrome.log').read_text(errors='replace')
    assert chrome.strip() and not diagnostics(chrome)
    frames = runner['gdiFrames']
    assert len(frames) >= 2 and len(frames) % 2 == 0
    assert [row['mode'] for row in frames] == ['GDI', 'GL'] * (len(frames) // 2)
    checks = [check_pixels(folder / (row['name'] + '-native.png'), row['mode']) for row in frames]
    return dict(passed=all(row['passed'] for row in checks), build_id=config['buildId'],
                cycles=len(frames) // 2, full_chrome_diagnostics=[], frames=checks)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('folder', type=Path)
    args = parser.parse_args()
    result = audit(args.folder)
    with (args.folder / 'gdi-pixel-audit.json').open('x') as stream:
        json.dump(result, stream, indent=2); stream.write('\n')
    print(json.dumps(result))
    raise SystemExit(0 if result['passed'] else 1)
