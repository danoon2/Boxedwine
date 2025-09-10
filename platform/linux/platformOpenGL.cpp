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

#ifdef BOXEDWINE_OPENGL
#include "platformOpenGL.h"
#include GLH
//#include "../../source/x11/x11.h"
#include "../../source/opengl/glcommon.h"

BHashTable<U32, GLPixelFormatPtr> PlatformOpenGL::formatsById;
std::vector<GLPixelFormatPtr> PlatformOpenGL::formats;
bool PlatformOpenGL::hardwareListLoaded;

#if defined(__linux__)
#include <GL/glx.h>
#include <dlfcn.h>

typedef Display* (*PFNXOpenDisplay)(_Xconst char*);
typedef int (*PFNXCloseDisplay)(Display*);
typedef int (*PFNXFree)(void*);
typedef GLXFBConfig* (*PFNglXGetFBConfigs)(Display* dpy, int screen, int* nelements);
typedef int (*PFNglXGetFBConfigAttrib)(Display* dpy, GLXFBConfig config, int attribute, int* value);

PFNglXGetFBConfigAttrib pfnglXGetFBConfigAttrib;

static int getGLXFBConfigAttrib(Display* dsp, GLXFBConfig fbconfig, int attrib)
{
    int value;
    pfnglXGetFBConfigAttrib(dsp, fbconfig, attrib, &value);
    return value;
}

void PlatformOpenGL::init() {
    formatsById.clear();
    formats.clear();
    hardwareListLoaded = false;

    if (KSystem::videoOption == VIDEO_NORMAL) {
        int nativeCount = 0;
        void* libx11 = dlopen("libX11.so", RTLD_NOW | RTLD_LOCAL);        
        if (!libx11) {
            libx11 = dlopen("libX11.so.6", RTLD_NOW | RTLD_LOCAL);
        }
        if (!libx11) {
            return;
        }
        void* libgl = dlopen("libGL.so", RTLD_NOW | RTLD_LOCAL);
        if (!libgl) {
            libgl = dlopen("libGL.so.1", RTLD_NOW | RTLD_LOCAL);
        }
        if (!libgl) {
            dlclose(libx11);
            return;
        }

        PFNXOpenDisplay pfnXOpenDisplay = (PFNXOpenDisplay)dlsym(libx11, "XOpenDisplay");
        PFNXCloseDisplay pfnXCloseDisplay = (PFNXCloseDisplay)dlsym(libx11, "XCloseDisplay");
        PFNXFree pfnXFree = (PFNXFree)dlsym(libgl, "XFree");

        PFNglXGetFBConfigs pfnglXGetFBConfigs = (PFNglXGetFBConfigs)dlsym(libgl, "glXGetFBConfigs");
        pfnglXGetFBConfigAttrib = (PFNglXGetFBConfigAttrib)dlsym(libgl, "glXGetFBConfigAttrib");

        if (!pfnXOpenDisplay || !pfnXCloseDisplay || !pfnXFree || !pfnglXGetFBConfigs || !pfnglXGetFBConfigAttrib) {
            dlclose(libx11);
            dlclose(libgl);
            return;
        }
        Display* dsp = pfnXOpenDisplay(0);
        if (!dsp) {
            klog("could not open display");
            dlclose(libx11);
            dlclose(libgl);
            return;
        }
        GLXFBConfig* nativeConfigs = pfnglXGetFBConfigs(dsp, DefaultScreen(dsp), &nativeCount);
        if (!nativeConfigs || !nativeCount) {
            pfnXCloseDisplay(dsp);
            dlclose(libx11);
            dlclose(libgl);
            return;
        }
        for (int i = 0; i < nativeCount; i++)
        {
            GLPixelFormatPtr format = std::make_shared<GLPixelFormat>();

            const GLXFBConfig cfg = nativeConfigs[i];
            int renderType = getGLXFBConfigAttrib(dsp, cfg, GLX_RENDER_TYPE);
            int drawableType = getGLXFBConfigAttrib(dsp, cfg, GLX_DRAWABLE_TYPE);
            bool doubleBuffer = false;

            if (renderType & GLX_RGBA_BIT) {
                format->pf.iPixelType = K_PFD_TYPE_RGBA;
            } else {
                format->pf.iPixelType = K_PFD_TYPE_COLORINDEX;
            }
            format->id = i | PIXEL_FORMAT_NATIVE_INDEX_MASK;
            format->nativeId = i;
            format->pf.dwFlags = K_PFD_SUPPORT_OPENGL;
            if (getGLXFBConfigAttrib(dsp, cfg, GLX_STEREO)) {
                format->pf.dwFlags |= K_PFD_STEREO;
            }
            if (getGLXFBConfigAttrib(dsp, cfg, GLX_DOUBLEBUFFER)) {
                format->pf.dwFlags |= K_PFD_DOUBLEBUFFER;
                doubleBuffer = true;
            }            
            if (drawableType & GLX_WINDOW_BIT) {
                format->pf.dwFlags |= K_PFD_DRAW_TO_WINDOW;
            }
            if (drawableType & GLX_PIXMAP_BIT && !doubleBuffer) {
                format->pf.dwFlags |= K_PFD_DRAW_TO_BITMAP | K_PFD_GENERIC_FORMAT | K_PFD_SUPPORT_GDI;
            }
            if (drawableType & GLX_PBUFFER) {
                format->pbuffer = true;
                format->pbufferMaxWidth = getGLXFBConfigAttrib(dsp, cfg, GLX_MAX_PBUFFER_WIDTH);
                format->pbufferMaxWidth = getGLXFBConfigAttrib(dsp, cfg, GLX_MAX_PBUFFER_HEIGHT);
                format->pbufferMaxPixels = getGLXFBConfigAttrib(dsp, cfg, GLX_MAX_PBUFFER_PIXELS);
            }
            format->pf.cColorBits = getGLXFBConfigAttrib(dsp, cfg, GLX_BUFFER_SIZE);

            if (format->pf.iPixelType == K_PFD_TYPE_RGBA) {
                format->pf.cRedBits = getGLXFBConfigAttrib(dsp, cfg, GLX_RED_SIZE);
                format->pf.cGreenBits = getGLXFBConfigAttrib(dsp, cfg, GLX_GREEN_SIZE);
                format->pf.cBlueBits = getGLXFBConfigAttrib(dsp, cfg, GLX_BLUE_SIZE);
                format->pf.cAlphaBits = getGLXFBConfigAttrib(dsp, cfg, GLX_ALPHA_SIZE);

                format->pf.cRedShift = format->pf.cGreenBits + format->pf.cBlueBits + format->pf.cAlphaBits;
                format->pf.cBlueShift = format->pf.cAlphaBits;
                format->pf.cGreenShift = format->pf.cBlueBits + format->pf.cAlphaBits;
                format->pf.cAlphaShift = 0;
            }
            
            format->pf.cDepthBits = getGLXFBConfigAttrib(dsp, cfg, GLX_DEPTH_SIZE);
            format->pf.cStencilBits = getGLXFBConfigAttrib(dsp, cfg, GLX_STENCIL_SIZE);
            format->pf.cAccumRedBits = getGLXFBConfigAttrib(dsp, cfg, GLX_ACCUM_RED_SIZE);
            format->pf.cAccumGreenBits = getGLXFBConfigAttrib(dsp, cfg, GLX_ACCUM_GREEN_SIZE);
            format->pf.cAccumBlueBits = getGLXFBConfigAttrib(dsp, cfg, GLX_ACCUM_BLUE_SIZE);
            format->pf.cAccumAlphaBits = getGLXFBConfigAttrib(dsp, cfg, GLX_ACCUM_ALPHA_SIZE);
            format->pf.cAccumBits = format->pf.cAccumRedBits + format->pf.cAccumGreenBits + format->pf.cAccumBlueBits + format->pf.cAccumAlphaBits;
 
            format->pf.cAuxBuffers = getGLXFBConfigAttrib(dsp, cfg, GLX_AUX_BUFFERS);    

            format->pf.iLayerType = K_PFD_MAIN_PLANE;

            format->samples = getGLXFBConfigAttrib(dsp, cfg, GLX_SAMPLES);
            format->sampleBuffers = getGLXFBConfigAttrib(dsp, cfg, GLX_SAMPLE_BUFFERS);
            format->depth = format->pf.cColorBits;
            formatsById.set(format->id, format);
            formats.push_back(format);    
        }
        pfnXFree(nativeConfigs);
        pfnXCloseDisplay(dsp);
        dlclose(libx11);
        dlclose(libgl);
    }    
}
#else
void PlatformOpenGL::init() {
    formatsById.clear();
    formats.clear();
    hardwareListLoaded = false;
    U32 count = KSystem::getPixelFormatCount();
    for (U32 i = 1; i < count; i++) {
        GLPixelFormatPtr format = std::make_shared<GLPixelFormat>();
        format->id = i;
        format->pf = *KSystem::getPixelFormat(i);
        format->nativeId = i;
        format->depth = format->pf.cColorBits;
        format->bitsPerPixel = format->pf.cColorBits;
        formatsById.set(format->id, format);
        formats.push_back(format);
    }
    if (KSystem::videoOption == VIDEO_NORMAL) {
        //hardwareListLoaded = queryOpenGL(formatsById, formats);
    }
}
#endif

void PlatformOpenGL::iterateFormats(std::function<void(const GLPixelFormatPtr& format)> callback) {
	for (auto& format : formats) {
		callback(format);
	}
}

GLPixelFormatPtr PlatformOpenGL::getFormat(U32 pixelFormatId) {
	return formatsById.get(pixelFormatId);
}
#endif
