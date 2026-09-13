/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

//
//  MacPlatform.m
//  Boxedwine
//
//  Created by James Bryant on 4/19/20.
//  Copyright © 2020 Boxedwine. All rights reserved.
//

#import "MacPlatform.h"
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#ifdef BOXEDWINE_DEVELOPMENT
#include <sys/sysctl.h>
#include <poll.h>
#include <unistd.h>
#endif

@class MacPlatformSwift;

#ifdef BOXEDWINE_NATIVE_RUNTIME
static NSURL* runtimeProgramFolder;

#ifdef BOXEDWINE_DEVELOPMENT
// Optional developer-only pause before boxedmain, allowing startup breakpoints
// without changing the guest command line or relying on attachment timing.
int MacPlatformWaitForDebugger(void) {
    const char* setting = getenv("BOXEDWINE_WAIT_FOR_DEBUGGER");
    BOOL wait = setting && strcmp(setting, "1") == 0;
    unsetenv("BOXEDWINE_WAIT_FOR_DEBUGGER");
    if (!wait) { return 1; }
    fprintf(stderr, "Boxedwine PID %d waiting for debugger. Stop cancels this launch.\n", getpid());
    fflush(stderr);
    while (1) {
        struct kinfo_proc process = {0};
        size_t size = sizeof(process);
        int query[] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid()};
        if (sysctl(query, 4, &process, &size, NULL, 0) != 0) { return 0; }
        if (process.kp_proc.p_flag & P_TRACED) { return 1; }
        // stdin belongs to the UI: quit or a closed pipe must still stop a
        // waiting child if the user cancels, quits, or the launcher crashes.
        struct pollfd input = {STDIN_FILENO, POLLIN | POLLHUP, 0};
        if (poll(&input, 1, 50) > 0 && input.revents) { return 0; }
    }
}
#endif

// PowerBox grants are dynamic rights, so transfer the implicit-scope bookmark
// explicitly instead of relying on the helper's static sandbox inheritance.
int MacPlatformAcquireRuntimeProgramFolder(void) {
    const char* encoded = getenv("BOXEDWINE_PROGRAM_FOLDER_BOOKMARK");
    if (!encoded) { return 1; }
    NSString* value = strnlen(encoded, 64 * 1024 + 1) <= 64 * 1024 ? [NSString stringWithUTF8String:encoded] : nil;
    unsetenv("BOXEDWINE_PROGRAM_FOLDER_BOOKMARK");
    NSData* data = value ? [[NSData alloc] initWithBase64EncodedString:value options:0] : nil;
    if (!data) { return 0; }
    BOOL stale = NO;
    NSError* error = nil;
    NSURL* url = [NSURL URLByResolvingBookmarkData:data options:NSURLBookmarkResolutionWithoutUI
                                   relativeToURL:nil bookmarkDataIsStale:&stale error:&error];
    NSNumber* directory = nil;
    // Resolving an implicit-scope bookmark activates access in this process.
    // Retain its URL until boxedmain returns (or the process terminates).
    if (!url || stale || !url.isFileURL ||
        ![url getResourceValue:&directory forKey:NSURLIsDirectoryKey error:&error] || !directory.boolValue) { return 0; }
    runtimeProgramFolder = url;
    return 1;
}

void MacPlatformReleaseRuntimeProgramFolder(void) {
    [runtimeProgramFolder stopAccessingSecurityScopedResource];
    runtimeProgramFolder = nil;
}

// Called on the main thread after SDL has initialized NSApplication. Each
// process changes only its own Dock / Command-Tab icon; the signed bundle stays intact.
void MacPlatformSetRuntimeDockIcon(void) {
    const char* encoded = getenv("BOXEDWINE_DOCK_ICON_PNG");
    if (!encoded) {
        return;
    }
    NSString* value = strnlen(encoded, 96 * 1024 + 1) <= 96 * 1024 ? [NSString stringWithUTF8String:encoded] : nil;
    unsetenv("BOXEDWINE_DOCK_ICON_PNG");
    if (!value) {
        return;
    }
    NSData* data = [[NSData alloc] initWithBase64EncodedString:value options:0];
    NSBitmapImageRep* bitmap = data ? [NSBitmapImageRep imageRepWithData:data] : nil;
    if (bitmap.pixelsWide == 128 && bitmap.pixelsHigh == 128) {
        NSImage* icon = [[NSImage alloc] initWithSize:NSMakeSize(128, 128)];
        [icon addRepresentation:bitmap];
        [NSApplication sharedApplication].applicationIconImage = icon;
    }
}
#endif

void MacPlatormSetThreadPriority(void) {
    [NSThread setThreadPriority:1.0];
}
void MacPlatformOpenFileLocation(const char* str) {
   NSString* path = [NSString stringWithUTF8String:str];
    if ([path hasPrefix:@"http"]) {
        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:path]];
    } else {
        [[NSWorkspace sharedWorkspace] selectFile:path inFileViewerRootedAtPath:@""];
    }
}

static char buffer[1024];

const char* MacPlatformGetResourcePath(const char* pName) {
    CFStringRef name = CFStringCreateWithCString(NULL, pName, kCFStringEncodingUTF8);
    CFURLRef appUrlRef = CFBundleCopyResourceURL(CFBundleGetMainBundle(), name, NULL, NULL);
    bool result = false;
    if (appUrlRef) {
        CFStringRef filePathRef = CFURLCopyFileSystemPath(appUrlRef, kCFURLPOSIXPathStyle);
        const char* filePath = CFStringGetCStringPtr(filePathRef, kCFStringEncodingUTF8);
        strncpy(buffer, filePath, 1024);
        // Release references
        CFRelease(filePathRef);
        CFRelease(appUrlRef);
        result = true;
    }
    CFRelease(name);
    if (result) {
        return buffer;
    }
    return NULL;
}
#if !defined(BOXEDWINE_UI_LAUNCH_IN_PROCESS) && !defined(BOXEDWINE_DISABLE_UI)
#import "Boxedwine-Swift.h"
int MacPlatformIsTaskRunning(void) {
    return [MacPlatformSwift isAppRunning] ? 1 : 0;
}

int MacPlatformIsTaskFinishedLaunching(void) {
    return [MacPlatformSwift hasFinishedStartup] ? 1 : 0;
}

int MacPlatformLaunchAnotherInstance(void)
{
    [MacPlatformSwift launchAnotherInstance];
    return 1;
}
#endif
