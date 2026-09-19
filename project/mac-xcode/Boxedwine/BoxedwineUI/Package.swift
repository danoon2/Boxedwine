// swift-tools-version: 6.0
import PackageDescription

let package = Package(
    name: "BoxedwineLibrary",
    platforms: [.macOS(.v13)],
    products: [.library(name: "BoxedwineLibrary", targets: ["BoxedwineLibrary"]),
               .executable(name: "BoxedwinePackageCheck", targets: ["BoxedwinePackageCheck"])],
    targets: [
        .target(name: "CBoxedwineZIP", path: "ZipSupport", publicHeadersPath: "include", linkerSettings: [.linkedLibrary("z")]),
        .target(name: "BoxedwineLibrary", dependencies: ["CBoxedwineZIP"], path: "Core"),
        .executableTarget(name: "BoxedwinePackageCheck", dependencies: ["BoxedwineLibrary"], path: "Tools/PackageCheck"),
        .testTarget(name: "BoxedwineLibraryTests", dependencies: ["BoxedwineLibrary"], path: "Tests", exclude: ["Fixtures"])
    ]
)
