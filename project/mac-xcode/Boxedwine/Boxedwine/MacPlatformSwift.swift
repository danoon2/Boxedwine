//
//  MacPlatformSwift.swift
//  Boxedwine
//
//  Created by James Bryant on 9/7/25.
//  Copyright © 2025 Boxedwine. All rights reserved.
//
import Cocoa

@objc
class MacPlatformSwift : NSObject {
    nonisolated(unsafe) static var runningApp: NSRunningApplication?
    
    @objc
    class func launchAnotherInstance() {
        let appURL = Bundle.main.bundleURL
        let configuration = NSWorkspace.OpenConfiguration()
        configuration.createsNewApplicationInstance = true
        
        NSWorkspace.shared.openApplication(at: appURL, configuration: configuration) { (app, error) in
            if let error = error {
                print("Error launching app: \(error.localizedDescription)")
            } else if let launchedApp = app {
                print("Successfully launched new instance of \(launchedApp.bundleIdentifier ?? "unknown app")")
            }
            runningApp = app
        }
    }
    
    @objc
    class func isAppRunning() -> Bool {
        let terminated = runningApp?.isTerminated ?? true
        return !terminated
    }
    
    @objc
    class func hasFinishedStartup() -> Bool {
        return runningApp?.isFinishedLaunching ?? false
    }
}
