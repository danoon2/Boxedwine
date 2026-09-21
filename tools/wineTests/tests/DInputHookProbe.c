/* Foreground DirectInput acquisition must not strand CBT hooks after Unacquire.
 * Wine 11.0 skips the application's hook after about 12 keyboard/mouse cycles.
 * No renderer, MFC, or game assets are needed to reproduce the failure.
 *
 * Build PE32 with MinGW (define PROBE_DINPUT7 to cover dinput.dll as well):
 * i686-w64-mingw32-gcc -O2 -Wall -Wextra DInputHookProbe.c \
 *     -o DInputHookProbe.exe -luser32 -ldinput8 -ldxguid
 * Run: wine DInputHookProbe.exe [cycles-per-phase, default 40]
 * Requires an interactive desktop; keep the probe window in the foreground.
 */
#define COBJMACROS
#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <dinput.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PROBE_DINPUT7
typedef IDirectInput7W Input;
typedef IDirectInputDeviceW Device;
#define INPUT_VERSION 0x0700
#define INPUT_DLL "dinput.dll"
#define create_device IDirectInput7_CreateDevice
#define release_input IDirectInput7_Release
#define set_format IDirectInputDevice_SetDataFormat
#define set_coop IDirectInputDevice_SetCooperativeLevel
#define acquire IDirectInputDevice_Acquire
#define unacquire IDirectInputDevice_Unacquire
#define release_device IDirectInputDevice_Release
#else
typedef IDirectInput8W Input;
typedef IDirectInputDevice8W Device;
#define INPUT_VERSION 0x0800
#define INPUT_DLL "dinput8.dll"
#define create_device IDirectInput8_CreateDevice
#define release_input IDirectInput8_Release
#define set_format IDirectInputDevice8_SetDataFormat
#define set_coop IDirectInputDevice8_SetCooperativeLevel
#define acquire IDirectInputDevice8_Acquire
#define unacquire IDirectInputDevice8_Unacquire
#define release_device IDirectInputDevice8_Release
#endif

static unsigned checks, failures, notifications;
static int window_token;

static LRESULT CALLBACK cbt_proc(int code, WPARAM wp, LPARAM lp)
{
    if (code == HCBT_CREATEWND &&
        ((CBT_CREATEWNDA *)lp)->lpcs->lpCreateParams == &window_token)
        ++notifications;
    return CallNextHookEx(NULL, code, wp, lp);
}

static void pump(void)
{
    MSG msg;
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

static BOOL check(BOOL success, const char *expression, unsigned line,
                  unsigned phase, unsigned cycle, HRESULT result)
{
    ++checks;
    if (!success)
    {
        ++failures;
        printf("DInputHookProbe.c:%u: Test failed: phase %u cycle %u: %s (0x%08lx)\n",
               line, phase, cycle, expression, result);
    }
    return success;
}

#define REQUIRE(expr) do { if (!check(!!(expr), #expr, __LINE__, phase, cycle, 0)) goto cleanup; } while (0)
#define CALL(expr) do { HRESULT hr_ = (expr); if (!check(hr_ == DI_OK, #expr, __LINE__, phase, cycle, hr_)) goto cleanup; } while (0)

int main(int argc, char **argv)
{
    HRESULT (WINAPI *create_input)(HINSTANCE, DWORD, REFIID, void **, IUnknown *);
    HMODULE module = LoadLibraryA(INPUT_DLL);
    Input *input = NULL;
    Device *mouse = NULL, *keyboard = NULL;
    HWND window = NULL;
    HHOOK hook = NULL;
    unsigned phase = 0, cycle = 0, count = 40;
    char *end;
    HRESULT hr;
    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc > 1)
    {
        unsigned long value = strtoul(argv[1], &end, 10);
        if (*end || value < 1 || value > 10000) return 2;
        count = value;
    }
    REQUIRE(module != NULL);
#ifdef PROBE_DINPUT7
    create_input = (void *)GetProcAddress(module, "DirectInputCreateEx");
    REQUIRE(create_input != NULL);
    hr = create_input(GetModuleHandleW(NULL), INPUT_VERSION, &IID_IDirectInput7W, (void **)&input, NULL);
#else
    create_input = (void *)GetProcAddress(module, "DirectInput8Create");
    REQUIRE(create_input != NULL);
    hr = create_input(GetModuleHandleW(NULL), INPUT_VERSION, &IID_IDirectInput8W, (void **)&input, NULL);
#endif
    CALL(hr);
    hook = SetWindowsHookExA(WH_CBT, cbt_proc, NULL, GetCurrentThreadId());
    REQUIRE(hook != NULL);
    /* Cover explicit Unacquire with reused devices, recreated devices, and
     * releasing acquired devices without an explicit Unacquire. */
    for (phase = 0; phase < 3; ++phase)
    {
        for (cycle = 0; cycle < count; ++cycle)
        {
            unsigned before = notifications;
            POINT center = {160, 120};
            window = CreateWindowExA(0, "STATIC", "DirectInput hook regression",
                                     WS_POPUP | WS_VISIBLE, 80, 80, 320, 240,
                                     NULL, NULL, GetModuleHandleW(NULL), &window_token);
            REQUIRE(window != NULL);
            REQUIRE(notifications == before + 1);
            SetForegroundWindow(window);
            pump();
            REQUIRE(GetForegroundWindow() == window);
            /* Reproduce with the pointer inside the client area too. */
            REQUIRE(ClientToScreen(window, &center));
            REQUIRE(SetCursorPos(center.x, center.y));
            if (!keyboard)
            {
                CALL(create_device(input, &GUID_SysKeyboard, &keyboard, NULL));
                CALL(set_format(keyboard, &c_dfDIKeyboard));
            }
            if (!mouse)
            {
                CALL(create_device(input, &GUID_SysMouse, &mouse, NULL));
                CALL(set_format(mouse, &c_dfDIMouse));
            }
            CALL(set_coop(keyboard, window, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));
            CALL(set_coop(mouse, window, DISCL_FOREGROUND | DISCL_EXCLUSIVE));
            CALL(acquire(keyboard));
            CALL(acquire(mouse));
            if (phase != 2)
            {
                CALL(unacquire(mouse));
                CALL(unacquire(keyboard));
            }
            if (phase != 0)
            {
                REQUIRE(release_device(mouse) == 0);
                mouse = NULL;
                REQUIRE(release_device(keyboard) == 0);
                keyboard = NULL;
            }
            REQUIRE(DestroyWindow(window));
            window = NULL;
            pump();
        }
        if (mouse) { release_device(mouse); mouse = NULL; }
        if (keyboard) { release_device(keyboard); keyboard = NULL; }
        printf("%s phase %u: %u cycles completed\n", INPUT_DLL, phase, count);
    }
cleanup:
    if (mouse) { unacquire(mouse); release_device(mouse); }
    if (keyboard) { unacquire(keyboard); release_device(keyboard); }
    if (input) release_input(input);
    if (window) DestroyWindow(window);
    if (hook) UnhookWindowsHookEx(hook);
    if (module) FreeLibrary(module);
    printf("DInputHookProbe: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", checks, failures);
    return failures ? 1 : 0;
}
