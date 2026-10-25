# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Ordered host disc images, stored using the shared library metadata format."""
from pathlib import Path, PureWindowsPath
from .files import beneath, validate_relative


def archive_paths(value):
    paths = (value or '').replace('\r\n', '\n').split('\n')
    if not 1 <= len(paths) <= 256 or len({p.casefold() for p in paths}) != len(paths):
        raise ValueError('CDROMImages needs 1–256 distinct ISO paths, one per line.')
    for path in paths:
        validate_relative(path)
        if (len(path.encode('utf-8')) > 4096 or ':' in path or
                any(127 <= ord(c) <= 159 for c in path) or not path.lower().endswith('.iso')):
            raise ValueError('CDROMImages must name ISO files inside the ZIP.')
    return paths


def has_installer(app):
    return bool(app.get('installer') or app.get('installFromDisc'))


def validate_images(paths):
    if (not isinstance(paths, list) or len(paths) > 256 or
            any(not isinstance(path, str) or not path.strip() or len(path) > 8192 or
                any(c in path for c in '\0\r\n') or
                not (Path(path).is_absolute() or PureWindowsPath(path).is_absolute() or path.startswith('Discs/')) for path in paths)):
        raise ValueError('Choose up to 256 CD-ROM images using full paths or app-owned Discs/ paths.')
    for path in paths:
        if path.startswith('Discs/'):
            archive_paths(path)
    # Retain unavailable files (including paths imported from another host) so
    # the user can open settings and replace them before launching.


def launch_options(paths, root):
    validate_images(paths)
    paths = [str(beneath(root.parent, path)) if path.startswith('Discs/') else path for path in paths]
    if not paths:
        return []
    for path in paths:
        if not Path(path).is_absolute() or not Path(path).is_file():
            raise ValueError('CD-ROM image is unavailable: ' + path +
                             '\nChoose or remove it in App Settings → General.')
    devices = root / 'home/username/.wine/dosdevices'
    for letter in 'efghijklmnopqrstuvwxyz':
        # Include native symlinks (even dangling ones) and Boxedwine .link files.
        names = [letter + suffix for suffix in (':', '::', '(_colon_)', '(_colon_)(_colon_)')]
        if any((devices / name).exists() or (devices / name).is_symlink()
               for name in names + [name + '.link' for name in names]):
            continue
        return [value for path in paths for value in ('-cdrom', path, letter)]
    raise ValueError('No free CD-ROM drive letter is available from E: to Z:.')
