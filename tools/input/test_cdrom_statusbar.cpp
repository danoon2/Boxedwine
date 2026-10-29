/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later.
 * Run under Wayland or X11: make -C project/linux test-cdrom-statusbar
 * Also works with xvfb-run -a for the display/GL lifecycle checks. */
#include "boxedwine.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_syswm.h>
#include "sdlStatusBar.h"
#include "devcdrom.h"
#define private public
#include "knativeinputSDL.h"
#include "knativescreenSDL.h"
#undef private
#include "knativesystem.h"
#include "../../source/sdl/startupArgs.h"
#include <cassert>
#include <cstdio>
#if defined(__linux__) && defined(SDL_VIDEO_DRIVER_WAYLAND)
int testWaylandStatusBar(bool nativePicker);
#endif
#ifdef __APPLE__
#include "../../platform/mac/macOpenGL.h"
void testCocoaFullscreenButton(SDL_Window* guest, SDL_Window* host);
void testCocoaStatusBar(SDL_Window* guest, SDL_Window* host, SDL_Window* popup);
void testCocoaVulkanStatusBar();
#endif

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
    // Eject exercises a complete click in the actual menu without a picker.
    click(popup, 100, 52); assert(!visible() && sawPopupMotion);
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

static void testEmbeddedMouseCapture(SDL_Window* guest, SDL_Window* host) {
    // Exercise the real SDL focus/grab backend. A mocked SetWindowGrab cannot
    // catch the Windows child-view bug: SDL rejects grabs on unfocused windows.
    SDL_PumpEvents();
    SDL_Window* focused = SDL_GetKeyboardFocus();
    assert(focused && sdlStatusBarInputWindow(focused) == guest);
#ifdef BOXEDWINE_MSVC
    assert(focused == host);
#endif
    KNativeInputSDL input(640, 480, 100, 100);
    input.mouseConfinementRequested = true;
    auto focus = [&](bool gained) {
        SDL_Event event = {};
        event.type = SDL_WINDOWEVENT;
        event.window.windowID = SDL_GetWindowID(guest);
        event.window.event = gained ? SDL_WINDOWEVENT_FOCUS_GAINED : SDL_WINDOWEVENT_FOCUS_LOST;
        assert(input.handlSdlEvent(&event));
    };
    focus(true);
    assert(SDL_GetWindowGrab(focused) == SDL_TRUE);
    assert(input.confinedWindowId == SDL_GetWindowID(focused));
    assert(!strcmp(SDL_GetWindowTitle(guest), "Status bar harness"));
#ifdef BOXEDWINE_MSVC
    SDL_SysWMinfo info = {}; SDL_VERSION(&info.version);
    assert(SDL_GetWindowWMInfo(host, &info));
    RECT expected, actual;
    assert(GetClientRect(info.info.win.window, &expected));
    ClientToScreen(info.info.win.window, (POINT*)&expected.left);
    ClientToScreen(info.info.win.window, (POINT*)&expected.right);
#if SDL_VERSION_ATLEAST(2, 0, 18)
    expected.bottom = expected.top + 480; // Captured area excludes the status strip.
#endif
    assert(GetClipCursor(&actual) && EqualRect(&expected, &actual));
#endif
    // Reproduce leaving downward: motion in the outer window's status strip
    // used to call releaseMouseForHostUi(), allowing escape and pausing games.
    SDL_Event motion = {};
    motion.type = SDL_MOUSEMOTION;
    motion.motion.windowID = SDL_GetWindowID(host);
    motion.motion.x = 200;
    motion.motion.y = 510;
    assert(input.handlSdlEvent(&motion));
    assert(!input.isMouseCaptureReleased() && SDL_GetWindowGrab(focused) == SDL_TRUE);
    assert(motion.motion.windowID == SDL_GetWindowID(guest) && motion.motion.y == 479);
#ifdef BOXEDWINE_MSVC
    SDL_SysWMinfo child = {}; SDL_VERSION(&child.version);
    assert(SDL_GetWindowWMInfo(guest, &child));
    POINT cursor;
    assert(GetCursorPos(&cursor) && ScreenToClient(child.info.win.window, &cursor));
    assert(cursor.y >= 0 && cursor.y < 480);
#endif
    // Explicit release and focus loss must release the host without changing
    // the application's title or its guest input association.
    input.mouseCaptureReleased = true;
    focus(true);
    assert(SDL_GetWindowGrab(focused) == SDL_FALSE);
    assert(!strcmp(SDL_GetWindowTitle(guest), "Status bar harness"));
    input.mouseCaptureReleased = false;
    focus(true);
    assert(SDL_GetWindowGrab(focused) == SDL_TRUE);
    focus(false);
    assert(SDL_GetWindowGrab(focused) == SDL_FALSE);
    assert(!strcmp(SDL_GetWindowTitle(guest), "Status bar harness"));
    puts("Embedded mouse confinement, release, recapture, and caption checks passed");
}

static void settleStatusBar() {
    U32 until = SDL_GetTicks() + 250;
    do {
        SDL_Event event;
        while (SDL_PollEvent(&event)) sdlStatusBarEvent(event);
        sdlStatusBarTick();
        SDL_Delay(5);
    } while ((S32)(SDL_GetTicks() - until) < 0);
}

static void testViewportMouseCapture(KNativeInputSDL& input, SDL_Window* guest, SDL_Rect expected) {
    input.mouseConfinementRequested = true;
    input.updateMouseConfinement(guest);
    SDL_Window* focused = SDL_GetKeyboardFocus();
    assert(focused && SDL_GetWindowGrab(focused));
#if SDL_VERSION_ATLEAST(2, 0, 18)
    const SDL_Rect* rect = SDL_GetWindowMouseRect(focused);
    assert(rect && SDL_RectEquals(rect, &expected));
#endif
    const int left = expected.x, right = expected.x + expected.w - 1;
    const int top = expected.y, bottom = expected.y + expected.h - 1;
    for (auto point : {SDL_Point{left - 100, top - 100}, SDL_Point{right + 100, bottom + 100}}) {
        SDL_Event event = {};
        event.type = SDL_MOUSEMOTION;
        event.motion.windowID = SDL_GetWindowID(guest);
        event.motion.x = point.x; event.motion.y = point.y;
        event.motion.xrel = 7; event.motion.yrel = -9;
        assert(input.handlSdlEvent(&event));
        assert(event.motion.x == (point.x < left ? left : right));
        assert(event.motion.y == (point.y < top ? top : bottom));
        assert(event.motion.xrel == 7 && event.motion.yrel == -9); // Mouse-look deltas retain their scale.
        int globalX, globalY, windowX, windowY;
        SDL_GetGlobalMouseState(&globalX, &globalY);
        SDL_GetWindowPosition(focused, &windowX, &windowY);
        assert(std::abs(globalX - windowX - event.motion.x) <= 1);
        assert(std::abs(globalY - windowY - event.motion.y) <= 1);
        // A small move back must immediately leave the edge, regardless of how
        // far the preceding event overshot into the black bars.
        event.motion.x += point.x < left ? 3 : -3;
        event.motion.y += point.y < top ? 3 : -3;
        int x = event.motion.x, y = event.motion.y;
        assert(input.handlSdlEvent(&event));
        assert(event.motion.x == x && event.motion.y == y);
        assert(x > left && x < right && y > top && y < bottom);
    }
    input.mouseCaptureReleased = true;
    input.updateMouseConfinement(guest);
    assert(!SDL_GetWindowGrab(focused));
#if SDL_VERSION_ATLEAST(2, 0, 18)
    assert(!SDL_GetWindowMouseRect(focused));
#endif
    input.mouseCaptureReleased = false;
    input.updateMouseConfinement(guest);
    assert(SDL_GetWindowGrab(focused));
    input.updateMouseConfinement(nullptr);
    assert(!SDL_GetWindowGrab(focused));
#if SDL_VERSION_ATLEAST(2, 0, 18)
    assert(!SDL_GetWindowMouseRect(focused));
#endif
    input.mouseConfinementRequested = false;
}

void testSoftwareFullscreenScale() {
    sdlStatusBarConfigure(true);
    KNativeSystem::initWindow(640, 480, 32, 100, 100, B("0"), FULLSCREEN_NOTSET, VSYNC_DISABLED);
    auto screen = std::static_pointer_cast<KNativeScreenSDL>(KNativeSystem::getScreen());
    sdlStatusBarShowWindow(screen->window, true);
    assert(sdlStatusBarToggleFullscreen());
    settleStatusBar();
    int width, height;
    SDL_GetWindowSize(screen->window, &width, &height);
    U32 expected = std::min(width * 100 / 640, height * 100 / 480);
    assert(screen->input->scaleX == expected && screen->input->scaleY == expected);
    assert(screen->screenWidth() == 640 && screen->screenHeight() == 480);
    assert(screen->input->scaleXOffset == (width - 640 * expected / 100) / 2);
    assert(screen->input->scaleYOffset == (height - 480 * expected / 100) / 2);
    testViewportMouseCapture(*screen->input, screen->window,
        {(int)screen->input->scaleXOffset, (int)screen->input->scaleYOffset,
            (int)(640 * expected / 100), (int)(480 * expected / 100)});
    // A game can change its own mode while fullscreen. Return to a window
    // sized for that mode, retaining the original windowed scale.
    screen->setScreenSize(800, 600);
    assert(sdlStatusBarToggleFullscreen());
    settleStatusBar();
    SDL_GetWindowSize(screen->window, &width, &height);
    assert(width == 800 && height == 600);
    assert(screen->input->scaleX == 100 && screen->input->scaleY == 100);
    assert(screen->input->scaleXOffset == 0 && screen->input->scaleYOffset == 0);
    testViewportMouseCapture(*screen->input, screen->window, {0, 0, 800, 600});
    screen.reset();
    KNativeSystem::shutdown();
    puts("Software fullscreen scaling, guest mode preservation and mode-change restoration checks passed");
}

static void testRuntimeFullscreen(SDL_Window* guest, SDL_Window* host) {
    int x, y;
    SDL_GetWindowPosition(host, &x, &y);
    KNativeInputSDL input(640, 480, 100, 100);
    input.mouseConfinementRequested = true;
    auto refreshInput = [&]() {
        SDL_PumpEvents();
        SDL_Event event = {}; event.type = SDL_WINDOWEVENT;
        event.window.windowID = SDL_GetWindowID(guest);
        event.window.event = SDL_WINDOWEVENT_FOCUS_GAINED;
        assert(input.handlSdlEvent(&event));
    };
    refreshInput();
    // Entering fullscreen by shortcut recaptures a waiting game immediately.
    input.setMouseCaptureReleased(true);
    assert(input.isMouseCaptureReleased());
    SDL_Event key = {};
    key.type = SDL_KEYDOWN;
    key.key.windowID = SDL_GetWindowID(host);
    key.key.keysym.sym = SDLK_RETURN;
    key.key.keysym.mod = KMOD_ALT;
    assert(input.handlSdlEvent(&key));
    settleStatusBar();
    assert(SDL_GetWindowFlags(guest) & SDL_WINDOW_FULLSCREEN);
    assert(!(SDL_GetWindowFlags(host) & SDL_WINDOW_SHOWN));
    assert(!input.isMouseCaptureReleased());
    SDL_PumpEvents();
    refreshInput();
    assert(SDL_GetKeyboardFocus() == guest);
    assert(SDL_GetWindowGrab(guest));
    // Fullscreen hides the outer host, but motion queued for it may arrive
    // after the guest has capture. These are not user requests to release it.
    for (U32 type : {SDL_MOUSEMOTION, SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONUP, SDL_KEYDOWN}) {
        SDL_Event stale = {};
        stale.type = type;
        stale.window.windowID = SDL_GetWindowID(host);
        if (type == SDL_MOUSEMOTION) { stale.motion.x = 200; stale.motion.y = 495; }
        if (type == SDL_MOUSEBUTTONDOWN || type == SDL_MOUSEBUTTONUP) {
            stale.button.button = SDL_BUTTON_LEFT; stale.button.x = 580; stale.button.y = 495;
        }
        if (type == SDL_KEYDOWN) stale.key.keysym.sym = SDLK_SPACE;
        assert(input.handlSdlEvent(&stale));
        assert(!input.isMouseCaptureReleased() && SDL_GetWindowGrab(guest));
        assert(SDL_GetWindowFlags(guest) & SDL_WINDOW_FULLSCREEN);
    }
    key.key.repeat = 1;
    assert(input.handlSdlEvent(&key));
    assert(SDL_GetWindowFlags(guest) & SDL_WINDOW_FULLSCREEN);
    key.type = SDL_KEYUP;
    key.key.keysym.mod = 0; // Alt may be released before Enter.
    assert(input.handlSdlEvent(&key));
    sdlStatusBarTick();
    SDL_Window* hint = nullptr;
    for (U32 id = 1; id < 64; ++id) {
        auto candidate = SDL_GetWindowFromID(id);
        if (candidate && !strcmp(SDL_GetWindowTitle(candidate), "Boxedwine fullscreen hint")) hint = candidate;
    }
    if (!hint) fprintf(stderr, "Missing fullscreen hint: guest flags=%x, SDL: %s\n", SDL_GetWindowFlags(guest), SDL_GetError());
    assert(hint);
    SDL_Event passive = {}; passive.type = SDL_MOUSEMOTION;
    passive.motion.windowID = SDL_GetWindowID(hint);
    assert(input.handlSdlEvent(&passive));
    assert(!input.isMouseCaptureReleased() && SDL_GetWindowGrab(guest));
#ifdef BOXEDWINE_MSVC
    assert(SDL_GetWindowFlags(hint) & SDL_WINDOW_SHOWN);
    SDL_SysWMinfo info = {}; SDL_VERSION(&info.version);
    assert(SDL_GetWindowWMInfo(hint, &info));
    LONG_PTR style = GetWindowLongPtr(info.info.win.window, GWL_EXSTYLE);
    assert((style & (WS_EX_NOACTIVATE | WS_EX_TRANSPARENT | WS_EX_LAYERED)) ==
        (WS_EX_NOACTIVATE | WS_EX_TRANSPARENT | WS_EX_LAYERED));
#endif
    SDL_Delay(4050);
    sdlStatusBarTick();
    assert(!(SDL_GetWindowFlags(hint) & SDL_WINDOW_SHOWN));
    key.type = SDL_KEYDOWN;
    key.key.windowID = SDL_GetWindowID(guest);
    key.key.keysym.mod = KMOD_ALT;
    key.key.repeat = 0;
    assert(input.handlSdlEvent(&key));
    settleStatusBar();
    assert(!(SDL_GetWindowFlags(guest) & SDL_WINDOW_FULLSCREEN));
    key.type = SDL_KEYUP;
    assert(input.handlSdlEvent(&key));
    SDL_PumpEvents();
    refreshInput();
    SDL_Window* focused = SDL_GetKeyboardFocus();
    assert(focused && sdlStatusBarInputWindow(focused) == guest && SDL_GetWindowGrab(focused));
    assert(!input.isMouseCaptureReleased());
    int w, h, restoredX, restoredY;
    SDL_GetWindowSize(guest, &w, &h);
    SDL_GetWindowPosition(host, &restoredX, &restoredY);
    printf("Restored %dx%d at %d,%d (expected 640x480 at %d,%d)\n", w, h, restoredX, restoredY, x, y);
    assert(w == 640 && h == 480 && restoredX == x && restoredY == y);
    assert(input.width == 640 && input.height == 480);
    input.mouseConfinementRequested = false;
    refreshInput();
    // Both halves of a button click are required. Neither may reach Wine.
    SDL_Event click = {}; click.type = SDL_MOUSEBUTTONUP;
    click.button.windowID = SDL_GetWindowID(host);
    click.button.button = SDL_BUTTON_LEFT;
    click.button.x = 580; click.button.y = 495;
    assert(sdlStatusBarEvent(click));
    assert(!(SDL_GetWindowFlags(guest) & SDL_WINDOW_FULLSCREEN));
    click.type = SDL_MOUSEBUTTONDOWN; assert(sdlStatusBarEvent(click));
    click.type = SDL_MOUSEBUTTONUP; assert(sdlStatusBarEvent(click));
    settleStatusBar();
    assert(SDL_GetWindowFlags(guest) & SDL_WINDOW_FULLSCREEN);
    assert(sdlStatusBarToggleFullscreen());
    settleStatusBar();
    assert(!(SDL_GetWindowFlags(guest) & SDL_WINDOW_FULLSCREEN));
    // Exercise the status-bar button through the real input routing too. The
    // mouse-up that enters fullscreen must recapture a game, but not a desktop
    // app whose cursor was temporarily released over the host controls.
    for (bool game : {true, false}) {
        input.mouseConfinementRequested = game;
        refreshInput();
        input.releaseMouseForHostUi();
        assert(input.isMouseCaptureReleased());
        click.type = SDL_MOUSEBUTTONDOWN; assert(input.handlSdlEvent(&click));
        click.type = SDL_MOUSEBUTTONUP; assert(input.handlSdlEvent(&click));
        settleStatusBar();
        refreshInput();
        assert(SDL_GetWindowFlags(guest) & SDL_WINDOW_FULLSCREEN);
        assert(input.isMouseCaptureReleased() == !game);
        assert((SDL_GetWindowGrab(guest) == SDL_TRUE) == game);
        assert(input.pressedMouseButtons == 0); // Host clicks never reach Wine.
        if (game) {
            // The release shortcut remains effective while fullscreen.
            SDL_Event release = {}; release.type = SDL_KEYDOWN;
            release.key.windowID = SDL_GetWindowID(guest);
            release.key.keysym.sym = SDLK_g;
            release.key.keysym.mod = KMOD_CTRL | KMOD_ALT;
            assert(input.handlSdlEvent(&release));
            release.type = SDL_KEYUP; assert(input.handlSdlEvent(&release));
            assert(input.isMouseCaptureReleased() && !SDL_GetWindowGrab(guest));
        }
        key.type = SDL_KEYDOWN; assert(input.handlSdlEvent(&key));
        key.type = SDL_KEYUP; assert(input.handlSdlEvent(&key));
        settleStatusBar();
        refreshInput();
        assert(!(SDL_GetWindowFlags(guest) & SDL_WINDOW_FULLSCREEN));
        assert(input.isMouseCaptureReleased()); // Exiting never recaptures.
        input.setMouseCaptureReleased(false);
        input.mouseConfinementRequested = false;
        refreshInput();
    }
    puts("Fullscreen button and shortcut recapture waiting games; desktop input and explicit release are preserved");
    puts("Runtime fullscreen, capture, hint timeout, button and window restoration checks passed");
}

static void testCaptureWindowReplacement() {
    sdlStatusBarConfigure(true);
    auto create = [] {
        SDL_Window* window = SDL_CreateWindow("Capture handoff", 20, 20, 640, 480, SDL_WINDOW_HIDDEN);
        assert(window);
        sdlStatusBarShowWindow(window, true);
        return window;
    };
    SDL_Window* guest = create();
    assert(sdlStatusBarToggleFullscreen());
    settleStatusBar();
    KNativeInputSDL input(640, 480, 100, 100);
    input.mouseConfinementRequested = true;
    input.updateMouseConfinement(guest);
    assert(SDL_GetWindowGrab(guest) && !input.isMouseCaptureReleased());
    for (bool explicitlyReleased : {false, true}) {
        if (explicitlyReleased) input.setMouseCaptureReleased(true);
        sdlStatusBarWindowDestroyed(guest);
        SDL_DestroyWindow(guest);
        guest = create();
        settleStatusBar();
        assert(SDL_GetWindowFlags(guest) & SDL_WINDOW_FULLSCREEN);
        input.updateMouseConfinement(guest);
        assert(input.isMouseCaptureReleased() == explicitlyReleased);
        assert((SDL_GetWindowGrab(guest) == SDL_TRUE) == !explicitlyReleased);
        SDL_Window* host = nullptr;
        for (U32 id = 1; id < 128; ++id) {
            SDL_Window* candidate = SDL_GetWindowFromID(id);
            if (candidate && sdlStatusBarOwns(candidate) && !strcmp(SDL_GetWindowTitle(candidate), "Capture handoff")) host = candidate;
        }
        assert(host && !(SDL_GetWindowFlags(host) & SDL_WINDOW_SHOWN));
        SDL_Event stale = {}; stale.type = SDL_MOUSEMOTION;
        stale.motion.windowID = SDL_GetWindowID(host);
        stale.motion.x = 100; stale.motion.y = 495;
        assert(input.handlSdlEvent(&stale));
        assert(input.isMouseCaptureReleased() == explicitlyReleased);
        assert((SDL_GetWindowGrab(guest) == SDL_TRUE) == !explicitlyReleased);
    }
    input.mouseConfinementRequested = false;
    input.updateMouseConfinement(nullptr);
    sdlStatusBarWindowDestroyed(guest);
    SDL_DestroyWindow(guest);
    sdlStatusBarConfigure(true);
    puts("Fullscreen window replacement preserves capture and explicit release; hidden host input is ignored");
}

int main(int argc, char** argv) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    SDL_SetMainReady();
    extern bool doMainLoop();
    doMainLoop(); // Register this thread as SDL's owner; there are no guest threads.
    assert(KNativeSystem::init(VIDEO_NORMAL, false));
    // Recapture restores the virtual pointer through the native screen. Keep
    // an offscreen runtime screen while the harness owns its test windows.
    auto videoOption = KSystem::videoOption;
    KSystem::videoOption = VIDEO_NO_WINDOW;
    KNativeSystem::initWindow(640, 480, 32, 100, 100, B("0"), FULLSCREEN_NOTSET, VSYNC_DISABLED);
    KSystem::videoOption = videoOption;
    const BString root = B("../../tmp/statusbar-native-test-root");
    std::filesystem::create_directories(root.c_str());
    assert(Fs::initFileSystem(root));
    Fs::makeLocalDirs(B("/mnt"));
    cdromInit({}, B("/mnt/cdrom"), B(""));
    sdlStatusBarConfigure(true);
#if defined(__linux__) && defined(SDL_VIDEO_DRIVER_WAYLAND)
    if (!strcmp(SDL_GetCurrentVideoDriver(), "wayland")) return testWaylandStatusBar(argc > 1 && !strcmp(argv[1], "--native-picker"));
#endif
    for (bool useGl : {false, true}) {
        U32 flags = SDL_WINDOW_HIDDEN | (useGl ? SDL_WINDOW_OPENGL : 0);
#ifdef __APPLE__
        flags |= SDL_WINDOW_ALLOW_HIGHDPI;
#endif
        SDL_Window* guest = SDL_CreateWindow("Status bar harness", 20, 20, 640, 480, flags);
        assert(guest);
        SDL_GLContext context = useGl ? SDL_GL_CreateContext(guest) : nullptr;
        if (useGl) assert(context);
        int drawableWidth = 0, drawableHeight = 0;
        if (useGl) SDL_GL_GetDrawableSize(guest, &drawableWidth, &drawableHeight);
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
        settleStatusBar();
#ifdef SDL_VIDEO_DRIVER_X11
        // Wait for the WM's initial map/activation before requesting guest
        // focus, as a user click would before starting a capture gesture.
        SDL_SetWindowInputFocus(guest);
        settleStatusBar();
#endif
#ifdef __APPLE__
        testCocoaFullscreenButton(guest, host);
#endif
        testEmbeddedMouseCapture(guest, host);
        testRuntimeFullscreen(guest, host);
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
            assert(w == drawableWidth && h == drawableHeight);
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
#ifdef __APPLE__
        // Exercise AppKit event dispatch, ownership, and movement as well as
        // the platform-independent synthetic routing assertions below.
        testCocoaStatusBar(guest, host, popup);
#endif
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
        // Removal is available immediately, keeps the menu open, and requires
        // pressing and releasing the same control, not just the same row.
        const BString unused = root + "/unused.iso";
        FILE* image = fopen(unused.c_str(), "wb");
        assert(image); fclose(image);
        cdromInit({unused, root + "/missing.iso"}, B("/mnt/cdrom"), B(""));
        SDL_Delay(110); sdlStatusBarTick();
        SDL_GetWindowSize(popup, &w, &h);
        assert(h == 4 * 32 + 8); // Choose, two discs, Eject; no Manage row.
        assert(mouseButton(popup, SDL_MOUSEBUTTONUP, 420, 52));
        assert(mouseButton(popup, SDL_MOUSEBUTTONDOWN, 100, 52));
        assert(mouseButton(popup, SDL_MOUSEBUTTONUP, 420, 52));
        assert(mouseButton(popup, SDL_MOUSEBUTTONDOWN, 420, 52));
        assert(mouseButton(popup, SDL_MOUSEBUTTONUP, 100, 52));
        assert(cdromGetState().discs.size() == 2 && popupVisible());
        assert(mouseButton(popup, SDL_MOUSEBUTTONDOWN, 420, 52));
        assert(mouseButton(popup, SDL_MOUSEBUTTONUP, 420, 52));
        assert(cdromGetState().discs.size() == 1 && popupVisible());
        assert(std::filesystem::exists(unused.c_str())); // Reference only.
        SDL_Event hoverDisc = {}; hoverDisc.type = SDL_MOUSEMOTION;
        hoverDisc.motion.windowID = SDL_GetWindowID(popup); hoverDisc.motion.y = 52;
        assert(sdlStatusBarEvent(hoverDisc));
        SDL_Event remove = {}; remove.type = SDL_KEYDOWN;
        remove.key.windowID = SDL_GetWindowID(popup); remove.key.keysym.sym = SDLK_DELETE;
        assert(sdlStatusBarEvent(remove));
        assert(cdromGetState().discs.empty() && popupVisible());
        SDL_GetWindowSize(popup, &w, &h);
        assert(h == 2 * 32 + 8);
        assert(mouseButton(popup, SDL_MOUSEBUTTONDOWN, 100, 52));
        assert(mouseButton(popup, SDL_MOUSEBUTTONUP, 100, 52));
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
#ifdef __APPLE__
    testCocoaVulkanStatusBar();
    // Model a fresh launch, without the earlier runtime windowed preference.
    sdlStatusBarConfigure(true);
    // Fullscreen uses the original guest window; the menu must still be
    // available without exposing the windowed status-bar host.
    SDL_Window* full = SDL_CreateWindow("Fullscreen status bar harness", 0, 0, 640, 480,
        SDL_WINDOW_HIDDEN | SDL_WINDOW_FULLSCREEN_DESKTOP);
    assert(full);
    sdlStatusBarShowWindow(full, true);
    sdlStatusBarTick();
    assert(SDL_GetWindowFlags(full) & SDL_WINDOW_FULLSCREEN);
    assert(sdlStatusBarInputWindow(full) == full);
    settleStatusBar();
    int hostWidth, hostHeight;
    SDL_GetWindowSize(full, &hostWidth, &hostHeight);
    macOpenGLConfigureFullscreen(full, 640, 480, true);
    KNativeInputSDL capturedInput(640, 480, 100, 100);
    for (auto size : {SDL_Point{640, 480}, SDL_Point{1280, 480}}) {
        assert(macOpenGLResizeFullscreen(full, size.x, size.y));
        auto viewport = OpenGLViewport::fit(hostWidth, hostHeight, size.x, size.y, true);
        testViewportMouseCapture(capturedInput, full, {viewport.x, viewport.y, viewport.width, viewport.height});
        assert(capturedInput.openGLViewport.guestWidth == size.x && capturedInput.openGLViewport.guestHeight == size.y);
    }
    puts("GL pillarbox/letterbox capture, immediate edge reversal, release and focus-loss checks passed");
    for (U32 id = 1; id < 32; ++id) {
        SDL_Window* candidate = SDL_GetWindowFromID(id);
        if (sdlStatusBarOwns(candidate) && strcmp(SDL_GetWindowTitle(candidate), "Boxedwine fullscreen hint"))
            assert(!(SDL_GetWindowFlags(candidate) & SDL_WINDOW_SHOWN));
    }
    sdlStatusBarOpen();
    SDL_Window* fullscreenMenu = nullptr;
    for (U32 id = 1; id < 32; ++id) {
        SDL_Window* candidate = SDL_GetWindowFromID(id);
        if (candidate && !strcmp(SDL_GetWindowTitle(candidate), "Boxedwine CD-ROM menu")) fullscreenMenu = candidate;
    }
    assert(fullscreenMenu && (SDL_GetWindowFlags(fullscreenMenu) & SDL_WINDOW_SHOWN));
    SDL_Event escape = {}; escape.type = SDL_KEYDOWN;
    escape.key.windowID = SDL_GetWindowID(fullscreenMenu); escape.key.keysym.sym = SDLK_ESCAPE;
    assert(sdlStatusBarEvent(escape));
    assert(!(SDL_GetWindowFlags(fullscreenMenu) & SDL_WINDOW_SHOWN));
    sdlStatusBarWindowDestroyed(full);
    SDL_DestroyWindow(full);
    puts("Cocoa fullscreen menu lifecycle checks passed");
#endif
    testCaptureWindowReplacement();
    testSoftwareFullscreenScale();
    sdlStatusBarConfigure(false);
    Fs::shutDown();
    SDL_Quit();
#ifdef SDL_VIDEO_DRIVER_X11
    if (nativeXtest) SDL_UnloadObject(nativeXtest);
#endif
    puts("CD-ROM popup dismissal, click routing, host hierarchy, drawable size, and GL state checks passed");
}
