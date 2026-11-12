/* Compile: i686-w64-mingw32-gcc -O2 -Wall -Wextra wav-playback.c -o wav-playback.exe -lole32 -lstrmiids -luuid */
#define COBJMACROS
#include <windows.h>
#include <dshow.h>
#include <stdio.h>

int main(void)
{
    /* 250 ms of stereo, 16-bit PCM silence at 44.1 kHz. */
    static const unsigned char header[] = {
        'R','I','F','F', 0x68,0xac,0,0, 'W','A','V','E',
        'f','m','t',' ', 16,0,0,0, 1,0, 2,0, 0x44,0xac,0,0,
        0x10,0xb1,2,0, 4,0, 16,0, 'd','a','t','a', 0x44,0xac,0,0
    };
    unsigned char silence[44100] = {0};
    WCHAR directory[MAX_PATH], path[MAX_PATH];
    IGraphBuilder *graph = NULL;
    IMediaSeeking *seeking = NULL;
    IMediaControl *control = NULL;
    IMediaEvent *events = NULL;
    LONGLONG duration = 0;
    HANDLE file;
    DWORD written;
    long code = 0;
    HRESULT hr;
    int failed = 1;

    if (!GetTempPathW(MAX_PATH, directory) || !GetTempFileNameW(directory, L"wav", 0, path))
        return 1;
    file = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) { DeleteFileW(path); return 1; }
    if (!WriteFile(file, header, sizeof(header), &written, NULL) || written != sizeof(header) ||
        !WriteFile(file, silence, sizeof(silence), &written, NULL) || written != sizeof(silence))
    {
        CloseHandle(file);
        DeleteFileW(path);
        return 1;
    }
    CloseHandle(file);
    hr = CoInitialize(NULL);
    if (FAILED(hr)) { DeleteFileW(path); return 1; }
    hr = CoCreateInstance(&CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, &IID_IGraphBuilder, (void **)&graph);
    if (FAILED(hr)) goto done;
    hr = IGraphBuilder_RenderFile(graph, path, NULL);
    printf("RenderFile=%#lx\n", hr);
    if (hr != S_OK) goto done;
    hr = IGraphBuilder_QueryInterface(graph, &IID_IMediaSeeking, (void **)&seeking);
    if (FAILED(hr)) goto done;
    hr = IMediaSeeking_GetDuration(seeking, &duration);
    printf("GetDuration=%#lx duration=%lld\n", hr, duration);
    if (hr != S_OK || duration != 2500000) goto done;
    hr = IGraphBuilder_QueryInterface(graph, &IID_IMediaControl, (void **)&control);
    if (FAILED(hr)) goto done;
    hr = IGraphBuilder_QueryInterface(graph, &IID_IMediaEvent, (void **)&events);
    if (FAILED(hr)) goto done;
    hr = IMediaControl_Run(control);
    if (FAILED(hr)) goto done;
    hr = IMediaEvent_WaitForCompletion(events, 10000, &code);
    printf("WaitForCompletion=%#lx event=%ld\n", hr, code);
    if (hr != S_OK || code != EC_COMPLETE) goto done;
    failed = 0;
done:
    if (control) { IMediaControl_Stop(control); IMediaControl_Release(control); }
    if (events) IMediaEvent_Release(events);
    if (seeking) IMediaSeeking_Release(seeking);
    if (graph) IGraphBuilder_Release(graph);
    CoUninitialize();
    DeleteFileW(path);
    printf("WAV_PLAYBACK_%s (hr=%#lx)\n", failed ? "FAIL" : "PASS", hr);
    printf("%04lx:wavplayback: 1 tests executed (0 marked as todo, %d failures), 0 skipped\n",
           GetCurrentThreadId(), failed);
    return failed;
}
