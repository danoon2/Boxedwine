# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Window icons and desktop identities for the launcher and its SDL children."""
import base64
import hashlib
import os
from pathlib import Path
import sys
import tempfile
from gi.repository import Gdk, GdkPixbuf, GLib, Gtk
from .files import no_links

ICON = 'org.boxedwine.Boxedwine'


def data_home():
    return Path(os.environ.get('XDG_DATA_HOME', Path.home() / '.local/share')).absolute()


def launcher_id(directory):
    directory = Path(directory).absolute()
    if directory == data_home() / 'boxedwine':
        return ICON
    return ICON + '.Library' + hashlib.sha256(os.fsencode(directory)).hexdigest()[:16]


def write(path, data):
    path = no_links(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(dir=path.parent, delete=False) as stream:
        temporary = Path(stream.name)
        try:
            stream.write(data); stream.close()
            os.replace(temporary, path)
        finally:
            temporary.unlink(missing_ok=True)


def exec_argument(value):
    # Desktop Entry Exec quoting is deliberately different from shell quoting.
    value = str(value).replace('%', '%%')
    return '"' + ''.join('\\' + c if c in '\\"`$' else c for c in value) + '"'


def register(library, identity, name, icon, app_id=None):
    launcher = Path(__file__).resolve().parents[2] / 'boxedwine-ui'
    arguments = [str(launcher), '--library', str(library.directory)]
    if app_id:
        arguments += ['--open', app_id]
    entry = GLib.KeyFile()
    for key, value in {'Type': 'Application', 'Name': name, 'Icon': str(icon),
                       'Exec': ' '.join(map(exec_argument, arguments)),
                       'StartupWMClass': identity, 'X-Boxedwine-Generated': 'true'}.items():
        entry.set_string('Desktop Entry', key, value)
    entry.set_boolean('Desktop Entry', 'NoDisplay', app_id is not None or identity != ICON)
    entry.set_boolean('Desktop Entry', 'Terminal', False)
    path = data_home() / 'applications' / (identity + '.desktop')
    try:
        # Keep a manually installed entry; only refresh our own generated files.
        if path.exists() and 'X-Boxedwine-Generated=true' not in path.read_text():
            return
        write(path, entry.to_data()[0].encode())
    except OSError as error:
        # An unwritable desktop registry must not prevent running Windows apps.
        print('Could not register desktop icon:', error, file=sys.stderr)


def setup_launcher(library, resources, identity):
    icons = library.directory / 'LinuxDesktop/icons'
    icon = icons / 'hicolor/256x256/apps' / (ICON + '.png')
    write(icons / 'hicolor/index.theme', b'[Icon Theme]\nName=Boxedwine\nDirectories=256x256/apps\n\n[256x256/apps]\nSize=256\nType=Fixed\nContext=Applications\n')
    write(icon, resources.path(ICON + '.png').read_bytes())
    Gtk.IconTheme.get_for_display(Gdk.Display.get_default()).add_search_path(str(icons))
    Gtk.Window.set_default_icon_name(ICON)
    register(library, identity, 'Boxedwine', icon)


def app_environment(library, app, image, resources):
    texture = image.get_paintable()
    if isinstance(texture, Gdk.Texture):
        loader = GdkPixbuf.PixbufLoader.new_with_type('png')
        loader.write(texture.save_to_png_bytes().get_data()); loader.close()
        pixbuf = loader.get_pixbuf()
    else:
        pixbuf = GdkPixbuf.Pixbuf.new_from_file(str(resources.path(ICON + '.png')))
    pixbuf = pixbuf.scale_simple(64, 64, GdkPixbuf.InterpType.BILINEAR).add_alpha(False, 0, 0, 0)
    pixels, stride = pixbuf.get_pixels(), pixbuf.get_rowstride()
    bgra = bytearray()
    for y in range(64):
        for x in range(64):
            start = y * stride + x * 4
            r, g, b, a = pixels[start:start + 4]
            bgra.extend((b, g, r, a))
    identity = launcher_id(library.directory) + '.App' + app['id'].replace('-', '')
    icon = library.directory / 'LinuxDesktop/icons' / (identity + '.png')
    write(icon, pixbuf.save_to_bufferv('png', [], [])[1])
    register(library, identity, app['name'], icon, app['id'])
    return {'BOXEDWINE_APP_ICON_BGRA': base64.b64encode(bgra).decode('ascii'),
            'SDL_VIDEO_X11_WMCLASS': identity, 'SDL_VIDEO_WAYLAND_WMCLASS': identity,
            'SDL_APP_ID': identity}
