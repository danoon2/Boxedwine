// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import CoreGraphics
import ImageIO

/// Reads resource bytes only; never loads or executes Windows code.
/// Formats: Windows NE/PE executables and RT_GROUP_ICON / RT_ICON resources.
/// https://learn.microsoft.com/windows/win32/debug/pe-format
/// https://devblogs.microsoft.com/oldnewthing/20120720-00/?p=7083
struct WindowsIcon {
    private enum ParseError: Error { case invalid }
    private let bytes: Data

    static func read(from url: URL) -> Data? {
        let limit = 64 * 1024 * 1024
        guard let values = try? url.resourceValues(forKeys: [.isRegularFileKey, .fileSizeKey]),
              values.isRegularFile == true, let size = values.fileSize, size <= limit,
              let file = try? FileHandle(forReadingFrom: url) else { return nil }
        defer { try? file.close() }
        // Avoid mapping a file that an installer could replace or truncate.
        guard let data = try? file.read(upToCount: limit + 1), data.count <= limit else { return nil }
        return extract(from: data)
    }

    static func extract(from data: Data) -> Data? {
        // Normalize Data slices so all offsets are relative to zero.
        try? WindowsIcon(bytes: Data(data)).extract()
    }

    /// Choose by original pixel area before reducing the image for native UI.
    /// Equal sizes keep resource order; an unreadable image falls back to the
    /// next largest. Both the library and Dock use this selection.
    static func thumbnail(from data: Data) -> CGImage? {
        guard let source = CGImageSourceCreateWithData(data as CFData, nil) else { return nil }
        var candidates: [(index: Int, pixels: Int)] = []
        for index in 0..<min(CGImageSourceGetCount(source), 64) {
            guard let properties = CGImageSourceCopyPropertiesAtIndex(source, index, nil) as? [CFString: Any],
                  let width = (properties[kCGImagePropertyPixelWidth] as? NSNumber)?.intValue,
                  let height = (properties[kCGImagePropertyPixelHeight] as? NSNumber)?.intValue,
                  (1...256).contains(width), (1...256).contains(height) else { continue }
            candidates.append((index, width * height))
        }
        candidates.sort { $0.pixels == $1.pixels ? $0.index < $1.index : $0.pixels > $1.pixels }
        let options: [CFString: Any] = [kCGImageSourceCreateThumbnailFromImageAlways: true,
                                      kCGImageSourceThumbnailMaxPixelSize: 128,
                                      kCGImageSourceCreateThumbnailWithTransform: true]
        for candidate in candidates {
            if let image = CGImageSourceCreateThumbnailAtIndex(source, candidate.index, options as CFDictionary) { return image }
        }
        return nil
    }

    /// A small, square PNG for a running app's Dock icon. Uses the same decoded
    /// resource and square presentation as the library, including legacy icons.
    static func dockPNG(from data: Data) -> Data? {
        guard let best = thumbnail(from: data), let space = CGColorSpace(name: CGColorSpace.sRGB),
              let context = CGContext(data: nil, width: 128, height: 128, bitsPerComponent: 8,
                                      bytesPerRow: 128 * 4, space: space,
                                      bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue) else { return nil }
        context.interpolationQuality = .high
        context.draw(best, in: CGRect(x: 0, y: 0, width: 128, height: 128))
        guard let image = context.makeImage() else { return nil }
        let png = NSMutableData()
        guard let destination = CGImageDestinationCreateWithData(png, "public.png" as CFString, 1, nil) else { return nil }
        CGImageDestinationAddImage(destination, image, nil)
        guard CGImageDestinationFinalize(destination) else { return nil }
        return png as Data
    }

    private func range(_ offset: Int, _ length: Int) throws -> Range<Int> {
        guard offset >= 0, length >= 0, offset <= bytes.count, length <= bytes.count - offset else { throw ParseError.invalid }
        return offset..<(offset + length)
    }

    private func word(_ offset: Int) throws -> Int {
        _ = try range(offset, 2)
        return Int(bytes[offset]) | Int(bytes[offset + 1]) << 8
    }

    private func dword(_ offset: Int) throws -> Int {
        _ = try range(offset, 4)
        return (0..<4).reduce(0) { $0 | Int(bytes[offset + $1]) << ($1 * 8) }
    }

    private struct Section {
        let rva: Int, offset: Int, size: Int
    }

    private struct Entry {
        let id: Int, offset: Int, directory: Bool
    }

    private func extract() throws -> Data {
        guard try word(0) == 0x5a4d else { throw ParseError.invalid }
        let header = try dword(0x3c)
        if try word(header) == 0x454e { return try extractNE(header) }
        return try extractPE(header)
    }

    private struct NEResource {
        let id: Int, offset: Int, size: Int
    }

    /// NE uses a flat table of 16-bit resource records. Offsets AND lengths
    /// are in alignment units; the icon group gives each image's exact size.
    /// See the original Windows resource-table layout in source/ui/utils/readIcons.cpp.
    private func extractNE(_ header: Int) throws -> Data {
        _ = try range(header, 64)
        guard [2, 4].contains(bytes[header + 0x36]) else { throw ParseError.invalid } // Windows / Windows 386
        let start = try word(header + 0x24), end = try word(header + 0x26)
        guard start >= 64, end >= start + 4 else { throw ParseError.invalid }
        let table = header + start, tableSize = end - start
        _ = try range(table, tableSize)
        func tableRange(_ offset: Int, _ length: Int) throws {
            guard offset >= 0, length >= 0, offset <= tableSize, length <= tableSize - offset else { throw ParseError.invalid }
        }
        let shift = try word(table)
        guard shift <= 31 else { throw ParseError.invalid }
        var groups: [NEResource] = [], icons: [Int: NEResource] = [:]
        var cursor = 2, remainingEntries = 16_384
        while true {
            try tableRange(cursor, 2)
            let type = try word(table + cursor)
            if type == 0 { break }
            try tableRange(cursor, 8)
            let count = try word(table + cursor + 2)
            guard count <= 4096, count <= remainingEntries else { throw ParseError.invalid }
            remainingEntries -= count
            cursor += 8
            try tableRange(cursor, count * 12)
            if type == 0x800e || type == 0x8003 {
                for index in 0..<count {
                    let item = table + cursor + index * 12
                    let record = NEResource(id: try word(item + 6), offset: try word(item) << shift,
                                            size: try word(item + 2) << shift)
                    if type == 0x800e { groups.append(record) }
                    else if record.id & 0x8000 != 0 {
                        guard icons[record.id] == nil else { throw ParseError.invalid }
                        icons[record.id] = record
                    }
                }
            }
            cursor += count * 12
        }
        var remainingPayload = 8 * 1024 * 1024
        func payload(_ record: NEResource, size: Int? = nil) throws -> Data {
            _ = try range(record.offset, record.size)
            let length = size ?? record.size
            guard length > 0, length <= record.size, length <= 2 * 1024 * 1024,
                  length <= remainingPayload else { throw ParseError.invalid }
            remainingPayload -= length
            return bytes.subdata(in: record.offset..<(record.offset + length))
        }
        // A group name may be numeric or a string-table offset; either works
        // because the library uses resource order, not a requested group name.
        for group in groups.prefix(32) {
            guard let groupData = try? payload(group) else { continue }
            if let result = try? iconGroup(groupData, imageData: { id, size in
                guard (1...0x7fff).contains(id), let icon = icons[id | 0x8000] else { return nil }
                return try? payload(icon, size: size)
            }) { return result }
        }
        throw ParseError.invalid
    }

    private func extractPE(_ pe: Int) throws -> Data {
        guard try dword(pe) == 0x4550 else { throw ParseError.invalid }
        let count = try word(pe + 6)
        let optionalSize = try word(pe + 20)
        let optional = pe + 24
        let magic = try word(optional)
        guard count > 0, count <= 96, magic == 0x10b || magic == 0x20b else { throw ParseError.invalid }
        let directories = magic == 0x10b ? 96 : 112
        guard optionalSize >= directories + 24, try dword(optional + directories - 4) >= 3 else { throw ParseError.invalid }
        _ = try range(optional, optionalSize + count * 40)
        let resourceRVA = try dword(optional + directories + 16)
        let resourceSize = try dword(optional + directories + 20)
        guard resourceRVA != 0, resourceSize >= 16 else { throw ParseError.invalid }
        var sections: [Section] = []
        for index in 0..<count {
            let section = optional + optionalSize + index * 40
            sections.append(Section(rva: try dword(section + 12), offset: try dword(section + 20), size: try dword(section + 16)))
        }
        func fileOffset(_ rva: Int, length: Int) throws -> Int {
            for section in sections where rva >= section.rva {
                let delta = rva - section.rva
                if delta <= section.size, length <= section.size - delta {
                    _ = try range(section.offset + delta, length)
                    return section.offset + delta
                }
            }
            throw ParseError.invalid
        }
        let resource = try fileOffset(resourceRVA, length: resourceSize)
        var remainingEntries = 16_384
        var remainingPayload = 8 * 1024 * 1024
        func resourceRange(_ offset: Int, _ length: Int) throws -> Int {
            guard offset >= 0, offset <= resourceSize, length <= resourceSize - offset else { throw ParseError.invalid }
            _ = try range(resource + offset, length)
            return resource + offset
        }
        func entries(_ offset: Int) throws -> [Entry] {
            let table = try resourceRange(offset, 16)
            let total = try word(table + 12) + word(table + 14)
            guard total <= 4096, total <= remainingEntries else { throw ParseError.invalid }
            remainingEntries -= total
            _ = try resourceRange(offset + 16, total * 8)
            return try (0..<total).map { index in
                let item = table + 16 + index * 8
                let id = try dword(item)
                let target = try dword(item + 4)
                return Entry(id: id, offset: target & 0x7fffffff, directory: target & 0x80000000 != 0)
            }
        }
        func payload(_ entry: Entry) throws -> Data {
            guard !entry.directory else { throw ParseError.invalid }
            let header = try resourceRange(entry.offset, 16)
            let rva = try dword(header)
            let size = try dword(header + 4)
            guard size > 0, size <= 2 * 1024 * 1024, size <= remainingPayload else { throw ParseError.invalid }
            remainingPayload -= size
            let start = try fileOffset(rva, length: size)
            return bytes.subdata(in: start..<(start + size))
        }
        let types = try entries(0)
        guard let groups = types.first(where: { $0.id == 14 && $0.directory }),
              let icons = types.first(where: { $0.id == 3 && $0.directory }) else { throw ParseError.invalid }
        let iconEntries = try entries(icons.offset)
        // Fixed type / name / language depth prevents cycles in malformed trees.
        // Prefer the first icon group in resource order, as Windows shells do.
        for group in try entries(groups.offset).prefix(32) where group.directory {
            for language in try entries(group.offset).prefix(16) where !language.directory {
                guard let groupData = try? payload(language) else { continue }
                if let result = try? iconGroup(groupData, imageData: { id, size in
                    guard let icon = iconEntries.first(where: { $0.id == id && $0.directory }),
                          let languages = try? entries(icon.offset) else { return nil }
                    let candidates = languages.sorted { ($0.id == language.id ? 0 : 1) < ($1.id == language.id ? 0 : 1) }
                    guard let image = candidates.lazy.filter({ !$0.directory }).compactMap({ try? payload($0) }).first,
                          image.count == size else { return nil }
                    return image
                }) { return result }
            }
        }
        throw ParseError.invalid
    }

    /// Share decoding, legacy height repair and largest-image selection between
    /// NE and PE; only their resource-table addressing differs.
    private func iconGroup(_ groupData: Data, imageData: (Int, Int) -> Data?) throws -> Data {
        let reader = WindowsIcon(bytes: groupData)
        guard try reader.word(0) == 0, try reader.word(2) == 1 else { throw ParseError.invalid }
        let imageCount = try reader.word(4)
        guard imageCount > 0, imageCount <= 64 else { throw ParseError.invalid }
        _ = try reader.range(6, imageCount * 14)
        var images: [(header: Data, data: Data)] = []
        for index in 0..<imageCount {
            let item = 6 + index * 14
            let id = try reader.word(item + 12), size = try reader.dword(item + 8)
            let width = groupData[item] == 0 ? 256 : Int(groupData[item])
            let declaredHeight = groupData[item + 1] == 0 ? 256 : Int(groupData[item + 1])
            guard size > 0, size <= 2 * 1024 * 1024, let image = imageData(id, size), image.count == size,
                  let height = imageHeight(image, width: width, height: declaredHeight),
                  let display = try? displayImage(image, width: width, height: height) else { continue }
            var header = groupData.subdata(in: item..<(item + 12))
            header[1] = height == 256 ? 0 : UInt8(height)
            if display.converted {
                header[2] = 0 // PNG contains full-color pixels and alpha.
                header[3] = 0
                header[4] = 1; header[5] = 0
                header[6] = 32; header[7] = 0
                for byte in 0..<4 { header[8 + byte] = UInt8(truncatingIfNeeded: display.data.count >> (byte * 8)) }
            }
            images.append((header, display.data))
        }
        guard !images.isEmpty, images.reduce(0, { $0 + $1.data.count }) <= 4 * 1024 * 1024 else { throw ParseError.invalid }
        var result = Data([0, 0, 1, 0, UInt8(images.count), 0])
        var offset = 6 + images.count * 16
        for image in images {
            result.append(image.header)
            result.append(contentsOf: (0..<4).map { UInt8((offset >> ($0 * 8)) & 255) })
            offset += image.data.count
        }
        for image in images { result.append(image.data) }
        return result
    }

    private func imageHeight(_ data: Data, width: Int, height: Int) -> Int? {
        let pngSignature = Data([137, 80, 78, 71, 13, 10, 26, 10])
        if data.starts(with: pngSignature) {
            guard data.count >= 24, data.subdata(in: 12..<16) == Data("IHDR".utf8) else { return nil }
            let pngWidth = data[16..<20].reduce(0) { ($0 << 8) | Int($1) }
            let pngHeight = data[20..<24].reduce(0) { ($0 << 8) | Int($1) }
            return pngWidth == width && pngHeight == height ? height : nil
        }
        let reader = WindowsIcon(bytes: data)
        guard let headerSize = try? reader.dword(0), headerSize >= 40, headerSize <= data.count,
              (try? reader.dword(4)) == width, let bitmapHeight = try? reader.dword(8),
              (2...512).contains(bitmapHeight), bitmapHeight.isMultiple(of: 2),
              bitmapHeight == height * 2 || bitmapHeight == height else { return nil }
        // A DIB includes both the image and its mask. Some legacy groups (such
        // as Age of Empires) mistakenly repeat that combined height. Normalize
        // only this known mismatch in the generated ICO directory, leaving the
        // executable and bitmap bytes intact; unrelated mismatches stay invalid.
        // https://devblogs.microsoft.com/oldnewthing/20101018-00/?p=12513
        return bitmapHeight / 2
    }

    private func displayImage(_ data: Data, width: Int, height: Int) throws -> (data: Data, converted: Bool) {
        let reader = WindowsIcon(bytes: data)
        // ImageIO misreads the padded AND mask of some indexed ICO bitmaps
        // (including the 20-pixel-wide Age of Empires icon). Decode those rows
        // explicitly and embed lossless PNG so native drawing uses exact alpha.
        guard !data.starts(with: [137, 80, 78, 71, 13, 10, 26, 10]),
              let depth = try? reader.word(14), [1, 4, 8].contains(depth),
              (try? reader.dword(16)) == 0 else { return (data, false) }
        guard try reader.word(12) == 1 else { throw ParseError.invalid }
        let headerSize = try reader.dword(0)
        let usedColors = try reader.dword(32)
        let colors = usedColors == 0 ? 1 << depth : usedColors
        guard colors > 0, colors <= 1 << depth else { throw ParseError.invalid }
        let pixelStride = ((width * depth + 31) / 32) * 4
        let maskStride = ((width + 31) / 32) * 4
        let pixels = headerSize + colors * 4
        let mask = pixels + pixelStride * height
        _ = try reader.range(headerSize, colors * 4 + (pixelStride + maskStride) * height)
        var rgba = Data(repeating: 0, count: width * height * 4)
        for y in 0..<height {
            let row = height - 1 - y // DIB color and mask rows are bottom-up.
            for x in 0..<width {
                let bit = x * depth
                let index = Int(data[pixels + row * pixelStride + bit / 8] >> (8 - depth - bit % 8)) & ((1 << depth) - 1)
                guard index < colors else { throw ParseError.invalid }
                if data[mask + row * maskStride + x / 8] & (1 << (7 - x % 8)) != 0 { continue }
                let color = headerSize + index * 4
                let target = (y * width + x) * 4
                rgba[target] = data[color + 2]
                rgba[target + 1] = data[color + 1]
                rgba[target + 2] = data[color]
                rgba[target + 3] = 255
            }
        }
        guard let space = CGColorSpace(name: CGColorSpace.sRGB),
              let provider = CGDataProvider(data: rgba as CFData),
              let image = CGImage(width: width, height: height, bitsPerComponent: 8, bitsPerPixel: 32,
                                  bytesPerRow: width * 4, space: space,
                                  bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.premultipliedLast.rawValue),
                                  provider: provider, decode: nil, shouldInterpolate: false, intent: .defaultIntent) else { throw ParseError.invalid }
        let png = NSMutableData()
        guard let destination = CGImageDestinationCreateWithData(png, "public.png" as CFString, 1, nil) else { throw ParseError.invalid }
        CGImageDestinationAddImage(destination, image, nil)
        guard CGImageDestinationFinalize(destination) else { throw ParseError.invalid }
        return (png as Data, true)
    }
}

enum AppIconError: LocalizedError {
    case invalid, tooLarge
    var errorDescription: String? {
        switch self {
        case .invalid: "Choose a readable image, such as a PNG, JPEG, or icon file."
        case .tooLarge: "Choose an image smaller than 16 MB and no larger than 64 megapixels."
        }
    }
}

/// A bounded, self-contained image saved with the app's settings. Normalizing
/// once avoids retaining external-file access and makes Save/Cancel atomic.
enum CustomAppIcon {
    static let side = 256
    static let maximumBytes = 300 * 1024
    private static let maximumSourceBytes = 16 * 1024 * 1024

    static func read(_ url: URL) throws -> Data {
        let values = try url.resourceValues(forKeys: [.isRegularFileKey, .fileSizeKey])
        guard values.isRegularFile == true else { throw AppIconError.invalid }
        guard let size = values.fileSize, size <= maximumSourceBytes else { throw AppIconError.tooLarge }
        let file = try FileHandle(forReadingFrom: url)
        defer { try? file.close() }
        let data = try file.read(upToCount: maximumSourceBytes + 1) ?? Data()
        return try png(from: data)
    }

    static func png(from data: Data) throws -> Data {
        guard data.count <= maximumSourceBytes else { throw AppIconError.tooLarge }
        guard let source = CGImageSourceCreateWithData(data as CFData, nil) else { throw AppIconError.invalid }
        var candidates: [(index: Int, pixels: Int)] = []
        for index in 0..<min(CGImageSourceGetCount(source), 64) {
            guard let properties = CGImageSourceCopyPropertiesAtIndex(source, index, nil) as? [CFString: Any],
                  let width = (properties[kCGImagePropertyPixelWidth] as? NSNumber)?.intValue,
                  let height = (properties[kCGImagePropertyPixelHeight] as? NSNumber)?.intValue,
                  width > 0, height > 0 else { continue }
            guard width <= 16384, height <= 16384, width * height <= 64 * 1024 * 1024 else { throw AppIconError.tooLarge }
            candidates.append((index, width * height))
        }
        candidates.sort { $0.pixels == $1.pixels ? $0.index < $1.index : $0.pixels > $1.pixels }
        let options: [CFString: Any] = [kCGImageSourceCreateThumbnailFromImageAlways: true,
                                      kCGImageSourceThumbnailMaxPixelSize: side,
                                      kCGImageSourceCreateThumbnailWithTransform: true]
        let best = candidates.lazy.compactMap { CGImageSourceCreateThumbnailAtIndex(source, $0.index, options as CFDictionary) }.first
        guard let best, let space = CGColorSpace(name: CGColorSpace.sRGB),
              let context = CGContext(data: nil, width: side, height: side, bitsPerComponent: 8, bytesPerRow: side * 4,
                                      space: space, bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue) else { throw AppIconError.invalid }
        let scale = CGFloat(side) / CGFloat(max(best.width, best.height))
        let width = CGFloat(best.width) * scale, height = CGFloat(best.height) * scale
        context.interpolationQuality = .high
        context.draw(best, in: CGRect(x: (CGFloat(side) - width) / 2, y: (CGFloat(side) - height) / 2, width: width, height: height))
        guard let image = context.makeImage() else { throw AppIconError.invalid }
        let png = NSMutableData()
        guard let destination = CGImageDestinationCreateWithData(png, "public.png" as CFString, 1, nil) else { throw AppIconError.invalid }
        CGImageDestinationAddImage(destination, image, nil)
        guard CGImageDestinationFinalize(destination) else { throw AppIconError.invalid }
        let result = png as Data
        try validate(result)
        return result
    }

    static func validate(_ data: Data) throws {
        // ImageIO can decode a partial PNG; stored artwork must include IEND.
        guard data.count <= maximumBytes, data.starts(with: [137, 80, 78, 71, 13, 10, 26, 10]),
              data.suffix(12) == Data([0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130]),
              let source = CGImageSourceCreateWithData(data as CFData, nil), CGImageSourceGetCount(source) == 1,
              let properties = CGImageSourceCopyPropertiesAtIndex(source, 0, nil) as? [CFString: Any],
              (properties[kCGImagePropertyPixelWidth] as? NSNumber)?.intValue == side,
              (properties[kCGImagePropertyPixelHeight] as? NSNumber)?.intValue == side,
              CGImageSourceCreateImageAtIndex(source, 0, nil) != nil,
              CGImageSourceGetStatusAtIndex(source, 0) == .statusComplete else { throw AppIconError.invalid }
    }
}

struct AppIconRequest: Hashable, Sendable {
    var executable: URL?
    var customPNG: Data? = nil
    var demoImage: URL? = nil
}

/// Serializes file reads off the main actor and invalidates changed executables.
actor WindowsIconCache {
    static let shared = WindowsIconCache()
    private struct Key: Hashable {
        let url: URL, modified: Date?, size: Int?
    }
    private struct Cached { let data: Data? }
    private var cache: [Key: Cached] = [:]
    private var cachedBytes = 0

    func importCustomIcon(from url: URL) throws -> Data {
        let access = url.startAccessingSecurityScopedResource()
        defer { if access { url.stopAccessingSecurityScopedResource() } }
        return try CustomAppIcon.read(url)
    }

    func appIconData(_ request: AppIconRequest) -> Data? {
        if let custom = request.customPNG, (try? CustomAppIcon.validate(custom)) != nil { return custom }
        if let url = request.executable, let data = iconData(for: url), WindowsIcon.thumbnail(from: data) != nil { return data }
        if let url = request.demoImage { return try? CustomAppIcon.read(url) }
        return nil
    }

    func appDockIconData(_ request: AppIconRequest) -> Data? {
        appIconData(request).flatMap { WindowsIcon.dockPNG(from: $0) }
    }

    func dockIconData(for url: URL) -> Data? {
        iconData(for: url).flatMap { WindowsIcon.dockPNG(from: $0) }
    }

    func iconData(for url: URL) -> Data? {
        var freshURL = url
        freshURL.removeAllCachedResourceValues()
        let values = try? freshURL.resourceValues(forKeys: [.contentModificationDateKey, .fileSizeKey])
        let key = Key(url: url, modified: values?.contentModificationDate, size: values?.fileSize)
        if let cached = cache[key] { return cached.data }
        let data = WindowsIcon.read(from: freshURL)
        if cache.count >= 64 || cachedBytes + (data?.count ?? 0) > 16 * 1024 * 1024 {
            cache.removeAll(keepingCapacity: true)
            cachedBytes = 0
        }
        cache[key] = Cached(data: data)
        cachedBytes += data?.count ?? 0
        return data
    }
}
