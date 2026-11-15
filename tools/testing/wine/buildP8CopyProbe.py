"""Build the P8 regression in a configured Wine 11 Win32 test build."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import subprocess


def identity(path):
    raw = path.read_bytes()
    return dict(path=str(path.resolve()), bytes=len(raw), sha256=hashlib.sha256(raw).hexdigest())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--wine-source', type=Path, required=True)
    parser.add_argument('--wine-build', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    tree = args.wine_source.resolve()
    build = args.wine_build.resolve()
    source = tree / 'dlls/ddraw/tests/ddraw7.c'
    executable = build / 'dlls/ddraw/tests/ddraw_test.exe'
    probe = Path(__file__).resolve().parent / 'tests/p8_copy_presentation.c'
    original_source = source.read_bytes()
    original_exe = executable.read_bytes() if executable.exists() else None
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    (out / 'original-ddraw7.c').write_bytes(original_source)
    if original_exe is not None:
        (out / 'original-ddraw_test.exe').write_bytes(original_exe)
    record = dict(complete=False, passed=False, started_at=datetime.now(timezone.utc).isoformat(),
        builder=identity(Path(__file__)), probe=identity(probe), source_before=identity(source),
        executable_before=identity(executable) if original_exe is not None else None,
        command=['make', '-j4', 'dlls/ddraw/tests/ddraw_test.exe'])
    try:
        source.write_bytes(probe.read_bytes())
        with (out / 'build.log').open('xb') as log:
            record['exit_code'] = subprocess.run(record['command'], cwd=build,
                env=dict(os.environ, SOURCE_DATE_EPOCH='1768319767'),
                stdout=log, stderr=subprocess.STDOUT).returncode
        if record['exit_code'] != 0:
            raise RuntimeError('Wine probe compilation failed; see build.log')
        raw = executable.read_bytes()
        pe = int.from_bytes(raw[0x3c:0x40], 'little')
        if raw[:2] != b'MZ' or raw[pe:pe+4] != b'PE\0\0' or int.from_bytes(raw[pe+4:pe+6], 'little') != 0x14c:
            raise RuntimeError('Expected a Win32 x86 test executable')
        target = out / 'P8CopyPresentation.exe'
        target.write_bytes(raw)
        record.update(passed=True, executable=identity(target))
    finally:
        source.write_bytes(original_source)
        if original_exe is None:
            executable.unlink(missing_ok=True)
        else:
            executable.write_bytes(original_exe)
        restored = source.read_bytes() == original_source
        restored &= executable.read_bytes() == original_exe if original_exe is not None else not executable.exists()
        record.update(complete=True, restored=bool(restored), finished_at=datetime.now(timezone.utc).isoformat())
        (out / 'build.json').write_text(json.dumps(record, indent=2) + '\n', encoding='utf-8')
    print(json.dumps({key: record[key] for key in ('passed', 'restored', 'executable')}))
    return 0 if record['passed'] and record['restored'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
