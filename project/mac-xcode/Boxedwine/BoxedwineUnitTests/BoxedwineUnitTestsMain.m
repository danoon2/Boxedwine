//
//  BoxedwineSlowUnitTests.m
//  BoxedwineSlowUnitTests
//
//  Created by James on 9/8/23.
//  Copyright © 2023 Boxedwine. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

int runCpuTestsMac(void);

BOOL isUnitTestRunning(void) {
    Class testProbeClass;
    testProbeClass = NSClassFromString(@"XCTestProbe");
    return (testProbeClass != nil);
}

int main(int argc, char *argv[]) {
    if (isUnitTestRunning()) {
        [[NSApplication sharedApplication] run];
        return 0;
    }
    return runCpuTestsMac();
}
