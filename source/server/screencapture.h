/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
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

#ifndef __SCREEN_CAPTURE_H__
#define __SCREEN_CAPTURE_H__

#include <vector>
#include <atomic>

// Keeps a copy of what the guest shows, for the local server. Nothing here
// runs unless ScreenCapture::enabled is set, which only -localServer does.
//
// GDI output (every X window Wine draws) is composited the same way the SDL
// screen does it: clear(), putBitsOnWnd() per window, present(). OpenGL output
// goes to a separate host window, so its back buffer is read on swap and laid
// over the GDI picture at the window's position. Reading a GL frame stalls the
// game's GL pipeline, so it only happens while a consumer asked for frames in
// the last two seconds.
//
// Pixels are 0x00RRGGBB U32s with a pitch of width * 4.
class ScreenCapture {
public:
    struct Frame {
        U32 width = 0;
        U32 height = 0;
        std::vector<U8> pixels;
        // Counts changes to the picture. Presents that leave it identical don't count.
        U64 changeSeq = 0;
        U64 hash = 0;
        // KSystem::getMilliesSinceStart() when the picture last changed.
        U32 changedAt = 0;
    };

    static std::atomic<bool> enabled;

    // GDI compositing, called by KNativeScreenSDL on the UI thread.
    static void beginComposite(U32 width, U32 height);
    static void blit(const U8* bits, U32 pitch, S32 dstX, S32 dstY, U32 width, U32 height);
    static void endComposite();

    // OpenGL. Called on every swap of a window drawable. Cheap, and returns
    // whether a consumer wants this frame, in which case the caller reads the
    // back buffer and passes it to setGlFrame().
    static bool onGlSwap();
    // rows are bottom-up BGRA, as glReadPixels(GL_BGRA, GL_UNSIGNED_BYTE) returns them.
    static void setGlFrame(U32 drawableId, S32 screenX, S32 screenY, U32 width, U32 height, const U8* rows);
    static void clearGlFrame(U32 drawableId);

    // Consumers. Each call counts as demand for GL frames.
    static void noteDemand();
    // Copies the current picture. If OpenGL is presenting and freshGl is set,
    // first waits up to 250 ms for a GL frame read after this call. Returns
    // false before the first present.
    static bool getFrame(Frame& frame, bool freshGl = true);
    // Like getFrame without the pixels.
    static bool getInfo(Frame& frame, bool freshGl = true);
    // Waits until changeSeq differs from sinceSeq or the timeout passes.
    // Returns the current changeSeq.
    static U64 waitForChange(U64 sinceSeq, U32 timeoutMs);
    // Frames presented (GDI presents plus GL swaps), changed or not.
    static U64 presentCount();
    static bool glPresenting();

    // Drops all state. Used when the server stops and by tests.
    static void reset();
};

#endif
