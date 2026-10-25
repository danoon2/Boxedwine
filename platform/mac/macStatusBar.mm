/* Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later */
#import <Cocoa/Cocoa.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include "macStatusBar.h"

// The guest normally owns keyboard focus. Host controls must accept their
// first click without changing SDL's global click-through policy for guests.
@interface BWStatusBarInputView : NSView
@end
@implementation BWStatusBarInputView
- (BOOL)acceptsFirstMouse:(NSEvent*)event { return YES; }
- (void)mouseDown:(NSEvent*)event { [self.superview mouseDown:event]; }
- (void)mouseUp:(NSEvent*)event { [self.superview mouseUp:event]; }
- (void)rightMouseDown:(NSEvent*)event { [self.superview rightMouseDown:event]; }
- (void)rightMouseUp:(NSEvent*)event { [self.superview rightMouseUp:event]; }
@end

@interface BWStatusBarCommands : NSObject
@property(nonatomic, weak) NSWindow* host;
- (void)performClose:(id)sender;
- (void)performMiniaturize:(id)sender;
@end
@implementation BWStatusBarCommands
- (void)performClose:(id)sender { [self.host performClose:sender]; }
- (void)performMiniaturize:(id)sender { [self.host performMiniaturize:sender]; }
@end

namespace {
bool guestBordered;
NSWindowCollectionBehavior guestBehavior;
bool guestShadow, guestExcluded;
NSString* selectedPath;
NSMutableArray<NSURL*>* runtimeSelections;
BWStatusBarCommands* commands;
NSMenuItem* closeItem;
NSMenuItem* minimizeItem;
id closeTarget, minimizeTarget;

NSWindow* nativeWindow(SDL_Window* window) {
    SDL_SysWMinfo info = {};
    SDL_VERSION(&info.version);
    return window && SDL_GetWindowWMInfo(window, &info) && info.subsystem == SDL_SYSWM_COCOA ? info.info.cocoa.window : nil;
}
}

void macStatusBarSetOwner(SDL_Window* child, SDL_Window* parent) {
    NSWindow* c = nativeWindow(child);
    NSWindow* p = nativeWindow(parent);
    if (!c || c.parentWindow == p) return;
    [c.parentWindow removeChildWindow:c];
    if (!p) return;
    c.excludedFromWindowsMenu = YES;
    c.collectionBehavior = NSWindowCollectionBehaviorFullScreenAuxiliary | NSWindowCollectionBehaviorIgnoresCycle;
    [p addChildWindow:c ordered:NSWindowAbove];
}

void macStatusBarFocus(SDL_Window* window) {
    NSWindow* native = nativeWindow(window);
    if (native.isVisible && !native.isMiniaturized) {
        [native makeKeyWindow];
        [native.parentWindow makeMainWindow];
    }
}

void macStatusBarPositionGuest(SDL_Window* guest, SDL_Window* host) {
    NSWindow* child = nativeWindow(guest);
    NSWindow* parent = nativeWindow(host);
    if (!child || child.parentWindow != parent) return;
    // AppKit child windows move/minimize/order with their parent. Keeping the
    // guest's own SDL content view preserves GL/Metal drawables, Retina scale,
    // text input, capture, and mouse coordinates without SDL private APIs.
    NSRect content = [parent contentRectForFrameRect:parent.frame];
    NSPoint origin = NSMakePoint(NSMinX(content), NSMaxY(content) - NSHeight(child.frame));
    if (!NSEqualPoints(child.frame.origin, origin)) [child setFrameOrigin:origin];
}

bool macStatusBarEmbedGuest(SDL_Window* guest, SDL_Window* host, bool attach) {
    NSWindow* child = nativeWindow(guest);
    NSWindow* parent = nativeWindow(host);
    if (!child || !parent) return false;
    if (attach) {
        guestBordered = !(SDL_GetWindowFlags(guest) & SDL_WINDOW_BORDERLESS);
        guestBehavior = child.collectionBehavior;
        guestShadow = child.hasShadow;
        guestExcluded = child.excludedFromWindowsMenu;
        SDL_SetWindowBordered(guest, SDL_FALSE);
        child.hasShadow = NO;
        bool hasInputView = false;
        for (NSView* view in parent.contentView.subviews) {
            if ([view isKindOfClass:[BWStatusBarInputView class]]) hasInputView = true;
        }
        if (!hasInputView) {
            NSView* input = [[BWStatusBarInputView alloc] initWithFrame:NSMakeRect(0, 0, NSWidth(parent.contentView.bounds), 32)];
            input.autoresizingMask = NSViewWidthSizable | NSViewMaxYMargin;
            [parent.contentView addSubview:input];
        }
        macStatusBarSetOwner(guest, host);
        macStatusBarPositionGuest(guest, host);
        // Standard Window menu commands must act on the decorated host even
        // while its borderless guest owns keyboard focus.
        if (!commands) commands = [BWStatusBarCommands new];
        commands.host = parent;
        for (NSMenuItem* item in NSApp.windowsMenu.itemArray) {
            if (item.action == @selector(performClose:)) {
                closeItem = item; closeTarget = item.target; item.target = commands;
            } else if (item.action == @selector(performMiniaturize:)) {
                minimizeItem = item; minimizeTarget = item.target; item.target = commands;
            }
        }
    } else {
        SDL_HideWindow(guest);
        [parent removeChildWindow:child];
        SDL_SetWindowBordered(guest, guestBordered ? SDL_TRUE : SDL_FALSE);
        child.collectionBehavior = guestBehavior;
        child.hasShadow = guestShadow;
        child.excludedFromWindowsMenu = guestExcluded;
        closeItem.target = closeTarget;
        minimizeItem.target = minimizeTarget;
        closeItem = nil; minimizeItem = nil;
        closeTarget = nil; minimizeTarget = nil;
        commands.host = nil;
    }
    return true;
}

const char* macStatusBarChooseImage() {
    @autoreleasepool {
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        panel.title = @"Choose CD-ROM Disc Image";
        panel.allowedContentTypes = @[[UTType typeWithFilenameExtension:@"iso"], [UTType typeWithFilenameExtension:@"cue"]];
        panel.canChooseDirectories = NO;
        panel.allowsMultipleSelection = NO;
        if ([panel runModal] != NSModalResponseOK) return nullptr;
        NSURL* image = panel.URL;
        NSURL* grant = image;
        if ([image.pathExtension.lowercaseString isEqualToString:@"cue"]) {
            NSOpenPanel* folder = [NSOpenPanel openPanel];
            folder.title = @"Allow Access to Disc Tracks";
            folder.message = @"Choose the folder containing this CUE image and its track files.";
            folder.prompt = @"Use Folder";
            folder.canChooseFiles = NO;
            folder.canChooseDirectories = YES;
            folder.directoryURL = image.URLByDeletingLastPathComponent;
            if ([folder runModal] != NSModalResponseOK) return nullptr;
            if (![folder.URL.URLByStandardizingPath isEqual:image.URLByDeletingLastPathComponent.URLByStandardizingPath]) return nullptr;
            grant = folder.URL;
        }
        if (!runtimeSelections) runtimeSelections = [NSMutableArray array];
        if (![runtimeSelections containsObject:grant]) {
            [grant startAccessingSecurityScopedResource];
            [runtimeSelections addObject:grant]; // Keep PowerBox grants for runtime swaps.
        }
        selectedPath = image.path;
        return selectedPath.UTF8String;
    }
}

void macStatusBarReleaseImages() {
    for (NSURL* url in runtimeSelections) [url stopAccessingSecurityScopedResource];
    runtimeSelections = nil;
    selectedPath = nil;
}
