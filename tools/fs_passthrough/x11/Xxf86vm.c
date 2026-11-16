/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
#include <X11/Xlib.h>
#include <X11/extensions/xf86vmode.h>
#include "X11_def.h"

Bool XF86VidModeQueryVersion(Display* dpy, int* major, int* minor) {
    int opcode, event, error;
    /* Probe through the old ABI first: newer filesystems remain usable with
     * older engines, which simply report this extension as unavailable. */
    if (!XQueryExtension(dpy, "XFree86-VidModeExtension", &opcode, &event, &error)) return False;
    CALL_3_R(X11_VM_QUERY_VERSION, dpy, major, minor);
}

Bool XF86VidModeQueryExtension(Display* dpy, int* event, int* error) {
    int major, minor;
    if (!XF86VidModeQueryVersion(dpy, &major, &minor)) return False;
    *event = *error = 0;
    return True;
}

Bool XF86VidModeGetGammaRampSize(Display* dpy, int screen, int* size) {
    int major, minor;
    if (screen || !size || !XF86VidModeQueryVersion(dpy, &major, &minor)) return False;
    *size = 256;
    return True;
}

Bool XF86VidModeGetGammaRamp(Display* dpy, int screen, int size,
        unsigned short* red, unsigned short* green, unsigned short* blue) {
    CALL_6_R(X11_VM_GET_GAMMA_RAMP, dpy, screen, size, red, green, blue);
}

Bool XF86VidModeSetGammaRamp(Display* dpy, int screen, int size,
        unsigned short* red, unsigned short* green, unsigned short* blue) {
    CALL_6_R(X11_VM_SET_GAMMA_RAMP, dpy, screen, size, red, green, blue);
}

/* Display modes remain owned by the existing XRandR implementation. Wine
 * resolves these symbols together with gamma, but selects XRandR for modes. */
Bool XF86VidModeGetAllModeLines(Display* dpy, int screen, int* count, XF86VidModeModeInfo*** modes) {
    *count = 0; *modes = NULL;
    return False;
}
Bool XF86VidModeGetModeLine(Display* dpy, int screen, int* dotclock, XF86VidModeModeLine* mode) { return False; }
Bool XF86VidModeSwitchToMode(Display* dpy, int screen, XF86VidModeModeInfo* mode) { return False; }
Bool XF86VidModeLockModeSwitch(Display* dpy, int screen, int lock) { return False; }
Bool XF86VidModeSetViewPort(Display* dpy, int screen, int x, int y) { return False; }
/* Clients use the precise ramp API. Scalar gamma cannot represent arbitrary ramps. */
Bool XF86VidModeGetGamma(Display* dpy, int screen, XF86VidModeGamma* gamma) { return False; }
Bool XF86VidModeSetGamma(Display* dpy, int screen, XF86VidModeGamma* gamma) { return False; }
