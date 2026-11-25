#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>

static int slowFrames, keyboard, frameButton;

static LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    if ((!keyboard && message == WM_LBUTTONDOWN) || (keyboard && message == WM_KEYDOWN && wparam == VK_RETURN)) {
        frameButton = 1;
        // Model a game frame that takes longer than the old 100 ms press.
        if (slowFrames) Sleep(300);
    } else if ((!keyboard && message == WM_LBUTTONUP) || (keyboard && message == WM_KEYUP && wparam == VK_RETURN)) {
        frameButton = 0;
    }
    return DefWindowProcA(window, message, wparam, lparam);
}

/* Use VERSION=2, WHILEWAITING=1,LBUTTON,100,100, DONE as the recording.
 * A guest warp must not leave subsequent recovery clicks at stale coordinates,
 * and polling must see the same held button as the delivered mouse events. */
int main(void)
{
    slowFrames = strstr(GetCommandLineA(), "--slow-frames") != NULL;
    keyboard = strstr(GetCommandLineA(), "--keyboard") != NULL;
    WNDCLASSA cls = {0};
    cls.lpfnWndProc = windowProc;
    cls.hInstance = GetModuleHandleA(NULL);
    cls.lpszClassName = "AutomationInputProbe";
    cls.hbrBackground = CreateSolidBrush(RGB(13, 61, 107));
    if (!RegisterClassA(&cls)) return 2;
    HWND window = CreateWindowA(cls.lpszClassName, "Automation input probe",
        WS_POPUP | WS_VISIBLE, 0, 0, 800, 600, NULL, NULL, cls.hInstance, NULL);
    if (!window) return 2;
    SetForegroundWindow(window);
    SetFocus(window);
    SetCursorPos(600, 300);
    UpdateWindow(window);
    DWORD start = GetTickCount();
    int positioned = 0, pressed = 0, together = 0, stale = 0, observedFrame = 0;
    POINT point = {0};
    while (GetTickCount() - start < 20000)
    {
        MSG message;
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        GetCursorPos(&point);
        int down = (GetAsyncKeyState(keyboard ? VK_RETURN : VK_LBUTTON) & 0x8000) != 0;
        int expected = keyboard || (point.x == 100 && point.y == 100);
        stale |= positioned && !expected;
        positioned |= expected;
        pressed |= down;
        together |= expected && down;
        observedFrame |= expected && frameButton;
        Sleep(2);
    }
    int passed = together && !stale && (!slowFrames || observedFrame);
    char result[256];
    snprintf(result, sizeof(result), "automation input position=%d button=%d together=%d stale=%d frame=%d last=%ld,%ld %s\n",
        positioned, pressed, together, stale, observedFrame, point.x, point.y, passed ? "PASS" : "FAIL");
    FILE *file = fopen("Z:\\files\\input-result.txt", "w");
    if (!file) return 2;
    fputs(result, file);
    fclose(file);
    fputs(result, stdout);
    DestroyWindow(window);
    return passed ? 0 : 1;
}
