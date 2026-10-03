/*
 *  Copyright (C) 2012-2026 The BoxedWine Team
 *  GPL-2.0-or-later
 *
 *  Headless regression test for the production Mac cursor lifecycle. SDL and
 *  AppKit focus are simulated; no real windows are opened or cursors hidden.
 *  Run with: python3 tools/test_mac_cursor.py
 */

#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include "macCursor.h"
#include <cstdio>
#include <cstdlib>

@interface CursorTestApplication : NSObject
@property(getter=isActive) BOOL active;
@end
@implementation CursorTestApplication
@end

@interface CursorTestView : NSObject
@property NSRect bounds;
- (NSPoint)convertPoint:(NSPoint)point fromView:(NSView*)view;
@end
@implementation CursorTestView
- (NSPoint)convertPoint:(NSPoint)point fromView:(NSView*)view { return point; }
@end

@interface CursorTestWindow : NSObject
@property(getter=isVisible) BOOL visible;
@property(getter=isMiniaturized) BOOL miniaturized;
@property NSPoint mouseLocationOutsideOfEventStream;
@property(strong) CursorTestView* contentView;
@end
@implementation CursorTestWindow
@end

static int windowToken;
static SDL_Window* window = reinterpret_cast<SDL_Window*>(&windowToken);
static SDL_Window* mouseFocus = window;
static SDL_Window* keyboardFocus = window;
static int cursorVisible = SDL_DISABLE;
static SDL_bool relativeMode = SDL_FALSE;
static bool lookupSucceeds = true;
static CursorTestWindow* nativeWindow;
static int hideCount;
static int hideCalls;

static void require(bool condition, const char* message) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        std::exit(1);
    }
}

static void hideCursor(id, SEL) { ++hideCount; ++hideCalls; }
static void unhideCursor(id, SEL) {
    require(hideCount > 0, "unbalanced cursor unhide");
    --hideCount;
}

SDL_Window* SDL_GetMouseFocus() { return mouseFocus; }
SDL_Window* SDL_GetKeyboardFocus() { return keyboardFocus; }
SDL_bool SDL_GetRelativeMouseMode() { return relativeMode; }
int SDL_ShowCursor(int toggle) {
    require(toggle == SDL_QUERY, "native hiding must not change SDL's cursor state");
    return cursorVisible;
}
SDL_bool SDL_GetWindowWMInfo(SDL_Window* queriedWindow, SDL_SysWMinfo* info) {
    require(queriedWindow == window, "unexpected window lookup");
    info->subsystem = SDL_SYSWM_COCOA;
    info->info.cocoa.window = (NSWindow*)nativeWindow;
    return lookupSucceeds ? SDL_TRUE : SDL_FALSE;
}

int main() {
    @autoreleasepool {
        NSApplication* savedApplication = NSApp;
        CursorTestApplication* application = [CursorTestApplication new];
        application.active = YES;
        NSApp = (NSApplication*)application;
        nativeWindow = [CursorTestWindow new];
        nativeWindow.visible = YES;
        nativeWindow.contentView = [CursorTestView new];
        nativeWindow.contentView.bounds = NSMakeRect(0, 0, 640, 480);
        nativeWindow.mouseLocationOutsideOfEventStream = NSMakePoint(320, 240);

        Method hide = class_getClassMethod(NSCursor.class, @selector(hide));
        Method unhide = class_getClassMethod(NSCursor.class, @selector(unhide));
        IMP savedHide = method_setImplementation(hide, (IMP)hideCursor);
        IMP savedUnhide = method_setImplementation(unhide, (IMP)unhideCursor);

        for (int i = 0; i < 1000; ++i) macCursorUpdate();
        require(hideCount == 1 && hideCalls == 1, "mouse motion must own only one hide");
        application.active = NO;
        macCursorUpdate();
        require(hideCount == 0, "Command-Tab must restore the cursor");
        application.active = YES;
        macCursorUpdate();
        require(hideCount == 1, "returning to the game must hide again");

        nativeWindow.mouseLocationOutsideOfEventStream = NSMakePoint(320, 490);
        macCursorUpdate();
        require(hideCount == 0, "title bar must show the cursor despite stale SDL focus");
        nativeWindow.mouseLocationOutsideOfEventStream = NSMakePoint(320, 240);
        macCursorUpdate();
        mouseFocus = nullptr;
        macCursorUpdate();
        require(hideCount == 0, "leaving the window must restore the cursor");
        mouseFocus = window;
        keyboardFocus = nullptr;
        macCursorUpdate();
        require(hideCount == 0, "inactive game window must not hide the cursor");
        keyboardFocus = window;
        macCursorUpdate();
        nativeWindow.miniaturized = YES;
        macCursorUpdate();
        require(hideCount == 0, "minimizing must restore the cursor");
        nativeWindow.miniaturized = NO;
        macCursorUpdate();
        nativeWindow.visible = NO;
        macCursorUpdate();
        require(hideCount == 0, "hiding the window must restore the cursor");
        nativeWindow.visible = YES;
        macCursorUpdate();
        lookupSucceeds = false;
        macCursorUpdate();
        require(hideCount == 0, "failed window lookup must restore the cursor");
        lookupSucceeds = true;
        macCursorUpdate();
        cursorVisible = SDL_ENABLE;
        macCursorUpdate();
        require(hideCount == 0, "a guest requesting a visible cursor must restore it");

        // SDL may already own a hide in relative mode; leave that hide intact.
        hideCursor(nil, nullptr);
        relativeMode = SDL_TRUE;
        macCursorUpdate();
        require(hideCount == 2, "relative input must also suppress the native cursor");
        macCursorReset();
        macCursorReset();
        require(hideCount == 1, "shutdown must release only Boxedwine's own hide");
        unhideCursor(nil, nullptr);
        relativeMode = SDL_FALSE;
        macCursorUpdate();
        require(hideCount == 0, "cursor must stay visible after relative mode ends");

        method_setImplementation(hide, savedHide);
        method_setImplementation(unhide, savedUnhide);
        NSApp = savedApplication;
        std::puts("Mac cursor lifecycle checks passed");
    }
}
