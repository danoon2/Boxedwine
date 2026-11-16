#!/usr/bin/env python3
"""Compare GL/GDI probe text with the same font rendered into a native Wine DIB."""
import argparse
import json
from pathlib import Path

from PIL import Image
from auditCursorGdiFrames import audit as audit_frames
from compareGameFrames import identity


def pixels(image):
    return image.get_flattened_data() if hasattr(image, 'get_flattened_data') else image.getdata()


def audit(folder, reference):
    result = audit_frames(folder)
    with Image.open(reference) as image:
        if image.size != (400, 64):
            raise ValueError('Expected the complete 400x64 native font-control bitmap')
        # The existing cursor probe requests 22 characters, with an extent of
        # 153 pixels. The native DIB includes the final I as a 23rd character;
        # compare the requested prefix only, without rescaling either image.
        expected = image.convert('RGB').crop((20, 20, 173, 36))
        if set(pixels(expected)) != {(80, 30, 110), (255, 255, 255)}:
            raise ValueError('Reference is not the expected nonblank System-font bitmap')
    result['text_reference'] = identity(reference)
    result['text_reference_box'] = [20, 20, 173, 36]
    checks = []
    for frame in result['frames']:
        if frame['size'] != [1024, 768]:
            continue
        with Image.open(frame['image']['path']) as image:
            actual = image.convert('RGB').crop((104, 110, 257, 126))
            incorrect = sum(left != right for left, right in zip(pixels(actual), pixels(expected)))
        checks.append(dict(image=frame['image'], box=[104, 110, 257, 126],
                           checked_pixels=153 * 16, bad_pixels=incorrect, passed=incorrect == 0))
    if len(checks) != result['cycles']:
        raise ValueError('Missing GDI text checkpoints')
    result['text_checks'] = checks
    result['passed'] = result['passed'] and all(row['passed'] for row in checks)
    return result


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('folder', type=Path)
    parser.add_argument('reference', type=Path)
    args = parser.parse_args()
    result = audit(args.folder, args.reference)
    with (args.folder / 'gdi-text-audit.json').open('x') as stream:
        json.dump(result, stream, indent=2)
        stream.write('\n')
    print(json.dumps(result))
    raise SystemExit(0 if result['passed'] else 1)
