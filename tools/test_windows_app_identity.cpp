/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
#define SDL_MAIN_HANDLED
#define NOMINMAX
#include <Windows.h>
#include <wincrypt.h>
#include <shobjidl.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <array>
#include <cassert>
#include <cstdio>
#include <string>
#include "../platform/windows/windowsAppIdentity.h"

static void checkPixels(HICON icon) {
    ICONINFO info = {};
    assert(GetIconInfo(icon, &info));
    BITMAPINFO bitmap = {};
    bitmap.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap.bmiHeader.biWidth = 64;
    bitmap.bmiHeader.biHeight = -64;
    bitmap.bmiHeader.biPlanes = 1;
    bitmap.bmiHeader.biBitCount = 32;
    std::array<BYTE, 64 * 64 * 4> pixels = {};
    HDC dc = CreateCompatibleDC(nullptr);
    assert(GetDIBits(dc, info.hbmColor, 0, 64, pixels.data(), &bitmap, DIB_RGB_COLORS) == 64);
    // Transparent padding, red translucent top and opaque green bottom. Native
    // icons may premultiply color channels, so check color as well as alpha.
    assert(pixels[3] == 0);
    int top = (20 * 64 + 32) * 4, bottom = (44 * 64 + 32) * 4;
    assert(pixels[top] == 0 && pixels[top + 1] == 0 && pixels[top + 2] >= 127 && pixels[top + 3] == 128);
    assert(pixels[bottom] == 0 && pixels[bottom + 1] == 255 && pixels[bottom + 2] == 0 && pixels[bottom + 3] == 255);
    DeleteDC(dc); DeleteObject(info.hbmColor); DeleteObject(info.hbmMask);
}

int main(int argc, char** argv) {
    std::string mode = argc > 1 ? argv[1] : "valid";
    const char* expectedId = "Boxedwine.App.00112233445566778899aabbccddeeff";
    bool hasIcon = mode == "valid" || mode == "external";
    if (mode != "external") {
        _putenv_s("BOXEDWINE_APP_ID", mode == "absent" ? "" : mode == "invalid" ? "invalid taskbar identity" : expectedId);
        std::array<BYTE, 64 * 64 * 4> pixels = {};
        for (int y = 16; y < 48; ++y) for (int x = 0; x < 64; ++x) {
            int p = (y * 64 + x) * 4;
            pixels[p + (y < 32 ? 2 : 1)] = 255;
            pixels[p + 3] = y < 32 ? 128 : 255;
        }
        DWORD length = 0;
        assert(CryptBinaryToStringA(pixels.data(), (DWORD)pixels.size(), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &length));
        std::string encoded(length, '\0');
        assert(CryptBinaryToStringA(pixels.data(), (DWORD)pixels.size(), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, encoded.data(), &length));
        if (mode == "invalid") encoded.assign(21848, '!');
        if (mode == "oversize") encoded.assign(25000, 'A');
        _putenv_s("BOXEDWINE_APP_ICON_BGRA", mode == "absent" ? "" : encoded.c_str());
    }
    windowsAppInitialize();
    assert(!getenv("BOXEDWINE_APP_ID") && !getenv("BOXEDWINE_APP_ICON_BGRA"));
    assert(!GetEnvironmentVariableA("BOXEDWINE_APP_ICON_BGRA", nullptr, 0));
    PWSTR id = nullptr;
    HRESULT result = GetCurrentProcessExplicitAppUserModelID(&id);
    if (mode == "absent" || mode == "invalid") assert(FAILED(result));
    else { assert(SUCCEEDED(result)); assert(std::wstring(id) == L"Boxedwine.App.00112233445566778899aabbccddeeff"); }
    CoTaskMemFree(id);
    SDL_SetMainReady(); assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    HICON original = nullptr;
    for (int pass = 0; pass < 12; ++pass) {
        auto window = SDL_CreateWindow("App icon regression", 10, 10, 200, 150, SDL_WINDOW_HIDDEN | (pass % 2 ? SDL_WINDOW_OPENGL : 0));
        assert(window);
        SDL_SysWMinfo info = {}; SDL_VERSION(&info.version); assert(SDL_GetWindowWMInfo(window, &info));
        auto oldIcon = SendMessageW(info.info.win.window, WM_GETICON, ICON_BIG, 0);
        windowsAppSetWindowIcon(window);
        auto icon = (HICON)SendMessageW(info.info.win.window, WM_GETICON, ICON_BIG, 0);
        if (hasIcon) {
            assert(icon && SendMessageW(info.info.win.window, WM_GETICON, ICON_SMALL, 0));
            if (original) assert(original == icon); else original = icon;
            checkPixels(icon);
        } else assert((LRESULT)icon == oldIcon);
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
    std::printf("PASS app identity: %s (taskbar ID, icon pixels, GDI/GL recreation, shared handles, consumed metadata)\n", mode.c_str());
}
