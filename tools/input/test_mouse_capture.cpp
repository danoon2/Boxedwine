/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
// Run with: make -C project/linux test-native-input
#include "boxedwine.h"
#include <SDL.h>
#include "knativesystem.h"
#define private public
#include "knativeinputSDL.h"
#include "knativescreenSDL.h"
#include "../../source/x11/x11.h"
#undef private
#include <cassert>
#include <cstdio>
#include <thread>
#include <string>

// Model host focus/grabs without capturing the developer's real desktop.
static SDL_Window* focused;
static SDL_Window* grabbed;
static std::string titles[4] = {"", "Alice", "OpenGL Window", "Alice Gameplay"};
static int cursorShown = SDL_ENABLE;
static SDL_Cursor* currentCursor;
static SDL_bool relativeMode = SDL_FALSE;
static U32 physicalButtons;
static int warpCount;
static int physicalX = 400, physicalY = 300, warpX, warpY;
extern "C" void SDL_WarpMouseInWindow(SDL_Window*, int x, int y) { ++warpCount; warpX = x; warpY = y; }
extern "C" const char* SDL_GetWindowTitle(SDL_Window* w) { return titles[uintptr_t(w)].c_str(); }
extern "C" void SDL_SetWindowTitle(SDL_Window* w, const char* title) { titles[uintptr_t(w)] = title; }
extern "C" int SDL_ShowCursor(int toggle) { if (toggle >= 0) cursorShown = toggle; return cursorShown; }
extern "C" void SDL_SetCursor(SDL_Cursor* cursor) { currentCursor = cursor; }
extern "C" SDL_Cursor* SDL_GetDefaultCursor() { return reinterpret_cast<SDL_Cursor*>(1); }
extern "C" int SDL_SetRelativeMouseMode(SDL_bool enabled) { relativeMode = enabled; return 0; }
extern "C" SDL_bool SDL_GetRelativeMouseMode() { return relativeMode; }
extern "C" Uint32 SDL_GetMouseState(int* x, int* y) { if (x) *x = physicalX; if (y) *y = physicalY; return physicalButtons; }
static bool alive[4] = {false, true, true, true};
static SDL_Window* window(U32 id) { return reinterpret_cast<SDL_Window*>(uintptr_t(id)); }
extern "C" SDL_Window* SDL_GetKeyboardFocus() { return focused; }
extern "C" SDL_Window* SDL_GetWindowFromID(Uint32 id) { return id < 4 && alive[id] ? window(id) : nullptr; }
extern "C" Uint32 SDL_GetWindowID(SDL_Window* w) { return Uint32(reinterpret_cast<uintptr_t>(w)); }
extern "C" SDL_bool SDL_GetWindowGrab(SDL_Window* w) { return grabbed == w ? SDL_TRUE : SDL_FALSE; }
extern "C" void SDL_SetWindowGrab(SDL_Window* w, SDL_bool enabled) {
    if (enabled) grabbed = w;
    else if (grabbed == w) grabbed = nullptr;
}
static void focus(KNativeInputSDL& input, U32 id, bool gained) {
    SDL_Event e = {};
    e.type = SDL_WINDOWEVENT;
    e.window.windowID = id;
    e.window.event = gained ? SDL_WINDOWEVENT_FOCUS_GAINED : SDL_WINDOWEVENT_FOCUS_LOST;
    assert(input.handlSdlEvent(&e));
}

static void testMouseLookScaling(KNativeInputSDL& input) {
    auto server = XServer::getServer();
    auto root = server->getRoot();
    auto game = server->createNewWindow(43, root, 800, 600, 32, 70, 30, InputOutput, 0, server->visual);
    U32 serial = 0;
    auto display = std::make_shared<DisplayData>();
    display->displayId = 43;
    display->displayAddress = 123;
    display->pCurrentRequest = &serial;
    server->displays.set(43, display);
    display->setInput2Mask(root->id, XI_RawMotionMask);
    server->grabbedId = game->id;
    server->grabbedDisplayId = 43;
    server->grabbedConfinedId = 0;
    server->grabbedMask = PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
    server->isGrabbed = true;
    server->setFakeFullScreenWindow(game);
    server->setMouseFocused(true);
    server->setMouseCursorVisible(false);
    const U32 sensitivity = KSystem::relativeMouseSensitivity;
    const bool forceRelative = KSystem::forceRelativeMouse;
    for (int mode = 0; mode < 5; ++mode) {
        input.scaleX = input.scaleY = mode == 0 ? 100 : 200;
        input.scaleXOffset = 13; input.scaleYOffset = 21;
        input.openGLViewport = mode < 2 ? OpenGLViewport{} :
            OpenGLViewport::fit(mode == 2 ? 2000 : mode == 3 ? 1920 : 400,
                mode == 2 ? 1200 : mode == 3 ? 1080 : 300, 800, 600, true);
        KSystem::forceRelativeMouse = false;
        for (U32 percent : {100u, 200u}) {
            KSystem::relativeMouseSensitivity = percent;
            input.setMousePos(400, 300);
            // One physical pixel must survive even a fractional enlargement.
            for (int delta : {10, 1, -1, -10}) {
                physicalX = warpX + delta; physicalY = warpY - delta;
                int x, y;
                int expectedX = 400 + delta * (int)percent / 100;
                int expectedY = 300 - delta * (int)percent / 100;
                for (int repeat = 0; repeat < 2; ++repeat) {
                    assert(input.getMousePos(&x, &y));
                    assert(x == expectedX && y == expectedY);
                }
                display->eventQueue.clear();
                assert(input.mouseMove(physicalX, physicalY, false));
                assert(display->eventQueue.size() == 1);
                auto event = display->eventQueue.back();
                assert(event.type == MotionNotify && event.xmotion.x_root == expectedX && event.xmotion.y_root == expectedY);
                assert(input.mouseButton(1, 0, physicalX, physicalY));
                event = display->eventQueue.back();
                assert(event.type == ButtonPress && event.xbutton.x_root == expectedX && event.xbutton.y_root == expectedY);
                input.mouseWheel(1, physicalX, physicalY);
                event = display->eventQueue.back();
                assert(event.type == ButtonRelease && event.xbutton.x_root == expectedX && event.xbutton.y_root == expectedY);
            }
        }
        // Fractional sensitivity accumulates across warps, not across repeated
        // queries/events for the same physical position.
        KSystem::relativeMouseSensitivity = 50;
        input.setMousePos(400, 300);
        for (int i = 0; i < 6; ++i) {
            physicalX = warpX + 1; physicalY = warpY - 1;
            int x, y;
            input.getMousePos(&x, &y);
            assert(x == 400 + i % 2 && y == 300 - i % 2);
            input.getMousePos(&x, &y);
            assert(x == 400 + i % 2 && y == 300 - i % 2);
            input.setMousePos(400, 300);
            physicalX = warpX; physicalY = warpY;
            input.getMousePos(&x, &y);
            assert(x == 400 && y == 300);
        }
        KSystem::forceRelativeMouse = true;
        for (int delta : {10, 1, -1, -10}) {
            display->eventQueue.clear();
            input.mouseMove(delta, -delta, true);
            assert(display->eventQueue.size() == 1);
            auto event = display->eventQueue.back();
            assert(event.type == GenericEvent && event.xcookie.evtype == XI_RawMotion);
            // The serialized XIRawEvent stores deltas in its two valuator
            // address slots; x11_GetEventData expands these for the guest.
            assert((S32)event.pad[8 + 12] == delta && (S32)event.pad[8 + 13] == -delta);
        }
    }
    // A visible menu retains absolute coordinate scaling and the game offset.
    KSystem::forceRelativeMouse = false;
    input.openGLViewport = {};
    input.scaleX = input.scaleY = 200;
    server->setMouseCursorVisible(true);
    input.setMousePos(400, 300);
    physicalX = warpX + 10; physicalY = warpY - 6;
    int x, y;
    input.getMousePos(&x, &y);
    assert(x == 405 && y == 297);
    display->eventQueue.clear();
    input.mouseMove(physicalX, physicalY, false);
    auto event = display->eventQueue.back();
    assert(event.xmotion.x_root == 405 && event.xmotion.y_root == 297);
    server->isGrabbed = false;
    server->clearFakeFullScreenWindow(game);
    server->displays.remove(43);
    input.scaleX = input.scaleY = 100;
    input.scaleXOffset = input.scaleYOffset = 0;
    KSystem::relativeMouseSensitivity = sensitivity;
    KSystem::forceRelativeMouse = forceRelative;
    puts("Mouse look is independent of software/GL scale; recentering, polling, buttons, sensitivity and absolute menus passed");
}

static void testMappedWindowFocus() {
    XServer* server = XServer::getServer();
    auto root = server->getRoot();
    auto makeWindow = [&](const XWindowPtr& parent) {
        auto wnd = server->createNewWindow(42, parent, 640, 480, 32, 0, 0, InputOutput, 0, server->visual);
        return wnd;
    };
    auto splash = makeWindow(root);
    auto game = makeWindow(root);
    auto edit = makeWindow(game);
    splash->isMapped = true;
    U32 serial = 0;
    auto display = std::make_shared<DisplayData>();
    display->displayId = 42;
    display->displayAddress = 123;
    display->pCurrentRequest = &serial;
    server->displays.set(42, display);
    U32 protocols = server->internAtom(B("WM_PROTOCOLS"), false);
    U32 takeFocus = server->internAtom(B("WM_TAKE_FOCUS"), false);
    game->setProperty(protocols, XA_ATOM, 32, sizeof(takeFocus), (U8*)&takeFocus, false);
    display->setEventMask(game->id, PropertyChangeMask | StructureNotifyMask);

    // Alice's splash refreshes focus after the timestamp of the game's request.
    // Keep rejecting that stale request, just as a real X server does.
    U32 oldTime = server->getEventTime();
    SDL_Delay(2);
    assert(server->setInputFocus(nullptr, splash->id, RevertToParent, 0) == Success);
    SDL_Delay(2);
    game->mapWindow();
    assert(display->eventQueue.size() == 3);
    assert(display->eventQueue[0].type == PropertyNotify);
    assert(display->eventQueue[0].xproperty.atom == WM_STATE);
    assert(display->eventQueue[1].type == MapNotify);
    assert(server->getInputFocusWindow() == splash);
    assert(server->setInputFocus(nullptr, game->id, RevertToParent, oldTime) == Success);
    assert(server->getInputFocusWindow() == splash);
    splash->isMapped = false;
    server->revertInputFocus(splash);
    assert(server->getInputFocusWindow() == root);

    auto event = display->eventQueue.back();
    display->eventQueue.clear();
    display->setEventMask(game->id, 0);
    assert(event.type == ClientMessage && event.xclient.window == game->id);
    assert(event.xclient.message_type == protocols && event.xclient.format == 32);
    assert(event.xclient.data.l[0] == (S32)takeFocus && event.xclient.send_event);
    assert((S32)((U32)event.xclient.data.l[1] - oldTime) > 0);
    // WM_TAKE_FOCUS lets the client choose focus; the server must not force it.
    assert(server->getInputFocusWindow() == root);
    assert(server->setInputFocus(nullptr, game->id, RevertToParent, event.xclient.data.l[1]) == Success);
    assert(server->getInputFocusWindow() == game);

    // Repeated activation must preserve an already-focused guest child.
    edit->mapWindow();
    assert(display->eventQueue.empty()); // Mapping a drawable/control is not activation.
    assert(server->setInputFocus(nullptr, edit->id, RevertToParent, 0) == Success);
    server->requestFocusWindow(game);
    assert(server->getInputFocusWindow() == edit && display->eventQueue.empty());
    server->setInputFocus(nullptr, root->id, RevertToNone, 0);
    game->isMapped = false;
    server->requestFocusWindow(game);
    assert(server->getInputFocusWindow() == root && display->eventQueue.empty());
    game->deleteProperty(protocols);
    game->mapWindow(); // Clients must explicitly offer the protocol.
    assert(server->getInputFocusWindow() == root && display->eventQueue.empty());
    server->displays.remove(42);
}

int main() {
    assert(SDL_Init(SDL_INIT_EVENTS) == 0);
#if defined(__linux__) && defined(BOXEDWINE_NATIVE_RUNTIME)
    // SDL's default text-input state must not activate host IMEs for guest keys.
    SDL_StartTextInput();
    assert(SDL_IsTextInputActive() == SDL_TRUE);
    assert(KNativeSystem::init(VIDEO_NO_WINDOW, false));
    assert(SDL_IsTextInputActive() == SDL_FALSE);
#endif
    KSystem::videoOption = VIDEO_NO_WINDOW;
    KNativeSystem::initWindow(800, 600, 32, 100, 100, B("nearest"), 0, 0);
    auto& input = *std::static_pointer_cast<KNativeInputSDL>(KNativeSystem::getCurrentInput());
    extern bool doMainLoop();
    doMainLoop(); // Mark this thread as SDL's owner; no guest threads are running.
    focused = window(1);
    // Requests from a guest worker must return before the SDL thread runs.
    std::thread worker([&] { input.setMouseConfinement(true); });
    worker.join();
    assert(!grabbed);
    assert(input.processEvents());
    assert(grabbed == window(1));
    focused = nullptr;
    focus(input, 1, false);
    assert(!grabbed);
    focused = window(1);
    focus(input, 1, true);
    assert(grabbed == window(1));
    // Switching from the screen window to a GL window transfers the grab.
    focused = window(2);
    focus(input, 2, true);
    assert(grabbed == window(2));
    focus(input, 1, false); // A late event for the old window must not release it.
    assert(grabbed == window(2));
    input.setMouseConfinement(false);
    input.processEvents();
    assert(!grabbed);
    focus(input, 2, true);
    assert(!grabbed);
    // A grab requested while unfocused only applies when focus returns.
    focused = nullptr;
    input.setMouseConfinement(true);
    input.processEvents();
    assert(!grabbed);
    focused = window(2);
    focus(input, 2, true);
    assert(grabbed == window(2));
    // Destruction of an old host window must not leave a dangling pointer.
    alive[2] = false;
    grabbed = nullptr;
    focused = window(3);
    focus(input, 3, true);
    assert(grabbed == window(3));
    // An ungrab queued before focus returns prevents re-confinement.
    focused = nullptr;
    focus(input, 3, false);
    input.setMouseConfinement(false);
    focused = window(3);
    focus(input, 3, true);
    input.processEvents();
    assert(!grabbed);
    // Gameplay: the guest captures a hidden cursor with no confine_to.
    XServer* server = XServer::getServer();
    server->grabbedConfinedId = 0;
    server->isGrabbed = true;
    server->setMouseCursorVisible(true);
    input.processEvents();
    assert(!grabbed);
    server->setMouseCursorVisible(false);
    input.processEvents();
    assert(grabbed == window(3));
    focused = nullptr;
    focus(input, 3, false);
    input.processEvents();
    assert(!grabbed);
    focused = window(3);
    focus(input, 3, true);
    assert(grabbed == window(3));
    server->setMouseCursorVisible(true);
    input.processEvents();
    assert(!grabbed);
    server->isGrabbed = false;
    // SDL relative mode must grab even without any guest confinement request.
    input.ownsRelativeMouseMode = true;
    input.updateMouseConfinement(focused);
    assert(grabbed == window(3));
    focused = nullptr;
    focus(input, 3, false);
    assert(!grabbed);
    focused = window(3);
    focus(input, 3, true);
    assert(grabbed == window(3));
    input.ownsRelativeMouseMode = false;
    input.updateMouseConfinement(focused);
    assert(!grabbed);
    auto& screen = *std::static_pointer_cast<KNativeScreenSDL>(KNativeSystem::getScreen());
    screen.window = window(3);
    // Ctrl+Alt+G releases both relative mode and the grab, preserving the app title.
    server->isGrabbed = true;
    server->grabbedConfinedId = 0;
    server->setMouseCursorVisible(false);
    input.setCursor(nullptr);
    input.updateMouseConfinement(focused);
    assert(titles[3] == "Alice Gameplay");
    input.ownsRelativeMouseMode = true;
    relativeMode = SDL_TRUE;
    SDL_Event shortcut = {};
    shortcut.type = SDL_KEYDOWN;
    shortcut.key.keysym.sym = SDLK_g;
    // Ordinary game keys and other shortcuts must still reach the guest.
    const int otherModifiers[] = {KMOD_NONE, KMOD_LCTRL, KMOD_LALT,
        KMOD_LCTRL | KMOD_LALT | KMOD_LSHIFT, KMOD_LCTRL | KMOD_LALT | KMOD_LGUI};
    for (int modifiers : otherModifiers) {
        shortcut.key.keysym.mod = modifiers;
        assert(!input.handleMouseCaptureEvent(&shortcut));
        assert(!input.isMouseCaptureReleased() && grabbed == window(3));
    }
    shortcut.key.keysym.sym = SDLK_F10;
    shortcut.key.keysym.mod = KMOD_LCTRL;
    assert(!input.handleMouseCaptureEvent(&shortcut));
    shortcut.key.keysym.sym = SDLK_g;
    shortcut.key.keysym.mod = KMOD_LCTRL | KMOD_LALT;
    // Queue a guest warp before releasing; it must be suppressed even if its
    // SDL callback runs after the shortcut. The caller waits for completion.
    std::thread pendingWarp([&] { input.setMousePos(100, 200); });
    extern U32 sdlCustomEvent;
    for (int i = 0; i < 1000 && !SDL_HasEvent(sdlCustomEvent); ++i) SDL_Delay(1);
    assert(SDL_HasEvent(sdlCustomEvent));
    assert(input.handleMouseCaptureEvent(&shortcut));
    input.processEvents();
    pendingWarp.join();
    assert(warpCount == 0);
    assert(SDL_GetHintBoolean(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, SDL_FALSE));
    assert(input.isMouseCaptureReleased() && !grabbed && relativeMode == SDL_FALSE);
    assert(cursorShown == SDL_ENABLE);
    assert(titles[3] == "Alice Gameplay");
    shortcut.key.repeat = 1;
    shortcut.key.keysym.mod = KMOD_NONE; // Modifiers may be released before G.
    assert(input.handleMouseCaptureEvent(&shortcut));
    assert(input.isMouseCaptureReleased());
    shortcut.type = SDL_KEYUP;
    shortcut.key.keysym.mod = KMOD_NONE;
    assert(input.handleMouseCaptureEvent(&shortcut));
    // Repeated game grabs/warps and focus changes cannot undo a manual release.
    input.setMouseConfinement(true);
    input.setMousePos(123, 234);
    input.processEvents();
    int x, y;
    input.getMousePos(&x, &y);
    assert(x == 123 && y == 234 && !grabbed);
    KNativeSystem::warpMouse(10, 20);
    screen.warpMouse(30, 40);
    assert(warpCount == 0);
    focused = nullptr;
    focus(input, 3, false);
    assert(titles[3] == "Alice Gameplay");
    focused = window(3);
    focus(input, 3, true);
    input.processEvents();
    assert(!grabbed && titles[3] == "Alice Gameplay");
    input.setCursor(nullptr); // The guest cannot hide the released desktop cursor.
    assert(cursorShown == SDL_ENABLE);
    SDL_Event motion = {};
    motion.type = SDL_MOUSEMOTION;
    assert(input.handleMouseCaptureEvent(&motion));
    motion.type = SDL_MOUSEWHEEL;
    assert(input.handleMouseCaptureEvent(&motion));
    // Consume both halves of the recapture click, including polled button state.
    SDL_Event click = {};
    click.type = SDL_MOUSEBUTTONDOWN;
    click.button.windowID = 3;
    click.button.button = SDL_BUTTON_LEFT;
    physicalButtons = SDL_BUTTON_LMASK;
    assert(input.handleMouseCaptureEvent(&click));
    assert(!input.isMouseCaptureReleased() && grabbed == window(3));
    assert(warpCount == 1);
    assert(!SDL_GetHintBoolean(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, SDL_FALSE));
    assert(cursorShown == SDL_DISABLE);
    assert(!(input.getInputModifiers() & NATIVE_LEFT_BUTTON_MASK));
    click.type = SDL_MOUSEBUTTONUP;
    physicalButtons = 0;
    assert(input.handleMouseCaptureEvent(&click));
    assert(input.swallowedMouseButtons == 0);
    // App/FPS title changes remain untouched while captured.
    titles[3] = "Alice - 60 FPS";
    input.processEvents();
    input.processEvents();
    assert(titles[3] == "Alice - 60 FPS");
    // Shortcut works with either side's modifiers and does not leak its G key-up.
    shortcut.type = SDL_KEYDOWN;
    shortcut.key.repeat = 0;
    shortcut.key.keysym.mod = KMOD_RCTRL | KMOD_RALT | KMOD_CAPS | KMOD_NUM;
    assert(input.handleMouseCaptureEvent(&shortcut));
    shortcut.type = SDL_KEYUP;
    assert(input.handleMouseCaptureEvent(&shortcut));
    shortcut.type = SDL_KEYDOWN;
    assert(input.handleMouseCaptureEvent(&shortcut));
    assert(!input.isMouseCaptureReleased() && grabbed);
    shortcut.type = SDL_KEYUP;
    assert(input.handleMouseCaptureEvent(&shortcut));
    server->isGrabbed = false;
    input.setMouseConfinement(false);
    input.processEvents();
    assert(!grabbed && titles[3] == "Alice - 60 FPS");
    shortcut.type = SDL_KEYDOWN;
    assert(!input.handleMouseCaptureEvent(&shortcut)); // No capture to toggle.
    // Entering the status bar releases capture with SDL's position below the
    // guest. Recapture must warp inside the guest, or the bar immediately
    // releases capture again and every subsequent guest click repeats the loop.
    physicalY = 615;
    input.setMouseCaptureReleased(true);
    input.getMousePos(&x, &y);
    assert(x == 400 && y == 599);
    click.type = SDL_MOUSEBUTTONDOWN;
    assert(input.handleMouseCaptureEvent(&click));
    assert(!input.isMouseCaptureReleased() && warpX == 400 && warpY == 599);
    click.type = SDL_MOUSEBUTTONUP;
    assert(input.handleMouseCaptureEvent(&click));
    assert(!input.isMouseCaptureReleased() && input.swallowedMouseButtons == 0);
    physicalY = 300;
    // Normal desktop apps only suspend input over host UI. Moving back resumes
    // at the actual pointer position: no capture hint, extra click, or warp.
    input.setCursor(reinterpret_cast<SDL_Cursor*>(2));
    int previousWarps = warpCount;
    for (U32 eventType : {SDL_MOUSEMOTION, SDL_MOUSEBUTTONDOWN}) {
        input.releaseMouseForHostUi();
        assert(input.isMouseCaptureReleased() && input.hostUiMouseReleased && !grabbed);
        assert(titles[3] == "Alice - 60 FPS" && currentCursor == SDL_GetDefaultCursor());
        // Typing while the pointer is over the bar must not move the pointer
        // or intercept the desktop app's Ctrl+Alt+G shortcut.
        assert(!input.handleMouseCaptureEvent(&shortcut));
        assert(input.isMouseCaptureReleased() && input.hostUiMouseReleased && !input.captureShortcutDown);
        focused = nullptr;
        SDL_Event enter = {};
        enter.type = SDL_MOUSEMOTION;
        enter.window.windowID = 3;
        assert(input.handleMouseCaptureEvent(&enter)); // A popup still owns focus.
        assert(input.isMouseCaptureReleased());
        focused = window(3);
        physicalX = 210; physicalY = 180;
        enter.type = eventType;
        if (eventType == SDL_MOUSEBUTTONDOWN) enter.button.button = SDL_BUTTON_LEFT;
        assert(!input.handleMouseCaptureEvent(&enter));
        assert(!input.isMouseCaptureReleased() && !input.hostUiMouseReleased && !grabbed);
        assert(warpCount == previousWarps && currentCursor == reinterpret_cast<SDL_Cursor*>(2));
        input.getMousePos(&x, &y);
        assert(x == 210 && y == 180 && titles[3] == "Alice - 60 FPS");
        if (eventType == SDL_MOUSEBUTTONDOWN) {
            enter.type = SDL_MOUSEBUTTONUP;
            assert(!input.handleMouseCaptureEvent(&enter));
        }
    }
    // Games that really lock the pointer still require an explicit recapture.
    input.setMouseConfinement(true);
    input.releaseMouseForHostUi();
    assert(input.isMouseCaptureReleased() && !input.hostUiMouseReleased && !grabbed);
    motion.type = SDL_MOUSEMOTION;
    motion.motion.windowID = 3;
    assert(input.handleMouseCaptureEvent(&motion));
    assert(input.isMouseCaptureReleased() && titles[3] == "Alice - 60 FPS");
    input.releaseMouseForHostUi(); // Repeated host events cannot change that policy.
    click.type = SDL_MOUSEBUTTONDOWN;
    assert(input.handleMouseCaptureEvent(&click));
    assert(!input.isMouseCaptureReleased() && grabbed == window(3) && warpCount == previousWarps + 1);
    click.type = SDL_MOUSEBUTTONUP;
    assert(input.handleMouseCaptureEvent(&click));
    input.setMouseConfinement(false);
    physicalX = 400; physicalY = 300;
    testMappedWindowFocus();
    testMouseLookScaling(input);
    screen.window = nullptr; // Do not destroy the fake host window.
    SDL_Quit();
    puts("Mouse confinement lifecycle checks passed");
}
