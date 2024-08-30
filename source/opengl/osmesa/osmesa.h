#ifndef __OSMESA_H__
#define __OSMESA_H__

#include "kopengl.h"

class OsMesaGL {
public:
	static KOpenGLPtr create();
	static bool isAvailable();
	static void iterateFormats(std::function<void(const GLPixelFormatPtr& format)> callback);
};

#endif