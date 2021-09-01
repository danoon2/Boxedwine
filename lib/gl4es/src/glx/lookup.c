#include "../gl/attributes.h"
#include "../gl/init.h"
#include "../gl/logs.h"

#define STUB_FCT glXStub
#include "../gl/gl_lookup.h"

#include "glx.h"
#include "hardext.h"


void glXStub(void *x, ...) {
    return;
}
#ifdef BOXEDWINE_OPENGL_ES
void* gl4es_glXGetProcAddress(const char* name);
#else
void *gl4es_glXGetProcAddress(const char *name) __attribute__((visibility("default")));
#endif
void *gl4es_glXGetProcAddress(const char *name) {

#ifndef NOX11
    // glX calls
    _EX(glXChooseVisual);
    _EX(glXCopyContext);
    _EX(glXCreateContext);
    _EX(glXCreateNewContext);
    _EX(glXCreateContextAttribsARB);
    _EX(glXDestroyContext);
    _EX(glXGetConfig);
    _EX(glXGetCurrentDisplay);
    _EX(glXGetCurrentDrawable);
    _EX(glXIsDirect);
    _EX(glXMakeCurrent);
    _EX(glXMakeContextCurrent);
    _EX(glXQueryExtensionsString);
    _EX(glXQueryServerString);
    _EX(glXSwapBuffers);
    _EX(glXSwapIntervalEXT);
#endif //NOX11
#ifndef BOXEDWINE_OPENGL_ES
    MAP("glXSwapIntervalMESA", gl4es_glXSwapInterval);
    MAP("glXSwapIntervalSGI", gl4es_glXSwapInterval);
#endif
#ifndef NOX11
    _EX(glXUseXFont);
    _EX(glXWaitGL);
    _EX(glXWaitX);
    _EX(glXGetCurrentContext);
    _EX(glXQueryExtension);
    _EX(glXQueryDrawable);
    _EX(glXQueryVersion);
    _EX(glXGetClientString);
    _EX(glXGetFBConfigs);
    _EX(glXChooseFBConfig);
    MAP("glXChooseFBConfigSGIX", gl4es_glXChooseFBConfig);
    _EX(glXGetFBConfigAttrib);
    _EX(glXQueryContext);
    _EX(glXGetVisualFromFBConfig);
    _EX(glXCreateWindow);
    _EX(glXDestroyWindow);
    
    _EX(glXCreatePbuffer);
    _EX(glXDestroyPbuffer);
    _EX(glXCreatePixmap);
    _EX(glXDestroyPixmap);
    _EX(glXCreateGLXPixmap);
    _EX(glXDestroyGLXPixmap);
    STUB(glXGetCurrentReadDrawable);
    STUB(glXGetSelectedEvent);
    STUB(glXSelectEvent);

    _EX(glXCreateContextAttribs);
    _ARB(glXCreateContextAttribs);
#endif //NOX11
    _EX(glXGetProcAddress);
    _ARB(glXGetProcAddress);

    return gl4es_GetProcAddress(name);
}
#ifdef AMIGAOS4
//AliasExport(void*,aglGetProcAddress,,(const char* name));
#else
#ifndef BOXEDWINE_OPENGL_ES
AliasExport(void*,glXGetProcAddress,,(const char* name));
AliasExport(void*,glXGetProcAddress,ARB,(const char* name));
#endif
#endif