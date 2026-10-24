#!/usr/bin/env python3
"""Run after project/mac-xcode/buildNative.sh in a logged-in macOS session.

Reuse the actual engine objects/settings, replacing only its entry point.
All generated files stay in tmp; the built app is not modified.
"""
from pathlib import Path
import shlex
import subprocess
import sys

def run(command, **kwargs):
    result = subprocess.run(command, **kwargs)
    if result.returncode:
        sys.exit(result.returncode)


root = Path(__file__).resolve().parents[2]
objects = root / 'project/mac-xcode/bin/native-build/Build/Intermediates.noindex/Boxedwine.build/Debug/Boxedwine.build/Objects-normal/arm64'
output = root / 'tmp/cdrom-statusbar-mac'
output.mkdir(parents=True, exist_ok=True)
settings = next(p for p in objects.glob('*-common-args.resp') if '-std=c++20' in p.read_text() and '-fobjc-arc' not in p.read_text())
flags = shlex.split(settings.read_text())
sdk = subprocess.check_output(['xcrun', '--show-sdk-path'], text=True).strip()
compiler = ['xcrun', 'clang++', *flags, '-isysroot', sdk, '-target', 'arm64-apple-macos15.0', '-I' + str(root / 'include'), '-I' + str(root / 'platform/sdl')]
for source, name, extra in [
    ('platform/sdl/knativesystem.cpp', 'system.o', ['-Dmain=boxedwineHarnessOriginalMain']),
    ('tools/input/test_cdrom_statusbar.cpp', 'test.o', ['-UNDEBUG']),
    ('tools/input/test_cdrom_statusbar_mac.mm', 'cocoa.o', ['-UNDEBUG', '-fobjc-arc']),
]:
    run([*compiler, *extra, '-c', str(root / source), '-o', str(output / name)])
files = [line for line in (objects / 'Boxedwine.LinkFileList').read_text().splitlines() if not line.endswith('/knativesystem.o')]
frameworks = ['OpenGL', 'Carbon', 'CoreMIDI', 'ForceFeedback', 'AudioToolbox', 'CoreAudio', 'SDL2', 'Cocoa', 'UniformTypeIdentifiers']
command = ['xcrun', 'clang++', '-target', 'arm64-apple-macos15.0', '-isysroot', sdk, *files,
           *(str(output / name) for name in ['system.o', 'test.o', 'cocoa.o']),
           '-F' + str(root / 'lib/mac'), '-L' + str(root / 'lib/mac/vulkan/lib'), '-lMoltenVK', '-lcurl.4',
           '-Wl,-rpath,' + str(root / 'lib/mac'), '-Wl,-rpath,' + str(root / 'lib/mac/vulkan/lib'), '-fobjc-arc']
for framework in frameworks:
    command += ['-framework', framework]
run([*command, '-o', str(output / 'test-cdrom-statusbar')])
run(['codesign', '--force', '--sign', '-', '--entitlements', str(root / 'project/mac-xcode/Boxedwine/BoxedwineUI/Boxedwine-Development.entitlements'), str(output / 'test-cdrom-statusbar')])
run([str(output / 'test-cdrom-statusbar')], cwd=root / 'project/mac-xcode')
