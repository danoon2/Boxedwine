# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""App settings shared in intent and metadata with the Windows launcher."""
import base64
import copy
from pathlib import Path, PureWindowsPath
import uuid
from gi.repository import Adw, Gdk, GdkPixbuf, Gio, GLib, Gtk
from .cdrom import validate_images
from .dialogs import button, choose_file, combo, group, program_picker, row, text_dialog
from .files import no_links
from .library import DRIVE_C, WINDOWS, RENDERERS, built_in, preference
from .runtime import (OPTIONS_HELP, lines, mouse_sensitivity, with_mouse_sensitivity,
                      transfer_mouse_sensitivity)


def label(text, muted=False):
    result = Gtk.Label(label=text, xalign=0, wrap=True)
    if muted:
        result.add_css_class('dim-label'); result.add_css_class('caption')
    return result


def accessible(widget, name):
    widget.update_property([Gtk.AccessibleProperty.LABEL], [name])


class CdromEditor(Gtk.Box):
    def __init__(self, owner, paths):
        super().__init__(orientation=Gtk.Orientation.VERTICAL, spacing=8)
        self.owner, self.images = owner, list(paths)
        self.token = str(uuid.uuid4()) + ':'
        self.rows = Gtk.ListBox(selection_mode=Gtk.SelectionMode.NONE)
        self.rows.add_css_class('boxed-list'); self.append(self.rows)
        self.empty = label('No images added. The drive starts empty.', True); self.append(self.empty)
        self.add_button = button(owner, 'Add CD-ROM image…', self.choose, icon='list-add-symbolic')
        self.add_button.set_halign(Gtk.Align.START); self.append(self.add_button)
        self.append(label('First disc is inserted at launch. Drag to reorder.\n'
                          'Switch with Ctrl+Shift+F12, or choose a new image from the status bar.', True))
        self.refresh()

    def choose(self):
        picker = Gtk.FileDialog(title='Add CD-ROM images', modal=True)
        filters = Gio.ListStore.new(Gtk.FileFilter)
        images = Gtk.FileFilter(name='Disc images (*.iso, *.cue)')
        for pattern in ('*.iso', '*.ISO', '*.cue', '*.CUE'):
            images.add_pattern(pattern)
        filters.append(images)
        all_files = Gtk.FileFilter(name='All files'); all_files.add_pattern('*'); filters.append(all_files)
        picker.set_filters(filters)
        def finished(dialog, result):
            try:
                files = dialog.open_multiple_finish(result)
                paths = [file.get_path() for file in files]
                if any(path is None for path in paths):
                    raise ValueError('Choose local CD-ROM images.')
                self.add_images(paths)
            except GLib.Error as error:
                if not error.matches(Gtk.dialog_error_quark(), Gtk.DialogError.DISMISSED):
                    self.owner.error(error)
            except Exception as error:
                self.owner.error(error)
        picker.open_multiple(self.owner, None, finished)

    def add_images(self, paths):
        additions = list(dict.fromkeys(str(Path(path).absolute()) for path in paths))
        result = self.images + [path for path in additions if path not in self.images]
        validate_images(result)
        self.images = result; self.refresh()

    def move(self, source, target):
        if 0 <= source < len(self.images) and 0 <= target < len(self.images) and source != target:
            self.images.insert(target, self.images.pop(source)); self.refresh()
            self.rows.get_row_at_index(target).grab_focus()

    def remove(self, index):
        self.images.pop(index); self.refresh()
        focus = self.rows.get_row_at_index(min(index, len(self.images) - 1)) if self.images else self.add_button
        focus.grab_focus()

    def drop(self, value, target, after=False):
        if not isinstance(value, str) or not value.startswith(self.token):
            return False
        try:
            source = int(value[len(self.token):])
        except ValueError:
            return False
        if not 0 <= source < len(self.images):
            return False
        target += int(after)
        if source < target:
            target -= 1
        self.move(source, target)
        return True

    def refresh(self):
        while child := self.rows.get_first_child():
            self.rows.remove(child)
        self.empty.set_visible(not self.images); self.rows.set_visible(bool(self.images))
        for index, path in enumerate(self.images):
            file = Path(path) if Path(path).is_absolute() else PureWindowsPath(path)
            subtitle = str(file.parent) + ('\nInserted at launch' if index == 0 else '')
            item = Adw.ActionRow(title=f'{index + 1}. {file.name}', subtitle=subtitle,
                                 use_markup=False, focusable=True, tooltip_text=path)
            item.set_title_lines(1); item.set_subtitle_lines(2 if index == 0 else 1)
            grip = Gtk.Image(icon_name='list-drag-handle-symbolic', tooltip_text='Drag to reorder. Alt+Up/Down also moves this disc.')
            item.add_prefix(grip)
            for offset, icon, title in ((-1, 'go-up-symbolic', 'Move up'), (1, 'go-down-symbolic', 'Move down')):
                move = button(self.owner, None, lambda i=index, d=offset: self.move(i, i + d), icon=icon)
                move.add_css_class('flat'); move.set_tooltip_text(title); accessible(move, title + ' ' + file.name)
                move.set_sensitive(0 <= index + offset < len(self.images)); item.add_suffix(move)
            remove = button(self.owner, None, lambda i=index: self.remove(i), icon='window-close-symbolic')
            remove.add_css_class('flat'); remove.set_tooltip_text('Remove from this list (keeps the file)')
            accessible(remove, 'Remove ' + file.name); item.add_suffix(remove)
            drag = Gtk.DragSource(actions=Gdk.DragAction.MOVE)
            drag.connect('prepare', lambda *_, i=index: Gdk.ContentProvider.new_for_value(self.token + str(i)))
            grip.add_controller(drag)
            drop = Gtk.DropTarget.new(str, Gdk.DragAction.MOVE)
            drop.connect('drop', lambda _, value, x, y, i=index, r=item: self.drop(value, i, y >= r.get_height() / 2))
            item.add_controller(drop)
            keys = Gtk.EventControllerKey()
            def key(_, keyval, keycode, state, i=index):
                modifiers = state & (Gdk.ModifierType.ALT_MASK | Gdk.ModifierType.CONTROL_MASK |
                                     Gdk.ModifierType.SHIFT_MASK | Gdk.ModifierType.SUPER_MASK)
                if modifiers == Gdk.ModifierType.ALT_MASK and keyval in (Gdk.KEY_Up, Gdk.KEY_Down):
                    self.move(i, i + (-1 if keyval == Gdk.KEY_Up else 1)); return True
                return False
            keys.connect('key-pressed', key); item.add_controller(keys); self.rows.append(item)


class AppSettingsDialog(Adw.Dialog):
    def __init__(self, owner, app):
        self.owner, self.original, self.saved = owner, app, copy.deepcopy(app)
        saved = self.saved
        # Pick the opening height once. Added discs scroll inside the same sheet.
        height = min(800, 720 + min(3, len(saved.get('cdromImages') or [])) * 64)
        if owner.get_height() > 0:
            height = min(height, max(360, owner.get_height() - 40))
        super().__init__(title='App Settings', content_width=660, content_height=height)
        toolbar = Adw.ToolbarView()
        header = Adw.HeaderBar(show_end_title_buttons=False)
        self.stack = Adw.ViewStack()
        switcher = Adw.ViewSwitcher(stack=self.stack, policy=Adw.ViewSwitcherPolicy.WIDE)
        header.set_title_widget(switcher); toolbar.add_top_bar(header)
        self.general = Adw.PreferencesPage(title='General')
        self.advanced = Adw.PreferencesPage(title='Advanced')
        self.stack.add_titled_with_icon(self.general, 'General', 'General', 'preferences-system-symbolic')
        self.stack.add_titled_with_icon(self.advanced, 'Advanced', 'Advanced', 'applications-engineering-symbolic')
        toolbar.set_content(self.stack)
        footer = Gtk.Box(spacing=8, halign=Gtk.Align.END, margin_top=12, margin_bottom=12, margin_start=18, margin_end=18)
        footer.append(button(owner, 'Cancel', self.close))
        self.save_button = button(owner, 'Save', self.save, 'suggested-action'); footer.append(self.save_button)
        toolbar.add_bottom_bar(footer); self.set_child(toolbar)

        identity = group(self.general, '')
        self.name = Adw.EntryRow(title='Name', text=saved['name']); identity.add(self.name)
        if not built_in(saved):
            self.program = row(identity, 'Program', self.program_path())
            choose = button(owner, 'Choose…', lambda: program_picker(owner, saved, self.select_program))
            self.program.add_suffix(choose); self.program.set_activatable_widget(choose)

        display = group(self.general, '')
        display_row = row(display, 'Window size')
        self.size = Gtk.Entry(text=saved.get('resolution', '1024x768'), width_chars=9, max_width_chars=9, valign=Gtk.Align.CENTER)
        accessible(self.size, 'Window size'); display_row.add_suffix(self.size)
        presets = Gtk.Popover(); choices = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        for value in ('640x480', '800x600', '1024x768', '1280x720', '1920x1080'):
            choices.append(button(owner, value, lambda v=value: (self.size.set_text(v), presets.popdown())))
        presets.set_child(choices)
        display_row.add_suffix(Gtk.MenuButton(icon_name='pan-down-symbolic', popover=presets, valign=Gtk.Align.CENTER, tooltip_text='Choose a preset'))
        self.fullscreen = Gtk.CheckButton(label='Open in full screen', active=saved.get('fullScreen', False), margin_start=8)
        display_row.add_suffix(self.fullscreen)

        mouse_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=4)
        heading = Gtk.Box(spacing=12); title = label('Mouse sensitivity'); title.add_css_class('heading')
        heading.append(title); heading.append(label('100% is normal · Captured mouse only', True)); mouse_box.append(heading)
        percent = mouse_sensitivity(saved.get('boxedwineArguments') or [])
        self.mouse = Gtk.Scale.new_with_range(Gtk.Orientation.HORIZONTAL, min(25, percent), max(400, percent), 1)
        self.mouse.set_digits(0); self.mouse.set_draw_value(False); self.mouse.set_hexpand(True); self.mouse.set_value(percent)
        accessible(self.mouse, 'Mouse sensitivity')
        self.mouse.set_tooltip_text('Adjusts mouse movement while a game captures the cursor. Applies the next time you launch the app.')
        mouse_row = Gtk.Box(spacing=8); mouse_row.append(self.mouse)
        self.mouse_value = Gtk.Label(label=f'{percent}%', width_chars=5); mouse_row.append(self.mouse_value)
        self.mouse_reset = button(owner, 'Reset', lambda: self.mouse.set_value(100)); accessible(self.mouse_reset, 'Reset mouse sensitivity')
        mouse_row.append(self.mouse_reset); mouse_box.append(mouse_row)
        # Keep the slider outside the boxed rows so it has the full content width.
        mouse_group = group(self.general, ''); mouse_group.add(mouse_box)
        compatibility = group(self.general, '')
        self.windows = self.configuration_choice(compatibility, 'Windows version', WINDOWS, 'windowsVersion', 'Applies on the next launch.')
        self.discs = CdromEditor(owner, saved.get('cdromImages') or [])
        group(self.general, 'CD-ROM images').add(self.discs)

        wine = saved.get('savedWineVersion') or (owner.library.default_wine() or {}).get('wineVersion') or 'not configured'
        summary = 'Wine ' + wine + ' · ' + ('saved for this app' if saved.get('savedWineVersion') else 'library default')
        group(self.advanced, '').add(label(summary, True))
        icon_group = group(self.advanced, '')
        self.icon_row = row(icon_group, 'App icon')
        self.icon = owner.icon(saved, 40); self.icon_row.add_prefix(self.icon)
        self.icon_row.add_suffix(button(owner, 'Choose image…', lambda: choose_file(owner, 'Choose App Icon', self.set_icon)))
        self.automatic_icon = button(owner, 'Use automatic', self.reset_icon); self.icon_row.add_suffix(self.automatic_icon)
        self.refresh_icon()
        graphics = group(self.advanced, '')
        self.renderer = self.configuration_choice(graphics, 'Wine renderer', {**RENDERERS, 'gdi': 'GDI (compatibility)'}, 'wineRenderer',
                                                  'GDI supports older 2D apps; disables Direct3D acceleration.')
        self.backend = self.configuration_choice(graphics, 'Wine OpenGL interface', {'wineDefault': 'Use Wine’s default', 'glx': 'GLX', 'egl': 'EGL'}, 'openGLBackend')
        supported = button(owner, 'Supported options', lambda: text_dialog(owner, 'Supported Boxedwine options', OPTIONS_HELP, True))
        self.boxed, self.boxed_value, section = self.arguments_editor('Boxedwine arguments',
            '\n'.join(with_mouse_sensitivity(saved.get('boxedwineArguments') or [], 100)), 'One option or value per line', supported)
        self.transfer_notice = label('', True); self.transfer_notice.set_visible(False); section.append(self.transfer_notice)
        self.arguments, self.arguments_value, _ = self.arguments_editor('App arguments', '\n'.join(saved.get('arguments', [])), 'One argument per line')
        self.mouse.connect('value-changed', self.mouse_changed)
        focus = Gtk.EventControllerFocus(); focus.connect('leave', lambda *_: self.try_transfer_mouse())
        self.boxed.add_controller(focus)
        self.boxed.get_buffer().connect('paste-done', lambda *_: self.try_transfer_mouse())

    def set_visible_page_name(self, name):
        self.stack.set_visible_child_name(name)

    def configuration_choice(self, parent, title, choices, field, subtitle=None):
        custom = self.saved.get('wineConfigurationCustom', {}).get(field)
        if custom:
            choices = {**choices, 'custom': 'Custom (set in Wine)'}; subtitle = custom
        return combo(parent, title, choices, 'custom' if custom else preference(self.saved, field), subtitle)[1]

    def arguments_editor(self, title, text, hint, action=None):
        section = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=6)
        heading = Gtk.Box(spacing=8)
        name = label(title); name.add_css_class('heading'); heading.append(name)
        hint_label = label(hint, True); hint_label.set_hexpand(True); heading.append(hint_label)
        if action:
            heading.append(action)
        section.append(heading)
        editor = Gtk.TextView(wrap_mode=Gtk.WrapMode.WORD_CHAR, monospace=True,
                              top_margin=8, bottom_margin=8, left_margin=10, right_margin=10)
        accessible(editor, title); editor.get_buffer().set_text(text)
        scroll = Gtk.ScrolledWindow(min_content_height=92, max_content_height=92, propagate_natural_height=True)
        scroll.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        scroll.set_child(editor); scroll.add_css_class('card'); section.append(scroll)
        if title == 'App arguments':
            section.append(label('Spaces are preserved; omit surrounding quotes.', True))
        group(self.advanced, '').add(section)
        def value():
            buffer = editor.get_buffer()
            return buffer.get_text(buffer.get_start_iter(), buffer.get_end_iter(), False)
        return editor, value, section

    def program_path(self):
        path = self.saved.get('executable')
        return 'C:/' + path[len(DRIVE_C) + 1:] if path else 'Choose after installation'

    def select_program(self, path):
        self.saved['executable'] = path; self.program.set_subtitle(self.program_path()); self.refresh_icon()

    def set_icon(self, path):
        if no_links(path).stat().st_size > 32 * 1024**2:
            raise ValueError('Choose an image smaller than 32 MB.')
        pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_scale(str(path), 256, 256, True)
        success, data = pixbuf.save_to_bufferv('png', [], [])
        if not success:
            raise ValueError('Could not read this image.')
        self.saved['customIconPNG'] = base64.b64encode(data).decode(); self.refresh_icon()

    def reset_icon(self):
        self.saved.pop('customIconPNG', None); self.refresh_icon()

    def refresh_icon(self):
        self.icon_row.remove(self.icon); self.icon = self.owner.icon(self.saved, 40); self.icon_row.add_prefix(self.icon)
        custom = bool(self.saved.get('customIconPNG'))
        self.icon_row.set_subtitle('Custom icon' if custom else 'Automatic icon'); self.automatic_icon.set_sensitive(custom)

    def mouse_changed(self, *_):
        self.mouse_value.set_text(f'{round(self.mouse.get_value())}%'); self.transfer_notice.set_visible(False)

    def transfer_mouse(self):
        buffer = self.boxed.get_buffer()
        cursor = buffer.get_iter_at_mark(buffer.get_insert()).get_offset()
        text, percent, cursor = transfer_mouse_sensitivity(self.boxed_value(), cursor)
        if percent is None:
            return
        self.mouse.set_range(min(25, percent), max(400, percent)); self.mouse.set_value(percent)
        buffer.set_text(text); buffer.place_cursor(buffer.get_iter_at_offset(cursor))
        self.transfer_notice.set_text(f'Mouse sensitivity set to {percent}% in General.'); self.transfer_notice.set_visible(True)

    def try_transfer_mouse(self):
        try:
            self.transfer_mouse()
        except ValueError:
            pass  # Leave incomplete/invalid typing editable; Save reports it.

    def save(self):
        if not self.owner.can_modify(self.original):
            return
        self.transfer_mouse()
        saved = copy.deepcopy(self.saved)
        saved.update(name=self.name.get_text().strip(), resolution=self.size.get_text().strip(), fullScreen=self.fullscreen.get_active(),
                     arguments=lines(self.arguments_value()), cdromImages=list(self.discs.images),
                     boxedwineArguments=with_mouse_sensitivity(lines(self.boxed_value()), round(self.mouse.get_value())))
        for field, value in [('windowsVersion', self.windows()), ('wineRenderer', self.renderer()), ('openGLBackend', self.backend())]:
            if value == 'custom':
                continue
            if value != preference(self.original, field) or field in saved.get('wineConfigurationCustom', {}):
                saved[field] = value; saved[field + 'Pending'] = True
                saved.get('wineConfigurationCustom', {}).pop(field, None)
        if not saved.get('wineConfigurationCustom'):
            saved.pop('wineConfigurationCustom', None)
        self.owner.library.validate_app(saved); self.owner.library.update(saved)
        self.close(); self.owner.refresh(); self.owner.toast('App settings saved')
