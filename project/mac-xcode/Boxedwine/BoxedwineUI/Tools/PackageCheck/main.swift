// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
import Foundation
import BoxedwineLibrary

do {
    if CommandLine.arguments.count == 4, CommandLine.arguments[1] == "--wine-downloads" {
        print(try PackageCheck.checkWineDownloads(catalog: URL(fileURLWithPath: CommandLine.arguments[2]),
                                                 downloads: URL(fileURLWithPath: CommandLine.arguments[3])))
        exit(0)
    }
    if CommandLine.arguments.count == 3, CommandLine.arguments[1] == "--wine-catalog" {
        print(try PackageCheck.describeWineCatalog(URL(fileURLWithPath: CommandLine.arguments[2])))
        exit(0)
    }
    #if !BOXEDWINE_APP_STORE
    if CommandLine.arguments.count == 5, CommandLine.arguments[1] == "--demo-downloads" {
        print(try await PackageCheck.checkDemoDownloads(catalog: URL(fileURLWithPath: CommandLine.arguments[2]),
                                                       downloads: URL(fileURLWithPath: CommandLine.arguments[3]),
                                                       runtime: URL(fileURLWithPath: CommandLine.arguments[4])))
        exit(0)
    }
    #endif
    if CommandLine.arguments.count == 3, CommandLine.arguments[1] == "--catalog" {
        print(try PackageCheck.describeCatalog(URL(fileURLWithPath: CommandLine.arguments[2])))
        exit(0)
    }
    guard CommandLine.arguments.count == 2 else {
        throw NSError(domain: "Boxedwine", code: 1, userInfo: [NSLocalizedDescriptionKey: "Usage: BoxedwinePackageCheck /path/to/wine.zip OR --catalog /path/to/catalog.xml OR --demo-downloads /path/to/catalog.xml /path/to/downloads /path/to/wine.zip"])
    }
    print(try PackageCheck.describe(URL(fileURLWithPath: CommandLine.arguments[1])))
} catch {
    FileHandle.standardError.write(Data("error: \(error.localizedDescription)\n".utf8))
    exit(1)
}
