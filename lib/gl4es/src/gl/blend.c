#include "blend.h"

#include "../glx/hardext.h"
#include "debug.h"
#include "gl4es.h"
#include "glstate.h"
#include "init.h"
#include "loader.h"
#include "logs.h"

void APIENTRY_GL4ES gl4es_glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
    PUSH_IF_COMPILING(glBlendColor);
    LOAD_GLES2_OR_OES(glBlendColor);
	if  (gles_glBlendColor)
		gles_glBlendColor(red, green, blue, alpha);
	else {
        static int test = 1;
        if (test) {
            LOGD("stub glBlendColor(%f, %f, %f, %f)\n", red, green, blue, alpha);
            test = 0;
        }
    }
}
AliasExport(void,glBlendColor,,(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha));
AliasExport(void,glBlendColor,EXT,(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha));
AliasExport(void,glBlendColor,ARB,(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha));

void APIENTRY_GL4ES gl4es_glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    if(!glstate->list.pending) 
        PUSH_IF_COMPILING(glBlendFuncSeparate)

    LOAD_GLES2_OR_OES(glBlendFuncSeparate);
    if(sfactorRGB==glstate->blendsfactorrgb && dfactorRGB==glstate->blenddfactorrgb 
        && sfactorAlpha==glstate->blendsfactoralpha && dfactorAlpha==glstate->blenddfactoralpha)
        return; // no change...

    FLUSH_BEGINEND;

#ifndef PANDORA
    if(gles_glBlendFuncSeparate==NULL) {
        // some fallback function to have better rendering with SDL2, better then nothing...
        if(sfactorRGB==GL_SRC_ALPHA && dfactorRGB==GL_ONE_MINUS_SRC_ALPHA && sfactorAlpha==GL_ONE && dfactorAlpha==GL_ONE_MINUS_SRC_ALPHA)
            gl4es_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        else if (sfactorRGB==GL_SRC_ALPHA && dfactorRGB==GL_ONE && sfactorAlpha==GL_ZERO && dfactorAlpha==GL_ONE)
            gl4es_glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        else if (sfactorRGB==GL_ZERO && dfactorRGB==GL_SRC_COLOR && sfactorAlpha==GL_ZERO && dfactorAlpha==GL_ONE)
            gl4es_glBlendFunc(GL_ZERO, GL_SRC_COLOR);
        else if (sfactorRGB==sfactorAlpha && dfactorRGB==dfactorAlpha)
            gl4es_glBlendFunc(sfactorRGB, dfactorRGB);
    } else
#endif
    gles_glBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);

    glstate->blendsfactorrgb = sfactorRGB;
    glstate->blenddfactorrgb = dfactorRGB;
    glstate->blendsfactoralpha = sfactorAlpha;
    glstate->blenddfactoralpha = dfactorAlpha;
}
AliasExport(void,glBlendFuncSeparate,,(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha));
AliasExport(void,glBlendFuncSeparate,EXT,(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha));

void APIENTRY_GL4ES gl4es_glBlendEquationSeparate(GLenum modeRGB, GLenum modeA) {
    PUSH_IF_COMPILING(glBlendEquationSeparate);
    LOAD_GLES2_OR_OES(glBlendEquationSeparate);
#ifndef PANDORA
    if(gles_glBlendEquationSeparate)
#endif
    gles_glBlendEquationSeparate(modeRGB, modeA);
}
AliasExport(void,glBlendEquationSeparate,,(GLenum modeRGB, GLenum modeA));
AliasExport(void,glBlendEquationSeparate,EXT,(GLenum modeRGB, GLenum modeA));

void APIENTRY_GL4ES gl4es_glBlendFunc(GLenum sfactor, GLenum dfactor) {
    if(!glstate->list.pending) 
        PUSH_IF_COMPILING(glBlendFunc)

    if(sfactor==glstate->blendsfactorrgb && dfactor==glstate->blenddfactorrgb 
        && sfactor==glstate->blendsfactoralpha && dfactor==glstate->blenddfactoralpha)
        return; // already set

    FLUSH_BEGINEND;

    LOAD_GLES(glBlendFunc);
    LOAD_GLES2_OR_OES(glBlendFuncSeparate);
    errorGL();
    
    glstate->blendsfactorrgb = sfactor;
    glstate->blenddfactorrgb = dfactor;
    glstate->blendsfactoralpha = sfactor;
    glstate->blenddfactoralpha = dfactor;
    // There are some limitations in GLES1.1 Blend functions
    switch(sfactor) {
        #if 0
        case GL_SRC_COLOR:
            if (gles_glBlendFuncSeparate) {
                gles_glBlendFuncSeparate(sfactor, dfactor, sfactor, dfactor);
                return;
            }
            sfactor = GL_ONE;   // approx...
            break;
        case GL_ONE_MINUS_SRC_COLOR:
            if (gles_glBlendFuncSeparate) {
                gles_glBlendFuncSeparate(sfactor, dfactor, sfactor, dfactor);
                return;
            }
            sfactor = GL_ONE;  // not sure it make sense...
            break;
        #endif
        // here, we need support for glBlendColor...
        case GL_CONSTANT_COLOR:
        case GL_CONSTANT_ALPHA:
            if(hardext.blendcolor==0)
                sfactor = GL_ONE;
            break;
        case GL_ONE_MINUS_CONSTANT_COLOR:
        case GL_ONE_MINUS_CONSTANT_ALPHA:
            if(hardext.blendcolor==0)
                sfactor = GL_ZERO;
            break;
        default:
            break;
    }
    
    switch(dfactor) {
        #if 0
        case GL_DST_COLOR:
            sfactor = GL_ONE;   // approx...
            break;
        case GL_ONE_MINUS_DST_COLOR:
            sfactor = GL_ZERO;  // not sure it make sense...
            break;
        #endif
        // here, we need support for glBlendColor...
        case GL_CONSTANT_COLOR:
        case GL_CONSTANT_ALPHA:
            if(hardext.blendcolor==0)
                sfactor = GL_ONE;
            break;
        case GL_ONE_MINUS_CONSTANT_COLOR:
        case GL_ONE_MINUS_CONSTANT_ALPHA:
            if(hardext.blendcolor==0)
                sfactor = GL_ZERO;
            break;
        default:
            break;
    }
    
    if ((globals4es.blendhack) && (sfactor==GL_SRC_ALPHA) && (dfactor==GL_ONE)) {
        // special case, as seen in Xash3D, but it breaks torus_trooper, so behind a parameter
        sfactor = GL_ONE;
    }
#ifdef ODROID
    if(gles_glBlendFunc)
#endif
    gles_glBlendFunc(sfactor, dfactor);
}
AliasExport(void,glBlendFunc,,(GLenum sfactor, GLenum dfactor));

void APIENTRY_GL4ES gl4es_glBlendEquation(GLenum mode) {
    PUSH_IF_COMPILING(glBlendEquation)
    LOAD_GLES2_OR_OES(glBlendEquation);
    errorGL();
#ifdef ODROID
    if(gles_glBlendEquation)
#endif
    gles_glBlendEquation(mode);
}
AliasExport(void,glBlendEquation,,(GLenum mode));
AliasExport(void,glBlendEquation,EXT,(GLenum mode));