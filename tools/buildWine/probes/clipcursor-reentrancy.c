/* Internal cursor clipping must preserve message wait masks and must not
 * dispatch a pending sent window message while polling queue status.
 * See clipcursor-reentrancy.md for the installer regression and controls.
 * Build: i686-w64-mingw32-gcc -O2 -Wall -Wextra -Werror clipcursor-reentrancy.c -luser32 -o clipcursor-reentrancy.exe
 */
#include <windows.h>
#include <stdio.h>

static HWND window;
static unsigned int clip_reentries, poll_reentries, delivered;
static BOOL in_clip, in_poll;

static LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
    if (message == WM_APP)
    {
        ++delivered;
        if (in_clip) ++clip_reentries;
        if (in_poll) ++poll_reentries;
        return 123;
    }
    return DefWindowProcA(hwnd, message, wp, lp);
}

static DWORD WINAPI sender(void *arg)
{
    (void)arg;
    return SendMessageA(window, WM_APP, 0, 0) == 123 ? 0 : 1;
}

static DWORD WINAPI clip_and_post(void *arg)
{
    RECT rect = {110,110,490,390};
    (void)arg;
    Sleep(100);
    if (!ClipCursor(&rect)) return 1;
    Sleep(100);
    return !PostMessageA(window, WM_APP + 1, 0, 0);
}

static void pump(void)
{
    MSG msg;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

int main(void)
{
    WNDCLASSA cls = {0};
    RECT rect = {100,100,500,400};
    unsigned int i, failures = 0, wake_misses = 0;
    DWORD start;
    cls.hInstance = GetModuleHandleA(NULL);
    cls.lpfnWndProc = window_proc;
    cls.lpszClassName = "ClipCursorReentrancy";
    if (!RegisterClassA(&cls)) return 2;
    window = CreateWindowA(cls.lpszClassName, "ClipCursor reentrancy probe",
                           WS_OVERLAPPEDWINDOW | WS_VISIBLE, 20, 20, 640, 480,
                           NULL, NULL, cls.hInstance, NULL);
    if (!window) return 2;
    SetForegroundWindow(window);
    start = GetTickCount();
    do { pump(); Sleep(10); } while (GetTickCount() - start < 500);
    /* Initialize the capture helper before queuing application messages. */
    if (!ClipCursor(&rect)) return 3;
    for (i = 0; i < 16; ++i)
    {
        HANDLE thread = CreateThread(NULL, 0, sender, NULL, 0, NULL);
        DWORD code;
        if (!thread) return 4;
        start = GetTickCount();
        for (;;)
        {
            DWORD status;
            /* This can process a pending internal cursor-clipping request. */
            in_poll = TRUE;
            status = GetQueueStatus(QS_SENDMESSAGE);
            in_poll = FALSE;
            if ((HIWORD(status) & QS_SENDMESSAGE) || delivered > i) break;
            if (GetTickCount() - start > 3000) return 5;
            Sleep(1);
        }
        in_clip = TRUE;
        if (!ClipCursor(i % 2 ? &rect : NULL)) ++failures;
        in_clip = FALSE;
        start = GetTickCount();
        while (WaitForSingleObject(thread, 0) != WAIT_OBJECT_0)
        {
            pump();
            if (GetTickCount() - start > 3000) return 6;
            Sleep(1);
        }
        GetExitCodeThread(thread, &code);
        if (code) ++failures;
        CloseHandle(thread);
    }
    for (i = 0; i < 4; ++i)
    {
        HANDLE thread;
        DWORD code;
        ClipCursor(NULL);
        pump();
        thread = CreateThread(NULL, 0, clip_and_post, NULL, 0, NULL);
        if (!thread) return 4;
        /* Internal clipping arrives while this thread waits for a posted
         * message. The driver's helper must preserve this wait's queue mask. */
        if (MsgWaitForMultipleObjects(0, NULL, FALSE, 2000, QS_POSTMESSAGE) != WAIT_OBJECT_0)
            ++wake_misses;
        if (WaitForSingleObject(thread, 5000) != WAIT_OBJECT_0) return 6;
        GetExitCodeThread(thread, &code);
        if (code) ++failures;
        CloseHandle(thread);
        pump();
    }
    ClipCursor(NULL);
    DestroyWindow(window);
    if (clip_reentries || poll_reentries || wake_misses || delivered != 16) ++failures;
    printf("CLIP_REENTRANCY sent=%u clip=%u poll=%u\n", delivered, clip_reentries, poll_reentries);
    printf("CLIP_QUEUE_WAKE waits=4 missed=%u failures=%u\n", wake_misses, failures);
    return failures ? 1 : 0;
}
