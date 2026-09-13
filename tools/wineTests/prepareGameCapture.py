#!/usr/bin/env python3
"""Verify a local game capture launch and write a new configuration with content identity.

This prepares inputs only. It does not launch a browser or establish game acceptance.
"""
from __future__ import annotations

import argparse
import copy
import json
from pathlib import Path
import re
import sys
import zipfile
from urllib.parse import parse_qsl, unquote, urlsplit, urlunsplit

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / 'tools/buildWine'))
import webgl_build_identity as identity


def inside(site: Path, path: Path) -> Path:
    path = path.resolve(strict=True)
    if not path.is_relative_to(site) or not path.is_file():
        raise ValueError(f'Launch input must be a file inside the site: {path}')
    return path


def prepare(config: dict, *, mode: str, commit: str | None = None,
            source_dirty: bool | None = None) -> dict:
    """Bind the supported local app/overlay launch to every supplied input hash.

    Base URL overrides, inline payloads and multiple overlays need their own
    resolver. Reject them here instead of identifying a different set of bytes.
    """
    result = copy.deepcopy(config)
    site = Path(config['site']).resolve(strict=True)
    launch = urlsplit(config['launchPath'])
    if (launch.scheme or launch.netloc or launch.fragment or not launch.path.startswith('/')
            or launch.path.startswith('//') or '\\' in launch.path):
        raise ValueError('Capture launch must be a local absolute URL path without a fragment')
    pairs = parse_qsl(launch.query, keep_blank_values=True, strict_parsing=True)
    raw_pairs = [part.split('=', 1) for part in launch.query.split('&')]
    if any(raw[0] != decoded[0] for raw, decoded in zip(raw_pairs, pairs)):
        raise ValueError('Capture parameter names must not be encoded')
    params = dict(pairs)
    if len(params) != len(pairs):
        raise ValueError('Duplicate launch parameters are ambiguous')
    allowed = {'root', 'app', 'overlay', 'p', 'args', 'storage', 'regressionBuild',
               'sound', 'auto', 'bpp', 'resolution', 'buildid'}
    if set(params) - allowed:
        raise ValueError('Unsupported capture launch parameters: ' + ', '.join(sorted(set(params) - allowed)))
    if params.get('storage') != 'memory' or dict(raw_pairs).get('storage') != 'memory':
        raise ValueError('Capture must use fresh memory storage')
    apps = [(name, params[name]) for name in ('app', 'overlay') if name in params]
    if len(apps) != 1:
        raise ValueError('Capture requires exactly one app or overlay ZIP')
    for name, value in [('root', params.get('root', '')), *apps]:
        if (not re.fullmatch(r'[A-Za-z0-9_.-]+\.zip', value)
                or dict(raw_pairs).get(name) != value):
            raise ValueError(f'{name} must name one local ZIP without path components')
    decoded = unquote(launch.path)
    if '\\' in decoded or any(part in ('.', '..') for part in decoded.split('/')):
        raise ValueError('Launch path cannot contain traversal components')
    page = inside(site, site / decoded.lstrip('/'))
    if page.name != 'boxedwine.html':
        raise ValueError('Capture launch must select boxedwine.html')
    selected_mode = identity.MODES.get(mode, mode)
    if identity.MODES.get(page.parent.name) != selected_mode:
        raise ValueError('Capture mode does not match the launch directory')

    pinned = {}
    for entry in config['inputs']:
        path = inside(site, Path(entry['path']))
        if path in pinned:
            raise ValueError(f'Duplicate capture input: {path}')
        actual = identity.file_identity(path)
        if actual['sha256'] != entry['sha256'] or ('bytes' in entry and actual['bytes'] != entry['bytes']):
            raise ValueError(f'Capture input changed: {path}')
        pinned[path] = actual
    runtime_paths = {name: inside(site, page.parent / name) for name in identity.RUNTIME_FILES}
    root = inside(site, page.parent / params['root'])
    app = inside(site, page.parent / apps[0][1])
    required = {*runtime_paths.values(), root, app}
    if set(pinned) != required:
        raise ValueError('Capture input pins must match exactly the five runtime files and launched root/app ZIPs')
    runtime = {name: pinned[path] for name, path in runtime_paths.items()}
    filesystem = identity.root_identity(root)
    if filesystem['archive'] != pinned[root]:
        raise ValueError('Root changed while preparing capture')
    build = identity.build_identity(mode=mode, runtime=runtime, filesystem=filesystem,
                                    commit=commit, source_dirty=source_dirty)
    # The app is a run input, not part of the shared runtime/root build identity.
    result['buildIdentity'] = build
    result['buildId'] = build['id']
    result['applicationIdentity'] = {'kind': apps[0][0], 'filename': apps[0][1], **pinned[app]}
    result['inputs'] = [{'path': str(path), **pinned[path]} for path in sorted(required)]
    # Preserve argument values and ordering; replace any previous build label.
    query = '&'.join(part for part in launch.query.split('&') if not part.startswith('buildid='))
    result['launchPath'] = urlunsplit(('', '', launch.path, query + '&buildid=' + build['id'], ''))
    result['site'] = str(site)
    result['preparation'] = {
        'scope': 'Verified local launch inputs; no browser execution or game acceptance inferred.',
        'source_commit_policy': 'Explicit caller-supplied build label; existing binary provenance is not inferred from the checkout.',
        'tools': [{'path': str(path), **identity.file_identity(path)}
                  for path in (Path(__file__).resolve(), Path(identity.__file__).resolve())]}
    # Detect changes during the complete preflight, including the ZIP DLL read.
    for path, expected in pinned.items():
        if identity.file_identity(path) != expected:
            raise ValueError(f'Capture input changed during preparation: {path}')
    return result


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('config', type=Path)
    parser.add_argument('--output', type=Path, required=True, help='new prepared configuration file')
    parser.add_argument('--mode', choices=identity.MODES, required=True)
    parser.add_argument('--commit', help='explicit full commit label for these binaries; default unknown')
    parser.add_argument('--source-dirty', choices=('true', 'false', 'unknown'), default='unknown')
    args = parser.parse_args(argv)
    try:
        original = args.config.read_bytes()
        result = prepare(json.loads(original), mode=args.mode, commit=args.commit,
                         source_dirty={'true': True, 'false': False, 'unknown': None}[args.source_dirty])
        result['preparation']['source_config'] = {
            'path': str(args.config.resolve()), **identity.file_identity(args.config)}
        if args.config.read_bytes() != original:
            raise ValueError('Source configuration changed during preparation')
        with args.output.open('x', encoding='utf-8', newline='\n') as stream:
            json.dump(result, stream, indent=2)
            stream.write('\n')
    except (OSError, ValueError, KeyError, zipfile.BadZipFile) as error:
        print('ERROR: ' + str(error), file=sys.stderr)
        return 2
    print(result['buildId'])
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
