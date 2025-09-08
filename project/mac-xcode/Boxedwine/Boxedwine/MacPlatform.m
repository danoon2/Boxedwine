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
#import "Boxedwine-Swift.h"

@class MacPlatformSwift;

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
