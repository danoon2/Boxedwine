# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Boxedwine's native GTK 4 / libadwaita Linux application."""
import argparse
import base64
import copy
import os
from pathlib import Path
import sys
import threading
import time

try:
    import gi
    gi.require_version('Gtk', '4.0')
    gi.require_version('Adw', '1')
    from gi.repository import Adw, Gdk, Gio, GLib, Gtk
except (ImportError, ValueError) as error:
    raise SystemExit('Boxedwine needs Python GObject, GTK 4.14+, and libadwaita 1.5+.\n'
                     'Debian/Ubuntu/Mint: sudo apt install python3-gi gir1.2-gtk-4.0 gir1.2-adw-1\n' + str(error))

from . import demos, dialogs, desktop
from .files import Work, Cancelled, beneath, no_links, walk
from .library import Library, DRIVE_C, built_in, now
from .packages import Resources, WineChecks, catalog_files, ensure_wine, import_wine
from .runtime import Session, WINE_TOOLS, apply_configuration, build_arguments, default_emulator, refresh_configuration


def idle(callback, *args):
    def call():
        callback(*args)
        return GLib.SOURCE_REMOVE
    GLib.idle_add(call)


class Window(Adw.ApplicationWindow):
    def __init__(self, application, library, resources, emulator):
        super().__init__(application=application, title='Boxedwine', default_width=1180, default_height=780, width_request=360, height_request=420)
        self.set_icon_name(desktop.ICON)
        self.library, self.resources, self.emulator = library, resources, Path(emulator)
        self.view, self.selected, self.query = 'All Apps', None, ''
        self.busy, self.work, self.progress_dialog = False, None, None
        self.sessions, self.stopping, self.program_queue = {}, set(), []
        self.configuration_queue, self.syncing = [], set()
        self.catalog, self.catalog_data, self.catalog_error = [], {}, None
        self.catalog_loaded = False
        try:
            cached = catalog_files(resources, library, Work(), allow_download=False)
            if cached:
                self.catalog_data = cached
                self.catalog = demos.load_catalog(cached['catalog.xml'])
                self.catalog_loaded = True
        except Exception as error:
            self.catalog_error = str(error)
        self.wine_checks = WineChecks()
        self.card_icons = {}
        self.cards = {}
        self.card_reveal = None
        self.closing = False
        self.apply_theme()
        self.build()
        self.refresh()
        self.connect('close-request', self.close_requested)

    def apply_theme(self):
        scheme = {'System': Adw.ColorScheme.DEFAULT, 'Light': Adw.ColorScheme.FORCE_LIGHT, 'Dark': Adw.ColorScheme.FORCE_DARK}
        Adw.StyleManager.get_default().set_color_scheme(scheme.get(self.library.settings().get('theme'), Adw.ColorScheme.DEFAULT))

    def build(self):
        self.toast_overlay = Adw.ToastOverlay()
        self.set_content(self.toast_overlay)
        self.navigation = Adw.OverlaySplitView(min_sidebar_width=205, max_sidebar_width=240, sidebar_width_fraction=.19)
        self.toast_overlay.set_child(self.navigation)
        sidebar = Adw.ToolbarView()
        sidebar_header = Adw.HeaderBar(show_end_title_buttons=False)
        sidebar_header.set_title_widget(Adw.WindowTitle(title='Boxedwine', subtitle='Windows apps, Linux home'))
        sidebar.add_top_bar(sidebar_header)
        side_content = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=12, margin_start=12, margin_end=12, margin_top=16, margin_bottom=12)
        self.sections = Gtk.ListBox(selection_mode=Gtk.SelectionMode.SINGLE)
        self.sections.add_css_class('navigation-sidebar')
        self.section_rows = {}
        for title, icon in [('All Apps', 'view-grid-symbolic'), ('Recently Opened', 'document-open-recent-symbolic'),
                            ('Demos', 'applications-games-symbolic'), ('Removed Apps', 'user-trash-symbolic'), ('Recovery', 'document-revert-symbolic')]:
            item = Gtk.ListBoxRow()
            box = Gtk.Box(spacing=10, margin_start=10, margin_end=10, margin_top=11, margin_bottom=11)
            box.append(Gtk.Image(icon_name=icon)); box.append(Gtk.Label(label=title, xalign=0))
            item.set_child(box); item.title = title
            self.sections.append(item); self.section_rows[title] = item
        # Focus relocation when an adaptive sidebar collapses can change the
        # selected row. Only activation is a request to navigate.
        self.sections.connect('row-activated', dialogs.guarded(self, self.section_selected))
        side_content.append(self.sections)
        side_content.append(Gtk.Separator(margin_top=4, margin_bottom=4))
        label = Gtk.Label(label='GET STARTED', xalign=0, margin_start=10); label.add_css_class('caption'); label.add_css_class('dim-label'); side_content.append(label)
        for title, program, icon in [('Try Notepad', 'notepad', 'accessories-text-editor-symbolic'), ('Play Minesweeper', 'minesweeper', 'applications-games-symbolic')]:
            action = dialogs.button(self, title, lambda p=program: self.add_builtin(p), 'flat', icon)
            side_content.append(action)
        side_content.append(Gtk.Box(vexpand=True))
        side_content.append(dialogs.button(self, 'Settings', lambda: dialogs.settings(self), 'flat', 'preferences-system-symbolic'))
        side_content.append(dialogs.button(self, 'Help', self.show_help, 'flat', 'help-browser-symbolic'))
        sidebar.set_content(side_content); self.navigation.set_sidebar(sidebar)
        toolbar = Adw.ToolbarView()
        header = Adw.HeaderBar(show_start_title_buttons=False)
        self.sidebar_toggle = dialogs.button(self, None, lambda: self.navigation.set_show_sidebar(not self.navigation.get_show_sidebar()), icon='sidebar-show-symbolic')
        self.sidebar_toggle.set_tooltip_text('Show Sidebar'); self.sidebar_toggle.set_visible(False); header.pack_start(self.sidebar_toggle)
        self.window_title = Adw.WindowTitle(title='All Apps')
        header.set_title_widget(self.window_title)
        header.pack_end(dialogs.button(self, 'Add App', lambda: dialogs.add_app(self), icon='list-add-symbolic'))
        menu = Gio.Menu()
        for label, action in [('Restore Backup…', 'restore-backup'), ('Settings', 'settings'), ('Keyboard Shortcuts', 'shortcuts'), ('About Boxedwine', 'about')]:
            menu.append(label, 'win.' + action)
        menu_button = Gtk.MenuButton(icon_name='open-menu-symbolic', menu_model=menu); header.pack_end(menu_button)
        toolbar.add_top_bar(header)
        self.details_split = Adw.OverlaySplitView(sidebar_position=Gtk.PackType.END, min_sidebar_width=285, max_sidebar_width=330, sidebar_width_fraction=.3, show_sidebar=False)
        toolbar.set_content(self.details_split)
        self.navigation.set_content(toolbar)
        main = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.search = Gtk.SearchEntry(placeholder_text='Search apps', margin_start=24, margin_end=24, margin_top=18, margin_bottom=16)
        self.search.connect('search-changed', dialogs.guarded(self, lambda entry: self.filter(entry.get_text())))
        main.append(self.search)
        self.runtime_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=6, margin_start=24, margin_end=24, margin_bottom=8)
        main.append(self.runtime_box)
        self.scroll = Gtk.ScrolledWindow(vexpand=True, hscrollbar_policy=Gtk.PolicyType.NEVER)
        main.append(self.scroll)
        self.details_split.set_content(main)
        self.details_toolbar = Adw.ToolbarView()
        details_header = Adw.HeaderBar(show_start_title_buttons=False, show_end_title_buttons=False)
        details_header.set_title_widget(Gtk.Label(label='App Details'))
        details_header.pack_end(dialogs.button(self, None, lambda: self.details_split.set_show_sidebar(False), icon='window-close-symbolic'))
        self.details_toolbar.add_top_bar(details_header)
        self.details_split.set_sidebar(self.details_toolbar)
        compact = Adw.Breakpoint.new(Adw.BreakpointCondition.parse('max-width: 1050px'))
        compact.add_setter(self.details_split, 'collapsed', True)
        self.add_breakpoint(compact)
        narrow = Adw.Breakpoint.new(Adw.BreakpointCondition.parse('max-width: 720px'))
        narrow.add_setter(self.navigation, 'collapsed', True)
        narrow.add_setter(self.details_split, 'collapsed', True)
        narrow.add_setter(self.sidebar_toggle, 'visible', True)
        self.add_breakpoint(narrow)
        self.navigation.connect('notify::collapsed', lambda *_: idle(self.restore_navigation_focus))
        drop = Gtk.DropTarget.new(Gio.File, Gdk.DragAction.COPY)
        drop.set_gtypes([Gio.File, Gdk.FileList])
        drop.connect('drop', dialogs.guarded(self, self.drop_file)); self.add_controller(drop)
        actions = {'add': lambda: dialogs.add_app(self), 'settings': lambda: dialogs.settings(self), 'restore-backup': self.restore_backup,
                   'search': lambda: self.search.grab_focus(), 'help': self.show_help, 'shortcuts': self.show_shortcuts, 'about': self.about,
                   'open': self.open_selected, 'app-settings': self.settings_selected, 'remove': self.remove_selected}
        for name, callback in actions.items():
            action = Gio.SimpleAction.new(name, None)
            action.connect('activate', dialogs.guarded(self, lambda *_, cb=callback: cb()))
            self.add_action(action)
        self.navigate('All Apps')

    def clear_box(self, box):
        while child := box.get_first_child():
            box.remove(child)

    def error(self, error):
        dialog = Adw.AlertDialog(heading='Could Not Complete the Action', body=str(error))
        dialog.add_response('close', 'Close'); dialog.set_close_response('close'); dialog.present(self)

    def toast(self, message):
        self.toast_overlay.add_toast(Adw.Toast(title=message))

    def can_modify(self, app=None, all_apps=False):
        if self.busy:
            self.toast('Wait for the current operation to finish')
            return False
        if (app and app['id'] in self.syncing) or (all_apps and self.syncing):
            self.toast('Wait for Wine settings to finish updating')
            return False
        if (app and app['id'] in self.sessions) or (all_apps and self.sessions):
            self.toast('Close the running app before changing its files')
            return False
        return True

    def perform(self, title, operation, completion=None, cancellable=True):
        if self.busy:
            self.toast('Wait for the current operation to finish')
            return
        self.busy = True
        dialog = Adw.Dialog(title=title, content_width=460, can_close=False)
        self.progress_dialog = dialog
        toolbar = Adw.ToolbarView(); toolbar.add_top_bar(Adw.HeaderBar())
        box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=18, margin_top=28, margin_bottom=28, margin_start=28, margin_end=28)
        label = Gtk.Label(label=title, wrap=True); box.append(label)
        progress = Gtk.ProgressBar(show_text=False); box.append(progress)
        updates = {'value': (title, 0, 0)}
        work = Work(lambda message, done=0, total=0: updates.update(value=(message, done, total)))
        self.work = work
        cancel = dialogs.button(self, 'Cancel', lambda: (work.cancel.set(), cancel.set_sensitive(False), label.set_text('Cancelling…')))
        cancel.set_visible(cancellable)
        box.append(cancel); toolbar.set_content(box); dialog.set_child(toolbar)
        shown = False
        frames = 0
        def presented_frame(*_):
            nonlocal frames
            frames += 1
            if frames < 3:
                return GLib.SOURCE_CONTINUE
            if self.work is not work:
                dialog.force_close()
            return GLib.SOURCE_REMOVE
        def present():
            nonlocal shown
            if self.work is work:
                shown = True
                dialog.present(self)
                dialog.add_tick_callback(presented_frame)
            return GLib.SOURCE_REMOVE
        # Cached preflight often finishes before the first frame. Never close a
        # dialog while it is still being presented by libadwaita.
        presentation = GLib.timeout_add(300, present)
        def tick():
            if self.work is not work:
                return GLib.SOURCE_REMOVE
            if not work.cancel.is_set():
                message, done, total = updates['value']; label.set_text(message)
                if total:
                    progress.set_fraction(min(1, done / total))
                else:
                    progress.pulse()
            return GLib.SOURCE_CONTINUE
        GLib.timeout_add(100, tick)
        def finish(result, error):
            self.work = None
            if not shown:
                GLib.source_remove(presentation)
            def complete(*_):
                self.busy = False; self.progress_dialog = None
                try:
                    self.refresh()
                    if error:
                        if isinstance(error, Cancelled):
                            self.toast('Operation cancelled')
                        else:
                            self.error(error)
                    elif completion:
                        completion(result)
                    if not self.busy and self.configuration_queue:
                        self.read_back_configuration(self.configuration_queue.pop(0))
                    if not self.busy and self.program_queue:
                        queued = self.program_queue.pop(0)
                        self.choose_program(queued)
                except Exception as failure:
                    self.error(failure)
            if shown:
                dialog.connect('closed', complete)
                # libadwaita 1.5 opens its sheet on the second frame. Closing
                # earlier is a no-op, then that deferred open leaves a ghost
                # modal. Wait through those frames before requesting closure.
                if frames >= 3:
                    dialog.force_close()
            else:
                complete()
        def run():
            try:
                result = operation(work)
                idle(finish, result, None)
            except Exception as error:
                idle(finish, None, error)
        threading.Thread(target=run, name='boxedwine-operation', daemon=True).start()

    def section_selected(self, _, item):
        if item is None:
            return
        self.view = item.title
        self.selected = None
        self.query = ''
        if self.view == 'Recovery' or self.details_split.get_collapsed():
            self.details_split.set_show_sidebar(False)
        self.search.set_text('')
        self.search.set_placeholder_text('Search demos' if self.view == 'Demos' else 'Search apps')
        if self.navigation.get_collapsed():
            self.navigation.set_show_sidebar(False)
        self.refresh()
        if self.view == 'Demos' and not self.catalog_loaded and not self.busy:
            self.load_demos()

    def navigate(self, title):
        item = self.section_rows[title]
        self.sections.select_row(item)
        self.section_selected(self.sections, item)

    def restore_navigation_focus(self):
        # Retain the active section when GTK relocates focus out of a hidden
        # sidebar. That focus movement is not a navigation request.
        self.search.grab_focus()
        self.sections.select_row(self.section_rows[self.view])

    def filter(self, query):
        self.query = query.casefold()
        self.refresh_content()

    def refresh(self):
        self.document = self.library.load()
        if self.view == 'Removed Apps' and not self.document['removedApps']:
            self.navigate('All Apps')
            return
        self.recovery = self.library.pending()
        self.window_title.set_title(self.view)
        self.window_title.set_subtitle(f"{len(self.document['apps'])} apps" if self.document['apps'] else '')
        self.section_rows['Recovery'].set_visible(bool(self.recovery) or self.view == 'Recovery')
        self.section_rows['Removed Apps'].set_visible(bool(self.document['removedApps']))
        self.refresh_content()
        self.refresh_running()
        if self.selected:
            if self.view == 'Demos':
                recipe = next((d for d in self.catalog if d['origin']['id'] == self.selected), None)
                if recipe:
                    self.show_demo(recipe, reveal=False)
            else:
                app = self.find_app(self.selected, include_removed=True)
                if app:
                    self.show_app(app, reveal=False)
                else:
                    self.selected = None; self.details_split.set_show_sidebar(False)

    def find_app(self, identifier, include_removed=False):
        values = self.document['apps'] + ([r['app'] for r in self.document['removedApps']] if include_removed else [])
        return next((a for a in values if a['id'] == identifier), None)

    def refresh_running(self):
        self.clear_box(self.runtime_box)
        for identifier, session in self.sessions.items():
            app = self.find_app(identifier)
            if not app:
                continue
            box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=8)
            box.add_css_class('card')
            row = Gtk.Box(spacing=12)
            label = Gtk.Label(label=('Installing ' if session.installing else 'Running ' if session.window.is_set() else 'Starting ') + (getattr(session, 'tool_name', None) or app['name']),
                              hexpand=True, xalign=0, margin_start=12, ellipsize=3)
            row.append(label)
            stop = dialogs.button(self, 'Stopping…' if identifier in self.stopping else 'Stop', lambda a=app: self.stop(a), 'flat')
            stop.set_sensitive(identifier not in self.stopping); row.append(stop)
            box.append(row)
            if not session.window.is_set() and identifier not in self.stopping:
                box.append(self.launch_progress())
            self.runtime_box.append(box)

    def launch_progress(self):
        progress = Gtk.ProgressBar(pulse_step=.10, margin_start=12, margin_end=12, margin_bottom=12)
        progress.update_property([Gtk.AccessibleProperty.LABEL], ['Waiting for the app window'])
        progress.pulse()
        last_pulse = 0
        def animate(bar, clock):
            nonlocal last_pulse
            frame_time = clock.get_frame_time()
            if frame_time - last_pulse >= 100_000:
                bar.pulse()
                last_pulse = frame_time
            return GLib.SOURCE_CONTINUE
        # Widget tick callbacks pause while unmapped and disappear with the bar,
        # including when a refresh replaces a starting banner.
        progress.add_tick_callback(animate)
        return progress

    def status(self, title, description, icon, action=None):
        page = Adw.StatusPage(title=title, description=description, icon_name=icon)
        if action:
            widget = dialogs.button(self, action[0], action[1], 'suggested-action')
            widget.set_halign(Gtk.Align.CENTER); page.set_child(widget)
        self.scroll.set_child(page)

    def refresh_content(self):
        if not hasattr(self, 'document'):
            return
        self.cancel_card_reveal()
        self.cards = {}
        if self.view == 'Recovery':
            self.recovery_content(); return
        if self.view == 'Demos':
            if not self.catalog:
                if self.selected is None:
                    self.details_split.set_show_sidebar(False)
                self.status('Try Something Familiar', self.catalog_error or 'Download the verified demo catalog to browse classic Windows apps and games.',
                            'applications-games-symbolic', ('Load Catalog', self.load_demos)); return
            items = [d for d in self.catalog if self.query in (d['name'] + ' ' + d['summary']).casefold()]
        elif self.view == 'Removed Apps':
            items = [r['app'] for r in self.document['removedApps'] if self.query in r['app']['name'].casefold()]
        else:
            items = [a for a in self.document['apps'] if self.query in a['name'].casefold()]
            if self.view == 'Recently Opened':
                items = [a for a in items if a.get('lastOpened') is not None]
                items.sort(key=lambda a: a['lastOpened'], reverse=True)
            else:
                items.sort(key=lambda a: a['name'].casefold())
        if not items:
            if self.selected is None:
                self.details_split.set_show_sidebar(False)
            if self.query:
                self.status('No Matches', 'Try a different search.', 'system-search-symbolic')
            elif self.view == 'Removed Apps':
                self.status('No Removed Apps', 'Apps you remove can be restored here until you delete them permanently.', 'user-trash-symbolic')
            elif self.view == 'Recently Opened':
                self.status('No Recent Apps', 'Open an app and it will appear here.', 'document-open-recent-symbolic')
            else:
                self.status('Your Windows Apps', 'Bring an installer or a portable app folder. Each app gets its own Windows environment.',
                            'application-x-executable-symbolic', ('Add Your First App', lambda: dialogs.add_app(self)))
            return
        if self.selected is None:
            # Select before laying out the grid so the details pane already has
            # its space. On narrow windows keep the overlay closed for browsing.
            show = self.show_demo if self.view == 'Demos' else self.show_app
            show(items[0], reveal=not self.details_split.get_collapsed())
        container = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=18, margin_start=24, margin_end=24, margin_bottom=24)
        heading = Gtk.Label(label={'All Apps': 'Your Windows Apps', 'Demos': 'Try Something Familiar'}.get(self.view, self.view), xalign=0)
        heading.add_css_class('title-1'); container.append(heading)
        if self.view == 'Removed Apps':
            delete_all = dialogs.button(self, 'Delete All Permanently…', self.delete_all, 'destructive-action')
            delete_all.set_halign(Gtk.Align.START); container.append(delete_all)
        grid = Gtk.FlowBox(selection_mode=Gtk.SelectionMode.NONE, homogeneous=True, max_children_per_line=6, min_children_per_line=1,
                           row_spacing=12, column_spacing=12, valign=Gtk.Align.START)
        for item in items:
            demo = self.view == 'Demos'
            card = Gtk.Button(); card.add_css_class('card'); card.add_css_class('app-card')
            identifier = item['origin']['id'] if demo else item['id']
            if identifier == self.selected:
                card.add_css_class('selected-app')
            card.set_size_request(142, 162)
            box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=10, margin_start=10, margin_end=10, margin_top=16, margin_bottom=14)
            box.append(self.icon(item, 48, demo))
            name = Gtk.Label(label=item['name'], wrap=True, justify=Gtk.Justification.CENTER, lines=2, ellipsize=3, max_width_chars=18)
            name.add_css_class('heading'); box.append(name)
            state = self.demo_state(item) if demo else ('Running' if identifier in self.sessions else 'Removed' if self.view == 'Removed Apps' else 'Choose program' if not item.get('executable') and not built_in(item) else 'Ready to open')
            subtitle = Gtk.Label(label=state, ellipsize=3); subtitle.add_css_class('caption'); subtitle.add_css_class('dim-label'); box.append(subtitle)
            card.set_child(box)
            self.cards[identifier] = card
            card.connect('clicked', dialogs.guarded(self, lambda _, a=item, d=demo: self.select_card(a, d)))
            if self.view in ('All Apps', 'Recently Opened', 'Demos'):
                gesture = Gtk.GestureClick.new()
                gesture.set_button(Gdk.BUTTON_PRIMARY)
                gesture.set_propagation_phase(Gtk.PropagationPhase.CAPTURE)
                gesture.connect('pressed', dialogs.guarded(self, lambda g, count, x, y, key=identifier: self.activate_card(g, count, key)))
                card.add_controller(gesture)
            grid.append(card)
        container.append(grid); self.scroll.set_child(container)

    def cancel_card_reveal(self):
        if self.card_reveal is not None:
            GLib.source_remove(self.card_reveal)
            self.card_reveal = None

    def update_card_selection(self):
        # Retain the widgets and their gestures between the two clicks.
        for identifier, card in self.cards.items():
            if identifier == self.selected:
                card.add_css_class('selected-app')
            else:
                card.remove_css_class('selected-app')

    def select_card(self, item, demo=False):
        self.cancel_card_reveal()
        if demo:
            self.show_demo(item, reveal=False)
        else:
            self.show_app(item, reveal=False)
        if self.view in ('All Apps', 'Recently Opened', 'Demos') and (self.details_split.get_collapsed() or not self.details_split.get_show_sidebar()):
            # Revealing details can move a card or cover it with an overlay.
            # Wait out the native double-click interval before changing layout.
            def reveal():
                self.card_reveal = None
                self.details_split.set_show_sidebar(True)
                return GLib.SOURCE_REMOVE
            delay = Gtk.Settings.get_default().get_property('gtk-double-click-time')
            self.card_reveal = GLib.timeout_add(delay, reveal)
        else:
            self.details_split.set_show_sidebar(True)

    def activate_card(self, gesture, count, identifier):
        if count != 2:
            return
        gesture.set_state(Gtk.EventSequenceState.CLAIMED)
        self.cancel_card_reveal()
        if self.view == 'Demos':
            recipe = next((d for d in self.catalog if d['origin']['id'] == identifier), None)
            if recipe:
                self.show_demo(recipe, reveal=False)
                existing = next((a for a in self.document['apps'] + [r['app'] for r in self.document['removedApps']]
                                 if (a.get('demo') or {}).get('id') == identifier), None)
                if existing:
                    active = self.find_app(existing['id'])
                    self.reveal_app(existing)
                    if active:
                        self.open_app(active)
                else:
                    self.install_demo(recipe)
            return
        app = self.find_app(identifier)
        if app and self.view in ('All Apps', 'Recently Opened'):
            self.show_app(app, reveal=False)
            self.open_app(app)

    def icon(self, app, size, demo=False):
        image = Gtk.Image(pixel_size=size)
        try:
            if demo:
                data = self.catalog_data.get(app['icon'])
            elif app.get('customIconPNG'):
                data = base64.b64decode(app['customIconPNG'])
            elif app.get('demo'):
                recipe = next((d for d in self.catalog if d['origin']['id'] == app['demo']['id']), None)
                data = self.catalog_data.get(recipe['icon']) if recipe else None
            elif built_in(app):
                name = 'notepad' if built_in(app) == 'notepad' else 'winemine'
                data = self.resources.path('AppIcons/' + name + '.png').read_bytes()
            else:
                data = None
            if not data and not demo and app.get('executable'):
                path = beneath(self.library.root(app), app['executable'])
                if path.is_file():
                    stamp = (path, path.stat().st_mtime_ns, path.stat().st_size)
                    cached = self.card_icons.get(app['id'])
                    if cached and cached[0] == stamp:
                        data = cached[1]
                    else:
                        from .icons import extract
                        data = extract(path)
                        self.card_icons[app['id']] = (stamp, data)
            if data:
                if data.startswith(b'\0\0\1\0'):
                    from gi.repository import GdkPixbuf
                    loader = GdkPixbuf.PixbufLoader.new_with_type('ico')
                    loader.write(data); loader.close()
                    texture = Gdk.Texture.new_for_pixbuf(loader.get_pixbuf())
                else:
                    texture = Gdk.Texture.new_from_bytes(GLib.Bytes.new(data))
                image.set_from_paintable(texture)
            else:
                image.set_from_icon_name('application-x-executable')
        except (OSError, ValueError, GLib.Error):
            image.set_from_icon_name('application-x-executable-symbolic')
        return image

    def details_content(self, app, demo=False):
        box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=18, margin_start=18, margin_end=18, margin_top=20, margin_bottom=24)
        box.append(self.icon(app, 72, demo))
        title = Gtk.Label(label=app['name'], wrap=True, justify=Gtk.Justification.CENTER); title.add_css_class('title-2'); box.append(title)
        scroll = Gtk.ScrolledWindow(hscrollbar_policy=Gtk.PolicyType.NEVER); scroll.set_child(box)
        self.details_toolbar.set_content(scroll)
        return box

    def show_app(self, app, reveal=True):
        self.selected = app['id']
        self.update_card_selection()
        if reveal:
            self.cancel_card_reveal()
            self.details_split.set_show_sidebar(True)
        box = self.details_content(app)
        removed = next((r for r in self.document['removedApps'] if r['app']['id'] == app['id']), None)
        if removed:
            if removed.get('deletionStartedAt') is None:
                box.append(dialogs.button(self, 'Restore App', lambda: self.restore_app(app), 'suggested-action', 'edit-undo-symbolic'))
            box.append(dialogs.button(self, 'Finish Deleting' if removed.get('deletionStartedAt') else 'Delete Permanently…', lambda: self.delete_app(app), 'destructive-action'))
            explanation = Gtk.Label(label='Removing an app keeps its files and saved games here. Permanent deletion cannot be undone.', wrap=True, xalign=0)
            explanation.add_css_class('dim-label'); box.append(explanation)
            return
        session = self.sessions.get(app['id'])
        if session:
            box.append(dialogs.button(self, 'Stop App', lambda: self.stop(app), 'destructive-action', 'media-playback-stop-symbolic'))
        else:
            label = 'Open App' if app.get('executable') or built_in(app) else 'Run Installer' if app.get('installer') else 'Choose Program'
            box.append(dialogs.button(self, label, lambda: self.open_app(app), 'suggested-action', 'media-playback-start-symbolic'))
        info = Adw.PreferencesGroup()
        dialogs.row(info, 'Display', app.get('resolution', '1024x768') + (' · Full screen' if app.get('fullScreen') else ' · Windowed'))
        dialogs.row(info, 'Windows support', 'Wine ' + (app.get('savedWineVersion') or 'default'))
        if any(app.get(f + 'Pending') for f in ('windowsVersion', 'wineRenderer', 'openGLBackend')):
            dialogs.row(info, 'Settings pending', 'Will be applied before the next launch')
        box.append(info)
        actions = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=8)
        actions.append(dialogs.button(self, 'Run Another Program…', lambda: self.run_another(app)))
        actions.append(dialogs.button(self, 'App Settings', lambda: dialogs.app_settings(self, app)))
        actions.append(dialogs.button(self, 'Troubleshooting', lambda: dialogs.troubleshoot(self, app)))
        actions.set_sensitive(session is None and not self.busy); box.append(actions)
        box.append(Gtk.Separator())
        box.append(dialogs.button(self, 'Export Backup…', lambda: self.export_backup(app)))
        box.append(dialogs.button(self, 'Open App Folder', lambda: self.open_path(self.library.path(app))))
        box.append(dialogs.button(self, 'Storage Details', lambda: self.storage_details(app)))
        box.append(dialogs.button(self, 'Remove from Library…', lambda: self.remove_app(app)))

    def demo_state(self, recipe):
        identifier = recipe['origin']['id']
        if any((a.get('demo') or {}).get('id') == identifier for a in self.document['apps']):
            return 'In your library'
        if any((r['app'].get('demo') or {}).get('id') == identifier for r in self.document['removedApps']):
            return 'In Removed Apps'
        size = f"{recipe['bytes'] / 1024:.0f} KB" if recipe['bytes'] < 1024**2 else f"{recipe['bytes'] / 1024**2:.1f} MB"
        return size + ' download'

    def show_demo(self, recipe, reveal=True):
        self.selected = recipe['origin']['id']
        self.update_card_selection()
        if reveal:
            self.cancel_card_reveal()
            self.details_split.set_show_sidebar(True)
        box = self.details_content(recipe, True)
        summary = Gtk.Label(label=recipe['summary'], wrap=True, xalign=0); summary.add_css_class('dim-label'); box.append(summary)
        existing = next((a for a in self.document['apps'] + [r['app'] for r in self.document['removedApps']] if (a.get('demo') or {}).get('id') == recipe['origin']['id']), None)
        if existing:
            box.append(dialogs.button(self, 'Show in Library' if self.find_app(existing['id']) else 'Show in Removed Apps', lambda: self.reveal_app(existing), 'suggested-action'))
        else:
            box.append(dialogs.button(self, 'Install Demo', lambda: self.install_demo(recipe), 'suggested-action', 'folder-download-symbolic'))
        details = Adw.PreferencesGroup()
        dialogs.row(details, 'Download', f"{recipe['bytes'] / 1024**2:.1f} MB · Verified package")
        dialogs.row(details, 'Windows support', 'Wine ' + recipe['wineVersion'])
        box.append(details)
        if recipe['help']:
            expander = Gtk.Expander(label='About this demo')
            text = Gtk.Label(label=recipe['help'], wrap=True, xalign=0, selectable=True, margin_top=12)
            expander.set_child(text); box.append(expander)

    def reveal_app(self, app):
        active = self.find_app(app['id'])
        self.navigate('All Apps' if active else 'Removed Apps')
        self.show_app(app)

    def open_selected(self):
        app = self.find_app(self.selected)
        if app:
            self.open_app(app)

    def settings_selected(self):
        app = self.find_app(self.selected)
        if app:
            dialogs.app_settings(self, app)

    def remove_selected(self):
        app = self.find_app(self.selected)
        if app:
            self.remove_app(app)

    def open_app(self, app):
        if app['id'] in self.sessions:
            self.toast('This app is already running'); return
        if app.get('executable') or built_in(app):
            self.launch(app)
        elif app.get('installer'):
            self.launch(app, installing=True)
        else:
            self.choose_program(app)

    def launch(self, app, installing=False, alternate=None, external=None, tool=None):
        if not self.can_modify(app):
            return
        # Read fresh metadata after settings or an installer changed the app.
        app = self.find_app(app['id'])
        if not app:
            raise ValueError('This app is no longer in the library.')
        def prepare(work):
            wine = self.library.wine_path(app)
            expected = app.get('winePackage') or (self.library.default_wine() if not app.get('savedWineVersion') else None)
            reference = self.wine_checks.verify(wine, expected, work)
            if app.get('savedWineVersion') and reference['wineVersion'] != app['savedWineVersion']:
                raise ValueError('This app’s saved Wine version differs from its package. Its files were kept.')
            prepared = refresh_configuration(self.emulator, self.library, app, wine, work) if app.get('wineConfigurationRefreshPending') else app
            prepared = apply_configuration(self.emulator, self.library, prepared, wine, work)
            arguments = build_arguments(self.library, prepared, wine, installing, alternate, external, tool)
            return prepared, wine, arguments
        def ready(result):
            prepared, wine, arguments = result
            auxiliary = tool is not None or alternate is not None or external is not None
            if auxiliary:
                prepared = copy.deepcopy(prepared); prepared['wineConfigurationRefreshPending'] = True
                self.library.update(prepared)
            environment = desktop.app_environment(self.library, prepared, self.icon(prepared, 64), self.resources)
            session = Session(self.emulator, arguments, wine, beneath(self.library.path(prepared), 'Logs/latest.log'), installing,
                              on_window=lambda _: idle(self.refresh), on_exit=lambda result: idle(self.runtime_finished, prepared, result), environment=environment)
            session.auxiliary = auxiliary
            session.tool_name = WINE_TOOLS[tool][0] + ' · ' + app['name'] if tool else None
            self.sessions[app['id']] = session
            prepared = copy.deepcopy(prepared); prepared['lastOpened'] = now(); self.library.update(prepared)
            self.refresh()
        self.perform('Preparing ' + (WINE_TOOLS[tool][0] if tool in WINE_TOOLS else app['name']), prepare, ready)

    def runtime_finished(self, app, session):
        self.sessions.pop(app['id'], None); self.stopping.discard(app['id'])
        self.refresh()
        if self.closing:
            if not self.sessions and not self.busy:
                self.close()
            return
        if session.error or (session.code != 0 and not session.stopped):
            dialog = Adw.AlertDialog(heading=app['name'] + ' Stopped', body=session.error or f'The emulator exited with code {session.code}. Check the launch log for details.')
            dialog.add_response('close', 'Close'); dialog.add_response('logs', 'View Logs')
            dialog.connect('response', dialogs.guarded(self, lambda _, response: self.show_logs(app) if response == 'logs' else None)); dialog.present(self)
        if getattr(session, 'auxiliary', False):
            current = self.find_app(app['id'])
            if current:
                self.syncing.add(app['id'])
                if self.busy:
                    self.configuration_queue.append(current)
                else:
                    self.read_back_configuration(current)
        if session.installing and not session.stopped and session.code == 0:
            current = self.find_app(app['id'])
            if current:
                # Program discovery is deferred if another file operation is active.
                if self.busy:
                    self.program_queue.append(current)
                else:
                    self.choose_program(current, after_install=True)

    def read_back_configuration(self, app, completion=None):
        self.syncing.add(app['id'])
        def read(work):
            try:
                current = next(a for a in self.library.load()['apps'] if a['id'] == app['id'])
                wine = self.library.wine_path(current)
                self.wine_checks.verify(wine, current.get('winePackage'), work)
                return refresh_configuration(self.emulator, self.library, current, wine, work), None
            except Exception as error:
                return None, error
        def finished(result):
            self.syncing.discard(app['id'])
            updated, error = result
            if error:
                self.error('Could not refresh Wine settings: ' + str(error) + '\nThey will be read again before the next launch or App Settings visit.')
            elif completion:
                completion(updated)
            else:
                self.toast('Wine settings updated')
            self.refresh()
        self.perform('Reading Wine Settings', read, finished)

    def stop(self, app):
        session = self.sessions.get(app['id'])
        if session and app['id'] not in self.stopping:
            self.stopping.add(app['id']); self.refresh()
            threading.Thread(target=session.stop, daemon=True).start()

    def choose_program(self, app, after_install=False):
        if not self.can_modify(app):
            return
        if after_install and app.get('demo'):
            def selected(result):
                self.refresh()
                if result.get('executable'):
                    self.toast('Installed ' + result['name']); self.reveal_app(result)
                else:
                    self.choose_program(result)
            self.perform('Finding installed program', lambda _: self.library.select_installed_demo(app), selected)
            return
        def save(path):
            current = self.find_app(app['id'])
            if not current or not self.can_modify(current):
                return
            updated = copy.deepcopy(current); updated['executable'] = path; self.library.update(updated)
            self.refresh(); self.reveal_app(updated); self.toast('Program selected')
        dialogs.program_picker(self, app, save)

    def run_another(self, app):
        if self.can_modify(app):
            dialogs.program_picker(self, app, lambda path: self.launch(app, alternate=path), title='Run Another Program', external=True)

    def run_external(self, app, path):
        from .library import check_installer
        check_installer(path)
        dialogs.confirm(self, 'Run External Program?', 'This program will run in ' + app['name'] + '’s Windows environment. It can read and change files in this host folder:\n\n' + str(path.parent),
                        'Run Program', lambda: self.launch(app, external=path))

    def with_wine(self, callback):
        if not self.can_modify():
            return
        wine = self.library.default_wine()
        if wine and self.library.package_path(wine).is_file():
            self.perform('Checking Windows support', lambda work: self.wine_checks.verify(self.library.package_path(wine), wine, work), callback)
            return
        release = self.resources.wines()[0]
        ref = release['reference']
        dialogs.confirm(self, 'Set Up Windows Support', f"Download Wine {ref['wineVersion']} ({ref['bytes'] / 1024**2:.0f} MB) from boxedwine.org? It will be shared by your apps.",
                        'Download', lambda: self.setup_wine(release, callback))

    def setup_wine(self, release, callback=None):
        if not self.can_modify():
            return
        def setup(work):
            wine = ensure_wine(self.library, release, work)
            self.library.set_default_wine(wine)
            return wine
        self.perform('Setting up Windows support', setup, callback or (lambda _: self.toast('Windows support is ready')))

    def import_wine(self, path):
        if not self.can_modify(all_apps=True):
            return
        def setup(work):
            wine = import_wine(self.library, path, work)
            if wine not in [r['reference'] for r in self.resources.wines()]:
                raise ValueError('Choose the Wine release listed in Settings. This file was not set as the default.')
            self.library.set_default_wine(wine)
        self.perform('Checking Windows support', setup, lambda _: self.toast('Windows support is ready'))

    def add_builtin(self, program):
        existing = next((a for a in self.document['apps'] if built_in(a) == program), None)
        if existing:
            self.reveal_app(existing); self.open_app(existing); return
        removed = next((r['app'] for r in self.document['removedApps'] if built_in(r['app']) == program), None)
        if removed:
            self.reveal_app(removed); return
        self.with_wine(lambda wine: self.perform('Adding Windows app', lambda work: self.library.add_builtin(program, wine, work),
                                                lambda app: (self.reveal_app(app), self.launch(app))))

    def app_added(self, app):
        self.reveal_app(app)
        if app.get('installer'):
            self.launch(app, installing=True)
        elif not app.get('executable') and not built_in(app):
            self.choose_program(app)
        else:
            self.toast('Added ' + app['name'])

    def load_demos(self):
        if self.busy:
            return
        def load(work):
            data = catalog_files(self.resources, self.library, work)
            return data, demos.load_catalog(data['catalog.xml'])
        def loaded(result):
            self.catalog_data, self.catalog = result; self.catalog_loaded = True; self.catalog_error = None; self.refresh()
        self.perform('Loading demo catalog', load, loaded)

    def install_demo(self, recipe):
        self.with_wine(lambda wine: self.perform('Installing ' + recipe['name'], lambda work: demos.install(self.library, recipe, wine, work), self.app_added))

    def remove_app(self, app):
        if not self.can_modify(app):
            return
        immediate = self.library.settings().get('deleteImmediately', False)
        def remove():
            if not self.can_modify(app):
                return
            self.library.remove(app)
            if immediate:
                self.perform('Deleting ' + app['name'], lambda _: self.library.delete(app), cancellable=False)
            else:
                self.refresh(); self.toast('Moved to Removed Apps')
        dialogs.confirm(self, ('Delete ' if immediate else 'Remove ') + app['name'] + '?',
                        'Permanently delete its Windows files and saved games. This cannot be undone.' if immediate else 'Its files and saved games will be kept in Removed Apps until you permanently delete them.',
                        'Delete Permanently' if immediate else 'Remove App', remove, destructive=immediate)

    def restore_app(self, app):
        if self.can_modify(app):
            self.library.restore(app); self.refresh(); self.reveal_app(app); self.toast('App restored')

    def delete_app(self, app):
        if not self.can_modify(app):
            return
        dialogs.confirm(self, 'Delete ' + app['name'] + ' Permanently?', 'Its Windows files and saved games will be deleted. This cannot be undone.', 'Delete Permanently',
                        lambda: self.perform('Deleting ' + app['name'], lambda _: self.library.delete(app), cancellable=False), True)

    def delete_all(self):
        if not self.can_modify(all_apps=True):
            return
        def remove(work):
            for removed in self.library.load()['removedApps']:
                work.report('Deleting ' + removed['app']['name']); self.library.delete(removed['app'])
        dialogs.confirm(self, 'Delete All Removed Apps?', 'All removed apps, their Windows files, and saved games will be permanently deleted. This cannot be undone.',
                        'Delete All', lambda: self.perform('Deleting removed apps', remove), True)

    def export_backup(self, app):
        if not self.can_modify(app):
            return
        def destination(path):
            if self.can_modify(app):
                self.perform('Exporting backup', lambda work: self.library.backup(app, path, work), lambda _: self.toast('Backup exported'))
        dialogs.choose_file(self, 'Export Backup', destination, save=True, name=''.join(c if c not in '/\\' else '-' for c in app['name']) + '.boxedwinebackup')

    def restore_backup(self, path=None):
        if not self.can_modify():
            return
        def restore(source):
            self.perform('Restoring backup', lambda work: self.library.restore_backup(source, work), lambda app: (self.reveal_app(app), self.toast('Backup restored')))
        if path:
            restore(path)
        else:
            dialogs.choose_file(self, 'Choose a .boxedwinebackup Folder', restore, folder=True)

    def clean_packages(self):
        if self.can_modify(all_apps=True):
            self.perform('Cleaning unused Wine packages', lambda work: self.library.prune_wine(), lambda _: self.toast('Unused Wine packages removed'))

    def recovery_content(self):
        if not self.recovery:
            self.status('Everything Is Up to Date', 'There are no interrupted file operations to review.', 'emblem-ok-symbolic'); return
        page = Adw.PreferencesPage()
        section = dialogs.group(page, 'Interrupted Operations', 'Finish a verified copy, or discard incomplete files. Original source files are kept.')
        for operation in self.recovery:
            item = dialogs.row(section, operation.get('name', 'Interrupted operation'), operation.get('problem') or ('Ready to finish' if operation.get('ready') else 'Incomplete copy'))
            if operation.get('ready'):
                item.add_suffix(dialogs.button(self, 'Finish', lambda op=operation: self.finish_recovery(op), 'suggested-action'))
            if not operation.get('problem') and not (operation['kind'] == 'backup' and operation.get('ready')):
                item.add_suffix(dialogs.button(self, 'Discard…', lambda op=operation: self.discard_recovery(op)))
        dialogs.action_row(self, section, 'Recovery records', str(self.library.directory / 'LinuxOperations'), 'Open Folder', lambda: self.open_path(self.library.directory / 'LinuxOperations'))
        self.scroll.set_child(page)

    def finish_recovery(self, operation):
        if self.can_modify(all_apps=True):
            self.perform('Finishing recovery', lambda work: self.library.finish(operation, work), lambda _: self.toast('Operation completed'))

    def discard_recovery(self, operation):
        if self.can_modify(all_apps=True):
            dialogs.confirm(self, 'Discard Incomplete Copy?', 'The incomplete copied files will be deleted. Original source files are kept.', 'Discard',
                            lambda: self.perform('Discarding incomplete copy', lambda _: self.library.discard(operation), cancellable=False), True)

    def storage_details(self, app):
        def measure(work):
            size, files = 0, 0
            for path in walk(self.library.path(app)):
                work.check()
                if path.is_file():
                    size += path.stat().st_size; files += 1
            return size, files
        self.perform('Measuring app storage', measure, lambda result: dialogs.text_dialog(self, 'Storage for ' + app['name'],
            f'{result[0] / 1024**2:.1f} MB in {result[1]:,} files\n\n' + str(self.library.path(app)) + '\n\nShared Wine packages are stored separately and are included when you export a backup.'))

    def show_logs(self, app):
        dialogs.logs(self, app)

    def set_emulator(self, path):
        if not self.can_modify(all_apps=True):
            return
        if not path.is_file() or not os.access(path, os.X_OK):
            raise ValueError('Choose an executable Boxedwine engine.')
        self.emulator = path
        preferences = self.library.settings(); preferences['emulator'] = str(path); self.library.save_settings(preferences)
        self.toast('Emulator updated')

    def open_uri(self, uri):
        Gtk.UriLauncher.new(uri).launch(self, None, self.uri_finished)

    def uri_finished(self, launcher, result):
        try:
            launcher.launch_finish(result)
        except GLib.Error as error:
            self.error(error)

    def open_path(self, path):
        path = no_links(path)
        if not path.exists():
            raise ValueError('This folder has not been created yet.')
        self.open_uri(path.as_uri())

    def drop_file(self, _, file, x, y):
        if isinstance(file, Gdk.FileList):
            files = file.get_files()
            if len(files) != 1:
                self.toast('Add one app or backup at a time')
                return False
            file = files[0]
        path = file.get_path()
        if not path or not self.can_modify():
            return False
        path = no_links(path)
        if path.is_dir() and path.name.endswith('.boxedwinebackup'):
            self.restore_backup(path)
        else:
            dialogs.add_app(self, path)
        return True

    def show_help(self):
        dialogs.text_dialog(self, 'Boxedwine Help',
            'Add a Windows app\n\nChoose an .exe or .msi installer, a folder containing an installer and its data files, or a portable app folder. Boxedwine copies the source into your library. When setup finishes, choose the program you want to open. You can also drop a source onto this window.\n\n'
            'Each app has its own Windows environment\n\nSaved files stay with that app. Use Run Another Program to launch a configuration utility in the same environment. An external program gets access to its containing host folder after confirmation.\n\n'
            'Demos and Windows support\n\nThe demo catalog and downloads are checked against release checksums. Wine is shared on disk, while each app keeps a specific package. Offline Windows support can be imported from the matching ZIP in Settings.\n\n'
            'Backups and removal\n\nExport Backup creates a .boxedwinebackup folder containing the app, its Wine package, and a verified manifest. Restore Backup creates a separate app. Keep the entire backup folder together. Removed Apps retains files until permanent deletion unless immediate deletion is enabled. Interrupted file operations appear in Recovery.\n\n'
            'Compatibility\n\nUse App Settings for Windows version, window size, full screen, Wine graphics, icons, and arguments. Troubleshooting offers installer repair, logs, and compatibility advice. Not every Windows program works in Boxedwine.\n\n'
            'Linux library location\n\n' + str(self.library.directory))

    def show_shortcuts(self):
        dialogs.text_dialog(self, 'Keyboard Shortcuts', 'Ctrl+N    Add App\nCtrl+F    Search\nCtrl+O    Open Selected App\nAlt+Return    App Settings\nDelete    Remove Selected App\nCtrl+,    Settings\nF1    Help\nCtrl+Q    Quit')

    def about(self):
        dialog = Adw.AboutDialog(application_name='Boxedwine', application_icon='org.boxedwine.Boxedwine', version='Linux native UI',
                                 developer_name='The Boxedwine Team', website='https://www.boxedwine.org/', issue_url='https://github.com/danoon2/Boxedwine/issues',
                                 license_type=Gtk.License.GPL_2_0, copyright='© 2016–2026 The Boxedwine Team',
                                 comments='Run Windows apps in a separate emulated environment.\nGTK 4 and libadwaita frontend for Linux.')
        dialog.add_legal_section('Wine and app icons', None, Gtk.License.LGPL_2_1, 'Wine and its program icons are distributed under the LGPL. See the bundled AppIcons licenses. Wine packages include their own license notices.')
        dialog.present(self)
        return dialog

    def close_requested(self, *_):
        if self.busy:
            self.toast('Cancel or finish the current operation before quitting')
            return True
        if self.sessions:
            if self.closing:
                return True
            def stop_all():
                self.closing = True
                for identifier in list(self.sessions):
                    app = self.find_app(identifier)
                    if app:
                        self.stop(app)
            dialogs.confirm(self, 'Quit Boxedwine?', 'Running Windows apps will be stopped. Save your work before quitting.', 'Stop Apps and Quit', stop_all, True)
            return True
        return False


class Application(Adw.Application):
    def __init__(self, options):
        super().__init__(application_id=desktop.launcher_id(options.library), flags=Gio.ApplicationFlags.HANDLES_COMMAND_LINE)
        GLib.set_prgname(self.get_application_id())
        GLib.set_application_name('Boxedwine')
        self.options, self.library, self.window = options, None, None
        self.connect('activate', self.activate_app)

    def do_command_line(self, command_line):
        self.activate()
        arguments = command_line.get_arguments()
        if '--open' in arguments and isinstance(self.window, Window):
            index = arguments.index('--open')
            if index + 1 < len(arguments):
                app = self.window.find_app(arguments[index + 1])
                if app:
                    if app['id'] in self.window.sessions:
                        self.window.toast(app['name'] + ' is already running')
                    else:
                        self.window.open_app(app)
        return 0

    def activate_app(self, *_):
        if self.window:
            self.window.present(); return
        try:
            self.library = Library(self.options.library)
            resources = Resources(self.options.resources)
            desktop.setup_launcher(self.library, resources, self.get_application_id())
            emulator = self.options.emulator or self.library.settings().get('emulator') or default_emulator()
            self.window = Window(self, self.library, resources, emulator)
            css = Gtk.CssProvider()
            css.load_from_data(b'.app-card { padding: 0; } .selected-app { outline: 2px solid @accent_color; outline-offset: -2px; }')
            Gtk.StyleContext.add_provider_for_display(Gdk.Display.get_default(), css, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)
            for action, shortcuts in {'add': ['<Control>n'], 'search': ['<Control>f'], 'open': ['<Control>o'], 'app-settings': ['<Alt>Return'],
                                       'remove': ['Delete'], 'settings': ['<Control>comma'], 'help': ['F1']}.items():
                self.set_accels_for_action('win.' + action, shortcuts)
            action = Gio.SimpleAction.new('quit', None); action.connect('activate', lambda *_: self.window.close()); self.add_action(action)
            self.set_accels_for_action('app.quit', ['<Control>q'])
            self.window.present()
            if self.library.pending():
                self.window.toast('An interrupted operation needs review in Recovery')
        except Exception as error:
            self.window = Adw.ApplicationWindow(application=self, title='Boxedwine', default_width=500, default_height=350)
            self.window.set_content(Adw.StatusPage(title='Could Not Open the Library', description=str(error), icon_name='dialog-error-symbolic'))
            self.window.present()
            print('Boxedwine:', error, file=sys.stderr)

    def do_shutdown(self):
        if self.library:
            self.library.close()
        Adw.Application.do_shutdown(self)


def main():
    parser = argparse.ArgumentParser(description='Native GTK/libadwaita Boxedwine UI')
    parser.add_argument('--library', type=Path, default=Path(os.environ.get('XDG_DATA_HOME', Path.home() / '.local/share')) / 'boxedwine')
    parser.add_argument('--emulator', type=Path, help='Path to the native Boxedwine engine')
    parser.add_argument('--resources', type=Path, help='Application resource directory')
    parser.add_argument('--open', metavar='APP_ID', help='Open a saved app (also works with a running launcher)')
    options = parser.parse_args()
    if (Gtk.get_major_version(), Gtk.get_minor_version()) < (4, 14) or (Adw.get_major_version(), Adw.get_minor_version()) < (1, 5):
        raise SystemExit('Boxedwine requires GTK 4.14 and libadwaita 1.5 or newer.')
    return Application(options).run(sys.argv)
