#define COBJMACROS
#include <windows.h>
#include <dshow.h>
#include <stdio.h>

static DWORD WINAPI watchdog(void *unused)
{
    Sleep(60000);
    puts("FAIL: graph operation timed out");
    fflush(stdout);
    ExitProcess(2);
    return 0;
}

int wmain(int argc, WCHAR **argv)
{
    const CLSID reader_clsid = {0x187463a0,0x5bb7,0x11d3,{0xac,0xbe,0,0x80,0xc7,0x5e,0x24,0x6e}};
    IGraphBuilder *graph = NULL;
    IBaseFilter *reader = NULL;
    IFileSourceFilter *source = NULL;
    IMediaSeeking *seeking = NULL;
    IMediaControl *control = NULL;
    IMediaEvent *event = NULL;
    IEnumPins *pins = NULL;
    IPin *pin;
    WCHAR path[MAX_PATH];
    HRESULT hr;
    int failures = 0, count = 0, pass;
    HANDLE thread;
#define REQUIRE(expr) do { hr = (expr); printf("%s: %08lx\n", #expr, hr); fflush(stdout); if (FAILED(hr)) { ++failures; goto done; } } while (0)
    setvbuf(stdout, NULL, _IONBF, 0);
    thread = CreateThread(NULL, 0, watchdog, NULL, 0, NULL);
    if (!thread) return 3;
    CloseHandle(thread);
    CoInitialize(NULL);
    GetFullPathNameW(argc > 1 ? argv[1] : L"data\\intro\\atvi_logo.wmv", MAX_PATH, path, NULL);
    REQUIRE(CoCreateInstance(&CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, &IID_IGraphBuilder, (void **)&graph));
    REQUIRE(CoCreateInstance(&reader_clsid, NULL, CLSCTX_INPROC_SERVER, &IID_IBaseFilter, (void **)&reader));
    REQUIRE(IBaseFilter_QueryInterface(reader, &IID_IFileSourceFilter, (void **)&source));
    REQUIRE(IFileSourceFilter_Load(source, path, NULL));
    REQUIRE(IGraphBuilder_AddFilter(graph, reader, L"ASF reader"));
    REQUIRE(IBaseFilter_EnumPins(reader, &pins));
    while (IEnumPins_Next(pins, 1, &pin, NULL) == S_OK)
    {
        hr = IGraphBuilder_Render(graph, pin);
        IPin_Release(pin);
        printf("Render output %d: %08lx\n", count++, hr);
        if (FAILED(hr)) { ++failures; goto done; }
    }
    if (count != 2) { puts("FAIL: expected audio and video outputs"); ++failures; goto done; }
    REQUIRE(IGraphBuilder_QueryInterface(graph, &IID_IMediaSeeking, (void **)&seeking));
    REQUIRE(IGraphBuilder_QueryInterface(graph, &IID_IMediaControl, (void **)&control));
    REQUIRE(IGraphBuilder_QueryInterface(graph, &IID_IMediaEvent, (void **)&event));
    for (pass = 0; pass < 2; ++pass)
    {
        LONGLONG current = pass ? 20000000 : 0;
        LONG code = 0;
        OAFilterState state;
        printf("Stopped seek/start pass %d\n", pass);
        REQUIRE(IMediaSeeking_SetPositions(seeking, &current, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning));
        REQUIRE(IMediaControl_Pause(control));
        REQUIRE(IMediaControl_GetState(control, 5000, &state));
        if (hr != S_OK || state != State_Paused) { puts("FAIL: graph never finished preroll"); ++failures; goto done; }
        REQUIRE(IMediaControl_Run(control));
        if (pass)
        {
            Sleep(750);
            current = 0;
            REQUIRE(IMediaSeeking_SetPositions(seeking, &current, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning));
        }
        REQUIRE(IMediaEvent_WaitForCompletion(event, 20000, &code));
        printf("Completion event: %ld\n", code);
        if (code != EC_COMPLETE) ++failures;
        REQUIRE(IMediaControl_Stop(control));
    }
done:
    if (control) IMediaControl_Stop(control);
    if (pins) IEnumPins_Release(pins);
    if (event) IMediaEvent_Release(event);
    if (control) IMediaControl_Release(control);
    if (seeking) IMediaSeeking_Release(seeking);
    if (source) IFileSourceFilter_Release(source);
    if (reader) IBaseFilter_Release(reader);
    if (graph) IGraphBuilder_Release(graph);
    CoUninitialize();
    printf("Reader graph probe: %d failures\n", failures);
    return failures;
}
