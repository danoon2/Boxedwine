#if 0
MAKE_DEP_UNIX
#endif
#include "wineboxed.h"
#include "wine/debug.h"
#include "wine/server.h"

#include <fcntl.h>
#include <unistd.h>

#if BOXED_WINE_VERSION >= 7120
#define CloseHandle NtClose
#define ExitProcess(x) NtTerminateProcess(0, x)
#define GetAncestor NtUserGetAncestor
#define GetForegroundWindow NtUserGetForegroundWindow
#define WindowFromPoint(p) NtUserWindowFromPoint(p.x, p.y)
#endif

WINE_DEFAULT_DEBUG_CHANNEL(boxeddrv);

int eventsInitialized = 0;
int eventQueueFD;

void initEvents(void)
{
    HANDLE handle;
    int ret;
    int fds[2];

    if (eventsInitialized)
        return;
    eventsInitialized = 1;
    pipe(fds);
    fcntl(fds[0], F_SETFD, 1);
    fcntl(fds[0], F_SETFL, O_NONBLOCK);
    fcntl(fds[1], F_SETFD, 1);
    fcntl(fds[1], F_SETFL, O_NONBLOCK);

    eventQueueFD = fds[0];
    CALL_NORETURN_1(BOXED_SET_EVENT_FD, fds[1]);

    if (wine_server_fd_to_handle(fds[0], GENERIC_READ | SYNCHRONIZE, 0, &handle))
    {
        MESSAGE("wineboxed.drv: Can't allocate handle for event queue fd\n");
        ExitProcess(1);
    }
    SERVER_START_REQ(set_queue_fd)
    {
        req->handle = wine_server_obj_handle(handle);
        ret = wine_server_call(req);
    }
    SERVER_END_REQ;
    if (ret)
    {
        MESSAGE("wineboxed.drv: Can't store handle for event queue fd\n");
        ExitProcess(1);
    }
    else {
        TRACE("event queue read fd=%d\n", eventQueueFD);
    }
    CloseHandle(handle);
}

BOOL processEvents(DWORD mask) {
    HWND hwnd;
    INPUT input;
    int r;
    BOOL result = FALSE;
    static BOOL inEvent;

    if (inEvent || (mask & (QS_KEY | QS_MOUSEBUTTON | QS_MOUSEMOVE)) == 0) {
        TRACE("skipping event inEvent=%d mask=%x\n", (int)inEvent, (int)mask);
        return FALSE;
    }
    while (1) {
        if ((r = read(eventQueueFD, &input, sizeof(INPUT))) == -1) {
            return result;
        }
        TRACE("read event: type=");
        if (input.type == 0) {
            const char* type = "rel";
            if (input.mi.dwFlags & MOUSEEVENTF_WHEEL) {
                hwnd = GetForegroundWindow();
            }
            else if (input.mi.dwFlags & MOUSEEVENTF_ABSOLUTE) {
                POINT p;
                p.x = input.mi.dx;
                p.y = input.mi.dy;
                hwnd = WindowFromPoint(p);
                if (!hwnd) {
                    continue;
                }
                hwnd = GetAncestor(hwnd, GA_ROOT);
            }
            else {
                hwnd = GetForegroundWindow();
            }
            if (input.mi.dwFlags & MOUSEEVENTF_ABSOLUTE) {
                type = "abs";
            }
            TRACE("mouse %s %s hwnd=%p dx=%d dy=%d dwFlags=%X time=%X\n", ((input.mi.dwFlags & MOUSEEVENTF_WHEEL) ? "wheel" : ""), type, hwnd, (int)input.mi.dx, (int)input.mi.dy, (int)input.mi.dwFlags, (int)input.mi.time);
        }
        else {
            hwnd = GetForegroundWindow();
        }

        TRACE("hwnd=%p GetForegroundWindow()=%p\n", hwnd, GetForegroundWindow());
        inEvent = TRUE;
        // Apr 15, 2021 wine-6.7
#if BOXED_WINE_VERSION >= 6070
        __wine_send_input(hwnd, &input, NULL);
#else
        __wine_send_input(hwnd, &input);
#endif
        inEvent = FALSE;
        result = TRUE;
    }
}