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

#include "boxedwine.h"
#include "knativesystem.h"
#include <SDL.h>
#include "knativescreenSDL.h"
#include "kvulkanSDL.h"
#include "../../source/x11/x11.h"
#include "../../source/util/threadutils.h"

#include UNISTD

#ifdef BOXEDWINE_OPENGL_OSMESA
#include "../../source/opengl/osmesa/osmesa.h"
#endif
#ifdef BOXEDWINE_OPENGL_SDL
#include "../../source/opengl/sdl/sdlgl.h"
#endif

#ifndef __TEST
int boxedmain(int argc, const char** argv);

int main(int argc, char** argv) {
    return boxedmain(argc, (const char**)argv);
}
#endif

static KNativeScreenSDLPtr screen;
static KOpenGLPtr opengl;
static KVulkanPtr vulkan;

bool KNativeSystem::init(VideoOption videoOption, bool allowAudio) {
    U32 flags = SDL_INIT_EVENTS;

    if (videoOption != VIDEO_NO_WINDOW) {
        flags |= SDL_INIT_VIDEO;
    }
    if (allowAudio) {
        flags |= SDL_INIT_AUDIO;
    }
    if (SDL_Init(flags) != 0) {
        klog_fmt("SDL_Init Error: %s", SDL_GetError());
        return false;
    }
    return true;
}

void KNativeSystem::initWindow(U32 cx, U32 cy, U32 bpp, int scaleX, int scaleY, const BString& scaleQuality, U32 fullScreen, U32 vsync) {
    screen = std::make_shared<KNativeScreenSDL>(cx, cy, bpp, scaleX, scaleY, scaleQuality, fullScreen, vsync);
}

KNativeInputPtr KNativeSystem::getCurrentInput() {
    return screen->getInput();
}

KNativeScreenPtr KNativeSystem::getScreen() {
    return screen;
}

void KNativeSystem::tick() {
    if (opengl && opengl->getLastUpdateTime() + 100 < screen->getLastUpdateTime()) {
        opengl->hideCurrentWindow();
    }
}

void KNativeSystem::showScreen(bool show) {
    if (screen) {
        screen->showWindow(show);
    }
}

void KNativeSystem::warpMouse(S32 x, S32 y) {
    if (!opengl || screen->isVisible()) {
        screen->warpMouse(x, y);
    } else {
        opengl->warpMouse(x, y);
    }    
}

KOpenGLPtr KNativeSystem::getOpenGL() {
    if (!opengl) {
#ifdef BOXEDWINE_OPENGL_OSMESA
        if (KSystem::openglLib == "osmesa") {
            opengl = OsMesaGL::create();
            return opengl;
        }
#endif
#ifdef BOXEDWINE_OPENGL_SDL
        opengl = SDLGL::create();
        return opengl;
#else
        klog("Failed to load OpenGL, will probably crash");
#endif
    }
    return opengl;
}

KVulkanPtr KNativeSystem::getVulkan() {
    if (!vulkan) {
#ifdef BOXEDWINE_VULKAN
        vulkan = KVulkanSDL::create(screen);
#endif
    }
    return vulkan;
}

void KNativeSystem::changeScreenSize(U32 cx, U32 cy) {
    screen->setScreenSize(cx, cy);
}

void KNativeSystem::moveWindow(const XWindowPtr& wnd) {
    if (opengl && opengl->isActive()) {
        opengl->glResizeWindow(wnd);
    }
}

void KNativeSystem::showWindow(const XWindowPtr & wnd, bool bShow) {
}

void KNativeSystem::shutdown() {
    screen = nullptr;
    opengl = nullptr;
}

void KNativeSystem::scheduledNewThread(KThread* thread) {
    // glUpdateContextForThread(currentThread);
}

void KNativeSystem::exit(const char* msg, U32 code) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", msg, nullptr);
    _exit(code);
}

void KNativeSystem::forceShutdown() {
    std::shared_ptr<KProcess> p = KSystem::getProcess(10);
    if (!p) {
        return;
    }
    klog("Forcing Shutdown");
#if defined (BOXEDWINE_MULTI_THREADED) && !defined (__TEST)
    runInBackgroundThread([p]() {
        p->killAllThreads();
        KSystem::eraseProcess(p->id);
        });
#else
    KThread::setCurrentThread(nullptr);
    p->killAllThreads();
    KSystem::eraseProcess(p->id);
#endif
}

void KNativeSystem::cleanup() {
    SDL_Quit();
}

void KNativeSystem::postQuit() {
    SDL_Event sdlevent;
    sdlevent.type = SDL_QUIT;
    SDL_PushEvent(&sdlevent);
}

U32 KNativeSystem::getTicks() {
    return SDL_GetTicks();
}

bool KNativeSystem::getScreenDimensions(U32* width, U32* height) {
    SDL_DisplayMode mode;
    if (!SDL_GetCurrentDisplayMode(0, &mode)) {
        *width = mode.w;
        *height = mode.h;
        return true;
    }
    return false;
}

BString KNativeSystem::getAppDirectory() {
    char* s = SDL_GetBasePath();
    BString result;

    if (s) {
        result = BString::copy(s);
        SDL_free(s);
    }
    return result;
}

BString KNativeSystem::getLocalDirectory() {
    char* s = SDL_GetPrefPath("", "Boxedwine");
    BString result;

    if (s) {
        result = BString::copy(s);
        SDL_free(s);
    }
    return result;
}

bool KNativeSystem::clipboardHasText() {
    return SDL_HasClipboardText()?true:false;
}

BString KNativeSystem::clipboardGetText() {
    char* s = SDL_GetClipboardText();
    BString result;
    if (s) {
        result = BString::copy(s);
        SDL_free(s);
    }
    return result;
}

bool KNativeSystem::clipboardSetText(BString text) {
    return SDL_SetClipboardText(text.c_str()) ? true : false;
}

U32 KNativeSystem::getDpiScale() {
    const float defaultDPI =
#ifdef __APPLE__
        72.0f;
#else
        96.0f;
#endif
    float dpi = defaultDPI;

    if (SDL_GetDisplayDPI(0, nullptr, &dpi, nullptr) != 0) {
        return SCALE_DENOMINATOR;
    }
    return (U32)(dpi / defaultDPI * SCALE_DENOMINATOR);
}

void KNativeSystem::preReturnToUI() {
    // make sure if the user closed the SDL windows for the game/app, that it doesn't carry over into the UI
    SDL_PumpEvents();
    SDL_FlushEvent(SDL_QUIT);
}
