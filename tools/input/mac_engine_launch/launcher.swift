// Copyright (C) 2026 The Boxedwine Team. GPL-2.0-or-later.
@testable import BoxedwineLibrary
import AppKit

@MainActor
@main struct Launcher {
    static var engine: EngineProcess?
    static var capture: RuntimeLogCapture?
    static var mode = ""
    static func main() throws {
        NSApplication.shared.setActivationPolicy(.accessory)
        mode = CommandLine.arguments.dropFirst().first ?? "success"
        let directory = FileManager.default.temporaryDirectory.appendingPathComponent("engine-probe-" + UUID().uuidString)
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: false)
        let file = directory.appendingPathComponent("file with spaces 日本語.iso")
        try Data("scoped file access works".utf8).write(to: file)
        let executable = Bundle.main.bundleURL.appendingPathComponent("Contents/Helpers/Engine.app/Contents/MacOS/Engine")
        var environment = ["BOXEDWINE_TEST_ENV":"environment passed"]
        if mode == "icon" {
            let bitmap = NSBitmapImageRep(bitmapDataPlanes: nil, pixelsWide: 128, pixelsHigh: 128,
                bitsPerSample: 8, samplesPerPixel: 4, hasAlpha: true, isPlanar: false,
                colorSpaceName: .deviceRGB, bytesPerRow: 128 * 4, bitsPerPixel: 32)!
            let red = NSColor(deviceRed: 1, green: 0, blue: 0, alpha: 1)
            for y in 0..<128 { for x in 0..<128 { bitmap.setColor(red, atX: x, y: y) } }
            environment["BOXEDWINE_DOCK_ICON_PNG"] = bitmap.representation(using: .png, properties: [:])!.base64EncodedString()
        }
        let process = try EngineProcess(executable: executable, arguments: [mode, file.path, "literal $(no-shell) 日本語"], directory: directory,
            environment: environment, access: [directory])
        engine = process
        let log = directory.appendingPathComponent("latest.log")
        let output = try RuntimeLogCapture(url: log, input: process.output) {
            Task { @MainActor in
                if mode == "stop" { engine?.stop() }
                if mode == "force" { engine?.stop(force: true) }
                if mode == "orphan" {
                    capture?.finish("Launcher exiting") { _ in
                        print((try? String(contentsOf: log, encoding: .utf8)) ?? "")
                        fflush(stdout)
                        exit(0)
                    }
                }
            }
        }
        capture = output
        output.start()
        print("FIXTURE \(file.path)")
        fflush(stdout)
        try process.run { process in
            let status = process.terminationStatus
            let problem = process.launchProblem
            output.finish("Probe complete") { logProblem in
                Task { @MainActor in
                    let contents = String(decoding: try! Data(contentsOf: log), as: UTF8.self)
                    print(contents)
                    print("RESULT mode=\(mode) status=\(status) problem=\(problem ?? logProblem ?? "none")")
                    fflush(stdout)
                    let expected: Int32 = mode == "success" ? 17 : mode == "force" ? 137 : 0
                    exit(status == expected && problem == nil && logProblem == nil && contents.contains("Showing Window") ? 0 : 1)
                }
            }
        }
        NSApplication.shared.run()
    }
}
