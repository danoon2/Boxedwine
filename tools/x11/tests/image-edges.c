/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
/* Run this 32-bit client inside Boxedwine with the shipped filesystem Xlib.
 * Its XGetImage wrapper dereferences the native result; a native-only bounds
 * test misses the crash caused by returning NULL at a drawable edge.
 * Boxedwine preserves the requested rectangle and clears offscreen pixels.
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    Display *display = XOpenDisplay(NULL);
    if (!display) return 1;
    Pixmap pixmap = XCreatePixmap(display, DefaultRootWindow(display), 4, 2, DefaultDepth(display, DefaultScreen(display)));
    GC gc = XCreateGC(display, pixmap, 0, NULL);
    XImage *source = XGetImage(display, pixmap, 0, 0, 4, 2, AllPlanes, ZPixmap);
    if (!source) return 1;
    for (int y = 0; y < 2; ++y) {
        for (int x = 0; x < 4; ++x) {
            unsigned long value = 7;
            memcpy(source->data + y * source->bytes_per_line + x * (source->bits_per_pixel / 8),
                   &value, source->bits_per_pixel / 8);
        }
    }
    XPutImage(display, pixmap, gc, source, 0, 0, 0, 0, 4, 2);
    XDestroyImage(source);
    const int rectangles[][4] = {{-1, 0, 2, 2}, {3, 0, 2, 2},
        {0, -1, 4, 2}, {0, 1, 4, 2}, {-1, -1, 6, 4}, {4, 2, 1, 1}};
    for (unsigned int i = 0; i < sizeof(rectangles) / sizeof(rectangles[0]); ++i) {
        const int *r = rectangles[i];
        printf("Capturing edge %u\n", i); fflush(stdout);
        XImage *image = XGetImage(display, pixmap, r[0], r[1], r[2], r[3], AllPlanes, ZPixmap);
        if (!image || image->width != r[2] || image->height != r[3]) return 2;
        for (int y = 0; y < r[3]; ++y) {
            for (int x = 0; x < r[2]; ++x) {
                int sx = r[0] + x, sy = r[1] + y;
                unsigned long expected = sx >= 0 && sx < 4 && sy >= 0 && sy < 2 ? 7 : 0;
                unsigned long actual = 0;
                memcpy(&actual, image->data + y * image->bytes_per_line + x * (image->bits_per_pixel / 8),
                       image->bits_per_pixel / 8);
                if (actual != expected) return 3;
            }
        }
        XDestroyImage(image);
    }
    XFreeGC(display, gc);
    XFreePixmap(display, pixmap);
    XCloseDisplay(display);
    puts("PASS: all four edges, corner, and offscreen image capture");
    return 0;
}
