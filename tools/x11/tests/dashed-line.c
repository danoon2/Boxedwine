/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    Display *d = XOpenDisplay(NULL);
    if (!d) return 1;
    Pixmap target = XCreatePixmap(d, DefaultRootWindow(d), 19, 19, DefaultDepth(d, DefaultScreen(d)));
    GC gc = XCreateGC(d, target, 0, NULL), clear = XCreateGC(d, target, 0, NULL);
    XGCValues v = {0}; v.foreground = 0x96; v.background = 0x35; v.function = GXxor; v.plane_mask = 0x3c;
    XChangeGC(d, gc, GCForeground | GCBackground | GCFunction | GCPlaneMask, &v);
    XSetForeground(d, clear, 0x5a);
    const char patterns[2][6] = {{3,2,1,3,2,1}, {2,5,2,5,2,5}};
    int checks = 0, failures = 0;
    for (int pattern = 0; pattern < 2; pattern++) for (int offset = 0; offset < 3; offset++)
    for (int style = LineOnOffDash; style <= LineDoubleDash; style++)
    for (int vertical = 0; vertical < 2; vertical++) for (int reverse = 0; reverse < 2; reverse++)
    for (int clipped = 0; clipped < 2; clipped++) for (int cap = 0; cap < 2; cap++) {
        int total = pattern ? 7 : 12;
        XSetDashes(d, gc, offset * 9, patterns[pattern], pattern ? 2 : 3);
        v.line_style = style; v.cap_style = cap ? CapButt : CapNotLast;
        XChangeGC(d, gc, GCLineStyle | GCCapStyle, &v);
        XRectangle clip = {0,0,10,10};
        if (clipped) XSetClipRectangles(d, gc, 3, 4, &clip, 1, Unsorted);
        else XSetClipMask(d, gc, None);
        XFillRectangle(d, target, clear, 0, 0, 19, 19);
        int start = reverse ? 16 : -3, end = reverse ? -3 : 16;
        XDrawLine(d, target, gc, vertical ? 8 : start, vertical ? start : 8, vertical ? 8 : end, vertical ? end : 8);
        XImage *image = XGetImage(d, target, 0, 0, 19, 19, AllPlanes, ZPixmap);
        if (!image) return 2;
        for (int y = 0; y < 19; y++) for (int x = 0; x < 19; x++) {
            int coordinate = vertical ? y : x;
            int visible = (vertical ? x == 8 : y == 8) && coordinate <= 16;
            visible &= !clipped || (x >= 3 && x < 13 && y >= 4 && y < 14);
            visible &= cap || coordinate != end;
            int phase = ((reverse ? start - coordinate : coordinate - start) + offset * 9 + 100 * total) % total;
            int index = 0;
            while (phase >= patterns[pattern][index]) phase -= patterns[pattern][index++];
            unsigned long source = index & 1 ? 0x35 : 0x96;
            if ((index & 1) && style == LineOnOffDash) visible = 0;
            unsigned long expected = visible ? ((0x5a & ~0x3c) | ((source ^ 0x5a) & 0x3c)) : 0x5a;
            unsigned long pixel = 0;
            memcpy(&pixel, image->data + y * image->bytes_per_line + x * (image->bits_per_pixel / 8), image->bits_per_pixel / 8);
            checks++;
            if (pixel != expected) {
                if (failures < 8) printf("FAIL p=%d off=%d style=%d vert=%d rev=%d clip=%d cap=%d xy=%d,%d got=%lx expected=%lx\n", pattern, offset, style, vertical, reverse, clipped, cap, x, y, pixel, expected);
                failures++;
            }
        }
        XDestroyImage(image);
    }
    XFreeGC(d, gc); XFreeGC(d, clear); XFreePixmap(d, target); XCloseDisplay(d);
    printf("RESULT checks=%d failures=%d\n", checks, failures);
    return failures != 0;
}
