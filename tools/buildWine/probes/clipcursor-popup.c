/* Reproduce a cursor clip release queued for a popup destroyed before dispatch.
 * Build: i686-w64-mingw32-gcc clipcursor-popup.c -o clipcursor-popup.exe -luser32
 */
#include <windows.h>
#include <stdio.h>

static void pump(DWORD duration)
{
    DWORD start = GetTickCount();
    MSG msg;
    do
    {
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        Sleep(10);
    } while (GetTickCount() - start < duration);
}

int main(void)
{
    WNDCLASSA cls = {0};
    HWND owner, popup;
    RECT clip = {100, 100, 132, 196};
    POINT pos;
    unsigned int failures = 0, iteration;

    cls.lpfnWndProc = DefWindowProcA;
    cls.hInstance = GetModuleHandleA(NULL);
    cls.lpszClassName = "ClipCursorPopupProbe";
    RegisterClassA(&cls);
    owner = CreateWindowA(cls.lpszClassName, "Cursor release probe", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                          20, 20, 640, 480, NULL, NULL, cls.hInstance, NULL);
    if (!owner) return 2;
    SetForegroundWindow(owner);
    pump(300);

    for (iteration = 0; iteration < 3; ++iteration)
    {
        popup = CreateWindowA(cls.lpszClassName, "Floor popup", WS_POPUP | WS_VISIBLE,
                              100, 100, 32, 96, owner, NULL, cls.hInstance, NULL);
        SetForegroundWindow(popup);
        SetFocus(popup);
        if (!ClipCursor(&clip)) return 3;
        pump(300);
        SetCursorPos(500, 400);
        pump(150);
        GetCursorPos(&pos);
        printf("clipped %u: %ld,%ld\n", iteration, pos.x, pos.y);
        if (pos.x < clip.left || pos.x >= clip.right || pos.y < clip.top || pos.y >= clip.bottom)
            ++failures;

        ClipCursor(NULL);
        /* Do not pump between releasing the clip and destroying its target. */
        DestroyWindow(popup);
        SetForegroundWindow(owner);
        SetFocus(owner);
        pump(300);
        SetCursorPos(500, 400);
        pump(150);
        GetCursorPos(&pos);
        printf("released %u: %ld,%ld\n", iteration, pos.x, pos.y);
        if (pos.x != 500 || pos.y != 400) ++failures;
    }
    ClipCursor(NULL);
    DestroyWindow(owner);
    printf("CLIPCURSOR_POPUP_%s failures=%u\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
