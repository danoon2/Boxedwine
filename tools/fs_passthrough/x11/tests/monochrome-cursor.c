/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
/* Wine requires a depth-1 pixmap format even on a 32-bit desktop before
 * uploading a monochrome cursor. Run this same client on Xvfb and Boxedwine. */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    Display *display = XOpenDisplay(NULL);
    XPixmapFormatValues *formats;
    XImage *image, *readback;
    XColor fg = {0}, bg = {0};
    Pixmap source, mask;
    Cursor cursor;
    GC gc;
    int count, found = 0, failures = 0;
    const unsigned width = 19, height = 7;
    if (!display) return 2;
    formats = XListPixmapFormats(display, &count);
    for (int i = 0; i < count; ++i) if (formats[i].depth == 1) {
        ++found;
        if (formats[i].bits_per_pixel != 1) ++failures;
    }
    if (found != 1) { printf("FAIL depth-1 formats=%d\n", found); ++failures; }
    XFree(formats);
    image = XCreateImage(display, DefaultVisual(display, DefaultScreen(display)),
        1, ZPixmap, 0, NULL, width, height, 32, 0);
    if (!image || image->bits_per_pixel != 1 || image->bytes_per_line != 4) return 3;
    image->data = calloc(height, image->bytes_per_line);
    for (unsigned y = 0; y < height; ++y)
        for (unsigned x = 0; x < width; ++x)
            if ((x + y) % 3 == 0) image->data[y * image->bytes_per_line + x / 8] |= 1 << (x % 8);
    source = XCreatePixmap(display, DefaultRootWindow(display), width, height, 1);
    mask = XCreatePixmap(display, DefaultRootWindow(display), width, height, 1);
    gc = XCreateGC(display, source, 0, NULL);
    XPutImage(display, source, gc, image, 0, 0, 0, 0, width, height);
    readback = XGetImage(display, source, 0, 0, width, height, AllPlanes, ZPixmap);
    if (!readback || readback->bits_per_pixel != 1) return 4;
    for (unsigned y = 0; y < height; ++y)
        for (unsigned x = 0; x < width; ++x)
            if (((readback->data[y * readback->bytes_per_line + x / 8] >> (x % 8)) & 1)
                != ((x + y) % 3 == 0)) ++failures;
    XDestroyImage(readback);
    XSetForeground(display, gc, 1);
    XFillRectangle(display, mask, gc, 0, 0, width, height);
    fg.red = fg.green = fg.blue = 0xffff;
    cursor = XCreatePixmapCursor(display, source, mask, &fg, &bg, 2, 3);
    if (!cursor) ++failures;
    XDefineCursor(display, DefaultRootWindow(display), cursor);
    XSync(display, False);
    XFreeCursor(display, cursor);
    XFreeGC(display, gc);
    XFreePixmap(display, source);
    XFreePixmap(display, mask);
    XDestroyImage(image);
    XCloseDisplay(display);
    printf("MONOCHROME_CURSOR failures=%d\n", failures);
    return failures ? 1 : 0;
}
