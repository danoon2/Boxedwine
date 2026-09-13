// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation

protocol DemoDownloading: Sendable {
    func fetch(_ demo: Demo, to destination: URL, control: ImportControl) async throws
}

struct DemoDownloader: DemoDownloading, @unchecked Sendable {
    // Tests may supply a URLProtocol-backed configuration. The app uses an
    // ephemeral session: no cookies, shared cache, credentials, or catalog fetch.
    var configuration: URLSessionConfiguration = .ephemeral
    func fetch(_ demo: Demo, to destination: URL, control: ImportControl) async throws {
        try await PackageDownloader(configuration: configuration).fetch(demo.url, bytes: demo.bytes, to: destination, control: control)
    }
}

struct PackageDownloader: @unchecked Sendable {
    var configuration: URLSessionConfiguration = .ephemeral
    func fetch(_ url: URL, bytes: Int64, to destination: URL, control: ImportControl) async throws {
        try control.checkCancellation()
        guard DemoCatalog.validDownloadURL(url) else { throw DemoError.download("Unsupported download URL.") }
        let operation = PackageDownloadOperation(url: url, bytes: bytes, destination: destination, control: control)
        try await operation.start(configuration: configuration)
    }
}

/// Mutable state is confined to the serial delegate queue, following setup
/// before task.resume(). The timer accesses only the thread-safe task/control.
private final class PackageDownloadOperation: NSObject, URLSessionDownloadDelegate, @unchecked Sendable {
    let url: URL
    let bytes: Int64
    let destination: URL
    let control: ImportControl
    private var continuation: CheckedContinuation<Void, any Error>?
    private var session: URLSession?
    private var timer: DispatchSourceTimer?
    private var result: Result<Void, any Error>?
    private var redirects = 0

    init(url: URL, bytes: Int64, destination: URL, control: ImportControl) { self.url = url; self.bytes = bytes; self.destination = destination; self.control = control }
    func start(configuration: URLSessionConfiguration) async throws {
        try await withCheckedThrowingContinuation { continuation in
            self.continuation = continuation
            let config = configuration.copy() as! URLSessionConfiguration
            config.urlCache = nil; config.httpCookieStorage = nil; config.urlCredentialStorage = nil
            config.requestCachePolicy = .reloadIgnoringLocalCacheData
            config.timeoutIntervalForRequest = 30; config.timeoutIntervalForResource = 15 * 60
            let queue = OperationQueue(); queue.maxConcurrentOperationCount = 1
            let session = URLSession(configuration: config, delegate: self, delegateQueue: queue)
            self.session = session
            var request = URLRequest(url: url)
            request.setValue("identity", forHTTPHeaderField: "Accept-Encoding")
            let task = session.downloadTask(with: request)
            let timer = DispatchSource.makeTimerSource()
            timer.schedule(deadline: .now(), repeating: .milliseconds(100))
            let control = self.control
            timer.setEventHandler { if control.progress.cancelled { task.cancel() } }
            self.timer = timer
            control.update(phase: .downloading, total: bytes, file: url.lastPathComponent)
            timer.resume(); task.resume()
        }
    }
    func urlSession(_ session: URLSession, task: URLSessionTask, willPerformHTTPRedirection response: HTTPURLResponse,
                    newRequest request: URLRequest, completionHandler: @escaping (URLRequest?) -> Void) {
        redirects += 1
        guard redirects <= 3, let url = request.url, DemoCatalog.validDownloadURL(url) else {
            result = .failure(DemoError.download("The server redirected to an unsupported location."))
            completionHandler(nil); task.cancel(); return
        }
        completionHandler(request)
    }
    func urlSession(_ session: URLSession, downloadTask: URLSessionDownloadTask, didWriteData bytesWritten: Int64,
                    totalBytesWritten: Int64, totalBytesExpectedToWrite: Int64) {
        if totalBytesWritten > bytes || (totalBytesExpectedToWrite > 0 && totalBytesExpectedToWrite != bytes) {
            result = .failure(DemoError.checksum); downloadTask.cancel(); return
        }
        control.update(phase: .downloading, copied: totalBytesWritten, total: bytes, file: url.lastPathComponent)
        if control.progress.cancelled { downloadTask.cancel() }
    }
    func urlSession(_ session: URLSession, downloadTask: URLSessionDownloadTask, didFinishDownloadingTo location: URL) {
        guard result == nil else { return }
        do {
            try control.checkCancellation()
            guard let response = downloadTask.response as? HTTPURLResponse, response.statusCode == 200 else {
                throw DemoError.download("The server did not return the package (HTTP \((downloadTask.response as? HTTPURLResponse)?.statusCode ?? 0)).")
            }
            guard try RuntimePackage.Stamp.read(location).size == UInt64(bytes) else { throw DemoError.checksum }
            // URLSession deletes its temporary file after this callback returns.
            // Move it into this import's private, journaled staging directory.
            try FileManager.default.moveItem(at: location, to: destination)
            result = .success(())
        } catch { result = .failure(error) }
    }
    func urlSession(_ session: URLSession, task: URLSessionTask, didCompleteWithError error: (any Error)?) {
        timer?.cancel(); timer = nil
        let outcome: Result<Void, any Error>
        if control.progress.cancelled { outcome = .failure(CancellationError()) }
        else if let result { outcome = result }
        else { outcome = .failure(error ?? DemoError.download("The download ended before the file arrived.")) }
        continuation?.resume(with: outcome); continuation = nil
        session.finishTasksAndInvalidate(); self.session = nil
    }
}
