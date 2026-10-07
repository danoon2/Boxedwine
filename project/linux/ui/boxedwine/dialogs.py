# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Native libadwaita dialogs. All callbacks run on GTK's main thread."""
import base64
import copy
from pathlib import Path
import re
from gi.repository import Adw, Gdk, GdkPixbuf, Gio, GLib, Gtk
from .files import beneath, no_links
from .library import DRIVE_C, WINDOWS, RENDERERS, built_in, preference, resolution
from .runtime import OPTIONS_HELP, WINE_TOOLS, lines, validate_overrides


def guarded(owner, callback):
    def invoke(*args):
        try:
            return callback(*args)
        except Exception as error:
            owner.error(error)
    return invoke


def button(owner, label, callback, style=None, icon=None):
    widget = Gtk.Button(label=label) if label else Gtk.Button(icon_name=icon)
    if label and icon:
        box = Gtk.Box(spacing=8)
        box.append(Gtk.Image(icon_name=icon)); box.append(Gtk.Label(label=label))
        widget.set_child(box)
    if style:
        widget.add_css_class(style)
    widget.set_valign(Gtk.Align.CENTER)
    widget.connect('clicked', guarded(owner, lambda *_: callback()))
    return widget


def row(group, title, subtitle=None):
    result = Adw.ActionRow(title=title, subtitle=subtitle or '', use_markup=False)
    result.set_subtitle_lines(3)
    group.add(result)
    return result


def action_row(owner, group, title, subtitle, label, callback, style=None):
    result = row(group, title, subtitle)
    action = button(owner, label, callback, style)
    result.add_suffix(action)
    result.set_activatable_widget(action)
    return result


def combo(group, title, values, selected, subtitle=None):
    keys = list(values)
    result = Adw.ComboRow(title=title, subtitle=subtitle or '', use_markup=False)
    result.set_model(Gtk.StringList.new(list(values.values())))
    result.set_selected(keys.index(selected) if selected in keys else 0)
    group.add(result)
    return result, lambda: keys[result.get_selected()]


def editable_combo(group, title, values, selected):
    result = Adw.EntryRow(title=title, text=selected)
    menu = Gio.Menu()
    action = Gio.SimpleAction.new_stateful('choose', GLib.VariantType.new('s'), GLib.Variant('s', selected))
    action.connect('activate', lambda _, value: result.set_text(value.get_string()))
    result.connect('notify::text', lambda *_: action.set_state(GLib.Variant('s', result.get_text())))
    actions = Gio.SimpleActionGroup(); actions.add_action(action)
    result.insert_action_group('presets', actions)
    for value in values:
        item = Gio.MenuItem.new(value, None)
        item.set_action_and_target_value('presets.choose', GLib.Variant('s', value))
        menu.append_item(item)
    dropdown = Gtk.MenuButton(icon_name='pan-down-symbolic', menu_model=menu,
                              valign=Gtk.Align.CENTER, tooltip_text='Choose a preset')
    dropdown.add_css_class('flat')
    result.add_suffix(dropdown)
    group.add(result)
    return result


def page(dialog, title, icon):
    result = Adw.PreferencesPage(title=title, icon_name=icon, name=title)
    dialog.add(result)
    return result


def group(parent, title, description=None):
    result = Adw.PreferencesGroup(title=title, description=description or '')
    parent.add(result)
    return result


def text_editor(parent, title, text, description):
    section = group(parent, title, description)
    editor = Gtk.TextView(wrap_mode=Gtk.WrapMode.WORD_CHAR, monospace=True,
                          top_margin=10, bottom_margin=10, left_margin=12, right_margin=12)
    editor.get_buffer().set_text(text)
    scroll = Gtk.ScrolledWindow(min_content_height=130, max_content_height=180, propagate_natural_height=True)
    scroll.set_child(editor); scroll.add_css_class('card'); section.add(scroll)
    def value():
        buffer = editor.get_buffer()
        return buffer.get_text(buffer.get_start_iter(), buffer.get_end_iter(), False)
    return editor, value


def choose_file(owner, title, callback, folder=False, save=False, name=None, patterns=None):
    chooser = Gtk.FileDialog(title=title, modal=True)
    if name:
        chooser.set_initial_name(name)
    if patterns:
        filters = Gio.ListStore.new(Gtk.FileFilter)
        file_filter = Gtk.FileFilter(name=title)
        for pattern in patterns:
            file_filter.add_pattern(pattern)
        filters.append(file_filter)
        chooser.set_filters(filters)
    method = 'select_folder' if folder else 'save' if save else 'open'
    def finished(dialog, result):
        try:
            file = getattr(dialog, method + '_finish')(result)
            path = file.get_path()
            if not path:
                raise ValueError('Choose a local file or folder.')
            callback(Path(path))
        except GLib.Error as error:
            if not error.matches(Gtk.dialog_error_quark(), Gtk.DialogError.DISMISSED):
                owner.error(error)
        except Exception as error:
            owner.error(error)
    getattr(chooser, method)(owner, None, finished)


def confirm(owner, heading, body, label, callback, destructive=False):
    dialog = Adw.AlertDialog(heading=heading, body=body)
    dialog.add_response('cancel', 'Cancel'); dialog.add_response('confirm', label)
    dialog.set_default_response('cancel' if destructive else 'confirm')
    dialog.set_close_response('cancel')
    dialog.set_response_appearance('confirm', Adw.ResponseAppearance.DESTRUCTIVE if destructive else Adw.ResponseAppearance.SUGGESTED)
    dialog.connect('response', guarded(owner, lambda _, response: callback() if response == 'confirm' else None))
    dialog.present(owner)
    return dialog


def text_dialog(owner, title, text, monospace=False):
    dialog = Adw.Dialog(title=title, content_width=680, content_height=540)
    toolbar = Adw.ToolbarView(); toolbar.add_top_bar(Adw.HeaderBar())
    view = Gtk.TextView(editable=False, cursor_visible=False, wrap_mode=Gtk.WrapMode.WORD_CHAR, monospace=monospace,
                        top_margin=20, bottom_margin=20, left_margin=24, right_margin=24)
    view.get_buffer().set_text(text)
    scroll = Gtk.ScrolledWindow(); scroll.set_child(view)
    toolbar.set_content(scroll); dialog.set_child(toolbar); dialog.present(owner)
    return dialog


def program_picker(owner, app, callback, title='Choose Program', external=False):
    def show(programs):
        dialog = Adw.Dialog(title=title, content_width=700, content_height=570)
        toolbar = Adw.ToolbarView(); toolbar.add_top_bar(Adw.HeaderBar())
        content = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=12, margin_start=18, margin_end=18, margin_bottom=18)
        search = Gtk.SearchEntry(placeholder_text='Search programs', hexpand=True)
        content.append(search)
        listing = Gtk.ListBox(selection_mode=Gtk.SelectionMode.NONE); listing.add_css_class('boxed-list')
        choices = []
        for path in programs:
            item = Adw.ActionRow(title=Path(path).stem, subtitle='C:/' + path[len(DRIVE_C) + 1:], use_markup=False, activatable=True)
            item.set_subtitle_lines(2)
            item.add_suffix(Gtk.Image(icon_name='go-next-symbolic'))
            item.connect('activated', guarded(owner, lambda _, p=path: (dialog.close(), callback(p))))
            listing.append(item); choices.append((item, path.casefold()))
        search.connect('search-changed', lambda entry: [item.set_visible(entry.get_text().casefold() in path) for item, path in choices])
        scroll = Gtk.ScrolledWindow(vexpand=True); scroll.set_child(listing); content.append(scroll)
        if external:
            sections = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=18)
            tool_group = Adw.PreferencesGroup(title='Wine Tools', description='Use this app’s Windows environment and registry.')
            tool_list = Gtk.ListBox(selection_mode=Gtk.SelectionMode.NONE); tool_list.add_css_class('boxed-list')
            for key, (name, description, _) in WINE_TOOLS.items():
                item = Adw.ActionRow(title=name, subtitle=description, use_markup=False, activatable=True)
                item.set_subtitle_lines(2); item.add_suffix(Gtk.Image(icon_name='go-next-symbolic'))
                item.connect('activated', guarded(owner, lambda _, key=key: (dialog.close(), owner.launch(app, tool=key))))
                tool_list.append(item); choices.append((item, (name + ' ' + key + ' ' + description).casefold()))
            tool_group.add(tool_list)
            scroll.set_child(None)
            installed = Adw.PreferencesGroup(title='Installed Programs')
            if programs:
                installed.add(listing)
            else:
                installed.set_description('No installed programs were found. You can still use Wine tools or choose an external program.')
            sections.append(installed); sections.append(tool_group); scroll.set_child(sections)
        elif not programs:
            empty = Adw.StatusPage(title='No Programs Found', description='Run the installer first, then choose the program it installed.', icon_name='application-x-executable-symbolic')
            scroll.set_child(empty)
        if external:
            def pick_external():
                dialog.close()
                choose_file(owner, 'Run a Windows Program', lambda path: owner.run_external(app, path), patterns=['*.exe', '*.EXE', '*.msi', '*.MSI'])
            content.append(button(owner, 'Choose External Program…', pick_external))
        toolbar.set_content(content); dialog.set_child(toolbar); dialog.present(owner)
    owner.perform('Finding programs', lambda work: owner.library.programs(app), show)


def app_settings(owner, app):
    if not owner.can_modify(app):
        return
    if app.get('wineConfigurationRefreshPending'):
        owner.read_back_configuration(app, lambda updated: app_settings(owner, updated))
        return
    saved = copy.deepcopy(app)
    dialog = Adw.PreferencesDialog(title='App Settings', content_width=660, content_height=680)
    general = page(dialog, 'General', 'preferences-system-symbolic')
    identity_group = group(general, 'App')
    name = Adw.EntryRow(title='Name', text=saved['name']); identity_group.add(name)
    program_row = None
    if not built_in(saved):
        def select_program():
            def selected(path):
                saved['executable'] = path
                program_row.set_subtitle('C:/' + path[len(DRIVE_C) + 1:])
            program_picker(owner, saved, selected)
        program_row = action_row(owner, identity_group, 'Program', 'C:/' + saved['executable'][len(DRIVE_C) + 1:] if saved.get('executable') else 'Choose after installation', 'Choose…', select_program)
    display = group(general, 'Display')
    size = editable_combo(display, 'Window size', ['640x480', '800x600', '1024x768', '1280x720', '1920x1080'],
                          saved.get('resolution', '1024x768'))
    fullscreen = Adw.SwitchRow(title='Open in Full Screen', active=saved.get('fullScreen', False)); display.add(fullscreen)
    compatibility = group(general, 'Windows Compatibility', 'Changes apply before the next app or installer launch.')
    row(compatibility, 'Windows support', 'Wine ' + (saved.get('savedWineVersion') or 'default') + ' · Pinned for this app')
    def configuration_choice(group, title, choices, field, subtitle=None):
        custom = saved.get('wineConfigurationCustom', {}).get(field)
        if custom:
            choices = {**choices, 'custom': 'Custom (set in Wine)'}
            subtitle = custom
        return combo(group, title, choices, 'custom' if custom else preference(saved, field), subtitle)[1]
    windows = configuration_choice(compatibility, 'Windows version', WINDOWS, 'windowsVersion')
    advanced = page(dialog, 'Advanced', 'applications-engineering-symbolic')
    icon_group = group(advanced, 'App Icon')
    def set_icon(path):
        if no_links(path).stat().st_size > 32 * 1024**2:
            raise ValueError('Choose an image smaller than 32 MB.')
        pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_scale(str(path), 256, 256, True)
        success, data = pixbuf.save_to_bufferv('png', [], [])
        if not success:
            raise ValueError('Could not read this image.')
        saved['customIconPNG'] = base64.b64encode(data).decode()
        icon_row.set_subtitle(path.name)
    icon_row = action_row(owner, icon_group, 'Custom icon', 'Use a PNG, JPEG, or other supported image', 'Choose…',
                          lambda: choose_file(owner, 'Choose App Icon', set_icon))
    action_row(owner, icon_group, 'Default icon', '', 'Reset', lambda: (saved.pop('customIconPNG', None), icon_row.set_subtitle('Using the default icon')))
    graphics = group(advanced, 'Wine Graphics', 'Compatibility settings for older DirectDraw and Direct3D programs.')
    renderer = configuration_choice(graphics, 'Wine renderer', RENDERERS, 'wineRenderer')
    backend = configuration_choice(graphics, 'Wine display interface', {'wineDefault': 'Use Wine’s default', 'glx': 'GLX', 'egl': 'EGL'}, 'openGLBackend', 'Controls how Wine connects to the Linux OpenGL driver.')
    _, arguments = text_editor(advanced, 'App Arguments', '\n'.join(saved.get('arguments', [])), 'One argument per line. Spaces are preserved; do not add shell quotes.')
    _, boxed_arguments = text_editor(advanced, 'Boxedwine Arguments', '\n'.join(saved.get('boxedwineArguments') or []), 'Advanced emulator options, one option or value per line.')
    help_group = group(advanced, '')
    action_row(owner, help_group, 'Supported options', '', 'View', lambda: text_dialog(owner, 'Supported Options', OPTIONS_HELP, True))
    actions = group(general, '')
    def save():
        if not owner.can_modify(app):
            return
        saved.update(name=name.get_text().strip(), resolution=size.get_text().strip(), fullScreen=fullscreen.get_active(),
                     arguments=lines(arguments()), boxedwineArguments=lines(boxed_arguments()))
        for field, value in [('windowsVersion', windows()), ('wineRenderer', renderer()), ('openGLBackend', backend())]:
            if value == 'custom':
                continue
            if value != preference(app, field) or field in saved.get('wineConfigurationCustom', {}):
                saved[field] = value; saved[field + 'Pending'] = True
                saved.get('wineConfigurationCustom', {}).pop(field, None)
        if not saved.get('wineConfigurationCustom'):
            saved.pop('wineConfigurationCustom', None)
        owner.library.validate_app(saved)
        owner.library.update(saved)
        dialog.close(); owner.refresh(); owner.toast('App settings saved')
    action_row(owner, actions, 'Save changes', '', 'Save', save, 'suggested-action')
    action_row(owner, group(advanced, ''), 'Save changes', '', 'Save', save, 'suggested-action')
    dialog.present(owner)
    return dialog


def add_app(owner, source=None):
    if not owner.can_modify():
        return
    dialog = Adw.PreferencesDialog(title='Add App', content_width=620, content_height=540)
    general = page(dialog, 'Add App', 'list-add-symbolic')
    source_group = group(general, 'Choose a Source', 'Boxedwine copies files into a separate Windows environment for this app.')
    selected = {'source': source, 'kind': 'folder' if source and source.is_dir() else 'installer', 'installer': None}
    types = {'installer': 'Windows installer (.exe or .msi)', 'installerFolder': 'Folder containing an installer', 'folder': 'Portable app folder'}
    kind_row, kind = combo(source_group, 'Source type', types, selected['kind'])
    def picked(path):
        selected['source'] = path; selected['installer'] = None
        source_row.set_subtitle(str(path)); installer_row.set_subtitle('Select setup.exe or an .msi inside the folder')
        if not name.get_text():
            name.set_text(path.name if path.is_dir() else path.stem)
    source_row = action_row(owner, source_group, 'Source', str(source) if source else 'No source selected', 'Choose…',
                            lambda: choose_file(owner, 'Choose ' + ('Folder' if kind() != 'installer' else 'Installer'), picked, folder=kind() != 'installer',
                                                patterns=None if kind() != 'installer' else ['*.exe', '*.EXE', '*.msi', '*.MSI']))
    def picked_installer(path):
        selected['installer'] = path; installer_row.set_subtitle(str(path))
    installer_row = action_row(owner, source_group, 'Installer', 'Select setup.exe or an .msi inside the folder', 'Choose…',
                               lambda: choose_file(owner, 'Choose Installer', picked_installer, patterns=['*.exe', '*.EXE', '*.msi', '*.MSI']))
    installer_row.set_visible(kind() == 'installerFolder')
    def type_changed(*_):
        selected.update(source=None, installer=None)
        source_row.set_subtitle('No source selected'); installer_row.set_visible(kind() == 'installerFolder')
    kind_row.connect('notify::selected', type_changed)
    details = group(general, 'App Details')
    name = Adw.EntryRow(title='Name', text=(source.name if source.is_dir() else source.stem) if source else ''); details.add(name)
    _, windows = combo(details, 'Windows version', WINDOWS, 'wineDefault')
    actions = group(general, '')
    def start():
        if not selected['source'] or not name.get_text().strip():
            raise ValueError('Choose a source and enter an app name.')
        if kind() == 'installerFolder' and not selected['installer']:
            raise ValueError('Choose the installer inside the selected folder.')
        source_path, source_kind, installer_path, title, version = selected['source'], kind(), selected['installer'], name.get_text().strip(), windows()
        dialog.close()
        owner.with_wine(lambda wine: owner.perform('Adding ' + title,
            lambda work: owner.library.import_app(source_path, source_kind, wine, work, version, installer_path, title), owner.app_added))
    action_row(owner, actions, 'Add to your library', '', 'Add App', start, 'suggested-action')
    dialog.present(owner)
    return dialog


def settings(owner):
    dialog = Adw.PreferencesDialog(title='Settings', content_width=670, content_height=670)
    general = page(dialog, 'General', 'preferences-system-symbolic')
    appearance = group(general, 'Appearance')
    preferences = owner.library.settings()
    theme_row, theme = combo(appearance, 'Style', {'System': 'Follow System', 'Light': 'Light', 'Dark': 'Dark'}, preferences.get('theme', 'System'))
    def save_theme(*_):
        value = owner.library.settings(); value['theme'] = theme(); owner.library.save_settings(value); owner.apply_theme()
    theme_row.connect('notify::selected', guarded(owner, save_theme))
    storage = group(general, 'Library')
    action_row(owner, storage, 'Location', str(owner.library.directory), 'Open', lambda: owner.open_path(owner.library.directory))
    immediate = Adw.SwitchRow(title='Delete apps immediately', subtitle='After confirmation, permanently delete app files instead of keeping them in Removed Apps.', active=preferences.get('deleteImmediately', False)); storage.add(immediate)
    def save_delete(*_):
        value = owner.library.settings(); value['deleteImmediately'] = immediate.get_active(); owner.library.save_settings(value)
    immediate.connect('notify::active', guarded(owner, save_delete))
    action_row(owner, storage, 'Unused Wine packages', 'Reclaim packages no app, recovery operation, or default setting needs.', 'Clean Up',
               lambda: owner.clean_packages())
    support = group(general, 'Windows Support', 'Wine packages are shared between apps. Each app keeps its selected package when the default changes.')
    current = owner.library.default_wine()
    row(support, 'Default Wine version', 'Wine ' + current['wineVersion'] + ' · Filesystem ' + current['filesystemVersion'] if current else 'Not installed')
    for release in owner.resources.wines():
        ref = release['reference']
        action_row(owner, support, 'Wine ' + ref['wineVersion'], f"Filesystem {ref['filesystemVersion']} · {ref['bytes'] / 1024**2:.0f} MB · SHA-256 verified",
                   'Check Again' if current == ref else 'Set Up', lambda r=release: (dialog.close(), owner.setup_wine(r)))
    action_row(owner, support, 'Use a downloaded package', 'Import the matching release ZIP for offline setup.', 'Choose ZIP…',
               lambda: choose_file(owner, 'Choose Wine Package', lambda p: (dialog.close(), owner.import_wine(p)), patterns=['*.zip']))
    dialog.present(owner)
    return dialog


def troubleshoot(owner, app):
    dialog = Adw.PreferencesDialog(title='Troubleshooting', content_width=680, content_height=650)
    main = page(dialog, app['name'], 'dialog-question-symbolic')
    tips = group(main, 'Things to Try', 'Close the app before changing settings. Change one thing at a time, save, then open the app again.')
    for title, text in [
        ('The app or installer will not start', 'Check the selected program and Windows version in App Settings. Some installers need other files beside setup.exe; add the entire installer folder in that case.'),
        ('A black screen or the wrong window size', 'Try windowed mode and 1024x768 in App Settings. Older games may require their own display mode in the game’s options.'),
        ('Missing 2D graphics, broken menus, or wrong colors', 'Try the GDI Wine renderer. Games that require hardware graphics may work better with OpenGL.'),
        ('OpenGL errors or missing 3D graphics', 'Use your distribution’s graphics drivers. Try changing Wine’s display interface between GLX and EGL in Advanced settings.'),
        ('Slow gameplay', 'Lower the game’s resolution and graphics detail. Boxedwine emulates an x86 computer; some programs are too demanding.')]:
        expander = Adw.ExpanderRow(title=title, use_markup=False)
        expander.add_row(Adw.ActionRow(title=text, title_lines=0, use_markup=False)); tips.add(expander)
    install_group = group(main, 'Installation')
    action_row(owner, install_group, 'Selected program', 'Choose the main program after an installer finishes.', 'Choose…', lambda: (dialog.close(), owner.choose_program(app)))
    if app.get('installer'):
        action_row(owner, install_group, 'Saved installer', 'Runs in this app’s existing Windows environment.', 'Run Again', lambda: (dialog.close(), owner.launch(app, installing=True)))
    logs = group(main, 'Launch Logs', 'Logs may contain file paths and app output. Review them before sharing.')
    action_row(owner, logs, 'Launch history', 'Latest, previous, and most recent failed launches', 'View Logs', lambda: owner.show_logs(app))
    action_row(owner, logs, 'App storage', str(owner.library.path(app)), 'Open Folder', lambda: owner.open_path(owner.library.path(app)))
    links = group(main, 'More Help')
    action_row(owner, links, 'Wine AppDB', 'Compatibility reports and suggestions from other Wine users', 'Open', lambda: owner.open_uri('https://appdb.winehq.org/'))
    action_row(owner, links, 'Boxedwine issues', 'Include the app version, Linux distribution, and steps to reproduce.', 'Open', lambda: owner.open_uri('https://github.com/danoon2/Boxedwine/issues'))
    dialog.present(owner)
    return dialog


def logs(owner, app):
    dialog = Adw.PreferencesDialog(title='Launch Logs', content_width=760, content_height=600)
    main = page(dialog, 'Logs', 'text-x-generic-symbolic')
    for name, title in [('latest.log', 'Latest Launch'), ('previous.log', 'Previous Launch'), ('last-failed.log', 'Last Failed Launch'), ('configuration.log', 'Wine Settings Readback')]:
        path = beneath(owner.library.path(app), 'Logs/' + name)
        section = group(main, title)
        if not path.is_file():
            row(section, 'No log yet')
            continue
        def show(p=path, label=title):
            with no_links(p).open('rb') as stream:
                data = stream.read(4 * 1024**2 + 1024)
            text_dialog(owner, label, re.sub(r'\x1b\[[0-?]*[ -/]*[@-~]', '', data.decode('utf-8', errors='replace')), True)
        item = action_row(owner, section, title, f'{path.stat().st_size / 1024:.0f} KB', 'View', show)
        def export(p=path):
            def save(destination):
                if destination.exists():
                    raise ValueError('Choose a new filename for the log.')
                from .files import copy_file
                owner.perform('Exporting log', lambda work: copy_file(p, destination, work), lambda _: owner.toast('Log exported'))
            choose_file(owner, 'Export Log', save, save=True, name=app['name'] + '-' + p.name)
        item.add_suffix(button(owner, 'Export…', export))
    dialog.present(owner)
    return dialog
