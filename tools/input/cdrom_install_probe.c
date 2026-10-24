/* Copyright (C) 2026 The BoxedWine Team. SPDX-License-Identifier: GPL-2.0-or-later
 * i686-w64-mingw32-gcc -Os -s -municode tools/input/cdrom_install_probe.c -o probe.exe
 */
#include <windows.h>
#include <wchar.h>
int wmain(void) {
    WCHAR cwd[1024]; DWORD written;
    HANDLE file = CreateFileW(L"C:\\disc-install-result.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (file == INVALID_HANDLE_VALUE || !GetCurrentDirectoryW(1024, cwd)) return 1;
    WriteFile(file, GetCommandLineW(), wcslen(GetCommandLineW()) * sizeof(WCHAR), &written, NULL);
    WriteFile(file, L"\n", sizeof(WCHAR), &written, NULL);
    WriteFile(file, cwd, wcslen(cwd) * sizeof(WCHAR), &written, NULL);
    CloseHandle(file);
    return 0;
}
