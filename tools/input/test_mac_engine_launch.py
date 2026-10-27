#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team. GPL-2.0-or-later.
"""Test the real NSWorkspace transport between separately sandboxed apps.

Requires a logged-in macOS 15+ desktop and the project's SDL dependency.
--picker adds an interactive disc selection check. Generated files stay in tmp.
--no-document-handler reproduces the unavailable document claim of a translocated
download. --legacy-document checks compatibility with version 1 engines.
"""
import argparse
import concurrent.futures
import os
from pathlib import Path
import plistlib
import re
import subprocess
import time


def run(*command, **kwargs):
    return subprocess.run([str(p) for p in command], check=True, **kwargs)


root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--picker', action='store_true')
transport = parser.add_mutually_exclusive_group()
transport.add_argument('--no-document-handler', action='store_true')
transport.add_argument('--legacy-document', action='store_true')
options = parser.parse_args()
source = root / 'tools/input/mac_engine_launch'
native = root / 'project/mac-xcode/Boxedwine/BoxedwineUI'
variant = '-no-document-handler' if options.no_document_handler else '-legacy' if options.legacy_document else ''
output = root / ('tmp/mac-engine-launch' + variant)
app = output / 'Launcher.app'
engine = app / 'Contents/Helpers/Engine.app'
for bundle in [app, engine]:
    (bundle / 'Contents/MacOS').mkdir(parents=True, exist_ok=True)
    info = plistlib.loads((native / 'Engine-Info.plist').read_bytes()) if bundle == engine else {}
    if bundle == engine and options.no_document_handler:
        info.pop('CFBundleDocumentTypes')
    if bundle == engine and options.legacy_document:
        info['BoxedwineLaunchProtocol'] = 1
    name = 'Engine' if bundle == engine else 'Launcher'
    info.update(CFBundleExecutable=name, CFBundleIdentifier='org.boxedwine.launchtest.' + name.lower() + variant,
                CFBundleName='Launch Test ' + name, CFBundleVersion='1',
                CFBundlePackageType='APPL', LSMinimumSystemVersion='15.0')
    (bundle / 'Contents/Info.plist').write_bytes(plistlib.dumps(info))
run('ditto', root / 'lib/mac/SDL2.framework', app / 'Contents/Frameworks/SDL2.framework')
run('codesign', '--force', '--sign', '-', app / 'Contents/Frameworks/SDL2.framework')
run('xcrun', 'clang', '-fobjc-arc', '-mmacosx-version-min=15.0',
    '-DBOXEDWINE_NATIVE_RUNTIME', '-DBOXEDWINE_DISABLE_UI', '-c',
    root / 'project/mac-xcode/Boxedwine/Boxedwine/MacPlatform.m', '-o', output / 'MacPlatform.o')
run('xcrun', 'clang++', '-std=c++17', '-fobjc-arc', '-mmacosx-version-min=15.0',
    '-I' + str(root / 'platform/mac'), '-I' + str(root / 'lib/mac/SDL2.framework/Headers'),
    '-F' + str(root / 'lib/mac'), '-framework', 'Cocoa', '-framework', 'UniformTypeIdentifiers',
    '-framework', 'SDL2', '-Wl,-rpath,@executable_path/../../../../Frameworks',
    source / 'engine.mm', root / 'platform/mac/macLaunch.mm', root / 'platform/mac/macStatusBar.mm',
    output / 'MacPlatform.o',
    '-o', engine / 'Contents/MacOS/Engine')
run('swift', 'build', '--package-path', native)
binary = Path(subprocess.check_output(['swift', 'build', '--package-path', str(native), '--show-bin-path'], text=True).strip())
run('xcrun', 'swiftc', '-swift-version', '6', '-parse-as-library', '-I', binary,
    '-I', native / 'ZipSupport/include', source / 'launcher.swift', binary / 'libBoxedwineLibrary.a',
    binary / 'CBoxedwineZIP.o', '-lz', '-o', app / 'Contents/MacOS/Launcher')
for bundle, entitlement in [(engine, 'Boxedwine.entitlements'), (app, 'BoxedwineUI.entitlements')]:
    run('codesign', '--force', '--sign', '-', '--options', 'runtime', '--entitlements', native / entitlement, bundle)
if options.no_document_handler:
    # Register the fixture before the sandboxed launcher queries its claim.
    # Otherwise LSCanURLAcceptURL may just report kLSNotRegisteredErr.
    run('/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister',
        '-f', engine)


def probe(mode):
    arguments = [str(app / 'Contents/MacOS/Launcher'), mode]
    if options.no_document_handler:
        arguments.append('no-document-handler')
    result = subprocess.run(arguments, capture_output=True, text=True,
                            timeout=180 if mode == 'picker' else 30)
    (output / (mode + '.log')).write_text(result.stdout + result.stderr)
    result.check_returncode()
    if options.no_document_handler:
        assert 'DOCUMENT_HANDLER_REJECTED' in result.stdout, result.stdout
    if mode == 'orphan':
        pid = int(re.search(r'ENGINE_PID (\d+)', result.stdout).group(1))
        for _ in range(50):
            try:
                os.kill(pid, 0)
            except ProcessLookupError:
                break
            time.sleep(.1)
        else:
            raise AssertionError('Engine survived its launcher: ' + str(pid))
    print('PASS', mode)


probe('success')  # Literal arguments, environment, scoped file access, logs, exit status.
probe('activation')  # Visible apps get Dock/menu integration; background jobs stay hidden.
probe('icon')  # Icon bytes survive the document, environment, decode, and activation.
# Concurrent launches must remain distinct even though the bundle ID is identical.
with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
    list(pool.map(probe, ['stop', 'force']))
probe('orphan')
if options.picker:
    probe('picker')
