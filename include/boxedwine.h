#ifndef __BOXEDWINE_H__
#define __BOXEDWINE_H__

#define BOXEDWINE_VERSION_STR "222"
#define BOXEDWINE_VERSION_DISPLAY "23.0.2 (beta)"

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <queue>
#include <functional>
#include <set>
#include <list>
#include <filesystem>

#include <errno.h>

#ifdef BOXEDWINE_SDL1
#define SDL_INIT_EVENTS 0
#define SDL_ShowSimpleMessageBox(a, b, c, d)
//#define SDL_HasClipboardText() false
//#define SDL_GetClipboardText() NULL
//#define SDL_SetClipboardText(x) NULL
#define SDL_GetDisplayDPI(a, b, c, d) 0
//#define SDL_GetCurrentDisplayMode(x, y) 1
//#define SDL_GetDesktopDisplayMode(a, b) 1
//#define SDL_DisplayMode SDL_Rect
#define SDL_GetBasePath() NULL
#define SDL_GetPrefPath(x, y) NULL
//! #define SDL_FlushEvent(x)
#ifdef BOXEDWINE_MULTI_THREADED
#error "BOXEDWINE_MULTI_THREADED cannot be defined for SDL1"
#endif
#ifdef BOXEDWINE_OPENGL_SDL
#error "BOXEDWINE_OPENGL_SDL cannot be defined for SDL1"
#endif
//#define AUDIO_S32LSB 0
//#define AUDIO_F32LSB 0
//#define SDL_CreateWindow(title, x, y, cx, cy, flags) SDL_SetVideoMode(cx, cy, 32, 0)
//#define SDL_WarpMouseInWindow(w, x, y) SDL_WarpMouse(x, y)
#define SDL_Texture SDL_Surface
#define SDL_Window SDL_Surface
//#define SDL_Renderer SDL_Surface
#define SDL_DestroyTexture SDL_FreeSurface
//! #define SDL_DestroyRenderer(x)
//! #define SDL_DestroyWindow(x)
//! #define SDL_SetWindowSize(window, x, y)
//! #define SDL_SetHint(a, b)
//#define SDL_WINDOW_HIDDEN 0
#define SDL_WINDOW_VULKAN 0
//! #define SDL_Log(x, y)
#define SDL_WINDOW_FULLSCREEN_DESKTOP 0
//#define SDL_RENDERER_ACCELERATED 0
//#define SDL_RENDERER_PRESENTVSYNC 0
//! #define SDL_CreateRenderer(a, b, c) window
//#define SDL_RENDERER_SOFTWARE 0
//#define SDL_CreateTexture(renderer, format, access, w, h) SDL_CreateRGBSurface(0, w, h, bpp, 0xff0000, 0xff00, 0xff, 0)
//#define SDL_UpdateTexture(s, a, bits, p) \
//        if (SDL_MUSTLOCK(s)) {      \
//            SDL_LockSurface(s);     \
//        }                           \
//        for (int y = 0; y < height; y++) {  \
//            memcpy((S8*)(s->pixels) + y * s->pitch, bits + (height - y - 1) * (p), p); \
//        }                           \
//        if (SDL_MUSTLOCK(s)) {      \
//            SDL_UnlockSurface(s);   \
//        }
//#define SDL_PIXELFORMAT_ARGB8888 0
//#define SDL_PIXELFORMAT_RGB565 0
//#define SDL_PIXELFORMAT_RGB555 0
//#define SDL_SetRenderDrawColor(renderer, r, g, b, a) U32 drawColor = SDL_MapRGB(renderer->format, r, g, b)
//#define SDL_RenderClear(renderer) SDL_FillRect(renderer, NULL, drawColor)
//#define SDL_RenderFillRect(renderer, rect) SDL_FillRect(renderer, rect, drawColor)
//#define SDL_RenderPresent(renderer) SDL_UpdateRect(renderer, 0, 0, 0, 0)
//#define SDL_RenderCopy(renderer, texture, srcRect, dstRect) SDL_BlitSurface(texture, srcRect, renderer, dstRect)
//! #define SDL_SetWindowTitle(window, title) SDL_WM_SetCaption(title, title)
//! #define SDL_RenderCopyEx(renderer, texture, srcRect, dstRect, angle, center, flip) SDL_BlitSurface(texture, srcRect, renderer, dstRect)
#define SDL_GetWindowGammaRamp(window, r, g, b) SDL_GetGammaRamp(r, g, b)

//! #define SDL_GetWindowSize(window, w, h)
//! #define SDL_ShowWindow(window)
//! #define SDL_RaiseWindow(window)
#endif

#if defined(BOXEDWINE_OPENGL_SDL) || defined(BOXEDWINE_OPENGL_ES) || defined(BOXEDWINE_OPENGL_OSMESA)
#define BOXEDWINE_OPENGL
#endif

#ifdef BOXEDWINE_MAC_JIT
#include "TargetConditionals.h"
#if TARGET_CPU_ARM64
#define BOXEDWINE_BINARY_TRANSLATOR
#define BOXEDWINE_ARMV8BT
#define BOXEDWINE_MULTI_THREADED
#define MAP_BOXEDWINE MAP_JIT
#define BOXEDWINE_64BIT_MMU
#define K_NATIVE_PAGE_SIZE 16384
#define K_NATIVE_NUMBER_OF_PAGES 0x40000
#define K_NATIVE_PAGE_MASK 0x3FFF
#define K_NATIVE_PAGE_SHIFT 14
#define K_NATIVE_PAGES_PER_PAGE 4
#ifdef _DEBUG
#define BOXEDWINE_BT_DEBUG_NO_EXCEPTIONS
#endif
#else
#undef BOXEDWINE_MAC_JIT
#ifdef _DEBUG
#define BOXEDWINE_BT_DEBUG_NO_EXCEPTIONS
#endif
#define BOXEDWINE_BINARY_TRANSLATOR
#define BOXEDWINE_X64
#define BOXEDWINE_MULTI_THREADED
#define BOXEDWINE_64BIT_MMU
#define MAP_BOXEDWINE 0
#endif
#else
#define MAP_BOXEDWINE 0
#endif

#include "../source/util/boxedptr.h"

#include "platform.h"
#include "log.h"

#include "../source/util/klist.h"
#include "ktimer.h"
#include "../source/util/synchronization.h"
#include "../source/util/karray.h"
#include "../source/util/stringutil.h"
#include "../source/util/vectorutils.h"
#include "../source/util/fileutils.h"

#include "../source/emulation/cpu/common/cpu.h"
#include "kpoll.h"
#include "memory.h"
#include "kthread.h"
#include "kfilelock.h"
#include "kobject.h"
#include "kfiledescriptor.h"
#include "../source/io/fs.h"
#include "../source/io/fsnode.h"
#include "../source/io/fsopennode.h"
#include "kfile.h"
#include "ksystem.h"
#include "kprocess.h"
#include "kscheduler.h"
#include "recorder.h"
#include "player.h"

#include "log.h"
#include "kerror.h"

#endif
