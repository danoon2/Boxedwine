/* Compare first-map and remap backgrounds against an ordinary X server. */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static unsigned long pixel(Display *display, Window window)
{
    XImage *image = XGetImage(display, window, 5, 5, 1, 1, AllPlanes, ZPixmap);
    uint32_t value = 0;
    /* Read the image directly: the Boxedwine bridge lacks XGetPixel callbacks. */
    if (!image || image->bits_per_pixel != 32 || image->byte_order != LSBFirst) {
        if (image) XDestroyImage(image);
        return ~0UL;
    }
    memcpy(&value, image->data, sizeof(value));
    XDestroyImage(image);
    return value & 0xffffff;
}

int main(void)
{
    Display *display = XOpenDisplay(NULL);
    if (!display) return 2;
    int screen = DefaultScreen(display), failures = 0;
    unsigned long background = 0x123456, previous = 0xabcdef;
    XSetWindowAttributes attributes = {0};
    attributes.background_pixel = background;
    attributes.backing_store = NotUseful;
    Window window = XCreateWindow(display, RootWindow(display, screen), 30, 30,
        100, 80, 0, CopyFromParent, InputOutput, CopyFromParent,
        CWBackPixel | CWBackingStore, &attributes);
    GC gc = XCreateGC(display, window, 0, NULL);
    XMapWindow(display, window);
    XSync(display, False);
    unsigned long first = pixel(display, window);
    failures += first != background;

    XSetForeground(display, gc, previous);
    XFillRectangle(display, window, gc, 0, 0, 100, 80);
    XMapWindow(display, window); /* Already mapped: preserve the client's paint. */
    XSync(display, False);
    unsigned long unchanged = pixel(display, window);
    failures += unchanged != previous;

    XUnmapWindow(display, window);
    XSync(display, False);
    XMapWindow(display, window);
    XSync(display, False);
    unsigned long remap = pixel(display, window);
    failures += remap != background;

    XUnmapWindow(display, window);
    attributes.background_pixel = 0x345678;
    XChangeWindowAttributes(display, window, CWBackPixel, &attributes);
    XMapWindow(display, window);
    XSync(display, False);
    unsigned long changed = pixel(display, window);
    failures += changed != 0x345678;
    printf("first=%06lx unchanged=%06lx remap=%06lx changed=%06lx\n",
        first, unchanged, remap, changed);
    printf("MAP_BACKGROUND checks=4 failures=%d\n", failures);
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return failures != 0;
}
