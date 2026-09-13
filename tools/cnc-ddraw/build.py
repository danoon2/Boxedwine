#!/usr/bin/env python3
"""Build the pinned, patched CNC DDraw source with an LLVM-MinGW toolchain."""
import argparse
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source', type=Path, required=True)
parser.add_argument('--toolchain', type=Path, required=True, help='LLVM-MinGW bin directory')
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()
source, toolchain, output = (p.resolve() for p in (args.source, args.toolchain, args.output))
revision = subprocess.check_output(['git', '-C', str(source), 'rev-parse', 'HEAD'], text=True).strip()
if revision != 'a902db06e9830a9feafda69da05c766a81722b9b':
    parser.error('Expected CNC DDraw 6.9 source revision a902db06e9830a9feafda69da05c766a81722b9b')
if 'real_GetProcAddress(g_oglu_hmodule, "glGetIntegerv")' not in (source / 'src/opengl_utils.c').read_text():
    parser.error('Apply patches/0001-load-glGetIntegerv-from-opengl32.patch first')
output.mkdir(parents=True, exist_ok=True)
objects = output / 'objects'
objects.mkdir(exist_ok=True)
(source / 'inc/git.h').write_text('#define GIT_COMMIT "a902db0-boxedwine1"\n#define GIT_BRANCH "master"\n')
cc = str(toolchain / 'i686-w64-mingw32-clang')
compiled = []
for item in sorted((source / 'src').rglob('*.c')):
    # Match upstream's src/*.c and src/*/*.c build inputs.
    if len(item.relative_to(source / 'src').parts) > 2:
        continue
    obj = objects / ('_'.join(item.relative_to(source).parts) + '.o')
    subprocess.run([cc, '-I' + str(source / 'inc'), '-O2', '-march=i486', '-Wall', '-std=c99',
                    '-c', str(item), '-o', str(obj)], check=True)
    compiled.append(str(obj))
resource = objects / 'res.o'
subprocess.run([str(toolchain / 'i686-w64-mingw32-windres'), '-J', 'rc', 'res.rc', str(resource)],
               cwd=source, check=True)
subprocess.run([cc, '-Wl,--enable-stdcall-fixup', '-s', '-static', '-shared', '-o', str(output / 'ddraw.dll'),
                *compiled, str(resource), str(source / 'exports.def'), '-lgdi32', '-lwinmm', '-ldbghelp', '-lole32'], check=True)
print(output / 'ddraw.dll')
