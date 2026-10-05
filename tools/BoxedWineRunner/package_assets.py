#!/usr/bin/env python3
"""Repackage native automation assets with current launchers and no filesystem."""
import argparse
import copy
import hashlib
import json
from pathlib import Path, PurePosixPath
import shutil
import zipfile

HERE = Path(__file__).resolve().parent


def sha256(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def member_sha256(archive, name):
    with archive.open(name) as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def package(source, output, runner):
    if output.exists():
        raise ValueError(f'Output already exists: {output}')
    replacements = {'automation/' + path.name: path.read_bytes()
                    for path in sorted((HERE / 'bundle').iterdir()) if path.is_file()}
    replacements['automation/bin/BoxedWineRunner.jar'] = runner.read_bytes()
    replacements['automation/filesystem.properties'] = (HERE / 'filesystem.properties').read_bytes()
    for path in sorted((HERE / 'overrides').rglob('*')):
        if path.is_file():
            replacements['automation/scripts/' + path.relative_to(HERE / 'overrides').as_posix()] = path.read_bytes()
    pin = dict(line.split('=', 1) for line in (HERE / 'filesystem.properties').read_text().splitlines()
               if line and not line.startswith('#'))
    copied = {}
    removed = []
    with zipfile.ZipFile(source) as original, zipfile.ZipFile(output, 'x') as result:
        names = original.namelist()
        if len(names) != len(set(names)):
            raise ValueError('Source contains duplicate ZIP members')
        for entry in original.infolist():
            name = entry.filename
            if not name.startswith('automation/') or '..' in PurePosixPath(name).parts or '\\' in name:
                raise ValueError(f'Unexpected source path: {name}')
            if name.startswith('automation/fs/'):
                removed.append(name)
                continue
            target = name
            if name in ('automation/README-Wine11.txt', 'automation/validation.json'):
                target = f'automation/provenance/{source.stem}-{PurePosixPath(name).name}'
                if target in names:
                    raise ValueError(f'Provenance path already exists: {target}')
            elif name in replacements:
                continue
            metadata = copy.copy(entry)
            metadata.filename = target
            with original.open(entry) as src, result.open(metadata, 'w') as dst:
                shutil.copyfileobj(src, dst)
            copied[target] = name
        for name, content in replacements.items():
            if name.endswith('.bat'):
                content = content.replace(b'\r\n', b'\n').replace(b'\n', b'\r\n')
            result.writestr(name, content, compress_type=zipfile.ZIP_DEFLATED)
        report = dict(source=source.name, source_sha256=sha256(source), filesystem=pin,
                      embedded_filesystem=False, removed=removed,
                      updated_or_added=sorted(replacements), preserved_entries=len(copied),
                      qualification='Packaging only; earlier runtime results are under provenance/')
        result.writestr('automation/validation.json', json.dumps(report, indent=2) + '\n',
                        compress_type=zipfile.ZIP_DEFLATED)
    with zipfile.ZipFile(source) as original, zipfile.ZipFile(output) as result:
        assert not any(name.startswith('automation/fs/') for name in result.namelist())
        assert result.testzip() is None
        for target, name in copied.items():
            assert member_sha256(original, name) == member_sha256(result, target), name
        for name, content in replacements.items():
            if name.endswith('.bat'):
                content = content.replace(b'\r\n', b'\n').replace(b'\n', b'\r\n')
            assert result.read(name) == content, name
    report.update(output=output.name, bytes=output.stat().st_size, sha256=sha256(output),
                  preserved_content_verified=True, crc_verified=True)
    output.with_suffix(output.suffix + '.sha256').write_text(report['sha256'] + '  ' + output.name + '\n')
    output.with_suffix(output.suffix + '.validation.json').write_text(json.dumps(report, indent=2) + '\n')
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--runner', type=Path, required=True, help='JAR built with Build.java')
    args = parser.parse_args()
    print(json.dumps(package(args.source, args.output, args.runner), indent=2))
