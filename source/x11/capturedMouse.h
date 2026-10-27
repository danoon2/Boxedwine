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
    struct HostPosition { int x, y; };

    void configure(bool captured, bool visible, bool focused, unsigned percent) {
        bool nextActive = captured && !visible && focused;
        unsigned next = percent ? percent : 100;
        if (nextActive != active || next != percentage) {
            reset();
            active = nextActive;
            percentage = next;
        }
    }

    void reset() {
        anchored = false;
        carryX = carryY = pendingX = pendingY = 0;
    }

    void warp(int x, int y, const HostPosition* host = nullptr) {
        if (!active) return;
        // Retain subpixel movement across recentering, but never multiply the
        // programmatic warp. Repeated warps without movement remain exact.
        carryX = pendingX;
        carryY = pendingY;
        anchorX = x;
        anchorY = y;
        hasHostAnchor = host != nullptr;
        if (host) hostAnchor = *host;
        anchored = true;
    }

    void map(int& x, int& y, const HostPosition* host = nullptr) {
        if (!active || !anchored) return;
        // Measure mouse look in host pixels before display scaling rounds
        // away small movement. The result remains about the guest warp point.
        // Recordings without a host position retain guest-coordinate mapping.
        bool physical = host && hasHostAnchor;
        x = axis(physical ? int64_t(host->x) - hostAnchor.x : int64_t(x) - anchorX, anchorX, carryX, pendingX);
        y = axis(physical ? int64_t(host->y) - hostAnchor.y : int64_t(y) - anchorY, anchorY, carryY, pendingY);
    }

private:
    int axis(int64_t delta, int anchor, int64_t carry, int64_t& pending) const {
        int64_t scaled = delta * percentage + carry;
        pending = scaled % 100;
        int64_t position = int64_t(anchor) + scaled / 100;
        return int(std::max<int64_t>((std::numeric_limits<int>::min)(),
            std::min<int64_t>((std::numeric_limits<int>::max)(), position)));
    }

    unsigned percentage = 100;
    bool active = false, hasHostAnchor = false;
    HostPosition hostAnchor = {};
    bool anchored = false;
    int anchorX = 0, anchorY = 0;
    int64_t carryX = 0, carryY = 0, pendingX = 0, pendingY = 0;
};

#endif
