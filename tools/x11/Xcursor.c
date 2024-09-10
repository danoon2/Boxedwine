#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>

#include "X11_def.h"

XcursorImage* XcursorImageCreate(int width, int height) {
	CALL_2_R(X11_CURSOR_IMAGE_CREATE, width, height);
}

void XcursorImageDestroy(XcursorImage* image) {
	CALL_1(X11_CURSOR_IMAGE_DESTROY, image);
}

Cursor XcursorImageLoadCursor(Display* dpy, const XcursorImage* image) {
	CALL_2_R(X11_CURSOR_IMAGE_LOAD_CURSOR, dpy, image);
}

XcursorImages* XcursorImagesCreate(int size) {
	CALL_1_R(X11_CURSOR_IMAGES_CREATE, size);
}

void XcursorImagesDestroy(XcursorImages* images) {
	CALL_1(X11_CURSOR_IMAGES_DESTROY, images);
}

Cursor XcursorImagesLoadCursor(Display* dpy, const XcursorImages* images) {
	CALL_2_R(X11_CURSOR_IMAGES_LOAD_CURSOR, dpy, images);
}

Cursor XcursorLibraryLoadCursor(Display* dpy, const char* file) {
	CALL_2_R(X11_CURSOR_LIBRARY_LOAD_CURSOR, dpy, file);
}