/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
#ifndef BOXEDWINE_CAPTURED_MOUSE_H
#define BOXEDWINE_CAPTURED_MOUSE_H

#include <algorithm>
#include <cstdint>
#include <limits>

// A game can implement relative input by polling/moving a hidden, captured
// cursor and warping it back. Map positions about that warp, not the desktop
// origin. Reading the same position through events and queries is idempotent.
class CapturedMouse {
public:
    void configure(bool captured, bool visible, bool focused, unsigned percent) {
        unsigned next = captured && !visible && focused && percent && percent != 100 ? percent : 100;
        if (next != percentage) {
            reset();
            percentage = next;
        }
    }

    void reset() {
        anchored = false;
        carryX = carryY = pendingX = pendingY = 0;
    }

    void warp(int x, int y) {
        if (percentage == 100) return;
        // Retain subpixel movement across recentering, but never multiply the
        // programmatic warp. Repeated warps without movement remain exact.
        carryX = pendingX;
        carryY = pendingY;
        anchorX = x;
        anchorY = y;
        anchored = true;
    }

    void map(int& x, int& y) {
        if (!anchored || percentage == 100) return;
        x = axis(x, anchorX, carryX, pendingX);
        y = axis(y, anchorY, carryY, pendingY);
    }

private:
    int axis(int value, int anchor, int64_t carry, int64_t& pending) const {
        int64_t scaled = (int64_t(value) - anchor) * percentage + carry;
        pending = scaled % 100;
        int64_t position = int64_t(anchor) + scaled / 100;
        return int(std::max<int64_t>((std::numeric_limits<int>::min)(),
            std::min<int64_t>((std::numeric_limits<int>::max)(), position)));
    }

    unsigned percentage = 100;
    bool anchored = false;
    int anchorX = 0, anchorY = 0;
    int64_t carryX = 0, carryY = 0, pendingX = 0, pendingY = 0;
};

#endif
