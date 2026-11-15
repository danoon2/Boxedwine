/* Copyright (C) 2012-2026 The BoxedWine Team. GPL-2.0-or-later. */
#include "macOpenGLViewport.h"
#include <cassert>
#include <cstdlib>

int main() {
    // 4:3 game on a widescreen display: full height, centered pillarboxing.
    auto v = MacOpenGLViewport::fit(1920, 1080, 640, 480, true);
    assert(v.valid() && v.x == 240 && v.y == 0 && v.width == 1440 && v.height == 1080);
    assert(v.toGuestX(240) == 0 && v.toGuestX(960) == 320);
    assert(v.toGuestY(540) == 240 && v.toHostX(320) == 960 && v.toHostY(240) == 540);
    assert(v.toGuestX(0) < 0); // bars must not compress into the game image
    assert(v.deltaX(90) == 40 && v.deltaX(-90) == -40);

    // Alice recenters at (319, 239) after mouse input. Every warp must read
    // back unchanged, or the rounding error becomes continuous menu motion.
    int mouseX = 319, mouseY = 239;
    for (int i = 0; i < 100; ++i) {
        mouseX = v.toGuestX(v.toHostX(mouseX));
        mouseY = v.toGuestY(v.toHostY(mouseY));
        assert(mouseX == 319 && mouseY == 239);
    }

    // Widescreen game on a taller display: horizontal bars, SDL top-left origin.
    v = MacOpenGLViewport::fit(1024, 768, 1280, 720, true);
    assert(v.x == 0 && v.y == 96 && v.width == 1024 && v.height == 576);
    assert(v.toGuestY(96) == 0 && v.toGuestY(384) == 360);
    assert(v.toHostY(360) == 384);

    // Stretch deliberately uses independent scales, with no letterbox offsets.
    v = MacOpenGLViewport::fit(1920, 1080, 640, 480, false);
    assert(v.x == 0 && v.y == 0 && v.width == 1920 && v.height == 1080);
    assert(v.toGuestX(960) == 320 && v.toGuestY(540) == 240);

    // Odd display sizes and resolution changes must retain valid bounds and
    // stable cursor warps. Input uses window points even on a Retina screen.
    for (int gw : {320, 640, 800, 1024, 1920}) {
        for (int gh : {200, 480, 600, 768, 1080}) {
            v = MacOpenGLViewport::fit(1512, 945, gw, gh, true);
            assert(v.valid() && v.guestWidth == gw && v.guestHeight == gh);
            assert(v.x >= 0 && v.y >= 0 && v.x + v.width <= 1512 && v.y + v.height <= 945);
            for (int x = 0; x < gw; ++x) {
                int hostX = v.toHostX(x);
                assert(hostX >= v.x && hostX < v.x + v.width);
                assert(std::abs(v.toGuestX(hostX) - x) <= (v.width >= gw ? 0 : 2));
            }
            for (int y = 0; y < gh; ++y) {
                int hostY = v.toHostY(y);
                assert(hostY >= v.y && hostY < v.y + v.height);
                assert(std::abs(v.toGuestY(hostY) - y) <= (v.height >= gh ? 0 : 2));
            }
        }
    }
    assert(!MacOpenGLViewport::fit(0, 1080, 640, 480, true).valid());
    assert(!MacOpenGLViewport::fit(1920, 1080, 0, 480, true).valid());
    assert(!MacOpenGLViewport::fit(1920, -1, 640, 480, true).valid());
}
