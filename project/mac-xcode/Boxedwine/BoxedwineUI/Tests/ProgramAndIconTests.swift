// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import ImageIO
import Testing
@testable import BoxedwineLibrary

struct ProgramAndIconTests {
    @Test func suggestsAppButNeverGuessesBetweenAppsOrPicksAnUninstaller() {
        let main = ProgramCandidate(path: LibraryRepository.driveC + "/Program Files/NetSurf/NetSurf.exe")
        let uninstall = ProgramCandidate(path: LibraryRepository.driveC + "/Program Files/NetSurf/uninstall.exe")
        #expect(ProgramCandidate.suggestedPath(in: [main, uninstall], current: nil) == main.path)
        #expect(ProgramCandidate.suggestedPath(in: [uninstall], current: nil) == nil)
        let second = ProgramCandidate(path: LibraryRepository.driveC + "/Other/NetSurf.exe")
        #expect(ProgramCandidate.suggestedPath(in: [main, second, uninstall], current: nil) == nil)
        #expect(ProgramCandidate.suggestedPath(in: [main, uninstall], current: uninstall.path) == uninstall.path)
        #expect(main.id != second.id)
        #expect(main.windowsPath == "C:/Program Files/NetSurf/NetSurf.exe")
        #expect(!ProgramCandidate(path: "UninstallSimulator.exe").isMaintenanceTool)
        #expect(ProgramCandidate(path: "unins000.EXE").isMaintenanceTool)
    }

    @Test func extractsDecodableIconsFromPE32AndPE32PlusWithOffsetResourceRoots() throws {
        for wide in [false, true] {
            let ico = try #require(WindowsIcon.extract(from: fixture(wide: wide)))
            #expect(ico.prefix(6) == Data([0, 0, 1, 0, 1, 0]))
            let source = try #require(CGImageSourceCreateWithData(ico as CFData, nil))
            let image = try #require(CGImageSourceCreateImageAtIndex(source, 0, nil))
            #expect(image.width == 16 && image.height == 16)
        }
    }

    @Test(arguments: [false, true])
    func extractsNEIconsWithAlignedResourcesAndNumericOrNamedGroups(namedGroup: Bool) throws {
        // A 32x32, 16-color icon padded to 512-byte resource units, as in Bang Bang.
        let pe = indexedFixture(depth: 4, width: 32, height: 32)
        let ne = neFixture([pe], namedGroup: namedGroup)
        let ico = try #require(WindowsIcon.extract(from: ne))
        #expect(ico == WindowsIcon.extract(from: pe))
        let image = try #require(WindowsIcon.thumbnail(from: ico))
        #expect(image.width == 32 && image.height == 32)
        #expect(WindowsIcon.dockPNG(from: ico) != nil)
        // Data slices use their own indexing; parsing must normalize offsets.
        var prefixed = Data([1, 2, 3]); prefixed.append(ne)
        #expect(WindowsIcon.extract(from: prefixed.dropFirst(3)) == ico)
    }

    @Test func neGroupsKeepAllSizesAndUseTheLargestReadableImage() throws {
        let small = fixture(), large = indexedFixture(depth: 4, width: 32, height: 32)
        for images in [[small, large], [large, small]] {
            let ico = try #require(WindowsIcon.extract(from: neFixture(images)))
            #expect(ico[4] == 2)
            let thumbnail = try #require(WindowsIcon.thumbnail(from: ico))
            #expect(thumbnail.width == 32 && thumbnail.height == 32)
        }
        var damaged = neFixture([small, large])
        // First image's resource ID is absent: retain the valid second image.
        put(&damaged, 0x212, 99, count: 2)
        let ico = try #require(WindowsIcon.extract(from: damaged))
        #expect(ico[4] == 1 && WindowsIcon.thumbnail(from: ico)?.width == 32)
    }

    @Test func malformedNEHeadersTablesAndPaddedPayloadsAreRejected() {
        let valid = neFixture([fixture()])
        for size in 0..<valid.count {
            #expect(WindowsIcon.extract(from: Data(valid.prefix(size))) == nil)
        }
        for mutation: (Int, UInt32, Int) in [
            (0x3c, 0xfffffff0, 4), (0xb6, 1, 1), // invalid header / OS2 resource layout
            (0xa4, 63, 2), (0xa6, 64, 2), (0xa6, 0xffff, 2), // table bounds
            (0xc0, 32, 2), (0xc4, 4097, 2), (0xec, 4097, 2), // alignment / counts
            (0xca, 0xffff, 2), (0xcc, 0, 2), // group offset / length
            (0xf2, 0xffff, 2), (0xf4, 1, 2), (0xf8, 0x8002, 2), // icon offset / truncated size / ID
            (0xfe, 0x8003, 2), // missing table terminator
            (0x204, 65, 2), (0x20e, 0x7fffffff, 4), (0x212, 99, 2) // group count / image size / reference
        ] {
            var damaged = valid
            put(&damaged, mutation.0, mutation.1, count: mutation.2)
            #expect(WindowsIcon.extract(from: damaged) == nil, "Mutation at \(mutation.0)")
        }
        var duplicates = neFixture([fixture(), fixture()])
        put(&duplicates, 0x104, 0x8001, count: 2)
        #expect(WindowsIcon.extract(from: duplicates) == nil)
    }

    @Test func truncatedOrMalformedExecutablesFailWithoutReadingPastTheirBuffers() {
        let valid = fixture()
        for size in 0..<0xe00 { #expect(WindowsIcon.extract(from: Data(valid.prefix(size))) == nil) }
        for mutation: (Int, UInt32) in [(0x3c, 0xfffffff0), (0x108, 0xfffffff0),
                                       (0x614, 0x8000ffff), (0x67c, 0xffffffff),
                                       (0x684, 0x80000070), (0x6b4, 0xffffffff),
                                       (0x904, 0x7fffffff)] {
            var damaged = valid
            put(&damaged, mutation.0, mutation.1, count: 4)
            #expect(WindowsIcon.extract(from: damaged) == nil)
        }
        var missingIcon = valid
        put(&missingIcon, 0x812, 99, count: 2)
        #expect(WindowsIcon.extract(from: missingIcon) == nil)
    }

    @Test func legacyGroupHeightUsesTheBitmapHeightWithoutStretchingTheIcon() throws {
        let ico = try #require(WindowsIcon.extract(from: legacyHeightFixture()))
        #expect(ico[6] == 20 && ico[7] == 31)
        let source = try #require(CGImageSourceCreateWithData(ico as CFData, nil))
        let image = try #require(CGImageSourceCreateImageAtIndex(source, 0, nil))
        #expect(image.width == 20 && image.height == 31)
    }

    @Test func dockIconsUseBoundedSquarePNGsAndRejectMissingArtwork() throws {
        for executable in [fixture(), legacyHeightFixture(), indexedFixture(depth: 8, width: 20, height: 31)] {
            let ico = try #require(WindowsIcon.extract(from: executable))
            let png = try #require(WindowsIcon.dockPNG(from: ico))
            #expect(png.starts(with: [137, 80, 78, 71, 13, 10, 26, 10]))
            #expect(png.count <= 72 * 1024)
            let source = try #require(CGImageSourceCreateWithData(png as CFData, nil))
            let image = try #require(CGImageSourceCreateImageAtIndex(source, 0, nil))
            #expect(image.width == 128 && image.height == 128)
            #expect(image.alphaInfo != .none && image.alphaInfo != .noneSkipFirst && image.alphaInfo != .noneSkipLast)
        }
        #expect(WindowsIcon.dockPNG(from: Data("not an icon".utf8)) == nil)
    }

    @Test func largestOriginalIconWinsBeforeResizingRegardlessOfResourceOrder() throws {
        for sizes in [[128, 256, 64], [256, 128, 64], [64, 128, 256]] {
            // The 256px representation is blue; smaller alternatives are red.
            try expectLargestIcon(try multipleSizeIcon(sizes))
            // ImageIO can reorder ICO entries. TIFF preserves page order and
            // exercises the same image-source selection without that shortcut.
            let tiff = NSMutableData()
            let destination = try #require(CGImageDestinationCreateWithData(tiff, "public.tiff" as CFString, sizes.count, nil))
            for size in sizes { CGImageDestinationAddImage(destination, try solidIconImage(size), nil) }
            try #require(CGImageDestinationFinalize(destination))
            try expectLargestIcon(tiff as Data)
        }
    }

    private func expectLargestIcon(_ data: Data) throws {
        let library = try #require(WindowsIcon.thumbnail(from: data))
        #expect(library.width == 128 && library.height == 128)
        #expect(try centerPixel(library) == [0, 0, 255, 255])
        let png = try #require(WindowsIcon.dockPNG(from: data))
        let source = try #require(CGImageSourceCreateWithData(png as CFData, nil))
        let dock = try #require(CGImageSourceCreateImageAtIndex(source, 0, nil))
        #expect(dock.width == 128 && dock.height == 128)
        #expect(try centerPixel(dock) == [0, 0, 255, 255])
    }

    @Test func legacyHeightCompatibilityStillRejectsInvalidDimensions() {
        for mutation: (Int, UInt32, Int) in [(0x904, 21, 4), (0x908, 61, 4), (0x908, 0, 4),
                                            (0x908, 1024, 4), (0x807, 60, 1)] {
            var damaged = legacyHeightFixture()
            put(&damaged, mutation.0, mutation.1, count: mutation.2)
            #expect(WindowsIcon.extract(from: damaged) == nil)
        }
    }

    @Test(arguments: [1, 4, 8])
    func indexedRowsPreservePaletteAndTransparencyAcrossPadding(depth: Int) throws {
        for width in [17, 20, 33] {
            let height = 31
            let ico = try #require(WindowsIcon.extract(from: indexedFixture(depth: depth, width: width, height: height)))
            let source = try #require(CGImageSourceCreateWithData(ico as CFData, nil))
            let image = try #require(CGImageSourceCreateImageAtIndex(source, 0, nil))
            #expect(image.width == width && image.height == height)
            var actual = [UInt8](repeating: 0, count: width * height * 4)
            try actual.withUnsafeMutableBytes { bytes in
                let context = try #require(CGContext(data: bytes.baseAddress, width: width, height: height,
                                                    bitsPerComponent: 8, bytesPerRow: width * 4,
                                                    space: CGColorSpace(name: CGColorSpace.sRGB)!,
                                                    bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue))
                context.draw(image, in: CGRect(x: 0, y: 0, width: width, height: height))
            }
            var expected = [UInt8](repeating: 0, count: width * height * 4)
            for y in 0..<height {
                let row = height - 1 - y
                for x in 0..<width where (x + 2 * row) % 5 != 0 {
                    let color = (x + row) % (1 << depth), target = (y * width + x) * 4
                    expected[target] = UInt8(truncatingIfNeeded: color * 37)
                    expected[target + 1] = UInt8(truncatingIfNeeded: color * 73)
                    expected[target + 2] = UInt8(truncatingIfNeeded: color * 19)
                    expected[target + 3] = 255
                }
            }
            let matches = actual == expected
            let firstDifference = actual.indices.first { actual[$0] != expected[$0] }
            #expect(matches, "depth \(depth), width \(width), first difference \(String(describing: firstDifference)); actual \(actual.prefix(16)), expected \(expected.prefix(16))")
        }
    }

    @Test func invalidIndexedPaletteAndTruncatedMasksAreRejected() {
        let valid = indexedFixture(depth: 8, width: 20, height: 31)
        for mutation: (Int, UInt32, Int) in [(0x90c, 2, 2), (0x920, 257, 4)] {
            var damaged = valid
            put(&damaged, mutation.0, mutation.1, count: mutation.2)
            #expect(WindowsIcon.extract(from: damaged) == nil)
        }
        var truncated = valid
        put(&truncated, 0x6b4, 1807, count: 4)
        put(&truncated, 0x80e, 1807, count: 4)
        #expect(WindowsIcon.extract(from: truncated) == nil)
    }

    @Test func iconCacheRefreshesWhenAnExecutableChanges() async throws {
        let directory = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: directory) }
        let file = directory.appendingPathComponent("app.exe")
        try fixture().write(to: file)
        try FileManager.default.setAttributes([.modificationDate: Date(timeIntervalSince1970: 1000)], ofItemAtPath: file.path)
        let cache = WindowsIconCache()
        let first = await cache.iconData(for: file)
        #expect(first != nil)
        var changed = fixture()
        changed[0x928] = 255
        try changed.write(to: file)
        try FileManager.default.setAttributes([.modificationDate: Date(timeIntervalSince1970: 2000)], ofItemAtPath: file.path)
        let second = await cache.iconData(for: file)
        #expect(second != nil && second != first)
    }

    @Test func customIconOverridesTheExecutableAndResetRestoresAutomaticSelection() async throws {
        let base = try AppIconFixture.temporary(); defer { try? FileManager.default.removeItem(at: base) }
        let executable = base.appendingPathComponent("app.exe"), artwork = base.appendingPathComponent("demo.png")
        try fixture().write(to: executable)
        try AppIconFixture.image().write(to: artwork)
        let cache = WindowsIconCache()
        let custom = try await cache.importCustomIcon(from: artwork)
        var request = AppIconRequest(executable: executable, customPNG: custom, demoImage: artwork)
        #expect(await cache.appIconData(request) == custom)
        #expect(await cache.appDockIconData(request) == WindowsIcon.dockPNG(from: custom))
        let withCustom = request
        request.customPNG = nil
        #expect(request != withCustom)
        let embedded = try #require(WindowsIcon.extract(from: fixture()))
        #expect(await cache.appIconData(request) == embedded)
        #expect(await cache.appDockIconData(request) == WindowsIcon.dockPNG(from: embedded))
        request.executable = base.appendingPathComponent("missing.exe")
        #expect(await cache.appIconData(request) == custom)
        // Images remain usable after the selected external file goes away.
        try FileManager.default.removeItem(at: artwork)
        #expect(await cache.appIconData(withCustom) == custom)
    }

    /// A tiny fixture we own: a red square in a PE resource tree. Resource RVAs
    /// deliberately start in the middle of a section, rather than at its boundary.
    private func fixture(wide: Bool = false) -> Data {
        var data = Data(repeating: 0, count: 0x1400)
        put(&data, 0, 0x5a4d, count: 2)
        put(&data, 0x3c, 0x80, count: 4)
        put(&data, 0x80, 0x4550, count: 4)
        put(&data, 0x86, 1, count: 2)
        let size = wide ? 240 : 224
        put(&data, 0x94, UInt32(size), count: 2)
        put(&data, 0x98, wide ? 0x20b : 0x10b, count: 2)
        let directories = 0x98 + (wide ? 112 : 96)
        put(&data, directories - 4, 16, count: 4)
        put(&data, directories + 16, 0x1200, count: 4)
        put(&data, directories + 20, 0x800, count: 4)
        let section = 0x98 + size
        put(&data, section + 12, 0x1000, count: 4)
        put(&data, section + 16, 0x1000, count: 4)
        put(&data, section + 20, 0x400, count: 4)
        put(&data, 0x60e, 2, count: 2)
        put(&data, 0x610, 3, count: 4)
        put(&data, 0x614, 0x80000030, count: 4)
        put(&data, 0x618, 14, count: 4)
        put(&data, 0x61c, 0x80000050, count: 4)
        for (offset, id, target): (Int, UInt32, UInt32) in [(0x630, 7, 0x80000070), (0x650, 1, 0x80000090),
                                                         (0x670, 1033, 0xb0), (0x690, 1033, 0xc0)] {
            put(&data, offset + 14, 1, count: 2)
            put(&data, offset + 16, id, count: 4)
            put(&data, offset + 20, target, count: 4)
        }
        put(&data, 0x6b0, 0x1500, count: 4)
        put(&data, 0x6b4, 1128, count: 4)
        put(&data, 0x6c0, 0x1400, count: 4)
        put(&data, 0x6c4, 20, count: 4)
        put(&data, 0x900, 40, count: 4)
        put(&data, 0x904, 16, count: 4)
        put(&data, 0x908, 32, count: 4)
        put(&data, 0x90c, 1, count: 2)
        put(&data, 0x90e, 32, count: 2)
        put(&data, 0x914, 1024, count: 4)
        for pixel in 0..<256 {
            data[0x928 + pixel * 4 + 2] = 255
            data[0x928 + pixel * 4 + 3] = 255
        }
        put(&data, 0x802, 1, count: 2)
        put(&data, 0x804, 1, count: 2)
        data[0x806] = 16
        data[0x807] = 16
        put(&data, 0x80a, 1, count: 2)
        put(&data, 0x80c, 32, count: 2)
        put(&data, 0x80e, 1128, count: 4)
        put(&data, 0x812, 7, count: 2)
        return data
    }

    /// Owned 20x31 indexed-color icon with the same directory-height error as
    /// Age of Empires: the group reports the combined bitmap + mask height (62).
    private func legacyHeightFixture() -> Data {
        indexedFixture(depth: 8, width: 20, height: 31, legacyHeight: true)
    }

    private func indexedFixture(depth: Int, width: Int, height: Int, legacyHeight: Bool = false) -> Data {
        var data = fixture()
        data.replaceSubrange(0x900..<0x1400, with: repeatElement(UInt8(0), count: 0xb00))
        let colors = 1 << depth, stride = ((width * depth + 31) / 32) * 4, maskStride = ((width + 31) / 32) * 4
        let size = 40 + colors * 4 + (stride + maskStride) * height
        put(&data, 0x6b4, UInt32(size), count: 4)
        data[0x806] = UInt8(width)
        data[0x807] = UInt8(legacyHeight ? height * 2 : height)
        data[0x808] = UInt8(truncatingIfNeeded: colors)
        put(&data, 0x80c, UInt32(depth), count: 2)
        put(&data, 0x80e, UInt32(size), count: 4)
        put(&data, 0x900, 40, count: 4)
        put(&data, 0x904, UInt32(width), count: 4)
        put(&data, 0x908, UInt32(height * 2), count: 4)
        put(&data, 0x90c, 1, count: 2)
        put(&data, 0x90e, UInt32(depth), count: 2)
        put(&data, 0x914, UInt32((stride + maskStride) * height), count: 4)
        for color in 0..<colors {
            data[0x928 + color * 4] = UInt8(truncatingIfNeeded: color * 19)
            data[0x928 + color * 4 + 1] = UInt8(truncatingIfNeeded: color * 73)
            data[0x928 + color * 4 + 2] = UInt8(truncatingIfNeeded: color * 37)
        }
        let pixels = 0x928 + colors * 4, mask = pixels + stride * height
        for y in 0..<height { for x in 0..<width {
            let bit = x * depth, index = (x + y) % colors
            data[pixels + y * stride + bit / 8] |= UInt8(index << (8 - depth - bit % 8))
            if (x + 2 * y) % 5 == 0 { data[mask + y * maskStride + x / 8] |= 1 << (7 - x % 8) }
        } }
        return data
    }

    /// Repackage owned PE icon fixtures into a distinct NE resource layout.
    /// Both the group and image resources have non-image alignment padding.
    private func neFixture(_ executables: [Data], namedGroup: Bool = false) -> Data {
        var data = Data(repeating: 0, count: 0x200)
        put(&data, 0, 0x5a4d, count: 2)
        put(&data, 0x3c, 0x80, count: 4)
        put(&data, 0x80, 0x454e, count: 2)
        data[0xb6] = 2
        put(&data, 0xa4, 0x40, count: 2)
        put(&data, 0xc0, 9, count: 2)
        put(&data, 0xc2, 0x800e, count: 2)
        put(&data, 0xc4, 1, count: 2)
        put(&data, 0xd0, 0x8001, count: 2)
        put(&data, 0xd6, 0x8006, count: 2) // unrelated string-table resource
        put(&data, 0xd8, 1, count: 2)
        put(&data, 0xea, 0x8003, count: 2)
        put(&data, 0xec, UInt32(executables.count), count: 2)
        var end = 0xf2 + 12 * executables.count + 2
        if namedGroup {
            put(&data, 0xd0, UInt32(end - 0xc0), count: 2)
            data.replaceSubrange(end..<(end + 5), with: [4, 77, 65, 73, 78])
            end += 5
        }
        put(&data, 0xa6, UInt32(end - 0x80), count: 2)
        var group = Data([0, 0, 1, 0, UInt8(executables.count), 0])
        var payloads: [Data] = []
        for (index, pe) in executables.enumerated() {
            var entry = pe.subdata(in: 0x806..<0x814)
            let size = (0..<4).reduce(0) { $0 | Int(entry[8 + $1]) << ($1 * 8) }
            put(&entry, 12, UInt32(index + 1), count: 2)
            group.append(entry)
            payloads.append(pe.subdata(in: 0x900..<(0x900 + size)))
            put(&data, 0xf8 + index * 12, UInt32(0x8001 + index), count: 2)
        }
        for (record, payload) in [(0xca, group)] + payloads.enumerated().map({ (0xf2 + $0.offset * 12, $0.element) }) {
            let capacity = ((payload.count + 511) / 512) * 512
            put(&data, record, UInt32(data.count >> 9), count: 2)
            put(&data, record + 2, UInt32(capacity >> 9), count: 2)
            data.append(payload)
            data.append(Data(repeating: 0xa5, count: capacity - payload.count))
        }
        return data
    }

    private func multipleSizeIcon(_ sizes: [Int]) throws -> Data {
        var ico = Data(repeating: 0, count: 6 + sizes.count * 16)
        put(&ico, 2, 1, count: 2)
        put(&ico, 4, UInt32(sizes.count), count: 2)
        for (index, size) in sizes.enumerated() {
            let image = try solidIconImage(size)
            let png = NSMutableData()
            let destination = try #require(CGImageDestinationCreateWithData(png, "public.png" as CFString, 1, nil))
            CGImageDestinationAddImage(destination, image, nil)
            try #require(CGImageDestinationFinalize(destination))
            let item = 6 + index * 16
            ico[item] = UInt8(truncatingIfNeeded: size)
            ico[item + 1] = UInt8(truncatingIfNeeded: size)
            put(&ico, item + 4, 1, count: 2)
            put(&ico, item + 6, 32, count: 2)
            put(&ico, item + 8, UInt32(png.length), count: 4)
            put(&ico, item + 12, UInt32(ico.count), count: 4)
            ico.append(png as Data)
        }
        return ico
    }

    private func solidIconImage(_ size: Int) throws -> CGImage {
        let context = try #require(CGContext(data: nil, width: size, height: size, bitsPerComponent: 8,
                                            bytesPerRow: size * 4, space: CGColorSpace(name: CGColorSpace.sRGB)!,
                                            bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue))
        context.setFillColor(red: size == 256 ? 0 : 1, green: 0, blue: size == 256 ? 1 : 0, alpha: 1)
        context.fill(CGRect(x: 0, y: 0, width: size, height: size))
        return try #require(context.makeImage())
    }

    private func centerPixel(_ image: CGImage) throws -> [UInt8] {
        var pixel = [UInt8](repeating: 0, count: 4)
        try pixel.withUnsafeMutableBytes { bytes in
            let context = try #require(CGContext(data: bytes.baseAddress, width: 1, height: 1, bitsPerComponent: 8,
                                                bytesPerRow: 4, space: CGColorSpace(name: CGColorSpace.sRGB)!,
                                                bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue))
            context.draw(image, in: CGRect(x: -image.width / 2, y: -image.height / 2, width: image.width, height: image.height))
        }
        return pixel
    }

    private func put(_ data: inout Data, _ offset: Int, _ value: UInt32, count: Int) {
        for index in 0..<count { data[offset + index] = UInt8(truncatingIfNeeded: value >> (index * 8)) }
    }
}
