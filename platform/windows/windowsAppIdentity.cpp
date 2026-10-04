/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
#define NOMINMAX
#include <Windows.h>
#include <wincrypt.h>
#include <shobjidl.h>
#include <SDL_syswm.h>
#include <array>
#include <cstdlib>
#include <cstring>
#include "windowsAppIdentity.h"

namespace {
constexpr int iconSize = 64;
constexpr int pixelBytes = iconSize * iconSize * 4;
constexpr int encodedBytes = (pixelBytes + 2) / 3 * 4;

template<size_t N> DWORD takeEnvironment(const char* name, std::array<char, N>& value) {
    DWORD length = GetEnvironmentVariableA(name, value.data(), (DWORD)value.size());
    // Clear both the CRT and Win32 copies, as on Mac. This metadata belongs to
    // the host window, not to Wine or to another child process.
    _putenv_s(name, "");
    return length < value.size() ? length : 0;
}

struct AppIdentity {
    HICON largeIcon = nullptr, smallIcon = nullptr;
    AppIdentity() {
        std::array<char, 128> id = {};
        DWORD idLength = takeEnvironment("BOXEDWINE_APP_ID", id);
        constexpr char prefix[] = "Boxedwine.App.";
        bool validId = idLength == sizeof(prefix) - 1 + 32 && !strncmp(id.data(), prefix, sizeof(prefix) - 1);
        for (size_t i = sizeof(prefix) - 1; validId && i < idLength; ++i) {
            char c = id[i];
            validId = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
        }
        if (validId) {
            wchar_t wide[128] = {};
            MultiByteToWideChar(CP_UTF8, 0, id.data(), -1, wide, 128);
            SetCurrentProcessExplicitAppUserModelID(wide);
        }

        std::array<char, encodedBytes + 1> encoded = {};
        DWORD length = takeEnvironment("BOXEDWINE_APP_ICON_BGRA", encoded);
        if (length != encodedBytes) return;
        std::array<BYTE, pixelBytes> pixels = {};
        DWORD decoded = (DWORD)pixels.size();
        if (!CryptStringToBinaryA(encoded.data(), length, CRYPT_STRING_BASE64 | CRYPT_STRING_STRICT,
            pixels.data(), &decoded, nullptr, nullptr) || decoded != pixelBytes) return;

        // Native icon resources contain bottom-up BGRA pixels followed by a
        // one-bit transparency mask. Bound dimensions and allocations above.
        constexpr int maskStride = iconSize / 8;
        std::array<BYTE, sizeof(BITMAPINFOHEADER) + pixelBytes + maskStride * iconSize> resource = {};
        BITMAPINFOHEADER header = {};
        header.biSize = sizeof(header);
        header.biWidth = iconSize;
        header.biHeight = iconSize * 2;
        header.biPlanes = 1;
        header.biBitCount = 32;
        header.biSizeImage = pixelBytes;
        memcpy(resource.data(), &header, sizeof(header));
        BYTE* color = resource.data() + sizeof(header);
        BYTE* mask = color + pixelBytes;
        for (int y = 0; y < iconSize; ++y) {
            const BYTE* row = pixels.data() + (iconSize - y - 1) * iconSize * 4;
            memcpy(color + y * iconSize * 4, row, iconSize * 4);
            for (int x = 0; x < iconSize; ++x) {
                if (!row[x * 4 + 3]) mask[y * maskStride + x / 8] |= 0x80 >> (x % 8);
            }
        }
        largeIcon = CreateIconFromResourceEx(resource.data(), (DWORD)resource.size(), TRUE, 0x00030000, iconSize, iconSize, LR_DEFAULTCOLOR);
        smallIcon = CreateIconFromResourceEx(resource.data(), (DWORD)resource.size(), TRUE, 0x00030000,
            GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    }
    ~AppIdentity() {
        if (largeIcon) DestroyIcon(largeIcon);
        if (smallIcon) DestroyIcon(smallIcon);
    }
    AppIdentity(const AppIdentity&) = delete;
    AppIdentity& operator=(const AppIdentity&) = delete;
};

AppIdentity& identity() {
    static AppIdentity value;
    return value;
}
}

void windowsAppInitialize() {
    identity();
}

void windowsAppSetWindowIcon(SDL_Window* window) {
    auto& app = identity();
    if (!window || !app.largeIcon) return;
    SDL_SysWMinfo info = {};
    SDL_VERSION(&info.version);
    if (!SDL_GetWindowWMInfo(window, &info)) return;
    SendMessageW(info.info.win.window, WM_SETICON, ICON_BIG, (LPARAM)app.largeIcon);
    SendMessageW(info.info.win.window, WM_SETICON, ICON_SMALL, (LPARAM)(app.smallIcon ? app.smallIcon : app.largeIcon));
}
