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
