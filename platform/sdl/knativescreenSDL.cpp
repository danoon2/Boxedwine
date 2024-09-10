#include "boxedwine.h"
#include "../source/ui/mainui.h"
#include "../source/sdl/startupArgs.h"

#include <SDL.h>
#include "sdlcallback.h"
#include "knativeinputSDL.h"
#include "knativescreenSDL.h"
#include "../../source/x11/x11.h"

KNativeScreenSDL::KNativeScreenSDL(U32 cx, U32 cy, U32 bpp, int scaleX, int scaleY, const BString& scaleQuality, U32 fullScreen, U32 vsync) {
    input = std::make_shared<KNativeInputSDL>(cx, cy, scaleX, scaleY);
    this->bpp = bpp;
    this->scaleQuality = scaleQuality;
    this->fullScreen = fullScreen;
    this->vsync = vsync;

    this->defaultScreenWidth = cx;
    this->defaultScreenHeight = cy;
    this->defaultScreenBpp = bpp;

    recreateMainWindow();
}

KNativeScreenSDL::~KNativeScreenSDL() {
    DISPATCH_MAIN_THREAD_BLOCK_THIS_BEGIN
        destroyMainWindow();
    DISPATCH_MAIN_THREAD_BLOCK_END
}

KNativeInputPtr KNativeScreenSDL::getInput() {
    return input;
}

void KNativeScreenSDL::setScreenSize(U32 cx, U32 cy) {
    input->setScreenSize(cx, cy);

    // If full screen, then we just have to change the scale
    if (fullScreen != FULLSCREEN_NOTSET) {
        SDL_DisplayMode dm;
        if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
            if (fullScreen == FULLSCREEN_STRETCH) {
                input->scaleX = dm.w * 100 / cx;
                input->scaleY = dm.h * 100 / cy;
            } else if (fullScreen == FULLSCREEN_ASPECT) {
                input->scaleX = dm.w * 100 / cx;
                input->scaleY = dm.h * 100 / cy;
                input->scaleXOffset = 0;
                input->scaleYOffset = 0;
                if (input->scaleY > input->scaleX) {
                    input->scaleY = input->scaleX;
                    input->scaleYOffset = (dm.h - cy * input->scaleY / 100) / 2;
                } else if (input->scaleX > input->scaleY) {
                    input->scaleX = input->scaleY;
                    input->scaleXOffset = (dm.w - cx * input->scaleX / 100) / 2;
                }
            }
        }
    } else {
        if (input->scaleX != 100) {
            cx = cx * input->scaleX / 100;
            cy = cy * input->scaleY / 100;
        }
        SDL_SetWindowSize(window, cx, cy);
    }
}

U32 KNativeScreenSDL::screenWidth() {
    return input->width;
}

U32 KNativeScreenSDL::screenHeight() {
    return input->height;
}

U32 KNativeScreenSDL::screenBpp() {
    return this->bpp;
}

U32 KNativeScreenSDL::screenRate() {
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    return DM.refresh_rate;
}

void KNativeScreenSDL::setTitle(const BString& title) {
    SDL_SetWindowTitle(window, title.c_str());
}

void KNativeScreenSDL::getPos(S32& x, S32& y) {
    SDL_GetWindowPosition(window, &x, &y);
}

U32 KNativeScreenSDL::getLastUpdateTime() {
    return lastUpdateTime;
}

void KNativeScreenSDL::showWindow(bool show) {
    if (show == visible) {
        return;
    }
    if (!isMainthread()) {
        DISPATCH_MAIN_THREAD_BLOCK_THIS_BEGIN
            showWindow(show);
        DISPATCH_MAIN_THREAD_BLOCK_END
    } else {
        showOnDraw = false;
        if (!show) {
            SDL_HideWindow(window);
            visible = false;            
        } else {
            SDL_ShowWindow(window);
            SDL_RaiseWindow(window);
            visible = true;
#if !defined(BOXEDWINE_DISABLE_UI) && !defined(__TEST) && defined(BOXEDWINE_UI_LAUNCH_IN_PROCESS)
            if (uiIsRunning()) {
                uiShutdown();
            }
#endif
        }
    }
}

void KNativeScreenSDL::clear() {
    if (KSystem::videoEnabled && renderer) {
        SDL_SetRenderDrawColor(renderer, 58, 110, 165, 255);
        SDL_RenderClear(renderer);
    }
}

void KNativeScreenSDL::putBitsOnWnd(U32 id, U8* bits, U32 bitsPerPixel, U32 srcPitch, S32 dstX, S32 dstY, U32 width, U32 height, U32* palette, bool isDirty) {
    if (!bitsPerPixel || !srcPitch) {
        return;
    }
    WndCachePtr wnd;
    
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(wndCacheMutex);
        wnd = wndCache.get(id);
        if (!wnd) {
            wnd = std::make_shared<WndCache>();
            wndCache.set(id, wnd);
        }
    }

    U32 bpp = 32;
    U32 dstPitch = (width * ((bpp + 7) / 8) + 3) & ~3;

    if (wnd->sdlTexture && (wnd->sdlTextureHeight != height || wnd->sdlTextureWidth != width)) {
        SDL_DestroyTexture(wnd->sdlTexture);
        wnd->sdlTexture = nullptr;
        isDirty = true;
    }
    if (isDirty) {
        lastUpdateTime = KSystem::getMilliesSinceStart();
    }
    if (!wnd->sdlTexture) {
        if (KSystem::videoEnabled && renderer) {
            wnd->sdlTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
        }
        wnd->sdlTextureHeight = height;
        wnd->sdlTextureWidth = width;
    }
    if (isDirty && bitsPerPixel == 8) {
        wnd->ensureSize(dstPitch * height);
        for (U32 y = 0; y < height; y++) {
            U8* srcLine = bits + srcPitch * y;
            U32* dstLine = (U32*)(wnd->bits + dstPitch * y);
            for (U32 x = 0; x < width; x++, dstLine++, srcLine++) {
                *dstLine = palette[*srcLine];
            }
        }
        bits = wnd->bits;
    } else if (isDirty && bitsPerPixel == 16) {
        wnd->ensureSize(dstPitch * height);
        for (U32 y = 0; y < height; y++) {
            U16* srcLine = (U16*)(bits + srcPitch * y);
            U32* dstLine = (U32*)(wnd->bits + dstPitch * y);
            for (U32 x = 0; x < width; x++, dstLine++, srcLine++) {
                U32 r = (*srcLine & 0xF800) >> 11;
                U32 g = (*srcLine & 0x07E0) >> 5;
                U32 b = *srcLine & 0x001F;

                r = (r * 255) / 31;
                g = (g * 255) / 63;
                b = (b * 255) / 31;
                *dstLine = (r << 16) | (g << 8) | b;
            }
        }
        bits = wnd->bits;
    }
#ifdef BOXEDWINE_RECORDER
    else if ((Recorder::instance || Player::instance) && isDirty) {
        U32 toCopy = dstPitch * height;
        wnd->ensureSize(toCopy);
        memcpy(wnd->bits, bits, toCopy);
    }
#endif     

    if (KSystem::videoEnabled && renderer) {
        if (isDirty) {
            SDL_UpdateTexture(wnd->sdlTexture, nullptr, bits, dstPitch);
        }

        SDL_Rect dstrect;
        dstrect.x = dstX * (int)input->scaleX / 100 + input->scaleXOffset;
        dstrect.y = dstY * (int)input->scaleY / 100 + input->scaleYOffset;
        dstrect.w = wnd->sdlTextureWidth * (int)input->scaleX / 100;
        dstrect.h = wnd->sdlTextureHeight * (int)input->scaleY / 100;
        SDL_RenderCopy(renderer, wnd->sdlTexture, nullptr, &dstrect);
    }
}

void KNativeScreenSDL::present() {
    if (KSystem::videoEnabled) {
        if (showOnDraw) {
            showWindow(true);
        }
        SDL_RenderPresent(renderer);
    }
    presented = true;
}

bool KNativeScreenSDL::presentedSinceLastCheck() {
    bool result = presented;
    presented = false;
    return presented;
}

void KNativeScreenSDL::clearTextureCache(U32 id) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(wndCacheMutex);
    wndCache.clear();
}

void KNativeScreenSDL::warpMouse(int x, int y) {
    DISPATCH_MAIN_THREAD_BLOCK_THIS_BEGIN
        if (window) {
            SDL_WarpMouseInWindow(window, x, y);
        }
    DISPATCH_MAIN_THREAD_BLOCK_END
}

bool KNativeScreenSDL::isVisible() {
    return visible;
}

#ifdef BOXEDWINE_RECORDER
// return true to continue processing for custom handlers
void KNativeScreenSDL::processCustomEvents(std::function<bool(bool isKeyDown, int key, bool isF11)> onKey, std::function<bool(bool isButtonDown, int button, int x, int y)> onMouseButton, std::function<bool(int x, int y)> onMouseMove) {
}

void KNativeScreenSDL::pushWindowSurface() {
}

void KNativeScreenSDL::popWindowSurface() {
}

void KNativeScreenSDL::drawRectOnPushedSurfaceAndDisplay(U32 x, U32 y, U32 w, U32 h, U8 r, U8 g, U8 b, U8 a) {
}

#endif

bool KNativeScreenSDL::partialScreenShot(const BString& filepath, U32 x, U32 y, U32 w, U32 h, U8* buffer, U32 bufferlen) {
    return true;
}

bool KNativeScreenSDL::screenShot(const BString& filepath, U8* buffer, U32 bufferlen) {
    return true;
}

bool KNativeScreenSDL::saveBmp(const BString& filepath, U8* buffer, U32 bpp, U32 w, U32 h) {
    return true;
}

void KNativeScreenSDL::buildCursor(KThread* thread, const std::shared_ptr<XCursor>& cursor, U32 pixelsAddress, U32 width, U32 height, S32 xHot, S32 yHot) {
    U8* buffer = thread->memory->lockReadOnlyMemory(pixelsAddress, width * height * 4);
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(buffer, width, height, 32, width * 4, 0xff0000, 0xff00, 0xff, 0xff000000);
    SDL_Cursor* sdlCursor = SDL_CreateColorCursor(surface, xHot, yHot);

    thread->memory->unlockMemory(buffer);
    SDL_FreeSurface(surface);

    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cursorsMutex);
    SDL_Cursor* prev = cursors.get(cursor->id);
    if (prev) {
        SDL_FreeCursor(prev);
    }
    cursors.set(cursor->id, sdlCursor);
}

void KNativeScreenSDL::setCursor(const std::shared_ptr<XCursor>& cursor) {
    SDL_Cursor* sdlCursor;

    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cursorsMutex);
        sdlCursor = cursors.get(cursor->id);
    }
    if (!sdlCursor) {
        switch (cursor->shape) {
        case 0:
            if (cursor->fg.red == 0 && cursor->fg.blue == 0 && cursor->fg.green == 0 && cursor->bg.red == 0 && cursor->bg.green == 0 && cursor->bg.blue == 0) {
                sdlCursor = nullptr;
            } else {
                sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
            }
            break;
        case 22: // XC_center_ptr
            // :TODO:
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
            break;
        case 52: // XC_fleur
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
            break;
        case 60: // XC_hand2
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
            break;
        case 64: // XC_icon
            // :TODO:
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
            break;
        case 68: // XC_left_ptr
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
            break;
        case 88: // XC_pirate
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);
            break;
        case 108: // XC_sb_h_double_arrow
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
            break;
        case 116: // XC_sb_v_double_arrow
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
            break;
        case 130: // XC_tcross
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
            break;
        case 132: // XC_top_left_arrow
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
            break;
        case 134: // XC_top_left_corner
        case 136: // XC_top_right_corner
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
            break;
        case 150: // XC_watch
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
            break;
        case 152: // XC_xterm
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
            break;    
        case 1000: // left_ptr_watch
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
            break;
        case 92: // XC_question_arrow
            // :TODO:
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
            break;
        default:
            klog("KNativeScreenSDL::setCursor cursor shape not defined for %d", cursor->shape);
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
            break;
        }
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cursorsMutex);
        cursors.set(cursor->id, sdlCursor);
    }
    DISPATCH_MAIN_THREAD_BLOCK_BEGIN
        if (sdlCursor) {
            SDL_ShowCursor(1);
            SDL_SetCursor(sdlCursor);
        } else {
            SDL_ShowCursor(0);
        }
    DISPATCH_MAIN_THREAD_BLOCK_END
}

void KNativeScreenSDL::destroyTextureCache() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(wndCacheMutex);
    wndCache.clear();
}

void KNativeScreenSDL::destroyMainWindow() {
    destroyTextureCache();

    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
}

void KNativeScreenSDL::recreateMainWindow() {
    if (KSystem::videoEnabled) {
        destroyMainWindow();
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, scaleQuality.c_str());

        int cx = input->width * input->scaleX / 100;
        int cy = input->height * input->scaleY / 100;
        int flags = SDL_WINDOW_HIDDEN;
        
        SDL_DisplayMode dm;

        if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
            SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
            fullScreen = FULLSCREEN_NOTSET;
        } else {
            if (fullScreen == FULLSCREEN_STRETCH) {
                cx = dm.w;
                cy = dm.h;
                flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
                input->scaleX = dm.w * 100 / input->width;
                input->scaleY = dm.h * 100 / input->height;
            } else if (fullScreen == FULLSCREEN_ASPECT) {
                cx = dm.w;
                cy = dm.h;
                flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
                input->scaleX = dm.w * 100 / input->width;
                input->scaleY = dm.h * 100 / input->height;
                input->scaleXOffset = 0;
                input->scaleYOffset = 0;
                if (input->scaleY > input->scaleX) {
                    input->scaleY = input->scaleX;
                    input->scaleYOffset = (dm.h - input->height * input->scaleY / 100) / 2;
                } else if (input->scaleX > input->scaleY) {
                    input->scaleX = input->scaleY;
                    input->scaleXOffset = (dm.w - input->width * input->scaleX / 100) / 2;
                }
            } else if (input->width == (U32)dm.w && input->height == (U32)dm.h) {
                flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
            }
            if (cx > dm.w || cy > dm.h) {
                cx = dm.w;
                cy = dm.h;
                input->scaleX = dm.w * 100 / input->width;
                input->scaleY = dm.h * 100 / input->height;
                flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
            }
        }

        window = SDL_CreateWindow("BoxedWine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, cx, cy, flags);
        if (!window) {
            klog("SDL_CreateWindow failed: %s", SDL_GetError());
        }
#ifdef BOXEDWINE_LINUX
        // NVidia drivers need this
        flags = SDL_RENDERER_SOFTWARE;
#else
        flags = SDL_RENDERER_ACCELERATED;
#endif
        if (this->vsync != VSYNC_DISABLED) {
            flags |= SDL_RENDERER_PRESENTVSYNC;
        }
        renderer = SDL_CreateRenderer(window, -1, flags);
        if (!renderer) {
            klog("Failed to create SDL accelerated renderer, will try software");
            flags &= ~SDL_RENDERER_ACCELERATED;
            flags |= SDL_RENDERER_SOFTWARE;
            renderer = SDL_CreateRenderer(window, -1, flags);
        }
    }
}