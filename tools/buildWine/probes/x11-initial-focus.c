/* X11 focus regression: a fresh GUI thread replaces a splash after another
 * thread has processed mouse input. See x11-initial-focus.py for the runner. */
#include <windows.h>
#include <stdio.h>

static HWND splash;
static HANDLE done;
static LONG started, failures;

static DWORD WINAPI worker(void *argument)
{
    HWND hwnd;
    MSG msg;
    DWORD end;

    printf("WORKER_INITIAL_MESSAGE_TIME %ld\n", GetMessageTime());
    if (GetMessageTime()) InterlockedIncrement(&failures);
    hwnd = CreateWindowA("focus_startup_probe", "Focus startup worker", WS_POPUP,
                        500, 100, 400, 300, NULL, NULL, GetModuleHandleA(NULL), NULL);
    if (!hwnd)
    {
        InterlockedIncrement(&failures);
        SetEvent(done);
        return 1;
    }
    ShowWindow(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    printf("WORKER_READY hwnd=%p foreground=%p message_time=%ld\n",
           hwnd, GetForegroundWindow(), GetMessageTime());
    if (GetForegroundWindow() != hwnd) InterlockedIncrement(&failures);
    PostMessageA(splash, WM_APP, 0, 0);
    end = GetTickCount() + 4000;
    while ((LONG)(end - GetTickCount()) > 0)
    {
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        Sleep(10);
    }
    DestroyWindow(hwnd);
    SetEvent(done);
    return 0;
}

static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (hwnd == splash && msg == WM_MOUSEMOVE && !InterlockedExchange(&started, 1))
    {
        printf("SPLASH_MOUSE message_time=%ld\n", GetMessageTime());
        SetTimer(hwnd, 1, 500, NULL);
    }
    if (hwnd == splash && msg == WM_TIMER)
    {
        HANDLE thread;
        KillTimer(hwnd, 1);
        thread = CreateThread(NULL, 0, worker, NULL, 0, NULL);
        if (thread) CloseHandle(thread);
        else
        {
            InterlockedIncrement(&failures);
            SetEvent(done);
        }
    }
    if (hwnd == splash && msg == WM_APP) ShowWindow(hwnd, SW_HIDE);
    return DefWindowProcA(hwnd, msg, wp, lp);
}

int main(void)
{
    WNDCLASSA wc = {0};
    MSG msg;
    DWORD end;

    setvbuf(stdout, NULL, _IONBF, 0);
    done = CreateEventA(NULL, TRUE, FALSE, NULL);
    wc.lpfnWndProc = window_proc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "focus_startup_probe";
    if (!done || !RegisterClassA(&wc)) return 1;
    splash = CreateWindowA(wc.lpszClassName, "Focus startup splash", WS_POPUP,
                          100, 100, 300, 200, NULL, NULL, wc.hInstance, NULL);
    if (!splash) return 1;
    ShowWindow(splash, SW_SHOW);
    SetForegroundWindow(splash);
    puts("SPLASH_READY");
    end = GetTickCount() + 15000;
    while (WaitForSingleObject(done, 0) != WAIT_OBJECT_0 && (LONG)(end - GetTickCount()) > 0)
    {
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        Sleep(10);
    }
    if (WaitForSingleObject(done, 0) != WAIT_OBJECT_0) ++failures;
    DestroyWindow(splash);
    CloseHandle(done);
    printf("PROBE_DONE failures=%ld\n", failures);
    return failures != 0;
}
