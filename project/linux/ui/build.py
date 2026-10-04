#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Stage a relocatable native frontend; optionally install beneath a prefix."""
import argparse
import importlib.util
import os
from pathlib import Path
import shutil
import sys

HERE = Path(__file__).resolve().parent
LINUX = HERE.parent
REPO = LINUX.parent.parent


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--console', type=Path, help='Include the standalone build as CommandLine/boxedwine')
    parser.add_argument('--prefix', type=Path, help='Also install into PREFIX (e.g. ~/.local or /usr)')
    parser.add_argument('--destdir', type=Path, default=Path('/'), help='Packaging root for --prefix')
    options = parser.parse_args()
    output = LINUX / 'Build/NativeUI'
    engine = LINUX / 'Build/Native/boxedwine-engine'
    if not engine.is_file() or not os.access(engine, os.X_OK):
        parser.error('Build the emulator first: make native-runtime')
    if options.console and (not options.console.is_file() or not os.access(options.console, os.X_OK)):
        parser.error('--console must point to an executable standalone Boxedwine build')
    if output.exists():
        if output.is_symlink():
            parser.error('Refusing to replace a linked build directory')
        shutil.rmtree(output)
    (output / 'ui').mkdir(parents=True)
    shutil.copytree(HERE / 'boxedwine', output / 'ui/boxedwine', ignore=shutil.ignore_patterns('__pycache__', '*.pyc'))
    shutil.copy2(LINUX / 'boxedwine-ui', output / 'boxedwine-ui')
    (output / 'boxedwine-ui').chmod(0o755)
    (output / 'Runtime').mkdir()
    shutil.copy2(engine, output / 'Runtime/boxedwine-engine')
    if options.console:
        (output / 'CommandLine').mkdir()
        shutil.copy2(options.console, output / 'CommandLine/boxedwine')
    resources = output / 'ui/resources'
    shared = REPO / 'project/mac-xcode/Boxedwine/BoxedwineUI/Resources'
    (resources / 'WindowsSupport').mkdir(parents=True)
    shutil.copy2(shared / 'WindowsSupport/packages.json', resources / 'WindowsSupport/packages.json')
    shutil.copytree(shared / 'AppIcons', resources / 'AppIcons')
    shutil.copy2(REPO / 'resources/demo-catalog.lock.json', resources)
    shutil.copy2(REPO / 'tools/demo_catalog.py', resources)
    shutil.copy2(REPO / 'license.txt', resources / 'LICENSE')
    icon = REPO / 'project/mac-xcode/Boxedwine/Media.xcassets/AppIcon.appiconset/icon_256x256.png'
    shutil.copy2(icon, resources / 'org.boxedwine.Boxedwine.png')
    spec = importlib.util.spec_from_file_location('catalog', REPO / 'tools/demo_catalog.py')
    catalog = importlib.util.module_from_spec(spec); spec.loader.exec_module(catalog)
    pin = catalog.read_pin(REPO / 'resources/demo-catalog.lock.json')
    cached = catalog.default_cache() / (pin['sha256'] + '.zip')
    if cached.is_file():
        catalog.read_zip(cached, pin)
        shutil.copy2(cached, resources / 'catalog.zip')
    else:
        print('Demo catalog is not cached; the UI will offer to download the pinned catalog.')
    shutil.copy2(HERE / 'org.boxedwine.Boxedwine.desktop', output)
    shutil.copy2(HERE / 'README.md', output)
    print('Staged ' + str(output / 'boxedwine-ui'))
    if options.prefix:
        prefix = options.prefix.expanduser().absolute()
        destination = options.destdir.absolute() / str(prefix).lstrip('/')
        install = destination / 'lib/boxedwine'
        install.mkdir(parents=True, exist_ok=True)
        # Replace only files owned by this package; do not remove the prefix.
        shutil.copytree(output, install, dirs_exist_ok=True)
        binary = destination / 'bin/boxedwine-ui'
        binary.parent.mkdir(parents=True, exist_ok=True)
        import shlex
        binary.write_text('#!/bin/sh\nexec ' + shlex.quote(str(prefix / 'lib/boxedwine/boxedwine-ui')) + ' "$@"\n')
        binary.chmod(0o755)
        desktop = destination / 'share/applications'
        desktop.mkdir(parents=True, exist_ok=True)
        shutil.copy2(HERE / 'org.boxedwine.Boxedwine.desktop', desktop)
        icons = destination / 'share/icons/hicolor/256x256/apps'
        icons.mkdir(parents=True, exist_ok=True)
        shutil.copy2(icon, icons / 'org.boxedwine.Boxedwine.png')
        print('Installed to ' + str(destination))
    return 0


if __name__ == '__main__':
    sys.exit(main())
