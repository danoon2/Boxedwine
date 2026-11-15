#!/usr/bin/env python3
"""Compile the actual Wine integer-exception decoder with bounded memory stubs."""
import argparse
from pathlib import Path
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument('--wine-source', type=Path, required=True)
args = parser.parse_args()
source = (args.wine_source / 'dlls/ntdll/unix/signal_i386.c').read_text()
start = source.index('static DWORD get_integer_divide_code(')
end = source.index('static void fpe_handler(', start)
with tempfile.TemporaryDirectory(prefix='wine-integer-exceptions-') as work:
    work = Path(work)
    (work / 'integer_exception_impl.h').write_text(source[start:end])
    subprocess.run(['gcc', '-m32', '-std=c99', '-Wall', '-Wextra', '-Werror',
                    '-I', str(work), str(Path(__file__).with_name('integer-exception-decode.c')),
                    '-o', str(work / 'test')], check=True)
    subprocess.run([str(work / 'test')], check=True)
