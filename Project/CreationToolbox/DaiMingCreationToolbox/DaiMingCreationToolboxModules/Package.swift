// swift-tools-version:5.5
import PackageDescription

let package = Package(
    name: "DaiMingCreationToolboxModules",
    defaultLocalization: "en",
    platforms: [
        .macOS(.v12),
    ],
    products: [
        .library(
            name: "WorkspaceClient",
            targets: ["WorkspaceClient"]
        ),
    ],
    dependencies: [
        .package(
            name: "Highlightr",
            url: "https://github.com/raspu/Highlightr.git",
            from: "2.1.2"
        ),
        .package(
            url: "https://github.com/groue/GRDB.swift.git",
            from: "5.22.2"
        ),
    ],
    targets: [
        .target(
            name: "WorkspaceClient",
            path: "Modules/WorkspaceClient/src"
        ),
    ]
)
