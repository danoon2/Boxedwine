/* Exercise real keyboard delivery, including focus reversion used by AoE and
 * AbiWord. See README.md for the Xvfb and Boxedwine playback commands. */
#include <X11/Xlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static Display *display;
static Window root, parent, child, other, marker;
static int checks, failures, error_code;

static int on_error(Display *d, XErrorEvent *event)
{
    (void)d;
    error_code = event->error_code;
    return 0;
}

static Bool any_event(Display *d, XEvent *event, XPointer arg)
{
    (void)d; (void)event; (void)arg;
    return True;
}

static Window make_window(Window owner, int x, int y, int w, int h)
{
    XSetWindowAttributes attr = {0};
    Window window;
    attr.override_redirect = True;
    window = XCreateWindow(display, owner, x, y, w, h, 0, CopyFromParent,
                           InputOutput, CopyFromParent, CWOverrideRedirect, &attr);
    XMapWindow(display, window);
    return window;
}

int main(int argc, char **argv)
{
    static const char *names[] = {
        "initial PointerRoot focus", "hidden request preserves initial focus",
        "explicit root focus", "pointer below focus", "propagate to focus",
        "pointer outside focus", "stop at focus", "None discards keys",
        "PointerRoot focus", "press and release select independently",
        "revert to parent", "revert to explicit root on unmap",
        "revert to explicit root on destroy"
    };
    int reference = argc > 1 && !strcmp(argv[1], "reference");
    int (*fake_key)(Display *, unsigned int, Bool, unsigned long) = NULL;
    int (*fake_motion)(Display *, int, int, int, unsigned long) = NULL;
    int (*fake_button)(Display *, unsigned int, Bool, unsigned long) = NULL;
    GC gc;
    display = XOpenDisplay(NULL);
    if (!display) return 2;
    XSetErrorHandler(on_error);
    if (reference) {
        void *lib = dlopen("libXtst.so.6", RTLD_NOW);
        if (!lib) { fprintf(stderr, "%s\n", dlerror()); return 2; }
        fake_key = dlsym(lib, "XTestFakeKeyEvent");
        fake_motion = dlsym(lib, "XTestFakeMotionEvent");
        fake_button = dlsym(lib, "XTestFakeButtonEvent");
        if (!fake_key || !fake_motion || !fake_button) return 2;
    }
    root = DefaultRootWindow(display);
    parent = make_window(root, 80, 80, 160, 160);
    child = make_window(parent, 10, 10, 80, 80);
    other = make_window(root, 300, 100, 80, 80);
    marker = make_window(root, 600, 10, 16, 16);
    XSelectInput(display, marker, ButtonReleaseMask);
    gc = XCreateGC(display, marker, 0, NULL);
    for (int i = -2; i < 11; ++i) {
        Window expected = child, expected_release_window = None, focus = parent;
        int expected_press = 1, expected_release = 1, presses = 0, releases = 0, pass = 1;
        int focus_result = 0;
        XEvent event;
        error_code = 0;
        XMapWindow(display, child);
        XMapWindow(display, other);
        XSelectInput(display, root, 0);
        XSelectInput(display, parent, KeyPressMask | KeyReleaseMask);
        XSelectInput(display, child, KeyPressMask | KeyReleaseMask);
        XSelectInput(display, other, KeyPressMask | KeyReleaseMask);
        switch (i) {
        case 0: focus = root; break;
        case 2:
            XSelectInput(display, child, 0); expected = parent; break;
        case 3: focus = expected = other; break;
        case 4:
            focus = child;
            XSelectInput(display, child, 0);
            expected_press = expected_release = 0; break;
        case 5: focus = None; expected_press = expected_release = 0; break;
        case 6: focus = PointerRoot; break;
        case 7:
            XSelectInput(display, child, KeyReleaseMask);
            expected = parent; expected_release_window = child; break;
        case 8: focus = child; expected = parent; break;
        case 9: case 10: focus = other; break;
        }
        if (i == -1) {
            XUnmapWindow(display, other);
            focus = other;
        }
        if (i != -2) focus_result = XSetInputFocus(display, focus, RevertToParent, CurrentTime);
        if (i == 8) XUnmapWindow(display, child);
        if (i == 9) XUnmapWindow(display, other);
        if (i == 10) XDestroyWindow(display, other);
        XSync(display, False);
        /* Boxedwine's Xlib bridge returns errors directly. */
        if (i == -1) {
            if (error_code != BadMatch && focus_result != BadMatch) pass = 0;
        } else if (error_code) pass = 0;
        if (i < 0) {
            Window actual;
            int revert;
            XGetInputFocus(display, &actual, &revert);
            if (actual != PointerRoot) pass = 0;
        }
        while (XCheckIfEvent(display, &event, any_event, NULL)) {}
        /* The recording waits for this marker before injecting each key pair.
         * A subsequent pointer-button release marks the end even for cases
         * where all keyboard events must be discarded, without timed handoffs. */
        XSetForeground(display, gc, 0x100000UL * (i + 3));
        XFillRectangle(display, marker, gc, 0, 0, 16, 16);
        XFlush(display);
        if (reference) {
            unsigned int key = XKeysymToKeycode(display, 'b');
            fake_motion(display, -1, 120, 120, 0);
            fake_key(display, key, True, 0);
            fake_key(display, key, False, 0);
            fake_motion(display, -1, 605, 15, 0);
            fake_button(display, 1, True, 0);
            fake_button(display, 1, False, 0);
            XFlush(display);
        }
        for (;;) {
            while (!XCheckIfEvent(display, &event, any_event, NULL)) usleep(1000);
            if (event.type == ButtonRelease && event.xbutton.window == marker) break;
            if (event.type != KeyPress && event.type != KeyRelease) continue;
            if (event.type == KeyPress) ++presses; else ++releases;
            Window target = event.type == KeyRelease && expected_release_window ? expected_release_window : expected;
            if (event.xkey.window != target || event.xkey.x_root != 120 || event.xkey.y_root != 120)
                pass = 0;
        }
        pass = pass && presses == expected_press && releases == expected_release;
        printf("%s %s: press=%d release=%d expected=%d/%d\n", pass ? "PASS" : "FAIL",
               names[i + 2], presses, releases, expected_press, expected_release);
        fflush(stdout);
        ++checks;
        if (!pass) ++failures;
    }
    printf("RESULT checks=%d failures=%d\n", checks, failures);
    XCloseDisplay(display);
    return failures != 0;
}
