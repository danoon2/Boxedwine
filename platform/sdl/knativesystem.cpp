#include "boxedwine.h"
#include "knativesystem.h"
#include <SDL.h>
#include "knativescreenSDL.h"
#include "../../source/x11/x11.h"

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

bool KNativeSystem::init(bool allowVideo, bool allowAudio) {
    U32 flags = SDL_INIT_EVENTS;

    if (allowVideo) {
        flags |= SDL_INIT_VIDEO;
    }
    if (allowAudio) {
        flags |= SDL_INIT_AUDIO;
    }
    if (SDL_Init(flags) != 0) {
        klog("SDL_Init Error: %s", SDL_GetError());
        return false;
    }
#ifdef BOXEDWINE_OPENGL
    PlatformOpenGL::init();
#endif
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
    screen->showWindow(show);
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
        if (KSystem::openglType == OPENGL_TYPE_OSMESA) {
            opengl = OsMesaGL::create();
            return opengl;
        }
#endif
#ifdef BOXEDWINE_OPENGL_SDL
        opengl = SDLGL::create();
        return opengl;
#endif
        klog("Failed to load OpenGL, will probably crash");
    }
    return opengl;
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