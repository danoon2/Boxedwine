/* Check pointer hit testing for owned popups without moving the host pointer.
 * Build: gcc -m32 -O2 -Wall pointer.c -lX11 -o pointer-probe
 * Run on an isolated X server (e.g. Xvfb), or inside Boxedwine.
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>

static Display *display;
static Window root;
static int checks, failures;

static Window make_window(Window parent, int x, int y, int width, int height)
{
    XSetWindowAttributes attr = {0};
    Window window;
    attr.override_redirect = True;
    window = XCreateWindow(display, parent, x, y, width, height, 0, CopyFromParent,
                           InputOutput, CopyFromParent, CWOverrideRedirect, &attr);
    XMapWindow(display, window);
    return window;
}

static void check_in_window(const char *name, Window destination, int x, int y, Window expected)
{
    Window child = None;
    int tx, ty, pass;
    XSync(display, False);
    XTranslateCoordinates(display, root, destination, x, y, &tx, &ty, &child);
    pass = child == expected;
    printf("%s %s: hit=%lx expected=%lx point=%d,%d\n", pass ? "PASS" : "FAIL", name, child, expected, x, y);
    checks++;
    if (!pass) failures++;
}

static void check(const char *name, int x, int y, Window expected)
{
    check_in_window(name, root, x, y, expected);
}

int main(void)
{
    Window background, owner, popup, nested, clipped, popup_child;
    setvbuf(stdout, NULL, _IONBF, 0);
    display = XOpenDisplay(NULL);
    if (!display) return 2;
    root = DefaultRootWindow(display);
    background = make_window(root, 0, 0, 700, 500);
    owner = make_window(root, 100, 100, 160, 160);
    popup = make_window(root, 350, 100, 80, 60);
    XSetTransientForHint(display, popup, owner);
    check("owner outside popup", 120, 120, owner);
    check("background outside owner and popup", 550, 350, background);
    check("popup outside owner", 360, 110, popup);
    check("popup left edge", 350, 110, popup);
    check("popup excludes right edge", 430, 110, background);
    check("popup excludes bottom edge", 360, 160, background);
    nested = make_window(root, 480, 220, 60, 40);
    XSetTransientForHint(display, nested, popup);
    check("nested popup outside both owners", 490, 230, nested);
    check("owner outside nested popup", 120, 120, owner);
    check("first popup outside nested popup", 360, 110, popup);
    check("background outside nested popup", 550, 350, background);
    XUnmapWindow(display, nested);
    check("unmapped popup does not intercept", 490, 230, background);
    XMapWindow(display, nested);
    XDestroyWindow(display, nested);
    check("destroyed popup does not intercept", 490, 230, background);
    clipped = make_window(owner, 150, 100, 60, 40);
    check("ordinary child clipped by parent", 280, 220, background);
    popup_child = make_window(popup, 5, 5, 20, 20);
    check_in_window("child of popup outside owner", popup, 360, 110, popup_child);
    XDestroyWindow(display, popup_child);
    XDestroyWindow(display, clipped);
    XMoveResizeWindow(display, popup, 110, 110, 80, 60);
    check("overlapping popup above owner", 120, 120, popup);
    check("owner beside overlapping popup", 240, 240, owner);
    XUnmapWindow(display, popup);
    check("owner after hiding popup", 120, 120, owner);
    printf("RESULT checks=%d failures=%d\n", checks, failures);
    XCloseDisplay(display);
    return failures ? 1 : 0;
}
