/* Build: gcc -m32 -O2 -Wall -Wextra -Werror window-attributes.c -lX11 -o window-attributes */
#include <X11/Xlib.h>
#include <stdio.h>
#include <string.h>

static unsigned int checks, failures;

#define CHECK(condition) do { ++checks; if (!(condition)) { \
    ++failures; printf("FAIL line %d: %s\n", __LINE__, #condition); } } while (0)

static int ignore_error(Display *display, XErrorEvent *event)
{
    (void)display;
    (void)event;
    return 0;
}

static void check_state(Display *display, Window window, int expected)
{
    XWindowAttributes attr;
    memset(&attr, 0xcc, sizeof(attr));
    CHECK(XGetWindowAttributes(display, window, &attr));
    CHECK(attr.map_state == expected);
}

int main(void)
{
    Display *first = XOpenDisplay(NULL), *second = XOpenDisplay(NULL);
    Window parent, child, input;
    XWindowAttributes attr, other;
    XSetWindowAttributes set = {0};
    unsigned int i;
    if (!first || !second) return 2;
    XSetErrorHandler(ignore_error);
    parent = XCreateWindow(first, DefaultRootWindow(first), 20, 30, 160, 120, 0,
                          DefaultDepth(first, DefaultScreen(first)), InputOutput, CopyFromParent, 0, NULL);
    child = XCreateWindow(first, parent, -7, 11, 60, 40, 2,
                         DefaultDepth(first, DefaultScreen(first)), InputOutput, CopyFromParent, 0, NULL);
    set.override_redirect = True;
    set.bit_gravity = SouthEastGravity;
    set.win_gravity = NorthEastGravity;
    set.do_not_propagate_mask = KeyPressMask;
    XChangeWindowAttributes(first, child, CWOverrideRedirect | CWBitGravity | CWWinGravity | CWDontPropagate, &set);
    XSelectInput(first, child, StructureNotifyMask);
    XSync(first, False);
    XSelectInput(second, child, ExposureMask);
    XSync(second, False);
    CHECK(XGetWindowAttributes(first, child, &attr));
    CHECK(XGetWindowAttributes(second, child, &other));
    CHECK(attr.x == -7 && attr.y == 11 && attr.width == 60 && attr.height == 40);
    CHECK(attr.border_width == 2 && attr.depth == DefaultDepth(first, DefaultScreen(first)));
    CHECK(attr.root == DefaultRootWindow(first) && attr.class == InputOutput);
    CHECK(attr.visual == DefaultVisual(first, DefaultScreen(first)));
    CHECK(other.visual == DefaultVisual(second, DefaultScreen(second)));
    CHECK(attr.screen == DefaultScreenOfDisplay(first));
    CHECK(other.screen == DefaultScreenOfDisplay(second));
    CHECK(attr.colormap == DefaultColormap(first, DefaultScreen(first)));
    CHECK(attr.bit_gravity == SouthEastGravity && attr.win_gravity == NorthEastGravity);
    CHECK(attr.do_not_propagate_mask == KeyPressMask && attr.override_redirect);
    CHECK(attr.your_event_mask == StructureNotifyMask && other.your_event_mask == ExposureMask);
    CHECK(attr.all_event_masks == (StructureNotifyMask | ExposureMask));
    CHECK(other.all_event_masks == attr.all_event_masks);
    check_state(first, child, IsUnmapped);
    XMapWindow(first, child);
    check_state(first, child, IsUnviewable);
    XMapWindow(first, parent);
    XSync(first, False);
    check_state(second, child, IsViewable);
    XUnmapWindow(first, parent);
    XSync(first, False);
    check_state(second, child, IsUnviewable);
    XMapWindow(first, parent);
    for (i = 0; i < 8; ++i)
    {
        XUnmapWindow(first, child);
        check_state(first, child, IsUnmapped);
        XMapWindow(first, child);
        /* Leave old UnmapNotify events queued, as Wine's capture helper does. */
        XSync(first, False);
        check_state(second, child, IsViewable);
    }
    input = XCreateWindow(first, parent, 1, 2, 10, 20, 0, 0, InputOnly, CopyFromParent, 0, NULL);
    XMapWindow(first, input);
    CHECK(XGetWindowAttributes(first, input, &attr));
    CHECK(attr.class == InputOnly && attr.depth == 0 && attr.map_state == IsViewable);
    XDestroyWindow(first, child);
    CHECK(!XGetWindowAttributes(first, child, &attr));
    XDestroyWindow(first, parent);
    XCloseDisplay(second);
    XCloseDisplay(first);
    printf("WINDOW_ATTRIBUTES checks=%u failures=%u\n", checks, failures);
    return failures ? 1 : 0;
}
