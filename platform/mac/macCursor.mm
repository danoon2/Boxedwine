/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifdef __APPLE__
#import <Cocoa/Cocoa.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include "macCursor.h"

static bool cursorHidden;

static void setCursorHidden(bool hidden) {
    if (hidden == cursorHidden) {
        return;
    }
    // NSCursor counts calls. Never accumulate hides on successive mouse events
    // or balance a hide owned by SDL's relative-mouse implementation.
    if (hidden) {
        [NSCursor hide];
    } else {
        [NSCursor unhide];
    }
    cursorHidden = hidden;
}

void macCursorUpdate() {
    @autoreleasepool {
        bool hidden = false;
        SDL_Window* window = SDL_GetMouseFocus();
        if ([NSApp isActive] && window && window == SDL_GetKeyboardFocus() &&
            (SDL_ShowCursor(SDL_QUERY) == SDL_DISABLE || SDL_GetRelativeMouseMode())) {
            SDL_SysWMinfo info;
            SDL_VERSION(&info.version);
            if (SDL_GetWindowWMInfo(window, &info) && info.subsystem == SDL_SYSWM_COCOA) {
                NSWindow* nativeWindow = info.info.cocoa.window;
                NSView* view = nativeWindow.contentView;
                NSPoint point = [view convertPoint:nativeWindow.mouseLocationOutsideOfEventStream fromView:nil];
                // SDL's focus can persist over window decorations. Keep the
                // macOS pointer available on the title bar and resize borders.
                hidden = nativeWindow.isVisible && !nativeWindow.isMiniaturized && view && NSPointInRect(point, view.bounds);
            }
        }
        setCursorHidden(hidden);
    }
}

void macCursorReset() {
    setCursorHidden(false);
}
#endif
