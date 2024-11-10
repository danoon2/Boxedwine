/* GLX 1.0 */
XVisualInfo* glXChooseVisual(Display* dpy, int screen, int* attribList) {
	CALL_3_R(kXChooseVisual, dpy, screen, attribList);
}

GLXContext glXCreateContext(Display* dpy, XVisualInfo* vis, GLXContext shareList, Bool direct) {
	CALL_4_R(kXCreateContext, dpy, vis, shareList, direct);
}

void glXDestroyContext(Display* dpy, GLXContext ctx) {
	CALL_2(kXDestroyContext, dpy, ctx);
}

Bool glXMakeCurrent(Display* dpy, GLXDrawable drawable, GLXContext ctx) {
	CALL_3_R(kXMakeCurrent, dpy, drawable, ctx);
}

void glXCopyContext(Display* dpy, GLXContext src, GLXContext dst, unsigned long mask) {
	CALL_4(kXCopyContext, dpy, src, dst, mask);
}

void glXSwapBuffers(Display* dpy, GLXDrawable drawable) {
	CALL_2(kXSwapBuffers, dpy, drawable);
}

Bool glXQueryVersion(Display* dpy, int* maj, int* min) {
	*maj = 1;
	*min = 3;
	return True;
	// CALL_3_R(XQueryVersion, dpy, maj, min);
}

Bool glXIsDirect(Display* dpy, GLXContext ctx) {
	CALL_2_R(kXIsDirect, dpy, ctx);
}

GLXContext glXGetCurrentContext(void) {
	CALL_0_R(kXGetCurrentContext);
}

GLXDrawable glXGetCurrentDrawable(void) {
	CALL_0_R(kXGetCurrentDrawable);
}

/* GLX 1.1 */
const char* glXQueryExtensionsString(Display* dpy, int screen) {
	CALL_2_R(kXQueryExtensionsString, dpy, screen);
}

const char* glXQueryServerString(Display* dpy, int screen, int name) {
	if (name == GLX_VERSION)
		return "1.3";
	if (name == GLX_VENDOR)
		return "BoxedWine GL";
	CALL_3_R(kXQueryServerString, dpy, screen, name);
}

const char* glXGetClientString(Display* dpy, int name) {
	if (name == GLX_VERSION)
		return "1.3";
	if (name == GLX_VENDOR)
		return "BoxedWine GL";
	CALL_2_R(kXGetClientString, dpy, name);
}


/* GLX 1.3 */
GLXFBConfig* glXChooseFBConfig(Display* dpy, int screen, const int* attribList, int* nitems) {
	CALL_4_R(kXChooseFBConfig, dpy, screen, attribList, nitems);
}

int glXGetFBConfigAttrib(Display* dpy, GLXFBConfig config, int attribute, int* value) {
	CALL_4_R(kXGetFBConfigAttrib, dpy, config, attribute, value);
}

GLXFBConfig* glXGetFBConfigs(Display* dpy, int screen, int* nelements) {
	CALL_3_R(kXGetFBConfigs, dpy, screen, nelements);
}

XVisualInfo* glXGetVisualFromFBConfig(Display* dpy, GLXFBConfig config) {
	CALL_2_R(kXGetVisualFromFBConfig, dpy, config);
}

GLXPbuffer glXCreatePbuffer(Display* dpy, GLXFBConfig config, const int* attribList) {
	CALL_3_R(kXCreatePbuffer, dpy, config, attribList);
}

void glXDestroyPbuffer(Display* dpy, GLXPbuffer pbuf) {
	CALL_2(kXDestroyPbuffer, dpy, pbuf);
}

void glXQueryDrawable(Display* dpy, GLXDrawable draw, int attribute, unsigned int* value) {
	CALL_4(kXQueryDrawable, dpy, draw, attribute, value);
}

GLXContext glXCreateNewContext(Display* dpy, GLXFBConfig config, int renderType, GLXContext shareList, Bool direct) {
	CALL_5_R(kXCreateNewContext, dpy, config, renderType, shareList, direct);
}

Bool glXMakeContextCurrent(Display* dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx) {
	CALL_4_R(kXMakeContextCurrent, dpy, draw, read, ctx);
}

GLXPixmap glXCreatePixmap(Display* dpy, GLXFBConfig config, Pixmap pixmap, const int* attrib_list) {
	CALL_4_R(kXCreatePixmap, dpy, config, pixmap, attrib_list);
}

void glXDestroyPixmap(Display* dpy, GLXPixmap pixmap) {
	CALL_2(kXDestroyPixmap, dpy, pixmap);
}

GLXWindow glXCreateWindow(Display* dpy, GLXFBConfig config, Window win, const int* attrib_list) {
	CALL_4_R(kXCreateWindow, dpy, config, win, attrib_list);
}

void glXDestroyWindow(Display* dpy, GLXWindow win) {
	CALL_2(kXDestroyWindow, dpy, win);
}


/* GLX Extensions */
GLXContext glXCreateContextAttribsARB(Display* dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int* attrib_list) {
	CALL_5_R(kXCreateContextAttribsARB, dpy, config, share_context, direct, attrib_list);
}

#include <dlfcn.h>
__GLXextFuncPtr glXGetProcAddressARB(const GLubyte* procName) {
	void* result = dlsym((void*)0, (const char*)procName);
	printf("libGL glXGetProcessAddressARB: %s result=%X\n", (const char*)procName, (int)result);
	return result;
}

void glXSwapIntervalEXT(Display* dpy, GLXDrawable drawable, int interval) {
	CALL_3(kXSwapIntervalEXT, dpy, drawable, interval);
}

/*
int pglXSwapIntervalSGI(int) {
}
*/
