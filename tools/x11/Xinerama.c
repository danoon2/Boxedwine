#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>

#include "X11_def.h"

Bool XineramaQueryExtension(Display* dpy, int* event_base_return, int* error_base_return) {
	return True;
}

XineramaScreenInfo* XineramaQueryScreens(Display* dpy, int* number) {
	CALL_2_R(X11_XINERAMA_QUERY_SCREENS, dpy, number);
}