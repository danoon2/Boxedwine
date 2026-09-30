/* Run with -forceRelativeMouse and raw-mouse-play/script.txt. The script
 * deliberately supplies button coordinates outside a one-pixel ClipCursor
 * rectangle, as SDL's virtual cursor can do in relative mouse mode. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

static unsigned int buttons, moves, failures;
static BOOL ready;

static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_INPUT && ready) {
        RAWINPUT raw;
        UINT size = sizeof(raw);
        if (GetRawInputData((HRAWINPUT)lp, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER)) != (UINT)-1
                && raw.header.dwType == RIM_TYPEMOUSE) {
            RAWMOUSE *mouse = &raw.data.mouse;
            printf("RAW buttons=%x x=%ld y=%ld\n", mouse->usButtonFlags, mouse->lLastX, mouse->lLastY);
            if (mouse->usButtonFlags) {
                ++buttons;
                if (mouse->lLastX || mouse->lLastY) ++failures;
            } else if (mouse->lLastX || mouse->lLastY) {
                ++moves;
            }
        }
    } else if (msg == WM_TIMER && wp == 1) {
        RAWINPUTDEVICE dev = {1, 2, RIDEV_NOLEGACY, hwnd};
        RECT clip = {400, 300, 401, 301};
        SetForegroundWindow(hwnd);
        SetCursorPos(400, 300);
        if (!RegisterRawInputDevices(&dev, 1, sizeof(dev)) || !ClipCursor(&clip)) ++failures;
        ready = TRUE;
        KillTimer(hwnd, 1);
    } else if (msg == WM_TIMER && wp == 2) {
        ClipCursor(NULL);
        DestroyWindow(hwnd);
    } else if (msg == WM_DESTROY) {
        if (buttons != 10 || !moves) ++failures;
        printf("%s: %u button events, %u motion events, %u failures\n",
                failures ? "FAIL" : "PASS", buttons, moves, failures);
        PostQuitMessage(failures ? 1 : 0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

int main(void)
{
    WNDCLASSA cls = {0};
    MSG msg;
    HWND hwnd;
    setvbuf(stdout, NULL, _IONBF, 0);
    cls.lpfnWndProc = window_proc;
    cls.hInstance = GetModuleHandleA(NULL);
    cls.lpszClassName = "RawMouseProbe";
    if (!RegisterClassA(&cls)) return 2;
    hwnd = CreateWindowA(cls.lpszClassName, "Raw mouse probe", WS_POPUP | WS_VISIBLE,
            0, 0, 800, 600, NULL, NULL, cls.hInstance, NULL);
    if (!hwnd) return 2;
    SetTimer(hwnd, 1, 2000, NULL);
    SetTimer(hwnd, 2, 22000, NULL);
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}
