"""Run keyboard.c through Boxedwine's actual native input/playback path."""
import argparse
from pathlib import Path
import struct
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('engine', type=Path)
parser.add_argument('filesystem', type=Path)
parser.add_argument('probe', type=Path, help='32-bit Linux keyboard probe')
parser.add_argument('output', type=Path, help='isolated output directory')
args = parser.parse_args()
out = args.output.resolve()
out.mkdir(parents=True, exist_ok=True)
play = out / 'play'
play.mkdir(exist_ok=True)
commands = ['VERSION=2']
for i in range(13):
    pixels = bytes((0, 0, 16 * (i + 1))) * 16 * 16
    header = struct.pack('<2sIHHI', b'BM', 54 + len(pixels), 0, 0, 54)
    header += struct.pack('<IiiHHIIiiII', 40, 16, 16, 1, 24, 0, len(pixels), 0, 0, 0, 0)
    name = f'ready-{i}.bmp'
    (play / name).write_bytes(header + pixels)
    commands += [f'SCREENSHOT=600,10,16,16,{name}', 'MOVETO=120,120',
                 'KEYDOWN=5', 'KEYUP=5', 'MOUSEDOWN=0,605,15', 'MOUSEUP=0,605,15']
commands += ['DONE']
(play / 'script.txt').write_text('\n'.join(commands) + '\n')
probe = args.probe.resolve()
command = [str(args.engine.resolve()), '-root', str(out / 'root'),
           '-zip', str(args.filesystem.resolve()), '-automation', str(play),
           '-resolution', '800x600', '-nosound', '-novideo',
           '-mount', str(probe.parent), '/mnt/probe', '/mnt/probe/' + probe.name]
with (out / 'run.log').open('w') as log:
    # Keep stdin connected for engines built with the desktop UI launcher.
    process = subprocess.Popen(command, cwd=out, stdin=subprocess.PIPE,
                               stdout=log, stderr=subprocess.STDOUT)
    try:
        process.wait(timeout=120)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait()
log = (out / 'run.log').read_text(errors='replace')
print(log)
raise SystemExit(0 if process.returncode in (0, 111) and 'RESULT checks=13 failures=0' in log else 1)
