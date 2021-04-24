//
//  MacPlatform.m
//  Boxedwine
//
//  Created by James Bryant on 4/19/20.
//  Copyright © 2020 Boxedwine. All rights reserved.
//

#import "MacPlatform.h"
#import <Cocoa/Cocoa.h>

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
