/* Compare Boxedwine's X11 focus semantics with a real X server.
 * Build: gcc -m32 -O2 -Wall focus.c -lX11 -o focus-probe
 * Run on an isolated display (e.g. Xvfb), or inside Boxedwine.
 */
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <unistd.h>

static Display *display;
static int failures, checks, error_code;
static Window root, parent, child, sibling, hidden;

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

static void drain(void)
{
    XEvent event;
    XSync(display, False);
    while (XCheckIfEvent(display, &event, any_event, NULL)) {}
}

static void check_focus(const char *name, Window expected, int expected_revert)
{
    Window actual = 0;
    int revert = -1, pass;
    XGetInputFocus(display, &actual, &revert);
    pass = actual == expected && revert == expected_revert;
    printf("%s %s: focus=%lx expected=%lx revert=%d expected=%d\n",
           pass ? "PASS" : "FAIL", name, actual, expected, revert, expected_revert);
    checks++;
    if (!pass) failures++;
}

static void check_event(const char *name, Window window, int type, int detail)
{
    XEvent event;
    int found = 0;
    XSync(display, False);
    while (XCheckTypedWindowEvent(display, window, type, &event))
    {
        printf("EVENT %s: window=%lx type=%d mode=%d detail=%d\n", name,
               window, type, event.xfocus.mode, event.xfocus.detail);
        if (event.xfocus.mode == NotifyNormal && event.xfocus.detail == detail) found = 1;
    }
    printf("%s %s: expected type=%d detail=%d\n", found ? "PASS" : "FAIL", name, type, detail);
    checks++;
    if (!found) failures++;
}

static Window make_window(Window owner, int x, int y, int mapped)
{
    XSetWindowAttributes attr = {0};
    Window window;
    attr.override_redirect = True;
    attr.event_mask = FocusChangeMask | StructureNotifyMask | PropertyChangeMask;
    window = XCreateWindow(display, owner, x, y, 32, 32, 0, CopyFromParent,
                           InputOutput, CopyFromParent, CWOverrideRedirect | CWEventMask, &attr);
    if (mapped) XMapWindow(display, window);
    return window;
}

static Time timestamp(void)
{
    XEvent event;
    char *name = "BOXEDWINE_FOCUS_PROBE_TIME";
    Atom atom;
    unsigned char value = 1;
    XInternAtoms(display, &name, 1, False, &atom);
    drain();
    XChangeProperty(display, sibling, atom, XA_INTEGER, 8, PropModeReplace, &value, 1);
    XSync(display, False);
    if (XCheckTypedWindowEvent(display, sibling, PropertyNotify, &event)) return event.xproperty.time;
    fprintf(stderr, "Missing timestamp event\n");
    return 0;
}

struct focus_event { Window window; int type, detail; };

static void check_sequence(const char *name, const struct focus_event *expected, int count)
{
    XEvent event;
    int index = 0, pass = 1;
    XSync(display, False);
    while (XCheckIfEvent(display, &event, any_event, NULL))
    {
        int detail;
        if (event.type != FocusIn && event.type != FocusOut && event.type != UnmapNotify) continue;
        detail = event.type == UnmapNotify ? 0 : event.xfocus.detail;
        printf("SEQUENCE %s: window=%lx type=%d detail=%d\n", name, event.xany.window, event.type, detail);
        if (index >= count || event.xany.window != expected[index].window ||
            event.type != expected[index].type || detail != expected[index].detail ||
            (event.type != UnmapNotify && event.xfocus.mode != NotifyNormal)) pass = 0;
        index++;
    }
    pass = pass && index == count;
    printf("%s %s: events=%d expected=%d\n", pass ? "PASS" : "FAIL", name, index, count);
    checks++;
    if (!pass) failures++;
}

static void extended_checks(void)
{
    Window middle = make_window(parent, 0, 0, 1);
    Window leaf = make_window(middle, 0, 0, 1);
    Window peer = make_window(sibling, 0, 0, 1);
    Time old_time;
    struct focus_event to_child[] = {{parent, FocusOut, NotifyInferior},
        {middle, FocusIn, NotifyVirtual}, {leaf, FocusIn, NotifyAncestor}};
    struct focus_event to_parent[] = {{leaf, FocusOut, NotifyAncestor},
        {middle, FocusOut, NotifyVirtual}, {parent, FocusIn, NotifyInferior}};
    struct focus_event to_peer[] = {{leaf, FocusOut, NotifyNonlinear},
        {middle, FocusOut, NotifyNonlinearVirtual}, {parent, FocusOut, NotifyNonlinearVirtual},
        {sibling, FocusIn, NotifyNonlinearVirtual}, {peer, FocusIn, NotifyNonlinear}};
    struct focus_event unmapped[] = {{sibling, UnmapNotify, 0}, {peer, FocusOut, NotifyAncestor},
        {sibling, FocusOut, NotifyVirtual}, {root, FocusIn, NotifyInferior}};
    struct focus_event to_none[] = {{parent, FocusOut, NotifyNonlinear},
        {root, FocusOut, NotifyNonlinearVirtual}, {root, FocusIn, NotifyDetailNone}};
    struct focus_event from_none[] = {{root, FocusOut, NotifyDetailNone},
        {root, FocusIn, NotifyNonlinearVirtual}, {parent, FocusIn, NotifyNonlinear}};
    struct focus_event to_pointer[] = {{parent, FocusOut, NotifyNonlinear},
        {root, FocusOut, NotifyNonlinearVirtual}, {root, FocusIn, NotifyPointerRoot},
        {root, FocusIn, NotifyPointer}};
    struct focus_event from_pointer[] = {{root, FocusOut, NotifyPointer},
        {root, FocusOut, NotifyPointerRoot}, {root, FocusIn, NotifyNonlinearVirtual},
        {parent, FocusIn, NotifyNonlinear}};
    struct focus_event pointer_descendant[] = {{sibling, FocusOut, NotifyNonlinear},
        {parent, FocusIn, NotifyNonlinear}, {middle, FocusIn, NotifyPointer},
        {leaf, FocusIn, NotifyPointer}};
    /* Keep the pointer outside all probe windows except for the final check. */
    /* Allow Boxedwine's deferred native window presentation to complete. */
    usleep(1500000);
    XWarpPointer(display, None, root, 0, 0, 0, 0, 700, 500);
    usleep(50000);
    XSelectInput(display, root, FocusChangeMask);
    XSetInputFocus(display, parent, RevertToParent, CurrentTime);
    drain();
    XSetInputFocus(display, leaf, RevertToParent, CurrentTime);
    check_sequence("ancestor to descendant", to_child, 3);
    XSetInputFocus(display, parent, RevertToParent, CurrentTime);
    check_sequence("descendant to ancestor", to_parent, 3);
    XSetInputFocus(display, leaf, RevertToParent, CurrentTime);
    drain();
    XSetInputFocus(display, peer, RevertToParent, CurrentTime);
    check_sequence("cousin transition", to_peer, 5);
    XUnmapWindow(display, sibling);
    check_sequence("unmap precedes focus events", unmapped, 4);
    check_focus("hidden ancestor skipped", root, RevertToNone);
    XMapWindow(display, sibling);

    XSetInputFocus(display, parent, RevertToParent, CurrentTime);
    drain();
    XSetInputFocus(display, None, RevertToNone, CurrentTime);
    check_sequence("window to None", to_none, 3);
    XSetInputFocus(display, parent, RevertToParent, CurrentTime);
    check_sequence("None to window", from_none, 3);
    XSetInputFocus(display, PointerRoot, RevertToNone, CurrentTime);
    check_sequence("window to PointerRoot", to_pointer, 4);
    XSetInputFocus(display, parent, RevertToParent, CurrentTime);
    check_sequence("PointerRoot to window", from_pointer, 4);
    XSetInputFocus(display, parent, RevertToNone, CurrentTime);
    check_sequence("same focus emits no events", NULL, 0);
    check_focus("same focus updates reversion", parent, RevertToNone);
    XSetInputFocus(display, sibling, 99, CurrentTime);
    check_focus("invalid reversion preserves focus", parent, RevertToNone);

    old_time = timestamp();
    XSetInputFocus(display, leaf, RevertToParent, old_time);
    usleep(30000);
    XUnmapWindow(display, middle);
    XSetInputFocus(display, sibling, RevertToNone, old_time);
    check_focus("automatic reversion preserves timestamp", sibling, RevertToNone);
    XMapWindow(display, middle);
    XSetInputFocus(display, leaf, RevertToParent, CurrentTime);
    XDestroyWindow(display, middle);
    check_focus("destroy ancestor reverts past subtree", parent, RevertToNone);

    middle = make_window(parent, 0, 0, 1);
    leaf = make_window(middle, 0, 0, 1);
    pointer_descendant[2].window = middle;
    pointer_descendant[3].window = leaf;
    XSetInputFocus(display, sibling, RevertToNone, CurrentTime);
    {
        Window pointer_root, pointer_child;
        int rx, ry, wx, wy;
        unsigned int mask;
        XQueryPointer(display, root, &pointer_root, &pointer_child, &rx, &ry, &wx, &wy, &mask);
        /* Move the test window under the pointer. A native desktop may ignore
         * a physical cursor warp while the emulator does not own host focus. */
        printf("POINTER root=%lx child=%lx x=%d y=%d\n", pointer_root, pointer_child, rx, ry);
        XMoveResizeWindow(display, parent, rx - 1, ry - 1, 32, 32);
    }
    drain();
    XSetInputFocus(display, parent, RevertToNone, CurrentTime);
    check_sequence("pointer descendant notifications", pointer_descendant, 4);
    XSelectInput(display, root, 0);
}

int main(int argc, char **argv)
{
    Time earlier, later;
    int result;
    setvbuf(stdout, NULL, _IONBF, 0);
    display = XOpenDisplay(NULL);
    if (!display) { fprintf(stderr, "Cannot open X display\n"); return 2; }
    XSetErrorHandler(on_error);
    root = DefaultRootWindow(display);
    parent = make_window(root, 10, 10, 1);
    child = make_window(parent, 0, 0, 1);
    sibling = make_window(root, 80, 10, 1);
    hidden = make_window(root, 120, 10, 0);
    drain();

    XSetInputFocus(display, parent, RevertToParent, CurrentTime);
    drain();
    XSetInputFocus(display, sibling, RevertToParent, CurrentTime);
    check_event("sibling focus-out", parent, FocusOut, NotifyNonlinear);
    check_event("sibling focus-in", sibling, FocusIn, NotifyNonlinear);

    XSetInputFocus(display, parent, RevertToParent, CurrentTime);
    drain();
    XSetInputFocus(display, child, RevertToParent, CurrentTime);
    check_event("child focus-out", parent, FocusOut, NotifyInferior);
    check_event("child focus-in", child, FocusIn, NotifyAncestor);
    XUnmapWindow(display, child);
    check_focus("unmap child reverts to parent", parent, RevertToNone);

    XMapWindow(display, child);
    XSetInputFocus(display, child, RevertToParent, CurrentTime);
    XUnmapWindow(display, parent);
    check_focus("unmap ancestor reverts to root", root, RevertToNone);
    XMapWindow(display, parent);

    XSetInputFocus(display, child, RevertToNone, CurrentTime);
    XUnmapWindow(display, child);
    check_focus("unmap reverts to None", None, RevertToNone);
    XMapWindow(display, child);
    XSetInputFocus(display, child, RevertToPointerRoot, CurrentTime);
    XUnmapWindow(display, child);
    check_focus("unmap reverts to PointerRoot", PointerRoot, RevertToPointerRoot);
    XMapWindow(display, child);

    XSetInputFocus(display, child, RevertToParent, CurrentTime);
    XDestroyWindow(display, child);
    check_focus("destroy child reverts to parent", parent, RevertToNone);

    earlier = timestamp();
    usleep(30000);
    later = timestamp();
    printf("TIMESTAMPS earlier=%lu later=%lu\n", earlier, later);
    if (!earlier || later == earlier) { fprintf(stderr, "Invalid timestamp fixture\n"); return 2; }
    XSetInputFocus(display, parent, RevertToParent, later);
    XSetInputFocus(display, sibling, RevertToNone, earlier);
    check_focus("stale focus request ignored", parent, RevertToParent);
    XSetInputFocus(display, parent, RevertToParent, CurrentTime);
    XSetInputFocus(display, sibling, RevertToNone, later + 60000);
    check_focus("future focus request ignored", parent, RevertToParent);

    XSetInputFocus(display, parent, RevertToParent, CurrentTime);
    error_code = 0;
    result = XSetInputFocus(display, hidden, RevertToNone, CurrentTime);
    XSync(display, False);
    /* Boxedwine currently returns X errors directly rather than invoking the handler. */
    printf("HIDDEN error=%d return=%d\n", error_code, result);
    check_focus("hidden window cannot take focus", parent, RevertToParent);

    if (argc > 1) extended_checks();

    XSetInputFocus(display, PointerRoot, RevertToPointerRoot, CurrentTime);
    XDestroyWindow(display, parent);
    XDestroyWindow(display, sibling);
    XDestroyWindow(display, hidden);
    XCloseDisplay(display);
    printf("RESULT checks=%d failures=%d\n", checks, failures);
    return failures ? 1 : 0;
}
