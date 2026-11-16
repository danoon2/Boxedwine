"""Keep build/source/license records in release sidecars, outside runtime ZIPs."""
from pathlib import Path
import zipfile

DOCUMENT_PREFIXES = (
    'usr/share/doc/', 'usr/local/share/doc/', 'opt/wine/share/doc/',
    'usr/share/licenses/', 'usr/share/licences/',
    'usr/local/share/licenses/', 'usr/local/share/licences/',
)
BUILD_RECORDS = {
    'build.txt', 'recreateInstructions.txt', 'filesystem-build.json',
    'media-runtime.json', 'wav-runtime.json', 'avi-runtime.json', 'web-runtime.json',
}


def is_documentation(name):
    return name in BUILD_RECORDS or name.startswith(DOCUMENT_PREFIXES)


def archive_documents(source_zip, overlay, output):
    """Collect the documents that a runtime assembly excludes, with overlay priority."""
    overlay = Path(overlay)
    replacements = {p.relative_to(overlay).as_posix(): p for p in overlay.rglob('*')
                    if p.is_file() and is_documentation(p.relative_to(overlay).as_posix())}
    with zipfile.ZipFile(source_zip) as source, zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED) as target:
        for entry in source.infolist():
            if not entry.is_dir() and is_documentation(entry.filename) and entry.filename not in replacements:
                target.writestr(entry, source.read(entry))
        for name, path in sorted(replacements.items()):
            target.write(path, name)


def validate_runtime(archive):
    with zipfile.ZipFile(archive) as source:
        unexpected = [name for name in source.namelist() if is_documentation(name)]
    if unexpected:
        raise RuntimeError(f'Build/source/license records remain in runtime: {unexpected}')
