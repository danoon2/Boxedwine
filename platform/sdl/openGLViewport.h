/*
 *  Copyright (C) 2012-2026 The BoxedWine Team
 *  GPL-2.0-or-later
 */

#ifndef BOXEDWINE_OPENGL_VIEWPORT_H
#define BOXEDWINE_OPENGL_VIEWPORT_H

#include <cstdint>
#include <algorithm>

// Host coordinates are SDL window coordinates, independent of backing pixels.
// Presentation and input must use the same rectangle and rounding rules.
struct OpenGLViewport {
    int guestWidth = 0, guestHeight = 0;
    int x = 0, y = 0, width = 0, height = 0;

    bool valid() const { return guestWidth > 0 && guestHeight > 0 && width > 0 && height > 0; }
    int toGuestX(int value) const { return (int)(((int64_t)value - x) * guestWidth / width); }
    int toGuestY(int value) const { return (int)(((int64_t)value - y) * guestHeight / height); }
    int toHostX(int value) const { return x + toHost(value, guestWidth, width); }
    int toHostY(int value) const { return y + toHost(value, guestHeight, height); }
    int deltaX(int value) const { return (int)((int64_t)value * guestWidth / width); }
    int deltaY(int value) const { return (int)((int64_t)value * guestHeight / height); }

    static OpenGLViewport fit(int hostWidth, int hostHeight, int guestWidth, int guestHeight, bool aspect) {
        OpenGLViewport result;
        if (hostWidth <= 0 || hostHeight <= 0 || guestWidth <= 0 || guestHeight <= 0) return result;
        result.guestWidth = guestWidth;
        result.guestHeight = guestHeight;
        result.width = hostWidth;
        result.height = hostHeight;
        if (aspect) {
            if ((int64_t)hostWidth * guestHeight > (int64_t)hostHeight * guestWidth) {
                result.width = std::max(1, (int)((int64_t)hostHeight * guestWidth / guestHeight));
            } else {
                result.height = std::max(1, (int)((int64_t)hostWidth * guestHeight / guestWidth));
            }
        }
        result.x = (hostWidth - result.width) / 2;
        result.y = (hostHeight - result.height) / 2;
        return result;
    }

private:
    static int toHost(int value, int guestSize, int hostSize) {
        int64_t scaled = (int64_t)value * hostSize;
        // With an enlarged image, round up to the first host coordinate that
        // maps back to this guest pixel. Rounding down makes a recentering game
        // read back one pixel less on each warp, causing continuous drift.
        if (hostSize >= guestSize && scaled > 0) scaled += guestSize - 1;
        return (int)(scaled / guestSize);
    }
};

#endif
