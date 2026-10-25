/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
/* Exercise Wine's software-cursor path through the shipped guest Xlib:
 * read the root window, draw over it with IncludeInferiors, then restore.
 * Run at both -bpp 8 and -bpp 32. */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <string.h>

static unsigned long pixel(XImage *image, int x, int y)
{
    unsigned long value = 0;
    memcpy(&value, image->data + y * image->bytes_per_line + x * (image->bits_per_pixel / 8), image->bits_per_pixel / 8);
    return value;
}

int main(void)
{
    Display *display = XOpenDisplay(NULL);
    if (!display) return 1;
    Window root = DefaultRootWindow(display);
    Window game = XCreateWindow(display, root, 10, 10, 80, 60, 0,
        DefaultDepth(display, DefaultScreen(display)), InputOutput,
        DefaultVisual(display, DefaultScreen(display)), 0, NULL);
    GC gc = XCreateGC(display, root, 0, NULL);
    XMapWindow(display, game);
    XImage *scene = XGetImage(display, game, 0, 0, 80, 60, AllPlanes, ZPixmap);
    for (int y = 0; y < 60; ++y) for (int x = 0; x < 80; ++x) {
        unsigned long color = 1 + (x + y * 3) % 200;
        memcpy(scene->data + y * scene->bytes_per_line + x * (scene->bits_per_pixel / 8), &color, scene->bits_per_pixel / 8);
    }
    XPutImage(display, game, gc, scene, 0, 0, 0, 0, 80, 60);
    XSetSubwindowMode(display, gc, IncludeInferiors);
    for (int offset = 0; offset < 3; ++offset) {
        int x = 15 + offset * 12, y = 15;
        XImage *saved = XGetImage(display, root, x, y, 17, 16, AllPlanes, ZPixmap);
        for (int sy = 0; sy < 16; ++sy) for (int sx = 0; sx < 17; ++sx) {
            if (pixel(saved, sx, sy) != pixel(scene, x - 10 + sx, y - 10 + sy)) {
                puts("FAIL: root read lost visible game pixels");
                return 2;
            }
        }
        XImage *cursor = XGetImage(display, root, x, y, 17, 16, AllPlanes, ZPixmap);
        memset(cursor->data, 0, cursor->bytes_per_line * cursor->height);
        XPutImage(display, root, gc, cursor, 0, 0, x, y, 17, 16);
        XPutImage(display, root, gc, saved, 0, 0, x, y, 17, 16);
        XDestroyImage(cursor);
        XDestroyImage(saved);
        XImage *after = XGetImage(display, game, 0, 0, 80, 60, AllPlanes, ZPixmap);
        for (int sy = 0; sy < 60; ++sy) for (int sx = 0; sx < 80; ++sx) {
            if (pixel(after, sx, sy) != pixel(scene, sx, sy)) {
                puts("FAIL: cursor movement damaged the background");
                return 3;
            }
        }
        XDestroyImage(after);
    }
    XDestroyImage(scene);
    XFreeGC(display, gc);
    XDestroyWindow(display, game);
    XCloseDisplay(display);
    puts("PASS: root readback and repeated cursor save/restore");
    return 0;
}
