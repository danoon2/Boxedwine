/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later.
 * Run under X11: make -C project/linux test-cdrom-statusbar
 * Also works with xvfb-run -a for the display/GL lifecycle checks. */
#include "boxedwine.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_syswm.h>
#include "sdlStatusBar.h"
#include "devcdrom.h"
#include <cassert>
#include <cstdio>

#ifdef SDL_VIDEO_DRIVER_X11
static void* nativeXtest;
// Real X11 pointer events cover WM/input routing that the synthetic SDL tests
// below cannot exercise. Run with --native-pointer, preferably under xvfb-run.
static void testNativePopupInput(SDL_Window* guest, SDL_Window* host, SDL_Window* popup) {
    SDL_SysWMinfo info = {}, parent = {};
    SDL_VERSION(&info.version); SDL_VERSION(&parent.version);
    assert(SDL_GetWindowWMInfo(popup, &info) && info.subsystem == SDL_SYSWM_X11);
    assert(SDL_GetWindowWMInfo(host, &parent));
    void* xlib = SDL_LoadObject("libX11.so.6");
    if (!nativeXtest) nativeXtest = SDL_LoadObject("libXtst.so.6");
    auto attributes = (Status (*)(Display*, Window, XWindowAttributes*))SDL_LoadFunction(xlib, "XGetWindowAttributes");
    auto transient = (Status (*)(Display*, Window, Window*))SDL_LoadFunction(xlib, "XGetTransientForHint");
    auto translate = (Bool (*)(Display*, Window, Window, int, int, int*, int*, Window*))SDL_LoadFunction(xlib, "XTranslateCoordinates");
    auto sync = (int (*)(Display*, Bool))SDL_LoadFunction(xlib, "XSync");
    auto motion = (int (*)(Display*, int, int, int, unsigned long))SDL_LoadFunction(nativeXtest, "XTestFakeMotionEvent");
    auto button = (int (*)(Display*, unsigned int, Bool, unsigned long))SDL_LoadFunction(nativeXtest, "XTestFakeButtonEvent");
    assert(attributes && transient && translate && sync && motion && button);
    Display* display = info.info.x11.display;
    bool sawPopupMotion = false;
    auto pump = [&]() {
        U32 until = SDL_GetTicks() + 100;
        do {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_MOUSEMOTION && event.motion.windowID == SDL_GetWindowID(popup)) sawPopupMotion = true;
                sdlStatusBarEvent(event);
            }
            sdlStatusBarTick();
            SDL_Delay(5);
        } while ((S32)(SDL_GetTicks() - until) < 0);
    };
    auto visible = [&]() { return (SDL_GetWindowFlags(popup) & SDL_WINDOW_SHOWN) != 0; };
    auto click = [&](SDL_Window* target, int x, int y) {
        SDL_SysWMinfo targetInfo = {}; SDL_VERSION(&targetInfo.version);
        assert(SDL_GetWindowWMInfo(target, &targetInfo));
        int screenX, screenY; Window child;
        assert(translate(display, targetInfo.info.x11.window, DefaultRootWindow(display), x, y, &screenX, &screenY, &child));
        motion(display, -1, screenX, screenY, 0); sync(display, False); pump();
        button(display, SDL_BUTTON_LEFT, True, 0); sync(display, False); pump();
        button(display, SDL_BUTTON_LEFT, False, 0); sync(display, False); pump();
    };
    pump();
    assert(visible());
    XWindowAttributes actual = {}; Window actualParent;
    assert(attributes(display, info.info.x11.window, &actual) && actual.override_redirect);
    assert(transient(display, info.info.x11.window, &actualParent) && actualParent == parent.info.x11.window);
    int expectedX, expectedY, actualX, actualY; Window child;
    SDL_GetWindowPosition(popup, &expectedX, &expectedY);
    assert(translate(display, info.info.x11.window, DefaultRootWindow(display), 0, 0, &actualX, &actualY, &child));
    assert(actualX == expectedX && actualY == expectedY);
    assert(SDL_GetKeyboardFocus() == popup);
    // Manage / Done requires two complete pointer clicks in the actual menu.
    click(popup, 100, 84); assert(visible() && sawPopupMotion);
    click(popup, 100, 84); assert(!visible());
    click(host, 100, 495); assert(visible());
    click(guest, 200, 150); assert(!visible());
    assert(SDL_GetKeyboardFocus() == guest);
    click(host, 100, 495); assert(visible());
    SDL_Event escape = {}; escape.type = SDL_KEYDOWN;
    escape.key.windowID = SDL_GetWindowID(popup); escape.key.keysym.sym = SDLK_ESCAPE;
    assert(sdlStatusBarEvent(escape)); pump(); assert(!visible());
    click(host, 100, 495); assert(visible()); // Leave the original test's initial state.
    // XTest registers Xlib display-close hooks. Keep it loaded until SDL_Quit.
    SDL_UnloadObject(xlib);
    puts("Native X11 popup pointer, ownership, focus, and dismissal checks passed");
}
#endif

int main(int argc, char** argv) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    const BString root = B("../../tmp/statusbar-native-test-root");
    std::filesystem::create_directories(root.c_str());
    assert(Fs::initFileSystem(root));
    Fs::makeLocalDirs(B("/mnt"));
    cdromInit({}, B("/mnt/cdrom"), B(""));
    sdlStatusBarConfigure(true);
    for (bool useGl : {false, true}) {
        SDL_Window* guest = SDL_CreateWindow("Status bar harness", 20, 20, 640, 480,
            SDL_WINDOW_HIDDEN | (useGl ? SDL_WINDOW_OPENGL : 0));
        assert(guest);
        SDL_GLContext context = useGl ? SDL_GL_CreateContext(guest) : nullptr;
        if (useGl) assert(context);
        sdlStatusBarShowWindow(guest, true);
        sdlStatusBarTick();
        SDL_Window* host = nullptr;
        for (U32 id = 1; id < 32; ++id) {
            SDL_Window* candidate = SDL_GetWindowFromID(id);
            if (candidate && sdlStatusBarOwns(candidate) &&
                    !strcmp(SDL_GetWindowTitle(candidate), "Status bar harness")) host = candidate;
        }
        assert(host && host != guest);
        int w, h;
        SDL_GetWindowSize(host, &w, &h);
        assert(w == 640 && h == 512);
        SDL_SetWindowPosition(host, 80, 90);
        sdlStatusBarTick();
        SDL_GetWindowSize(guest, &w, &h);
        assert(w == 640 && h == 480);
        assert(sdlStatusBarInputWindow(host) == guest);
        SDL_Event key = {};
        key.type = SDL_KEYDOWN;
        key.key.windowID = SDL_GetWindowID(host);
        key.key.keysym.sym = SDLK_F1;
        assert(!sdlStatusBarEvent(key) && key.key.windowID == SDL_GetWindowID(guest));
        SDL_Event motion = {};
        motion.type = SDL_MOUSEMOTION;
        motion.motion.windowID = SDL_GetWindowID(host);
        motion.motion.y = 200;
        assert(!sdlStatusBarEvent(motion) && motion.motion.windowID == SDL_GetWindowID(guest));
        motion.motion.windowID = SDL_GetWindowID(host);
        motion.motion.y = 490;
        assert(sdlStatusBarEvent(motion));
#ifdef SDL_VIDEO_DRIVER_X11
        SDL_SysWMinfo child = {}, parent = {};
        SDL_VERSION(&child.version); SDL_VERSION(&parent.version);
        assert(SDL_GetWindowWMInfo(guest, &child) && SDL_GetWindowWMInfo(host, &parent));
        void* library = SDL_LoadObject("libX11.so.6");
        auto query = (Status (*)(Display*, Window, Window*, Window*, Window**, unsigned int*))SDL_LoadFunction(library, "XQueryTree");
        auto freeData = (int (*)(void*))SDL_LoadFunction(library, "XFree");
        Window rootWindow, actualParent, *children;
        unsigned int count;
        assert(query && freeData && query(child.info.x11.display, child.info.x11.window,
            &rootWindow, &actualParent, &children, &count));
        assert(actualParent == parent.info.x11.window); // one native hierarchy, not a companion window
        if (children) freeData(children);
        SDL_UnloadObject(library);
#endif
        if (useGl) {
            assert(SDL_GL_GetCurrentContext() == context);
            SDL_GL_GetDrawableSize(guest, &w, &h);
            assert(w == 640 && h == 480);
            glClearColor(.25f, .5f, .75f, 1);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        sdlStatusBarOpen();
        sdlStatusBarTick();
        if (useGl) {
            unsigned char pixel[4];
            assert(SDL_GL_GetCurrentContext() == context);
            glReadPixels(320, 240, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
            assert(pixel[0] >= 63 && pixel[0] <= 65 && pixel[1] >= 127 && pixel[1] <= 129);
            assert(glGetError() == GL_NO_ERROR);
        }
        SDL_Window* popup = nullptr;
        for (U32 id = 1; id < 32; ++id) {
            SDL_Window* candidate = SDL_GetWindowFromID(id);
            if (candidate && !strcmp(SDL_GetWindowTitle(candidate), "Boxedwine CD-ROM menu")) popup = candidate;
        }
        assert(popup);
#ifdef SDL_VIDEO_DRIVER_X11
        if (argc > 1 && !strcmp(argv[1], "--native-pointer")) {
            // The synthetic lifecycle checks queue focus events without an
            // event loop. Begin the real-input checks from the current state.
            SDL_PumpEvents();
            SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
            testNativePopupInput(guest, host, popup);
        }
#endif
        auto popupVisible = [&]() { return (SDL_GetWindowFlags(popup) & SDL_WINDOW_SHOWN) != 0; };
        auto mouseButton = [&](SDL_Window* window, U32 type, int x, int y) {
            SDL_Event event = {};
            event.type = type;
            event.button.windowID = SDL_GetWindowID(window);
            event.button.button = SDL_BUTTON_LEFT;
            event.button.x = x;
            event.button.y = y;
            return sdlStatusBarEvent(event);
        };
        auto openFromBar = [&]() {
            assert(mouseButton(host, SDL_MOUSEBUTTONDOWN, 100, 495));
            assert(mouseButton(host, SDL_MOUSEBUTTONUP, 100, 495));
            assert(popupVisible());
        };
        // Dismiss through either the embedded child or the host's guest area.
        // A queued release retaining the bar's coordinates must not reopen it.
        struct OutsideClick { SDL_Window* window; int x, y; };
        for (const OutsideClick& outside : {OutsideClick{host, 200, 150}, OutsideClick{guest, 200, 150},
                OutsideClick{popup, 500, 50}, OutsideClick{popup, 100, -100}}) {
            assert(popupVisible());
            assert(mouseButton(outside.window, SDL_MOUSEBUTTONDOWN, outside.x, outside.y));
            assert(!popupVisible());
            assert(mouseButton(host, SDL_MOUSEBUTTONUP, 100, 495));
            assert(!popupVisible());
            assert(!mouseButton(host, SDL_MOUSEBUTTONDOWN, 200, 150));
            assert(!mouseButton(host, SDL_MOUSEBUTTONUP, 200, 150));
            assert(!popupVisible());
            openFromBar();
        }
        // Escape/focus loss can arrive before the rest of a mouse gesture.
        SDL_Event lost = {};
        lost.type = SDL_WINDOWEVENT;
        lost.window.windowID = SDL_GetWindowID(popup);
        lost.window.event = SDL_WINDOWEVENT_FOCUS_LOST;
        assert(sdlStatusBarEvent(lost) && !popupVisible());
        assert(mouseButton(host, SDL_MOUSEBUTTONUP, 100, 495));
        assert(!popupVisible());
        // Releasing outside a control cancels a press, including a drag out of
        // the bar and SDL-captured events below/right of the host window.
        assert(mouseButton(host, SDL_MOUSEBUTTONDOWN, 100, 495));
        assert(!mouseButton(host, SDL_MOUSEBUTTONUP, 200, 150));
        assert(mouseButton(host, SDL_MOUSEBUTTONUP, 100, 495));
        assert(mouseButton(host, SDL_MOUSEBUTTONDOWN, 100, 550));
        assert(mouseButton(host, SDL_MOUSEBUTTONUP, 100, 550));
        assert(mouseButton(host, SDL_MOUSEBUTTONDOWN, 500, 495));
        assert(mouseButton(host, SDL_MOUSEBUTTONUP, 500, 495));
        assert(!popupVisible());
        openFromBar();
        // Popup actions also need a complete click in the same row. Manage /
        // Done exercises a real action without invoking a native file picker.
        assert(mouseButton(popup, SDL_MOUSEBUTTONUP, 100, 84));
        assert(mouseButton(popup, SDL_MOUSEBUTTONDOWN, 100, 84));
        assert(mouseButton(popup, SDL_MOUSEBUTTONUP, 100, 84));
        assert(popupVisible());
        assert(mouseButton(popup, SDL_MOUSEBUTTONDOWN, 100, 84));
        assert(mouseButton(popup, SDL_MOUSEBUTTONUP, 100, 84));
        assert(!popupVisible());
        SDL_FlushEvent(SDL_QUIT);
        SDL_Event close = {};
        close.type = SDL_WINDOWEVENT;
        close.window.windowID = SDL_GetWindowID(guest);
        close.window.event = SDL_WINDOWEVENT_CLOSE;
        assert(sdlStatusBarEvent(close));
        SDL_Event quit = {};
        assert(SDL_PeepEvents(&quit, 1, SDL_GETEVENT, SDL_QUIT, SDL_QUIT) == 1);
        sdlStatusBarWindowDestroyed(guest);
        if (context) SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(guest);
    }
    sdlStatusBarConfigure(false);
    Fs::shutDown();
    SDL_Quit();
#ifdef SDL_VIDEO_DRIVER_X11
    if (nativeXtest) SDL_UnloadObject(nativeXtest);
#endif
    puts("CD-ROM popup dismissal, click routing, host hierarchy, drawable size, and GL state checks passed");
}
