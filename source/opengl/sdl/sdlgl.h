#ifndef __SDLGL_H__
#define __SDLGL_H__

#ifdef BOXEDWINE_OPENGL_SDL
#include "kopengl.h"

class SDLGL {
public:
	static KOpenGLPtr create();
	static void iterateFormats(std::function<void(const GLPixelFormatPtr& format)> callback);
};

#endif

#endif