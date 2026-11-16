#!/usr/bin/env python3
"""Read publishing metadata and prune explicitly retired builds on the server.

Sent to python3 over SSH stdin; no installation or archive transfers are needed.
Snapshot reads ZIP directories to distinguish apps from installed overlays, but
never extracts, copies, or hashes game/filesystem payloads.
"""
import argparse
import base64
import hashlib
import json
from pathlib import Path, PurePosixPath
import shutil
import zipfile


def program_from_names(names, stem):
    programs = [name for name in names if name.lower().endswith(('.bat', '.exe'))]
    batches = [name for name in programs if name.lower().endswith('.bat')]
    exact = [name for name in programs if Path(name).stem.lower() == stem.lower()]
    choices = batches or exact or programs
    return min(choices, key=lambda name: (name.count('/'), len(name), name.lower())) if choices else ''


def safe_path(site, relative):
    parts = PurePosixPath(relative).parts
    if not parts or relative != '/'.join(parts) or any(p in ('.', '..') for p in parts) or '\\' in relative:
        raise ValueError('Invalid site path: ' + relative)
    path = site
    for part in parts:
        path = path / part
        if path.is_symlink():
            raise ValueError('Linked site path: ' + relative)
    path.resolve().relative_to(site.resolve())
    return path


def snapshot(site, demos=False):
    state = {'schemaVersion': 1, 'metadata': {}, 'buildDirs': [], 'branchDirs': []}
    for pattern in ('branches/*/branch.json', 'branches/*/builds/*/build.json'):
        for path in sorted(site.glob(pattern)):
            relative = path.relative_to(site).as_posix()
            state['metadata'][relative] = json.loads(safe_path(site, relative).read_text())
    for pattern in ('branches/*', 'demos/build/*'):
        for path in sorted(site.glob(pattern)):
            relative = path.relative_to(site).as_posix()
            if safe_path(site, relative).is_dir():
                state['branchDirs'].append(relative)
    for pattern in ('branches/*/builds/*', 'demos/build/*/*'):
        for path in sorted(site.glob(pattern)):
            relative = path.relative_to(site).as_posix()
            if safe_path(site, relative).is_dir():
                state['buildDirs'].append(relative)
    if demos:
        apps = site / 'demos/apps'
        manifest = apps / 'demos.json'
        if not manifest.is_file():
            raise ValueError('Published demo manifest is missing: ' + str(manifest))
        state['demoManifest'] = json.loads(manifest.read_text())
        entries = state['demoManifest']
        if isinstance(entries, dict):
            entries = entries.get('demos', entries)
        if isinstance(entries, list):
            selected = {entry.get('zip') for entry in entries if isinstance(entry, dict)}
        else:
            selected = {name for name, entry in entries.items() if isinstance(entry, dict)} if isinstance(entries, dict) else set()
        state['archives'] = []
        for path in sorted(apps.glob('*.zip'), key=lambda p: p.name.lower()):
            if path.name not in selected or path.name.lower().startswith(('boxedwine', 'tinycore')):
                continue
            with zipfile.ZipFile(path) as archive:
                names = archive.namelist()
            item = {'zip': path.name, 'program': program_from_names(names, path.stem),
                    'zipParam': 'overlay' if any(name.startswith('home/username/.wine/drive_c/') for name in names) else 'app'}
            for image in (apps / 'images' / (path.stem + '.png'), site / 'demos/images' / (path.stem + '.png')):
                if image.is_file():
                    item['imagePath'] = image.relative_to(site).as_posix()
                    item['imageVersion'] = hashlib.sha256(image.read_bytes()).hexdigest()[:12]
                    break
            state['archives'].append(item)
    return state


def prepare(state, site):
    if state['schemaVersion'] != 1:
        raise ValueError('Unsupported site snapshot')
    for relative in state['branchDirs'] + state['buildDirs']:
        safe_path(site, relative).mkdir(parents=True, exist_ok=True)
    for relative, content in state['metadata'].items():
        path = safe_path(site, relative)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(content, indent=2) + '\n', encoding='utf-8')


def retired_paths(state, site):
    retired = []
    for relative in state['branchDirs'] + state['buildDirs']:
        if not safe_path(site, relative).exists() and not any(relative.startswith(p + '/') for p in retired):
            retired.append(relative)
    return retired


def prune(site, paths):
    # Only whole branch/build directories can be retired, never shared assets,
    # arbitrary paths, the site itself, or paths reached through symlinks.
    for relative in paths:
        parts = PurePosixPath(relative).parts
        allowed = (len(parts) == 2 and parts[0] == 'branches' or
                   len(parts) == 4 and parts[0] == 'branches' and parts[2] == 'builds' or
                   len(parts) in (3, 4) and parts[:2] == ('demos', 'build'))
        if not allowed:
            raise ValueError('Not a branch/build directory: ' + relative)
        safe_path(site, relative)
    for relative in paths:
        path = safe_path(site, relative)
        if path.is_dir():
            shutil.rmtree(path)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=('snapshot', 'prepare', 'plan', 'prune'))
    parser.add_argument('--site', type=Path, required=True)
    parser.add_argument('--state', type=Path)
    parser.add_argument('--demos', action='store_true')
    parser.add_argument('--paths-base64')
    args = parser.parse_args()
    if args.action == 'snapshot':
        print(json.dumps(snapshot(args.site, args.demos)))
    elif args.action == 'prune':
        prune(args.site, json.loads(base64.b64decode(args.paths_base64, validate=True)))
    else:
        state = json.loads(args.state.read_text())
        if args.action == 'prepare':
            prepare(state, args.site)
        else:
            print(base64.b64encode(json.dumps(retired_paths(state, args.site)).encode()).decode())


if __name__ == '__main__':
    main()
