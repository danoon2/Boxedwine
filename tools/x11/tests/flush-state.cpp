// Native regression checks using request sequences observed in AoE and Bang! Bang!.
// clang++ -std=c++17 tools/x11/tests/flush-state.cpp -o /tmp/x11-flush-state
#include "../../../source/x11/x11flush.h"
#include <cstdio>
#include <initializer_list>

static int checks, failures;

static void check(X11FlushState& state, const char* name, bool immediate) {
    ++checks;
    if (state.takePendingRequests() != immediate) {
        std::printf("FAIL %s\n", name);
        ++failures;
    }
    // callX11 records the completed XFlush too; it must not create more work.
    state.recordRequest(X11_FLUSH);
}

static void requests(X11FlushState& state, std::initializer_list<unsigned int> calls) {
    for (auto call : calls) state.recordRequest(call);
}

static void pollInput(X11FlushState& state) {
    requests(state, {X11_LOCK_EVENTS, X11_GET_EVENT, X11_FIND_CONTEXT,
        X11_FILTER_EVENT, X11_REMOVE_EVENT, X11_UNLOCK_EVENTS, X11_QUERY_POINTER});
}

int main() {
    X11FlushState state;
    check(state, "initial flush", true);

    // AoE's stationary menu: input checks, then two flushes without drawing.
    for (int i = 0; i < 100; ++i) {
        pollInput(state);
        check(state, "input-only GDI display flush", false);
        check(state, "input-only event display flush", false);
    }

    // Bang! Bang!'s two GC setup sequences per animation step. A pixel-dirty
    // check misses this work and makes the projectile run too quickly to see.
    for (int i = 0; i < 100; ++i) {
        requests(state, {X11_CREATE_GC, X11_SET_SUBWINDOW_MODE, X11_SET_GRAPHICS_EXPOSURES});
        check(state, "drawing setup before a pixel upload", true);
        requests(state, {X11_FREE_GC, X11_FREE_GC, X11_CREATE_GC,
            X11_SET_SUBWINDOW_MODE, X11_SET_GRAPHICS_EXPOSURES});
        check(state, "next drawing setup without a frame delay", true);
    }

    for (auto operation : {X11_PUT_IMAGE, X11_COPY_AREA, X11_FILL_RECTANGLE,
                          X11_MAP_WINDOW, X11_UNMAP_WINDOW, X11_CONFIGURE_WINDOW}) {
        state.recordRequest(operation);
        pollInput(state);
        check(state, "input polling preserves pending output", true);
        check(state, "output is consumed once", false);
    }
    // Multiple drawings between flushes are batched, but a subsequent drawing
    // followed by idle still forces its own final update.
    requests(state, {X11_PUT_IMAGE, X11_PUT_IMAGE});
    check(state, "batched output", true);
    state.recordRequest(X11_PUT_IMAGE);
    check(state, "final update before idle", true);
    std::printf("RESULT checks=%d failures=%d\n", checks, failures);
    return failures ? 1 : 0;
}
