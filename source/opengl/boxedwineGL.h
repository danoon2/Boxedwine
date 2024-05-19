#ifndef __BOXEDWINE_GL_H__
#define __BOXEDWINE_GL_H__

#include "knativewindow.h"

class BoxedwineGL {
public:
	static BoxedwineGL* current;

	virtual void deleteContext(void* context) = 0;
	virtual bool makeCurrent(void* context, void* window) = 0;
	virtual BString getLastError() = 0;
	virtual void* createContext(void* window, std::shared_ptr<Wnd> wnd, PixelFormat* pixelFormat, U32 width, U32 height, int major, int minor, int profile) = 0;
	virtual void swapBuffer(void* window) = 0;
	virtual void setSwapInterval(U32 vsync) = 0;
	virtual bool shareList(const std::shared_ptr<KThreadGlContext>& src, const std::shared_ptr<KThreadGlContext>& dst, void* window) = 0;
};

#endif