/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
/* Run on Xvfb and Boxedwine; compare every CASE checksum. */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const struct { int count, shape; XPoint points[12]; } cases[] = {
    {3, Convex, {{1,1}, {17,1}, {1,17}}},
    {8, Nonconvex, {{1,1},{17,1},{17,7},{10,7},{10,13},{17,13},{17,17},{1,17}}},
    {4, Complex, {{1,1},{17,17},{1,17},{17,1}}},
    {8, Complex, {{1,1},{17,1},{17,17},{1,17},{1,1},{17,1},{17,17},{1,17}}},
    {5, Complex, {{-10,-5},{20,3},{30,22},{8,12},{-4,21}}},
    {2, Complex, {{0,0},{20,20}}},
    {3, Convex, {{1,17},{17,1},{1,1}}},
    {6, Complex, {{1,1},{1,1},{17,1},{17,17},{1,17},{1,1}}},
    {4, Convex, {{2,-4},{5,18},{12,16},{22,1}}},
    {4, Complex, {{-32768,-32768},{32767,-32768},{32767,32767},{-32768,32767}}},
};

int main(void)
{
    setbuf(stdout, NULL);
    Display *d = XOpenDisplay(NULL);
    if (!d) return 1;
    Window root = DefaultRootWindow(d);
    int depth = DefaultDepth(d, DefaultScreen(d));
    Pixmap target = XCreatePixmap(d, root, 24, 20, depth);
    Pixmap tile = XCreatePixmap(d, root, 3, 2, depth);
    const char pattern[] = {5,2};
    Pixmap stipple = XCreateBitmapFromData(d, root, pattern, 3, 2);
    GC gc = XCreateGC(d, target, 0, NULL), clear = XCreateGC(d, target, 0, NULL);
    for (int y = 0; y < 2; y++) for (int x = 0; x < 3; x++) {
        XSetForeground(d, clear, 1 + x + y * 3);
        XFillRectangle(d, tile, clear, x, y, 1, 1);
    }
    XGCValues v = {0};
    v.foreground = 9; v.background = 12; v.tile = tile; v.stipple = stipple;
    v.ts_x_origin = -4; v.ts_y_origin = 3;
    XChangeGC(d, gc, GCForeground | GCBackground | GCTile | GCStipple | GCTileStipXOrigin | GCTileStipYOrigin, &v);
    XSetForeground(d, clear, 0x5a);
    int tests = 0;
    for (unsigned c = 0; c < sizeof(cases)/sizeof(cases[0]); c++)
    for (int rule = 0; rule < 2; rule++) for (int mode = 0; mode < 2; mode++)
    for (int style = 0; style < 4; style++) for (int clipped = 0; clipped < 2; clipped++)
    for (int masked = 0; masked < 2; masked++) {
        XPoint points[12];
        memcpy(points, cases[c].points, sizeof(points));
        if (mode == CoordModePrevious) for (int i = cases[c].count - 1; i > 0; i--) {
            points[i].x = (short)(points[i].x - points[i-1].x);
            points[i].y = (short)(points[i].y - points[i-1].y);
        }
        v.fill_rule = rule; v.fill_style = style;
        v.function = masked ? GXxor : GXcopy; v.plane_mask = masked ? 0x3c : AllPlanes;
        XChangeGC(d, gc, GCFillRule | GCFillStyle | GCFunction | GCPlaneMask, &v);
        XRectangle clip = {0,0,11,13};
        if (clipped) XSetClipRectangles(d, gc, 3, 2, &clip, 1, Unsorted);
        else XSetClipMask(d, gc, None);
        XFillRectangle(d, target, clear, 0, 0, 24, 20);
        XFillPolygon(d, target, gc, points, cases[c].count, cases[c].shape, mode);
        XImage *image = XGetImage(d, target, 0, 0, 24, 20, AllPlanes, ZPixmap);
        if (!image) return 2;
        uint32_t hash = 2166136261u;
        for (int y = 0; y < 20; y++) for (int x = 0; x < 24; x++) {
            unsigned long pixel = 0;
            memcpy(&pixel, image->data + y * image->bytes_per_line + x * (image->bits_per_pixel / 8), image->bits_per_pixel / 8);
            hash = (hash ^ (uint32_t)pixel) * 16777619u;
        }
        XDestroyImage(image);
        printf("CASE %u %d %d %d %d %d %08x\n", c, rule, mode, style, clipped, masked, hash);
        tests++;
    }
    printf("RESULT cases=%d\n", tests);
    XFreeGC(d, gc); XFreeGC(d, clear); XFreePixmap(d, tile); XFreePixmap(d, stipple);
    XFreePixmap(d, target); XCloseDisplay(d);
    return 0;
}
