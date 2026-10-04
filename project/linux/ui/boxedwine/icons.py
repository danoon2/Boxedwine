# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Bounded PE/NE icon-resource reader, matching the native Mac frontend."""
import struct
from .files import no_links


def extract(path):
    """Return ICO bytes or None. Never execute or map the Windows program."""
    try:
        with no_links(path).open('rb') as stream:
            return Reader(stream).extract()
    except (OSError, ValueError, KeyError, StopIteration, struct.error):
        return None


class Reader:
    def __init__(self, stream):
        self.stream = stream
        stream.seek(0, 2); self.size = stream.tell()
        self.remaining = 8 * 1024**2
        self.entries_remaining = 16384

    def read(self, offset, size):
        if offset < 0 or size < 0 or offset + size > self.size or size > 2 * 1024**2 or size > self.remaining:
            raise ValueError('Invalid icon resource range')
        self.remaining -= size
        self.stream.seek(offset); value = self.stream.read(size)
        if len(value) != size:
            raise ValueError('Icon resource changed')
        return value

    def word(self, offset):
        return struct.unpack('<H', self.read(offset, 2))[0]

    def dword(self, offset):
        return struct.unpack('<I', self.read(offset, 4))[0]

    def group(self, data, image_data):
        reserved, kind, count = struct.unpack_from('<HHH', data)
        if reserved or kind != 1 or not 0 < count <= 64 or len(data) < 6 + count * 14:
            raise ValueError('Invalid icon group')
        images = []
        for i in range(count):
            header = bytearray(data[6 + i * 14:18 + i * 14])
            size, identifier = struct.unpack_from('<IH', data, 14 + i * 14)
            try:
                image = image_data(identifier, size)
                if len(image) != size:
                    continue
                width, height = header[0] or 256, header[1] or 256
                if image.startswith(b'\x89PNG\r\n\x1a\n'):
                    if len(image) < 24 or struct.unpack_from('>II', image, 16) != (width, height):
                        continue
                else:
                    dib, dib_width, dib_height = struct.unpack_from('<Iii', image)
                    if dib < 40 or dib > len(image) or dib_width != width or abs(dib_height) % 2:
                        continue
                    # Some Win16 groups include the mask in their height.
                    actual_height = abs(dib_height) // 2
                    if actual_height not in (height, height // 2) or not 1 <= actual_height <= 256:
                        continue
                    header[1] = actual_height % 256
                images.append((bytes(header), image))
            except (ValueError, KeyError, StopIteration, struct.error):
                continue
        if not images or sum(len(i) for _, i in images) > 4 * 1024**2:
            raise ValueError('No usable icon images')
        output = bytearray(struct.pack('<HHH', 0, 1, len(images)))
        offset = 6 + 16 * len(images)
        for header, image in images:
            output.extend(header + struct.pack('<I', offset)); offset += len(image)
        return bytes(output) + b''.join(image for _, image in images)

    def extract(self):
        if self.word(0) != 0x5a4d:
            raise ValueError('Not a Windows executable')
        header = self.dword(0x3c)
        if self.word(header) == 0x454e:
            return self.ne(header)
        if self.dword(header) != 0x4550:
            raise ValueError('Unknown Windows executable')
        count, optional_size = self.word(header + 6), self.word(header + 20)
        optional = header + 24
        magic = self.word(optional)
        if not 0 < count <= 96 or magic not in (0x10b, 0x20b):
            raise ValueError('Invalid PE header')
        directories = 96 if magic == 0x10b else 112
        if optional_size < directories + 24 or self.dword(optional + directories - 4) < 3:
            raise ValueError('Missing PE resources')
        resource_rva, resource_size = self.dword(optional + directories + 16), self.dword(optional + directories + 20)
        sections = []
        for i in range(count):
            section = optional + optional_size + i * 40
            sections.append((self.dword(section + 12), self.dword(section + 20), self.dword(section + 16)))
        def file_offset(rva, length):
            for start, offset, size in sections:
                if start <= rva <= start + size and length <= size - (rva - start):
                    return offset + rva - start
            raise ValueError('Invalid PE resource address')
        resource = file_offset(resource_rva, resource_size)
        def address(offset, size):
            if offset < 0 or offset + size > resource_size:
                raise ValueError('Invalid resource directory')
            return resource + offset
        def entries(offset):
            table = address(offset, 16)
            total = self.word(table + 12) + self.word(table + 14)
            if total > 4096 or total > self.entries_remaining:
                raise ValueError('Too many icon resource entries')
            self.entries_remaining -= total
            data = self.read(address(offset + 16, total * 8), total * 8)
            return [(identifier, target & 0x7fffffff, bool(target & 0x80000000)) for identifier, target in struct.iter_unpack('<II', data)]
        def payload(entry):
            if entry[2]:
                raise ValueError('Expected icon payload')
            record = address(entry[1], 16)
            rva, size = self.dword(record), self.dword(record + 4)
            return self.read(file_offset(rva, size), size)
        types = entries(0)
        groups = next(e for e in types if e[0] == 14 and e[2])
        icons = entries(next(e for e in types if e[0] == 3 and e[2])[1])
        for group in entries(groups[1])[:32]:
            if not group[2]:
                continue
            for language in entries(group[1])[:16]:
                if language[2]:
                    continue
                def image(identifier, size):
                    icon = next(e for e in icons if e[0] == identifier and e[2])
                    candidates = sorted(entries(icon[1]), key=lambda e: e[0] != language[0])
                    return payload(next(e for e in candidates if not e[2]))
                try:
                    return self.group(payload(language), image)
                except (ValueError, StopIteration, struct.error):
                    continue
        raise ValueError('No icon group')

    def ne(self, header):
        start, end = self.word(header + 0x24), self.word(header + 0x26)
        if start < 64 or end < start + 4 or self.read(header + 0x36, 1)[0] not in (2, 4):
            raise ValueError('Invalid NE header')
        data = self.read(header + start, end - start)
        shift = struct.unpack_from('<H', data)[0]
        if shift > 31:
            raise ValueError('Invalid NE alignment')
        groups, icons, cursor = [], {}, 2
        while True:
            kind = struct.unpack_from('<H', data, cursor)[0]
            if kind == 0:
                break
            count = struct.unpack_from('<H', data, cursor + 2)[0]
            if count > 4096 or count > self.entries_remaining:
                raise ValueError('Too many NE resources')
            self.entries_remaining -= count
            cursor += 8
            for i in range(count):
                offset, size, _, identifier, _, _ = struct.unpack_from('<HHHHHH', data, cursor + i * 12)
                if kind == 0x800e:
                    groups.append((offset << shift, size << shift))
                elif kind == 0x8003:
                    if identifier in icons:
                        raise ValueError('Duplicate NE icon')
                    icons[identifier] = (offset << shift, size << shift)
            cursor += count * 12
        def image(identifier, size):
            offset, available = icons[identifier | 0x8000]
            if not 0 < size <= available:
                raise ValueError('Invalid NE payload')
            return self.read(offset, size)
        for offset, size in groups[:32]:
            try:
                return self.group(self.read(offset, size), image)
            except (ValueError, KeyError, struct.error):
                continue
        raise ValueError('No NE icon')
