/*
 *  Copyright (C) 2016  The BoxedWine Team
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

#include "boxedwine.h"
#include <SDL.h>
#include "../../source/x11/x11.h"
#include "sdlcallback.h"
#include "devinput.h"
#include "knativeinputSDL.h"
#include "knativesystem.h"

U32 sdlCustomEvent;

KNativeInputSDL::KNativeInputSDL(U32 cx, U32 cy, int scaleX, int scaleY) {
    if (!sdlCustomEvent) {
        sdlCustomEvent = SDL_RegisterEvents(1);
    }
    
    this->width = cx;
    this->height = cy;
    this->scaleX = scaleX;
    this->scaleY = scaleY;
    this->scaleXOffset = 0;
    this->scaleYOffset = 0;    
}

void KNativeInputSDL::runOnUiThread(std::function<void()> callback) {
    DISPATCH_MAIN_THREAD_BLOCK_THIS_BEGIN
        callback();
    DISPATCH_MAIN_THREAD_BLOCK_END
}


void KNativeInputSDL::setScreenSize(U32 cx, U32 cy) {
    width = cx;
    height = cy;
}

U32 KNativeInputSDL::screenWidth() {
    return width;
}

U32 KNativeInputSDL::screenHeight() {
    return height;
}

bool KNativeInputSDL::mouseMove(int x, int y, bool relative) {
    XServer* server = XServer::getServer(true);

    x = xFromScreen(x);
    y = yFromScreen(y);

#ifdef BOXEDWINE_RECORDER
    if (Player::instance) {
        lastX = x;
        lastY = y;
    }
#endif

    if (server) {
        server->mouseMove(x, y, relative);
        return true;
    }
    return false;
}

bool KNativeInputSDL::mouseWheel(int amount, int x, int y) {
    x = xFromScreen(x);
    y = yFromScreen(y);

    checkMousePos(x, y, true);

    XServer* server = XServer::getServer(true);
    if (server) {
        U32 btn = amount > 0 ? 4 : 5;
        server->mouseButton(btn, x, y, true);
        server->mouseButton(btn, x, y, false);
        return true;
    }
    return false;
}

bool KNativeInputSDL::mouseButton(U32 down, U32 button, int x, int y) {
    x = xFromScreen(x);
    y = yFromScreen(y);

    checkMousePos(x, y, true);

    XServer* server = XServer::getServer(true);
    if (server) {
        U32 btn = button + 1;
        if (btn == 2) {
            btn = 3;
        } else if (btn == 3) {
            btn = 2;
        } else if (btn == 4) {
            btn = 8;
        } else if (btn == 5) {
            btn = 9;
        }
        server->mouseButton(btn, x, y, down ? true : false);
        return true;
    }
    return false;
}

bool KNativeInputSDL::getMousePos(int* x, int* y, bool allowWarp) {
#ifdef BOXEDWINE_RECORDER
    if (Player::instance) {
        *x = lastX;
        *y = lastY;
        return 0;
    }
#endif
    unsigned int result = SDL_GetMouseState(x, y);

    *x = xFromScreen(*x);
    *y = yFromScreen(*y);

    return checkMousePos(*x, *y, false);
}

void KNativeInputSDL::setMousePos(int x, int y) {
#ifdef BOXEDWINE_RECORDER
    if (Player::instance) {
        lastX = x;
        lastY = y;
    }
#endif

    x = xToScreen(x);
    y = yToScreen(y);

    KNativeSystem::warpMouse(x, y);
}

int KNativeInputSDL::xToScreen(int x) {
    return x * (int)scaleX / 100 + (int)scaleXOffset;
}

int KNativeInputSDL::xFromScreen(int x) {
    return (x - (int)scaleXOffset) * 100 / (int)scaleX;
}

int KNativeInputSDL::yToScreen(int y) {
    return y * (int)scaleY / 100 + (int)scaleYOffset;
}

int KNativeInputSDL::yFromScreen(int y) {
    return (y - (int)scaleYOffset) * 100 / (int)scaleY;
}

bool KNativeInputSDL::checkMousePos(int& x, int& y, bool allowWarp) {
    bool warp = false;
    if (x < 0) {
        x = 0;
        warp = true;
    }
    if (x >= (int)width) {
        x = (int)width - 1;
        warp = true;
    }
    if (y < 0) {
        y = 0;
        warp = true;
    }
    if (y >= (int)height) {
        y = (int)height;
        warp = true;
    }
    if (allowWarp && warp) {
        int scaledX = xToScreen(x);
        int scaledY = yToScreen(y);
        KNativeSystem::getScreen()->warpMouse(scaledX, scaledY);
    }
    return allowWarp || !warp;
}

#ifndef KMOD_SCROLL
#define KMOD_SCROLL KMOD_RESERVED
#endif

U32 KNativeInputSDL::getInputModifiers() {
#ifdef BOXEDWINE_RECORDER
    if (Player::instance) {
        return Player::instance->currentInputModifiers;
    }
#endif
    int x, y;
    unsigned int result = SDL_GetMouseState(&x, &y);
    U32 modifiers = 0;
    if (result & SDL_BUTTON_LMASK) {
        modifiers |= NATIVE_LEFT_BUTTON_MASK;
    }
    if (result & SDL_BUTTON_RMASK) {
        modifiers |= NATIVE_RIGHT_BUTTON_MASK;
    }
    if (result & SDL_BUTTON_MMASK) {
        modifiers |= NATIVE_MIDDLE_BUTTON_MASK;
    }
    if (result & SDL_BUTTON_X1MASK) {
        modifiers |= NATIVE_BUTTON_4_MASK;
    }
    if (result & SDL_BUTTON_X2MASK) {
        modifiers |= NATIVE_BUTTON_5_MASK;
    }
    SDL_Keymod mods = SDL_GetModState();
    if (mods & KMOD_SHIFT) {
        modifiers |= NATIVE_SHIFT_MASK;
    }
    if (mods & KMOD_CAPS) {
        modifiers |= NATIVE_CAPS_MASK;
    }
    if (mods & KMOD_CTRL) {
        modifiers |= NATIVE_CONTROL_MASK;
    }
    if (mods & KMOD_ALT) {
        modifiers |= NATIVE_CONTROL_MASK;
    }
    if (mods & KMOD_NUM) {
        modifiers |= NATIVE_NUM_MASK;
    }
    if (mods & KMOD_SCROLL) {
        modifiers |= NATIVE_SCROLL_MASK;
    }
    return modifiers;
}

bool KNativeInputSDL::key(U32 sdlScanCode, U32 key, U32 down) {
    XServer* server = XServer::getServer(true);
    if (server) {
        U32 x11Key = XKeyboard::sdl2x11(sdlScanCode);
        if (x11Key) {
            server->key(x11Key, down ? true : false);
        }
        return true;
    }
    return false;
}

bool KNativeInputSDL::waitForEvent(U32 ms) {
    SDL_Event e = { 0 };
    if (SDL_WaitEventTimeout(&e, ms) == 1) {
#ifdef BOXEDWINE_MULTI_THREADED
        if (e.type == sdlCustomEvent) {
            SdlCallback* callback = (SdlCallback*)e.user.data1;
            callback->result = (U32)callback->pfn();
            BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(callback->cond);
            BOXEDWINE_CONDITION_SIGNAL(callback->cond);
            return true;
        }
#endif
        handlSdlEvent(&e);
        return true;
    }
    return false;
}

bool KNativeInputSDL::processEvents() {
    SDL_Event e = {};

    while (SDL_PollEvent(&e) == 1) {
#ifdef BOXEDWINE_MULTI_THREADED
        if (e.type == sdlCustomEvent) {
            SdlCallback* callback = (SdlCallback*)e.user.data1;
            callback->result = (U32)callback->pfn();
            BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(callback->cond);
            BOXEDWINE_CONDITION_SIGNAL(callback->cond);
        } else
#endif
            if (!handlSdlEvent(&e)) {
                return false;
            }
    }
    return true;
}

#ifndef SDLK_NUMLOCK
#define SDLK_NUMLOCK SDL_SCANCODE_NUMLOCKCLEAR
#endif
#ifndef SDLK_SCROLLOCK
#define SDLK_SCROLLOCK SDLK_SCROLLLOCK
#endif

static U32 translate(U32 key) {
    switch (key) {
    case SDLK_ESCAPE:
        return K_KEY_ESC;
    case SDLK_1:
        return K_KEY_1;
    case SDLK_2:
        return K_KEY_2;
    case SDLK_3:
        return K_KEY_3;
    case SDLK_4:
        return K_KEY_4;
    case SDLK_5:
        return K_KEY_5;
    case SDLK_6:
        return K_KEY_6;
    case SDLK_7:
        return K_KEY_7;
    case SDLK_8:
        return K_KEY_8;
    case SDLK_9:
        return K_KEY_9;
    case SDLK_0:
        return K_KEY_0;
    case SDLK_MINUS:
        return K_KEY_MINUS;
    case SDLK_EQUALS:
        return K_KEY_EQUAL;
    case SDLK_BACKSPACE:
        return K_KEY_BACKSPACE;
    case SDLK_TAB:
        return K_KEY_TAB;
    case SDLK_q:
        return K_KEY_Q;
    case SDLK_w:
        return K_KEY_W;
    case SDLK_e:
        return K_KEY_E;
    case SDLK_r:
        return K_KEY_R;
    case SDLK_t:
        return K_KEY_T;
    case SDLK_y:
        return K_KEY_Y;
    case SDLK_u:
        return K_KEY_U;
    case SDLK_i:
        return K_KEY_I;
    case SDLK_o:
        return K_KEY_O;
    case SDLK_p:
        return K_KEY_P;
    case SDLK_LEFTBRACKET:
        return K_KEY_LEFTBRACE;
    case SDLK_RIGHTBRACKET:
        return K_KEY_RIGHTBRACE;
    case SDLK_RETURN:
        return K_KEY_ENTER;
    case SDLK_LCTRL:
        return K_KEY_LEFTCTRL;
    case SDLK_RCTRL:
        return K_KEY_RIGHTCTRL;
    case SDLK_a:
        return K_KEY_A;
    case SDLK_s:
        return K_KEY_S;
    case SDLK_d:
        return K_KEY_D;
    case SDLK_f:
        return K_KEY_F;
    case SDLK_g:
        return K_KEY_G;
    case SDLK_h:
        return K_KEY_H;
    case SDLK_j:
        return K_KEY_J;
    case SDLK_k:
        return K_KEY_K;
    case SDLK_l:
        return K_KEY_L;
    case SDLK_SEMICOLON:
        return K_KEY_SEMICOLON;
    case SDLK_QUOTE:
        return K_KEY_APOSTROPHE;
    case SDLK_BACKQUOTE:
        return K_KEY_GRAVE;
    case SDLK_LSHIFT:
        return K_KEY_LEFTSHIFT;
    case SDLK_RSHIFT:
        return K_KEY_RIGHTSHIFT;
    case SDLK_BACKSLASH:
        return K_KEY_BACKSLASH;
    case SDLK_z:
        return K_KEY_Z;
    case SDLK_x:
        return K_KEY_X;
    case SDLK_c:
        return K_KEY_C;
    case SDLK_v:
        return K_KEY_V;
    case SDLK_b:
        return K_KEY_B;
    case SDLK_n:
        return K_KEY_N;
    case SDLK_m:
        return K_KEY_M;
    case SDLK_COMMA:
        return K_KEY_COMMA;
    case SDLK_PERIOD:
        return K_KEY_DOT;
    case SDLK_SLASH:
        return K_KEY_SLASH;
    case SDLK_LALT:
        return K_KEY_LEFTALT;
    case SDLK_RALT:
        return K_KEY_RIGHTALT;
    case SDLK_SPACE:
        return K_KEY_SPACE;
    case SDLK_CAPSLOCK:
        return K_KEY_CAPSLOCK;
    case SDLK_F1:
        return K_KEY_F1;
    case SDLK_F2:
        return K_KEY_F2;
    case SDLK_F3:
        return K_KEY_F3;
    case SDLK_F4:
        return K_KEY_F4;
    case SDLK_F5:
        return K_KEY_F5;
    case SDLK_F6:
        return K_KEY_F6;
    case SDLK_F7:
        return K_KEY_F7;
    case SDLK_F8:
        return K_KEY_F8;
    case SDLK_F9:
        return K_KEY_F9;
    case SDLK_F10:
        return K_KEY_F10;
    case SDLK_NUMLOCK:
        return K_KEY_NUMLOCK;
    case SDLK_SCROLLOCK:
        return K_KEY_SCROLLLOCK;
    case SDLK_F11:
        return K_KEY_F11;
    case SDLK_F12:
        return K_KEY_F12;
    case SDLK_HOME:
        return K_KEY_HOME;
    case SDLK_UP:
        return K_KEY_UP;
    case SDLK_PAGEUP:
        return K_KEY_PAGEUP;
    case SDLK_LEFT:
        return K_KEY_LEFT;
    case SDLK_RIGHT:
        return K_KEY_RIGHT;
    case SDLK_END:
        return K_KEY_END;
    case SDLK_DOWN:
        return K_KEY_DOWN;
    case SDLK_PAGEDOWN:
        return K_KEY_PAGEDOWN;
    case SDLK_INSERT:
        return K_KEY_INSERT;
    case SDLK_DELETE:
        return K_KEY_DELETE;
    case SDLK_PAUSE:
        return K_KEY_PAUSE;
    default:
        kdebug("Unhandled key: %d", key);
        return 0;
    }
}

static int getMouseButtonFromEvent(SDL_Event* e) {
    if (e->button.button == SDL_BUTTON_LEFT) {
        return 0;
    } else if (e->button.button == SDL_BUTTON_MIDDLE) {
        return 2;
    } else if (e->button.button == SDL_BUTTON_RIGHT) {
        return 1;
    }
    return 0;
}

// return true to continue processing for custom handlers
//
// should only call on main thread
void KNativeInputSDL::processCustomEvents(std::function<bool(bool isKeyDown, int key, bool isF11)> onKey, std::function<bool(bool isButtonDown, int button, int x, int y)> onMouseButton, std::function<bool(int x, int y)> onMouseMove) {
    SDL_Event e = {};

#ifdef _DEBUG
    if (!isMainthread()) {
        kpanic("KNativeInputSDL::processCustomEvents should only be called on main thread");
    }
#endif
    while (SDL_WaitEvent(&e)) {
        if (e.type == SDL_KEYUP) {
            if (!onKey(false, e.key.keysym.sym, e.key.keysym.sym == SDLK_F11)) {
                return;
            }
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (!onMouseButton(true, getMouseButtonFromEvent(&e), e.motion.x, e.motion.y)) {
                return;
            }
        } else if (e.type == SDL_MOUSEBUTTONUP) {
            if (!onMouseButton(false, getMouseButtonFromEvent(&e), e.motion.x, e.motion.y)) {
                return;
            }
        } else if (e.type == SDL_MOUSEMOTION) {
            if (!onMouseMove(e.motion.x, e.motion.y)) {
                return;
            }
        }
#ifdef BOXEDWINE_MULTI_THREADED
        else if (e.type == sdlCustomEvent) {
            SdlCallback* callback = (SdlCallback*)e.user.data1;
            callback->result = (U32)callback->pfn();
            BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(callback->cond);
            BOXEDWINE_CONDITION_SIGNAL(callback->cond);
        }
#endif
    }
}

bool KNativeInputSDL::handlSdlEvent(SDL_Event* e) {
#ifdef BOXEDWINE_RECORDER
    if (Player::instance) {
        if (e->type == SDL_QUIT) {
            return false;
        }
        return true;
    }
#endif    
    if (e->type == SDL_QUIT) {
        KThread::setCurrentThread(nullptr);
        KProcessPtr p = KSystem::getProcess(10);
        if (p && !KSystem::shutingDown) {
            p->killAllThreads();
            KSystem::eraseProcess(p->id);
            return true;
        }
        return false;
    } else if (e->type == SDL_MOUSEMOTION) {
        BOXEDWINE_RECORDER_HANDLE_MOUSE_MOVE(e->motion.x, e->motion.y);
        if (!mouseMove(e->motion.x, e->motion.y, false)) {
            onMouseMove(e->motion.x, e->motion.y, false);
        }
    } else if (e->type == SDL_MOUSEBUTTONDOWN) {    
        U32 button = getMouseButtonFromEvent(e);
        BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_DOWN(button, e->motion.x, e->motion.y);
        if (!mouseButton(1, button, e->motion.x, e->motion.y)) {
            onMouseButtonDown(button);
        }
    } else if (e->type == SDL_MOUSEBUTTONUP) {      
        U32 button = getMouseButtonFromEvent(e);
        BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_UP(button, e->motion.x, e->motion.y);
        if (!mouseButton(0, button, e->motion.x, e->motion.y)) {
            onMouseButtonUp(button);
        }
    } else if (e->type == SDL_MOUSEWHEEL) {
        // Handle up/down mouse wheel movements
        int x, y;
        SDL_GetMouseState(&x, &y);
        if (!mouseWheel(e->wheel.y * 80, x, y)) {
            onMouseWheel(e->wheel.y);
        }
    } else if (e->type == SDL_KEYDOWN) {       
        if (!BOXEDWINE_RECORDER_HANDLE_KEY_DOWN(e->key.keysym.scancode, e->key.keysym.sym == SDLK_F11)) {
            if (e->key.keysym.sym == SDLK_SCROLLOCK) {
                KSystem::printStacks();
            } else if (!key(e->key.keysym.scancode, e->key.keysym.sym, 1)) {
                onKeyDown(translate(e->key.keysym.sym));
            }
        }
    } else if (e->type == SDL_KEYUP) {
        if (!BOXEDWINE_RECORDER_HANDLE_KEY_UP(e->key.keysym.scancode, e->key.keysym.sym == SDLK_F11)) {
            if (!key(e->key.keysym.scancode, e->key.keysym.sym, 0)) {
                onKeyUp(translate(e->key.keysym.sym));
            }
        }
    }
    return true;
}
