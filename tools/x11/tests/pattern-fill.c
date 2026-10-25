/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
/* Compare GC brush semantics on Xvfb and through Boxedwine's shipped Xlib.
 * XCreateGC uses mask=0 because that is the ABI supported by the guest shim. */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <string.h>

static int checks, failures;
static unsigned long get_pixel(XImage *image, int x, int y)
{
    unsigned long value = 0;
    if (image->bits_per_pixel == 1) return (image->data[y * image->bytes_per_line + x / 8] >> (x & 7)) & 1;
    memcpy(&value, image->data + y * image->bytes_per_line + x * (image->bits_per_pixel / 8), image->bits_per_pixel / 8);
    return value;
}

static void check(Display *display, Pixmap target, int style, int clipped, int masked)
{
    XImage *image = XGetImage(display, target, 0, 0, 7, 5, AllPlanes, ZPixmap);
    if (!image) { failures++; return; }
    for (int y = 0; y < 5; y++) for (int x = 0; x < 7; x++) {
        int tx = (x + 2) % 3, ty = (y + 3) % 2;
        unsigned long source = style == FillTiled ? 1 + tx + ty * 3 : 9;
        if (style == FillOpaqueStippled && (tx + ty) % 2) source = 12;
        int visible = (!clipped || (x >= 1 && x < 5 && y >= 2 && y < 4));
        if (style == FillStippled && (tx + ty) % 2) visible = 0;
        unsigned long expected = !visible ? 0x5a : masked ? ((0x5a & ~0x3c) | ((source ^ 0x5a) & 0x3c)) : source;
        unsigned long actual = get_pixel(image, x, y);
        checks++;
        if (actual != expected) {
            if (failures < 8) printf("FAIL style=%d clip=%d mask=%d (%d,%d) got=%lx expected=%lx\n", style, clipped, masked, x, y, actual, expected);
            failures++;
        }
    }
    XDestroyImage(image);
}

int main(void)
{
    setbuf(stdout, NULL);
    Display *display = XOpenDisplay(NULL);
    if (!display) return 1;
    Window root = DefaultRootWindow(display);
    int depth = DefaultDepth(display, DefaultScreen(display));
    Pixmap target = XCreatePixmap(display, root, 7, 5, depth);
    Pixmap tile = XCreatePixmap(display, root, 3, 2, depth);
    char pattern[] = {5, 2};
    Pixmap stipple = XCreateBitmapFromData(display, root, pattern, 3, 2);
    GC gc = XCreateGC(display, target, 0, NULL);
    GC clear = XCreateGC(display, target, 0, NULL);
    for (int y = 0; y < 2; y++) for (int x = 0; x < 3; x++) {
        XSetForeground(display, clear, 1 + x + y * 3);
        XFillRectangle(display, tile, clear, x, y, 1, 1);
    }
    XGCValues values;
    memset(&values, 0, sizeof(values));
    values.tile = tile; values.stipple = stipple;
    values.ts_x_origin = 4; values.ts_y_origin = -3;
    values.foreground = 9; values.background = 12;
    XChangeGC(display, gc, GCTile | GCStipple | GCTileStipXOrigin | GCTileStipYOrigin | GCForeground | GCBackground, &values);
    /* GCs retain these resources after FreePixmap. */
    XFreePixmap(display, tile); XFreePixmap(display, stipple);
    XSetForeground(display, clear, 0x5a);
    for (int style = FillSolid; style <= FillOpaqueStippled; style++) {
        for (int clipped = 0; clipped < 2; clipped++) for (int masked = 0; masked < 2; masked++) {
            XFillRectangle(display, target, clear, 0, 0, 7, 5);
            values.fill_style = style;
            values.function = masked ? GXxor : GXcopy;
            values.plane_mask = masked ? 0x3c : AllPlanes;
            XChangeGC(display, gc, GCFillStyle | GCFunction | GCPlaneMask, &values);
            XRectangle clip = {0, 0, 4, 2};
            if (clipped) XSetClipRectangles(display, gc, 1, 2, &clip, 1, Unsorted);
            else XSetClipMask(display, gc, None);
            printf("FILL depth=%d style=%d clip=%d mask=%d\n", depth, style, clipped, masked);
            XFillRectangle(display, target, gc, -2, -1, 12, 8);
            check(display, target, style, clipped, masked);
        }
    }
    XFreeGC(display, clear); XFreeGC(display, gc);
    XFreePixmap(display, target); XCloseDisplay(display);
    printf("RESULT checks=%d failures=%d\n", checks, failures);
    return failures != 0;
}
