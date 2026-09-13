/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
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

#ifndef BOXEDWINE_GL_COUNTERS_H
#define BOXEDWINE_GL_COUNTERS_H

#include <array>
#include <atomic>
#include <cstdint>

#ifndef GL_FUNC_COUNT
#include "../../tools/opengl/gldef.h"
#endif

// Raw dispatched int99 calls, plus actual host binding changes and dispatches.
// Keep diagnostic state separate from the guest GL call ABI and object state.
// Include the host GL declarations first, as required by glfunctions.h.
class GLCallCounters {
public:
    static constexpr unsigned contextChanges = GL_FUNC_COUNT;
    static constexpr unsigned mainThreadDispatches = GL_FUNC_COUNT + 1;
    static constexpr unsigned count = GL_FUNC_COUNT + 2;

    void record(unsigned index) {
        if (index < count) {
            values[index].fetch_add(1, std::memory_order_relaxed);
        }
    }

    std::uint64_t get(unsigned index) const {
        return index < count ? values[index].load(std::memory_order_relaxed) : 0;
    }

    static const char* name(unsigned index) {
        return index < count ? names[index] : nullptr;
    }

private:
    std::array<std::atomic<std::uint64_t>, count> values{};
    static constexpr auto names = [] {
        std::array<const char*, count> result{};
#pragma push_macro("GL_FUNCTION")
#pragma push_macro("GL_FUNCTION_FMT")
#pragma push_macro("GL_FUNCTION_CUSTOM")
#pragma push_macro("GL_EXT_FUNCTION")
#undef GL_FUNCTION
#undef GL_FUNCTION_FMT
#undef GL_FUNCTION_CUSTOM
#undef GL_EXT_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG) result[func] = "gl" #func;
#define GL_FUNCTION_FMT(func, RET, PARAMS, ARGS, PRE, POST, LOG) result[func] = "gl" #func;
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS) result[func] = "gl" #func;
#define GL_EXT_FUNCTION(func, RET, PARAMS) result[func] = "gl" #func;
#include "glfunctions.h"
#pragma pop_macro("GL_EXT_FUNCTION")
#pragma pop_macro("GL_FUNCTION_CUSTOM")
#pragma pop_macro("GL_FUNCTION_FMT")
#pragma pop_macro("GL_FUNCTION")
        // SDL registers these callbacks directly, outside glfunctions.h.
        result[Finish] = "glFinish";
        result[Flush] = "glFlush";
        result[kXCreateContext] = "glXCreateContext";
        result[kXMakeCurrent] = "glXMakeCurrent";
        result[kXDestroyContext] = "glXDestroyContext";
        result[kXChooseVisual] = "glXChooseVisual";
        result[kXCopyContext] = "glXCopyContext";
        result[kXQueryVersion] = "glXQueryVersion";
        result[kXIsDirect] = "glXIsDirect";
        result[kXGetCurrentContext] = "glXGetCurrentContext";
        result[kXGetCurrentDrawable] = "glXGetCurrentDrawable";
        result[kXQueryExtensionsString] = "glXQueryExtensionsString";
        result[kXQueryServerString] = "glXQueryServerString";
        result[kXGetClientString] = "glXGetClientString";
        result[kXChooseFBConfig] = "glXChooseFBConfig";
        result[kXGetFBConfigAttrib] = "glXGetFBConfigAttrib";
        result[kXGetFBConfigs] = "glXGetFBConfigs";
        result[kXGetVisualFromFBConfig] = "glXGetVisualFromFBConfig";
        result[kXCreatePbuffer] = "glXCreatePbuffer";
        result[kXDestroyPbuffer] = "glXDestroyPbuffer";
        result[kXQueryDrawable] = "glXQueryDrawable";
        result[kXCreateNewContext] = "glXCreateNewContext";
        result[kXMakeContextCurrent] = "glXMakeContextCurrent";
        result[kXCreatePixmap] = "glXCreatePixmap";
        result[kXDestroyPixmap] = "glXDestroyPixmap";
        result[kXDestroyWindow] = "glXDestroyWindow";
        result[kXCreateContextAttribsARB] = "glXCreateContextAttribsARB";
        result[kXSwapIntervalEXT] = "glXSwapIntervalEXT";
        result[kXSwapBuffers] = "glXSwapBuffers";
        result[kXCreateWindow] = "glXCreateWindow";
        result[kEglGetDisplay] = "eglGetDisplay";
        result[kEglInitialize] = "eglInitialize";
        result[kEglTerminate] = "eglTerminate";
        result[kEglQueryString] = "eglQueryString";
        result[kEglGetConfigs] = "eglGetConfigs";
        result[kEglChooseConfig] = "eglChooseConfig";
        result[kEglGetConfigAttrib] = "eglGetConfigAttrib";
        result[kEglBindAPI] = "eglBindAPI";
        result[kEglCreateContext] = "eglCreateContext";
        result[kEglDestroyContext] = "eglDestroyContext";
        result[kEglCreateWindowSurface] = "eglCreateWindowSurface";
        result[kEglCreatePbufferSurface] = "eglCreatePbufferSurface";
        result[kEglDestroySurface] = "eglDestroySurface";
        result[kEglMakeCurrent] = "eglMakeCurrent";
        result[kEglSwapBuffers] = "eglSwapBuffers";
        result[kEglSwapInterval] = "eglSwapInterval";
        result[kEglGetCurrentContext] = "eglGetCurrentContext";
        result[kEglGetCurrentSurface] = "eglGetCurrentSurface";
        result[kEglGetCurrentDisplay] = "eglGetCurrentDisplay";
        result[kEglQuerySurface] = "eglQuerySurface";
        result[kEglGetError] = "eglGetError";
        result[kEglReleaseThread] = "eglReleaseThread";
        result[kEglWaitGL] = "eglWaitGL";
        result[kEglWaitNative] = "eglWaitNative";
        result[kEglCopyBuffers] = "eglCopyBuffers";
        result[kEglSurfaceAttrib] = "eglSurfaceAttrib";
        result[kEglBindTexImage] = "eglBindTexImage";
        result[kEglReleaseTexImage] = "eglReleaseTexImage";
        result[kEglCreateSync] = "eglCreateSync";
        result[kEglDestroySync] = "eglDestroySync";
        result[kEglClientWaitSync] = "eglClientWaitSync";
        result[kEglGetSyncAttrib] = "eglGetSyncAttrib";
        result[kEglWaitSync] = "eglWaitSync";
        result[kEglCreateImage] = "eglCreateImage";
        result[kEglDestroyImage] = "eglDestroyImage";
        result[kEglCreatePbufferFromClientBuffer] = "eglCreatePbufferFromClientBuffer";
        result[kEglCreatePixmapSurface] = "eglCreatePixmapSurface";
        result[kEglCreatePlatformPixmapSurface] = "eglCreatePlatformPixmapSurface";
        result[kEglCreatePlatformWindowSurface] = "eglCreatePlatformWindowSurface";
        result[kEglGetPlatformDisplay] = "eglGetPlatformDisplay";
        result[kEglQueryAPI] = "eglQueryAPI";
        result[kEglQueryContext] = "eglQueryContext";
        result[kEglWaitClient] = "eglWaitClient";
        result[kGlProcAddressAvailable] = "glProcAddressAvailable";
        result[contextChanges] = "hostContextChanges";
        result[mainThreadDispatches] = "mainThreadDispatches";
        return result;
    }();
};

#endif
