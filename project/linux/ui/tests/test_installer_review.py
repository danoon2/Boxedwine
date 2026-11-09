"""Optional GTK integration check: run under xvfb-run on UI build hosts."""
from pathlib import Path
import sys
import tempfile
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

try:
    import gi
    gi.require_version('Gtk', '4.0')
    gi.require_version('Adw', '1')
    from gi.repository import Adw, Gtk
    AVAILABLE = hasattr(Adw, 'PreferencesDialog') and Gtk.init_check()
except (ImportError, ValueError):
    AVAILABLE = False


@unittest.skipUnless(AVAILABLE, 'GTK 4/libadwaita and a display are needed')
class InstallerReviewTests(unittest.TestCase):
    def test_exe_review_import_mounts_original_folder(self):
        from boxedwine.dialogs import review_import
        from boxedwine.imports import PreparedSource
        from boxedwine.library import Library
        from boxedwine.files import Work
        from boxedwine.runtime import build_arguments

        class Owner(Adw.Window):
            def error(self, error): self.failure = error
            def with_wine(self, callback): callback(self.wine)
            def perform(self, title, operation, callback): callback(operation(Work()))
            def app_added(self, app): self.added = app

        def descendants(widget):
            yield widget
            child = widget.get_first_child()
            while child:
                yield from descendants(child)
                child = child.get_next_sibling()

        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp); source = root / 'Setup media'; source.mkdir()
            setup = source / 'TheMoviesSetup.exe'; setup.write_bytes(b'MZsetup')
            (source / 'TheMoviesSetup.split.bin').write_bytes(b'payload')
            owner = Owner(); owner.failure = None; owner.added = None
            owner.library = Library(root / 'library')
            owner.wine = dict(sha256='a' * 64, bytes=100, wineVersion='11.0', filesystemVersion='14')
            prepared = PreparedSource(setup, Work())
            try:
                dialog = review_import(owner, prepared)
                action = next(w for w in descendants(dialog) if isinstance(w, Adw.ComboRow) and w.get_title() == 'Add as')
                action.set_selected(2)
                final = next(w for w in descendants(dialog) if isinstance(w, Adw.ActionRow) and w.get_title() == 'Use installer files in their current location')
                self.assertIn(str(source), final.get_subtitle())
                self.assertIn('without copying', final.get_subtitle())
                final.get_activatable_widget().emit('clicked')
                if owner.failure: raise owner.failure
                self.assertEqual(owner.added['installerSource'], str(source))
                self.assertFalse((owner.library.path(owner.added) / 'Installer').exists())
                args = build_arguments(owner.library, owner.added, root / 'wine.zip', installing=True)
                self.assertEqual(args[args.index('-mount') + 1], str(source))
            finally:
                prepared.close(); owner.close(); owner.library.close()
