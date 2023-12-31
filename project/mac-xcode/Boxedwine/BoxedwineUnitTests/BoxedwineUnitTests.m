//
//  BoxedwineUnitTests.m
//  BoxedwineUnitTests
//
//  Created by James on 9/8/23.
//  Copyright © 2023 Boxedwine. All rights reserved.
//

#import <XCTest/XCTest.h>

int runCpuTestsMac(void);

@interface BoxedwineUnitTests : XCTestCase

@end

@implementation BoxedwineUnitTests

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testExample {
    XCTAssert(runCpuTestsMac() == 0);
}

@end
