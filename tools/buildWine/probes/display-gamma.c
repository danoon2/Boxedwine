/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
#include <windows.h>
#include <stdio.h>
#include <string.h>

/* Exercise the Wine GDI -> XF86VidMode -> Boxedwine boundary, not guest pow(). */
static unsigned isqrt(unsigned long long n) {
    unsigned lo = 0, hi = 65536;
    while (lo + 1 < hi) {
        unsigned mid = (lo + hi) / 2;
        if ((unsigned long long)mid * mid <= n) lo = mid;
        else hi = mid;
    }
    return lo;
}

int main(int argc, char** argv) {
    WORD saved[3][256], ramp[3][256], actual[3][256];
    int failures = 0;
    HDC dc;
    if (!GetProcAddress(GetModuleHandleA("ntdll.dll"), "wine_get_version")) {
        puts("This test changes virtual display gamma and only runs in Wine.");
        return 2;
    }
    setvbuf(stdout, NULL, _IONBF, 0);
    dc = CreateDCA("DISPLAY", NULL, NULL, NULL);
    memset(saved, 0xcc, sizeof(saved));
    if (!dc || !GetDeviceGammaRamp(dc, saved)) { puts("FAIL initial GetDeviceGammaRamp"); return 1; }
    for (unsigned c = 0; c < 3; ++c) for (unsigned i = 0; i < 256; ++i) {
        if (saved[c][i] != i * 257) ++failures;
        ramp[c][i] = isqrt((unsigned long long)i * 65535 * 65535 / 255);
    }
    if (argc == 1) {
        char exe[MAX_PATH], command[MAX_PATH + 32];
        STARTUPINFOA si = {sizeof(si)};
        PROCESS_INFORMATION pi;
        DWORD status = 1;
        GetModuleFileNameA(NULL, exe, sizeof(exe));
        snprintf(command, sizeof(command), "\"%s\" leave-ramp", exe);
        if (!CreateProcessA(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return 1;
        if (WaitForSingleObject(pi.hProcess, 120000) != WAIT_OBJECT_0) return 1;
        GetExitCodeProcess(pi.hProcess, &status);
        CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
        /* Wine can signal the Windows process handle before its Unix process
         * finishes teardown. Bound the wait for the virtual display cleanup. */
        int restored = 0;
        for (unsigned attempt = 0; attempt < 200; ++attempt) {
            if (GetDeviceGammaRamp(dc, actual) && !memcmp(actual, saved, sizeof(saved))) { restored = 1; break; }
            Sleep(10);
        }
        if (status || !restored) {
            puts("FAIL exiting gamma owner did not restore identity"); ++failures;
        } else puts("PASS exiting gamma owner restored identity");
    }
    if (!SetDeviceGammaRamp(dc, ramp)) { puts("FAIL SetDeviceGammaRamp"); ++failures; }
    memset(actual, 0xcc, sizeof(actual));
    if (!GetDeviceGammaRamp(dc, actual) || memcmp(actual, ramp, sizeof(ramp))) {
        puts("FAIL gamma round trip"); ++failures;
    }
    printf("Gamma red: [%u,%u,%u,%u,%u]\n", actual[0][0], actual[0][64], actual[0][128], actual[0][192], actual[0][255]);
    if (argc < 2 || strcmp(argv[1], "leave-ramp")) {
        if (!SetDeviceGammaRamp(dc, saved) || !GetDeviceGammaRamp(dc, actual) || memcmp(actual, saved, sizeof(saved))) {
            puts("FAIL gamma restore"); ++failures;
        }
    }
    DeleteDC(dc);
    printf("DISPLAY_GAMMA_%s failures=%d\n", failures ? "FAIL" : "PASS", failures);
    return failures != 0;
}
