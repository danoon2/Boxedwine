/* Build: i686-w64-mingw32-gcc -O2 timerPriorityProbe.c -o probe.exe -lwinmm
 * Measures lower-priority work during a busy multimedia-timer callback on one
 * CPU. Results, not process/recorder exit status, are the scheduling evidence.
 * An optional argument names the output file; otherwise results go to stdout.
 * A host Windows comparison does not establish behavior on original Windows 95.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

static volatile LONG normalCounter, complete;
static LONG overlap[12];
static int priorities[12];
static LARGE_INTEGER frequency;

static void CALLBACK timerCallback(UINT id, UINT message, DWORD_PTR user, DWORD_PTR a, DWORD_PTR b)
{
    LARGE_INTEGER start, now;
    unsigned index = (unsigned)user;
    priorities[index] = GetThreadPriority(GetCurrentThread());
    LONG before = InterlockedCompareExchange(&normalCounter, 0, 0);
    QueryPerformanceCounter(&start);
    do { QueryPerformanceCounter(&now); }
    while (now.QuadPart - start.QuadPart < frequency.QuadPart / 50);
    overlap[index] = InterlockedCompareExchange(&normalCounter, 0, 0) - before;
    InterlockedExchange(&complete, 1);
}

int main(int argc, char **argv)
{
    DWORD_PTR mask = 0, systemMask = 0;
    if (!GetProcessAffinityMask(GetCurrentProcess(), &mask, &systemMask) || !mask) return 2;
    DWORD_PTR oneCpu = mask & (0 - mask);
    if (!SetProcessAffinityMask(GetCurrentProcess(), oneCpu)) return 3;
    if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL)) return 4;
    if (!QueryPerformanceFrequency(&frequency)) return 5;
    FILE *out = argc > 1 ? fopen(argv[1], "w") : stdout;
    if (!out) return 6;
    fprintf(out, "normal_priority=%d affinity=%lx\n", GetThreadPriority(GetCurrentThread()), (unsigned long)oneCpu);
    int overlapping = 0;
    timeBeginPeriod(1);
    for (unsigned i = 0; i < 12; ++i) {
        InterlockedExchange(&complete, 0);
        InterlockedExchange(&normalCounter, 0);
        UINT timer = timeSetEvent(20, 1, timerCallback, i, TIME_ONESHOT | TIME_KILL_SYNCHRONOUS);
        if (!timer) return 7;
        DWORD started = GetTickCount();
        while (!InterlockedCompareExchange(&complete, 0, 0)) {
            InterlockedIncrement(&normalCounter);
            if (GetTickCount() - started > 5000) return 8;
        }
        timeKillEvent(timer);
        fprintf(out, "callback=%u priority=%d normal_thread_iterations_during_callback=%ld\n", i, priorities[i], overlap[i]);
        overlapping += overlap[i] != 0;
    }
    timeEndPeriod(1);
    fprintf(out, "overlapping_callbacks=%d/12\n", overlapping);
    if (out != stdout) fclose(out);
    return 0;
}
