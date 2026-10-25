# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Native libadwaita dialogs. All callbacks run on GTK's main thread."""
from pathlib import Path
import re
from gi.repository import Adw, Gdk, Gio, GLib, Gtk
from .files import beneath, no_links
from .library import DRIVE_C, WINDOWS, resolution
from .runtime import WINE_TOOLS, lines


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
    from .app_settings import AppSettingsDialog
    dialog = AppSettingsDialog(owner, app)
    dialog.present(owner)
    return dialog


def add_app(owner, source=None):
    """Purpose → source → content review. Dropped files skip source selection."""
    from .imports import PreparedSource
    def prepare(value, remote=False, collection=False):
        owner.perform('Checking source', lambda work: PreparedSource(value, work, remote, collection), lambda result: review_import(owner, result))
    if source is not None:
        prepare(source)
        return
    dialog = Adw.PreferencesDialog(title='Add to Library', content_width=560, content_height=420)
    choices = group(page(dialog, 'Add to Library', 'list-add-symbolic'), 'What would you like to add?')
    def select(purpose):
        dialog.close()
        if purpose == 'backup':
            owner.restore_backup(); return
        sources = Adw.PreferencesDialog(title='Add Collection' if purpose == 'collection' else 'Add App or Installer', content_width=560, content_height=420)
        section = group(page(sources, 'Source', 'document-open-symbolic'), 'Choose a source',
                        'A collection is a list of apps you can browse; apps are downloaded only when you choose to install them. '
                        'To add the extended Boxedwine demo collection, enter https://boxedwine.org/demos.json below.' if purpose == 'collection' else None)
        error = Gtk.Label(wrap=True, xalign=0, selectable=True, visible=False)
        error.add_css_class('error')
        def show_error(message=''):
            error.set_text('Could not use this source: ' + message if message else '')
            error.set_visible(bool(message))
        def ready():
            section.set_sensitive(True); sources.set_can_close(True)
        def failed(failure):
            from .files import Cancelled
            ready()
            if not isinstance(failure, Cancelled):
                show_error(str(failure))
        def prepared(result):
            ready()
            # Finish dismissing the source sheet before presenting its review.
            sources.connect('closed', lambda *_: review_import(owner, result))
            sources.close()
        def picked(path, remote=False):
            if owner.busy:
                return
            show_error(); section.set_sensitive(False); sources.set_can_close(False)
            owner.perform('Checking source', lambda work: PreparedSource(path, work, remote, purpose == 'collection'), prepared, failed=failed)
        action_row(owner, section, 'Choose file', 'Collection JSON' if purpose == 'collection' else 'ZIP, EXE, or MSI', 'Choose…',
                   lambda: choose_file(owner, 'Choose File', picked, patterns=['*.json'] if purpose == 'collection' else ['*.zip', '*.exe', '*.msi', '*.json', '*.ZIP', '*.EXE', '*.MSI']))
        if purpose != 'collection':
            action_row(owner, section, 'Choose folder', 'An app or setup with supporting files', 'Choose…', lambda: choose_file(owner, 'Choose Folder', picked, folder=True))
        link = Adw.EntryRow(title='File download URL'); section.add(link)
        link.connect('notify::text', lambda *_: show_error())
        section.add(error)
        def remote():
            from .imports import valid_url
            value = link.get_text().strip()
            if not valid_url(value):
                show_error('Enter a complete HTTP or HTTPS file link.'); link.grab_focus(); return
            picked(value, remote=True)
        action_row(owner, section, 'Download from link', 'The file is checked before anything is added or run.', 'Download & Continue', remote)
        action_row(owner, section, 'Choose another purpose', '', 'Back', lambda: (sources.close(), add_app(owner)))
        sources.present(owner)
    action_row(owner, choices, 'App or installer', 'Install an app or add one that is ready to run.', 'Choose…', lambda: select('app'))
    action_row(owner, choices, 'Collection', 'Add a folder of games and apps to browse and install later.', 'Choose…', lambda: select('collection'))
    action_row(owner, choices, 'Restore a backup', 'Restore saved app files and settings.', 'Choose…', lambda: select('backup'))
    dialog.present(owner)
    return dialog


def review_import(owner, prepared):
    dialog = Adw.PreferencesDialog(title='Review Collection' if prepared.collection else 'Review App', content_width=610, content_height=600)
    section = group(page(dialog, 'Review', 'document-open-symbolic'), 'Review before adding', prepared.original)
    retained = {'value': False}
    dialog.connect('closed', lambda *_: None if retained['value'] else prepared.close())
    if prepared.collection:
        incoming = prepared.collection
        destination = Adw.EntryRow(title='Folder name', text=incoming['folder']); section.add(destination)
        names = ['Demos'] + [f['name'] for f in owner.collections.load()['folders'] if f['name'] != 'Demos']
        choices, selected = combo(section, 'Destination', {'': 'Use folder name above', **{n: n for n in names}}, '')
        def folder():
            return destination.get_text().strip() if selected() == '' else selected()
        summary = row(section, 'Import changes')
        def changed(*_):
            try:
                added, updated, unchanged = owner.collections.preview(incoming, folder())
                summary.set_subtitle(f'{added} new · {updated} updated · {unchanged} unchanged. Existing listings not included in this import are kept.')
            except ValueError as error:
                summary.set_subtitle(str(error))
        destination.connect('notify::text', changed); choices.connect('notify::selected', changed); changed()
        row(section, 'Apps download when you install them', ', '.join(a['Name'] for a in incoming['apps'][:8]))
        def add():
            from .imports import valid_url
            name = owner.collections.add(incoming, folder(), source=prepared.original if valid_url(prepared.original) else None)
            dialog.close(); owner.refresh_collections(); owner.navigate(name)
            owner.toast('Collection saved. Available next time you open Boxedwine.')
        action_row(owner, section, 'Save to your library', '', 'Add Collection', add, 'suggested-action')
    else:
        name = Adw.EntryRow(title='App name', text=prepared.name); section.add(name)
        action_row_widget, action = combo(section, 'Add as', {'': 'Choose how to add these files', 'app': 'Ready-to-run app', 'installer': 'Run an installer'}, prepared.suggested_action)
        program_row = Adw.ComboRow(title='Program', use_markup=False); section.add(program_row)
        candidates = []
        def update(*_):
            candidates[:] = [p for p in prepared.programs if action() != 'app' or p.lower().endswith('.exe')]
            program_row.set_model(Gtk.StringList.new(candidates)); program_row.set_selected(0 if len(candidates) == 1 else Gtk.INVALID_LIST_POSITION)
            if action() == 'installer':
                setup = [i for i, p in enumerate(candidates) if Path(p).suffix.lower() == '.msi' or re.match(r'^(setup|install)', Path(p).stem, re.I)]
                if len(setup) == 1: program_row.set_selected(setup[0])
            program_row.set_title('Installer' if action() == 'installer' else 'Program to open')
        action_row_widget.connect('notify::selected', update); update()
        if not prepared.source.is_dir():
            action_row(owner, section, 'Ready-to-run apps may need nearby files', 'Choose the containing folder to include those files.', 'Choose Folder…',
                       lambda: choose_file(owner, 'Choose App Folder', lambda path: (dialog.close(), add_app(owner, path)), folder=True))
        options = Adw.ExpanderRow(title='Compatibility options'); section.add(options)
        windows_row = Adw.ComboRow(title='Windows version', model=Gtk.StringList.new(list(WINDOWS.values()))); options.add_row(windows_row)
        def start():
            index = program_row.get_selected()
            if not action() or not name.get_text().strip() or index >= len(candidates):
                raise ValueError('Choose an action and program, and enter an app name.')
            if action() == 'app' and not prepared.source.is_dir():
                raise ValueError('Choose the app’s containing folder to include its supporting files.')
            selected_program = candidates[index]
            kind = 'folder' if action() == 'app' else 'installerFolder' if prepared.source.is_dir() else 'installer'
            installer = beneath(prepared.source, selected_program) if kind == 'installerFolder' else None
            windows = list(WINDOWS)[windows_row.get_selected()]
            title = name.get_text().strip()
            retained['value'] = True
            dialog.close()
            def imported(work, wine):
                try:
                    return owner.library.import_app(prepared.source, kind, wine, work, windows, installer, title,
                                                    selected_program if kind == 'folder' else None)
                finally:
                    prepared.close()
            owner.with_wine(lambda wine: owner.perform('Adding ' + title, lambda work: imported(work, wine), owner.app_added))
        final = action_row(owner, section, 'Copy files into your library', 'Your original files stay in place.', 'Add App', start, 'suggested-action')
        # The final button describes the action; no program runs while inspecting.
        def action_changed(*_):
            final.get_activatable_widget().set_label('Run Installer' if action() == 'installer' else 'Add App')
        action_row_widget.connect('notify::selected', action_changed); action_changed()
    action_row(owner, section, 'Choose another source', '', 'Back', lambda: (dialog.close(), add_app(owner)))
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
    support = group(general, 'Windows Support', 'New apps use the current release. Existing apps keep their selected Wine package.')
    releases = owner.resources.wines()
    current = releases[0]['reference']
    installed = owner.library.package_path(current).is_file()
    row(support, 'Current release for new apps', f"Wine {current['wineVersion']} · Filesystem {current['filesystemVersion']} · " + ('Installed' if installed else 'Not installed'))
    for release in releases:
        ref = release['reference']
        action_row(owner, support, 'Wine ' + ref['wineVersion'], f"Filesystem {ref['filesystemVersion']} · {ref['bytes'] / 1024**2:.0f} MB · SHA-256 verified",
                   'Check Again' if owner.library.package_path(ref).is_file() else 'Set Up', lambda r=release: (dialog.close(), owner.setup_wine(r)))
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
