/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
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

#ifndef __KNATIVEWINDOW_SDL_H__
#define __KNATIVEWINDOW_SDL_H__

#include "knativeinput.h"
#if defined(__APPLE__) || defined(BOXEDWINE_MSVC) || (defined(__linux__) && !defined(__EMSCRIPTEN__))
#include "openGLViewport.h"
#endif

class KNativeInputSDL : public KNativeInput {
public:
    KNativeInputSDL(U32 cx, U32 cy, int scaleX, int scaleY);

    void setScreenSize(U32 cx, U32 cy) override;
    U32 screenWidth() override;
    U32 screenHeight() override;

    bool waitForEvent(U32 ms) override; // if return is true, then event is available
    bool processEvents() override; // if return is false, then shutdown    
    void processCustomEvents(std::function<bool(bool isKeyDown, int key, bool isF11)> onKey, std::function<bool(bool isButtonDown, int button, int x, int y)> onMouseButton, std::function<bool(int x, int y)> onMouseMove) override;

    void runOnUiThread(std::function<void()> callback) override;
    bool mouseMove(int x, int y, bool relative) override;
    bool mouseWheel(int amount, int x, int y) override;
    bool mouseButton(U32 down, U32 button, int x, int y) override;
    bool key(U32 sdlScanCode, U32 key, U32 down) override;  // the key code is specific to the back end

    bool getMousePos(int* x, int* y, bool allowWarp = true) override;
    void setMousePos(int x, int y) override;
    void setMouseConfinement(bool confined) override;
    bool isMouseCaptureReleased() const { return mouseCaptureReleased.load(); }
    void setCursor(SDL_Cursor* cursor);
    U32 getInputModifiers() override;

    int xToScreen(int x);
    int xFromScreen(int x);
    int yToScreen(int y);
    int yFromScreen(int y);

    bool checkMousePos(int& x, int& y, bool allowWarp);

    U32 scaleX = 100;
    U32 scaleXOffset = 0;
    U32 scaleY = 100;
    U32 scaleYOffset = 0;    
    int lastX = 0;
    int lastY = 0;
    U32 width = 0;
    U32 height = 0;           

    bool handlSdlEvent(SDL_Event* e);

#ifdef __EMSCRIPTEN__
    void setEmscriptenMousePosition(int x, int y);
    void clearEmscriptenMousePosition();
#endif

private:

#if defined(__APPLE__) || defined(BOXEDWINE_MSVC) || (defined(__linux__) && !defined(__EMSCRIPTEN__))
    OpenGLViewport openGLViewport;
    BOXEDWINE_MUTEX openGLViewportMutex;
#endif
	void updateRelativeMouseMode();
	bool ownsRelativeMouseMode = false;
#ifdef __EMSCRIPTEN__
    std::atomic<U32> emscriptenMouseButtons{0};
    std::atomic<U64> emscriptenMousePosition{~(U64)0};
#endif
    void updateMouseConfinement(SDL_Window* window);
    bool wantsMouseCapture();
    void setMouseCaptureReleased(bool released);
    void releaseMouseForHostUi();
    bool handleMouseCaptureEvent(SDL_Event* event);
    std::atomic<bool> mouseConfinementRequested{false};
    U32 confinedWindowId = 0; // Accessed only on the SDL thread.
    U32 captureGuestWindowId = 0; // Embedded guest focus events use a different ID than the host grab.
    std::atomic<bool> mouseCaptureReleased{false};
    bool hostUiMouseReleased = false; // Resume ordinary desktop input without a click or warp.
    std::atomic<U64> releasedMousePosition{0};
    std::atomic<U32> swallowedMouseButtons{0};
    U32 pressedMouseButtons = 0;
    bool captureShortcutDown = false;
    bool focusClickthroughBeforeRelease = false;
    SDL_Cursor* guestCursor = nullptr;
    bool guestCursorVisible = true;
};

typedef std::shared_ptr<KNativeInputSDL> KNativeInputSDLPtr;

#endif
