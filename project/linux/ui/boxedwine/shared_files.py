# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Per-app Y: storage. Completed copies are published outside the staging folder."""
from pathlib import Path
import ctypes
import os
import uuid
from .files import beneath, copy_file, delete_tree, no_links, within, walk


def directory(library, app):
    path = beneath(library.root(app), 'home/username/Shared')
    path.mkdir(parents=True, exist_ok=True)
    return path


def add(library, app, sources, work):
    target = directory(library, app)
    stage = beneath(library.path(app), 'SharedTransfer-' + uuid.uuid4().hex)
    stage.mkdir()
    try:
        names = {p.name.casefold() for p in target.iterdir()}
        items = []
        for source in sources:
            work.check()
            source = no_links(Path(source).absolute())
            if source == target or within(target, source):
                raise ValueError("Choose files outside this app's Shared Files folder and its parent folders.")
            name, number = source.name, 2
            while name.casefold() in names:
                name = (f'{source.name} ({number})' if source.is_dir()
                        else f'{source.stem} ({number}){source.suffix}')
                number += 1
            names.add(name.casefold())
            staged, destination = beneath(stage, name), beneath(target, name)
            if source.is_dir():
                def failed(error):
                    raise error
                staged.mkdir()
                for entry in walk(source, onerror=failed):
                    work.check()
                    copied = beneath(staged, entry.relative_to(source).as_posix())
                    if entry.is_dir():
                        copied.mkdir(parents=True, exist_ok=True)
                    else:
                        copy_file(entry, copied, work)
            elif source.is_file():
                copy_file(source, staged, work)
            else:
                raise ValueError('Choose regular files or folders.')
            items.append((staged, destination))
        work.check()
        for staged, destination in items:
            no_links(destination)
            # RENAME_NOREPLACE also protects against an external copy racing the
            # publication. Ordinary rename() would silently overwrite a file.
            rename = ctypes.CDLL(None, use_errno=True).renameat2
            rename.argtypes = [ctypes.c_int, ctypes.c_char_p, ctypes.c_int, ctypes.c_char_p, ctypes.c_uint]
            rename.restype = ctypes.c_int
            if rename(-100, os.fsencode(staged), -100, os.fsencode(destination), 1) != 0:
                error = ctypes.get_errno()
                raise OSError(error, os.strerror(error), str(destination))
        return [destination for _, destination in items]
    finally:
        delete_tree(stage, library.path(app))
