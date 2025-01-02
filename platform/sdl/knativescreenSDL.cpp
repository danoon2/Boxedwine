#include "boxedwine.h"
#include "../../source/ui/mainui.h"
#include "../../source/sdl/startupArgs.h"

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
    destroyMainWindow();
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
        if (window) {
            DISPATCH_MAIN_THREAD_BLOCK_THIS_BEGIN
                SDL_SetWindowSize(window, cx, cy);
            DISPATCH_MAIN_THREAD_BLOCK_END
        }
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
    if (KSystem::videoOption == VIDEO_NO_WINDOW) {
        return 60;
    }
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    return DM.refresh_rate;
}

void KNativeScreenSDL::setTitle(const BString& title) {
    if (window) {
        SDL_SetWindowTitle(window, title.c_str());
    }
}

void KNativeScreenSDL::getPos(S32& x, S32& y) {
    if (!window) {
        x = input->lastX;
        y = input->lastY;
    } else {
        SDL_GetWindowPosition(window, &x, &y);
    }
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
            if (KSystem::videoOption == VIDEO_NORMAL) {
                SDL_ShowWindow(window);
                SDL_RaiseWindow(window);
            }
            visible = true;
#if !defined(BOXEDWINE_DISABLE_UI) && !defined(__TEST) && defined(BOXEDWINE_UI_LAUNCH_IN_PROCESS)
            if (uiIsRunning()) {
                uiShutdown();
            }
#else
            klog("Showing Window");
#endif
        }
    }
}

void KNativeScreenSDL::clear() {
#ifdef BOXEDWINE_RECORDER
    if (Recorder::instance) {
        BOXEDWINE_MUTEX_LOCK(drawingMutex);
    }
    if (Recorder::instance || Player::instance) {
        U32 size = screenWidth() * screenHeight() * 4;
        if (recordBufferSize < size) {
            delete[] recordBuffer;
            recordBuffer = new U8[size];
            recordBufferSize = size;
        }
        if (bpp == 32) {
            U32* pixel = (U32*)recordBuffer;
            U32 len = screenWidth() * screenHeight();
            for (U32 i = 0; i < len; i++, pixel++) {
                *pixel = 165 | (110 << 8) | (58 << 16) | (255 << 24);
            }
        } else {
            memset(recordBuffer, 0, recordBufferSize);
        }
    }
#endif
    if (KSystem::videoOption != VIDEO_NO_WINDOW && renderer) {
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
        if (KSystem::videoOption != VIDEO_NO_WINDOW && renderer) {
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
    if ((Recorder::instance || Player::instance) && screenBpp() > 8) {
        int wndWidth = width;
        int wndHeight = height;
        S32 top = dstY;
        S32 left = dstX;
        S32 srcTopAdjust = 0;
        S32 srcLeftAdjust = 0;
        S32 bytesPerPixel = (this->bpp + 7) / 8;
        S32 recorderPitch = (screenWidth() * ((this->bpp + 7) / 8) + 3) & ~3;

        if (top < 0) {
            wndHeight += top;
            top = 0;
        }
        if (top >= (S32)screenHeight()) {
            return;
        }
        if (left >= (S32)screenWidth()) {
            return;
        }
        if (left < 0) {
            srcLeftAdjust = -left;
            left = 0;
        }
        if (top + wndHeight > (S32)screenHeight()) {
            wndHeight = screenHeight() - top;
        }

        int pitch = (wndWidth * ((this->bpp + 7) / 8) + 3) & ~3;
        if (dstX + wndWidth > (S32)screenWidth()) {
            wndWidth = screenWidth() - left;
        }
        int copyPitch = (wndWidth * ((this->bpp + 7) / 8) + 3) & ~3;
        for (int y = 0; y < wndHeight; y++) {
            S32 offset = recorderPitch * (y + top) + (left * bytesPerPixel);
            if (offset<0 || offset + copyPitch>(S32)recordBufferSize || copyPitch < 0) {
                kpanic("script recorder overwrote memory when copying screen");
            }
            memcpy(recordBuffer + offset, bits + pitch * (y + srcTopAdjust) + (srcLeftAdjust * bytesPerPixel), copyPitch);
        }
    }    
#endif     

    if (KSystem::videoOption != VIDEO_NO_WINDOW && renderer) {
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
    if (KSystem::videoOption != VIDEO_NO_WINDOW) {
        if (showOnDraw) {
            showWindow(true);
        }
        SDL_RenderPresent(renderer);
    }
    presented = true;
#ifdef BOXEDWINE_RECORDER
    if (Recorder::instance) {
        BOXEDWINE_MUTEX_UNLOCK(drawingMutex);
    }
#endif
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

bool KNativeScreenSDL::canBltToScreen() {
    return additionalSDLWindowFlags == 0;
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

bool KNativeScreenSDL::clipboardIsTextAvailable() {
    return SDL_HasClipboardText() != SDL_FALSE;
}

BString KNativeScreenSDL::clipboardGetText() {
    char* result = SDL_GetClipboardText();
    if (!result) {
        return BString::empty;
    }
    return BString::copy(result);
}

void KNativeScreenSDL::clipboardSetText(const char* text) {
    SDL_SetClipboardText(text);
}

#ifdef BOXEDWINE_RECORDER

void KNativeScreenSDL::startRecorderScreenShot() {
    BOXEDWINE_MUTEX_LOCK(drawingMutex);
    SDL_Surface* src = SDL_GetWindowSurface(window);
    SDL_Rect r = {};

    r.x = 0;
    r.y = 0;
    r.w = src->w;
    r.h = src->h;

    screenCopyTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, src->w, src->h);
    U32 len = src->w * src->h * src->format->BytesPerPixel;
    U8* pixels = new unsigned char[len];

    if (!SDL_RenderReadPixels(renderer, &src->clip_rect, src->format->format, pixels, src->w * src->format->BytesPerPixel)) {
        SDL_UpdateTexture(screenCopyTexture, nullptr, pixels, src->w * src->format->BytesPerPixel);
    }
    delete[] pixels;
}

void KNativeScreenSDL::finishRecorderScreenShot() {
    SDL_Surface* dst = SDL_GetWindowSurface(window);
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = dst->w;
    rect.h = dst->h;

    SDL_RenderCopy(renderer, screenCopyTexture, nullptr, &rect);
    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(screenCopyTexture);
    screenCopyTexture = nullptr;
    BOXEDWINE_MUTEX_UNLOCK(drawingMutex);
}

void KNativeScreenSDL::drawRectOnPushedSurfaceAndDisplay(U32 x, U32 y, U32 w, U32 h, U8 r, U8 g, U8 b, U8 a) {
    SDL_Surface* dst = SDL_GetWindowSurface(window);
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = dst->w;
    rect.h = dst->h;

    SDL_RenderCopy(renderer, screenCopyTexture, nullptr, &rect);

    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    SDL_RenderPresent(renderer);
}

#endif

bool KNativeScreenSDL::internalScreenShot(const BString& filepath, SDL_Rect* rect, U8* buffer, U32 bufferlen) {
#ifdef BOXEDWINE_RECORDER
    if (!recordBuffer) {
        if (filepath.length()) {
            klog("failed to save screenshot, %s, because recorderBuffer was NULL", filepath.c_str());
        }
        return false;
    }
    if (bpp == 8) {
        klog("KNativeScreenSDL::internalScreenShot 8 bit screen shots not supported");
        return false;
    }
    U8* pixels = nullptr;
    SDL_Surface* s = nullptr;
    U32 rMask = 0;
    U32 gMask = 0;
    U32 bMask = 0;

    if (bpp == 32) {
        rMask = 0x00FF0000;
        gMask = 0x0000FF00;
        bMask = 0x000000FF;
    } else if (bpp == 16) {
        rMask = 0xF800;
        gMask = 0x07E0;
        bMask = 0x001F;
    } else {
        kpanic("Unhandled bpp for screen shot: %d", bpp);
    }
    if (rect) {
        int inPitch = (screenWidth() * ((bpp + 7) / 8) + 3) & ~3;
        int outPitch = (rect->w * ((bpp + 7) / 8) + 3) & ~3;
        U32 bytesPerPixel = (bpp + 7) / 8;
        U32 len = outPitch * rect->h;

        if (!buffer) {
            pixels = new unsigned char[len];
            buffer = pixels;
        } else if (bufferlen < len) {
            return false;
        }

        for (int y = 0; y < rect->h; y++) {
            memcpy(buffer + y * outPitch, recordBuffer + (y + rect->y) * inPitch + (rect->x * bytesPerPixel), outPitch);
        }
        s = SDL_CreateRGBSurfaceFrom(buffer, rect->w, rect->h, bpp, outPitch, rMask, gMask, bMask, 0);
        if (!pixels && bpp <= 16) { // buffer was passed in and needs to be filled
            SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);

            SDL_Surface* tmp = SDL_ConvertSurface(s, format, 0);
            SDL_FreeFormat(format);

            outPitch = (rect->w * ((32 + 7) / 8) + 3) & ~3;
            int outLen = outPitch * rect->h;
            memcpy(buffer, tmp->pixels, outLen);
            SDL_FreeSurface(tmp);
        }
    } else {
        int pitch = (screenWidth() * ((bpp + 7) / 8) + 3) & ~3;

        s = SDL_CreateRGBSurfaceFrom(recordBuffer, screenWidth(), screenHeight(), bpp, pitch, rMask, gMask, bMask, 0);
        if (buffer) {
            int outPitch = (screenWidth() * ((32 + 7) / 8) + 3) & ~3;
            U32 outLen = outPitch * screenHeight();

            if (bufferlen < outLen) {
                return false;
            }
            if (bpp > 16) {
                memcpy(buffer, recordBuffer, outLen);
            } else {
                SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);                

                SDL_Surface* tmp = SDL_ConvertSurface(s, format, 0);
                SDL_FreeFormat(format);
                memcpy(buffer, tmp->pixels, outLen);
                SDL_FreeSurface(tmp);
            }
        }
    }

    if (!s) {
        klog("sdlScreenshot: %s", SDL_GetError());
        if (pixels) {
            delete[] pixels;
        }
        return false;
    }
    if (filepath.length()) {
        SDL_SaveBMP(s, filepath.c_str());
    }
    if (s) {
        SDL_FreeSurface(s);
    }
    if (pixels) {
        delete[] pixels;
    }
    return true;
#else
    return false;
#endif
}

bool KNativeScreenSDL::partialScreenShot(const BString& filepath, U32 x, U32 y, U32 w, U32 h, U8* buffer, U32 bufferlen) {
    SDL_Rect r;
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    return internalScreenShot(filepath, &r, buffer, bufferlen);
}

bool KNativeScreenSDL::screenShot(const BString& filepath, U8* buffer, U32 bufferlen) {
    return internalScreenShot(filepath, nullptr, buffer, bufferlen);
}

bool KNativeScreenSDL::saveBmp(const BString& filepath, U8* buffer, U32 bpp, U32 w, U32 h) {
    U32 rMask = 0;
    U32 gMask = 0;
    U32 bMask = 0;

    if (bpp == 32) {
        rMask = 0x00FF0000;
        gMask = 0x0000FF00;
        bMask = 0x000000FF;
    } else if (bpp == 16) {
        rMask = 0xF800;
        gMask = 0x07E0;
        bMask = 0x001F;
    } else {
        kpanic("Unhandled bpp for screen shot: %d", bpp);
    }
    int pitch = (screenWidth() * ((bpp + 7) / 8) + 3) & ~3;
    U32 len = pitch * screenHeight();

    SDL_Surface* s = SDL_CreateRGBSurfaceFrom(buffer, w, h, bpp, w * 4, rMask, gMask, bMask, 0);
    if (!s) {
        return false;
    }
    if (filepath.length()) {
        SDL_SaveBMP(s, filepath.c_str());
    }
    SDL_FreeSurface(s);
    return true;
}

void KNativeScreenSDL::buildCursor(KThread* thread, const std::shared_ptr<XCursor>& cursor, U32 pixelsAddress, U32 width, U32 height, S32 xHot, S32 yHot) {
    if (KSystem::videoOption == VIDEO_NO_WINDOW) {
        return;
    }
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
    if (KSystem::videoOption == VIDEO_NO_WINDOW) {
        return;
    }
    SDL_Cursor* sdlCursor;

    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cursorsMutex);
        sdlCursor = cursors.get(cursor->id);
    }
    if (!sdlCursor) {
        switch (cursor->shape) {
        case 0:
            if (!KSystem::disableHideCursor && cursor->fg.red == 0 && cursor->fg.blue == 0 && cursor->fg.green == 0 && cursor->bg.red == 0 && cursor->bg.green == 0 && cursor->bg.blue == 0) {
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
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

void KNativeScreenSDL::recreateMainWindow() {
    if (KSystem::videoOption != VIDEO_NO_WINDOW) {
        destroyMainWindow();
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, scaleQuality.c_str());

        int cx = input->width * input->scaleX / 100;
        int cy = input->height * input->scaleY / 100;
        int flags = SDL_WINDOW_HIDDEN | additionalSDLWindowFlags;
        
        visible = false;

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
