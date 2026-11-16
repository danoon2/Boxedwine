/* Generate an uncompressed AVI and verify DirectShow demuxing, a decoded frame,
 * duration and completion. No proprietary codecs or game data are required.
 * i686-w64-mingw32-gcc -O2 avi-playback.c -o avi-playback.exe -lavifil32 -lole32 -lstrmiids -luuid
 */
#define COBJMACROS
#include <windows.h>
#include <vfw.h>
#include <dshow.h>
#include <stdio.h>

int main(void)
{
    WCHAR temp[MAX_PATH], path[MAX_PATH];
    PAVIFILE file = NULL;
    PAVISTREAM stream = NULL;
    AVISTREAMINFOW info = {0};
    BITMAPINFOHEADER bitmap = {0};
    unsigned char pixels[16 * 16 * 3];
    IGraphBuilder *graph = NULL;
    IBaseFilter *renderer = NULL;
    IMediaControl *control = NULL;
    IMediaSeeking *seeking = NULL;
    IMediaEvent *events = NULL;
    IBasicVideo *video = NULL;
    IVideoWindow *window = NULL;
    LONGLONG duration = 0;
    LONG width = 0, height = 0, size = 0, event = 0;
    OAFilterState state;
    HRESULT hr = E_FAIL;
    int i, failed = 1;
    BYTE *frame = NULL;

    setvbuf(stdout, NULL, _IONBF, 0);
    if (!GetTempPathW(MAX_PATH, temp) || !GetTempFileNameW(temp, L"avi", 0, path)) return 1;
    DeleteFileW(path);
    if (lstrlenW(path) + 4 >= MAX_PATH) return 1;
    memcpy(path + lstrlenW(path), L".avi", sizeof(L".avi"));
    AVIFileInit();
    hr = AVIFileOpenW(&file, path, OF_CREATE | OF_WRITE, NULL);
    if (FAILED(hr)) goto done_avi;
    info.fccType = streamtypeVIDEO; info.fccHandler = mmioFOURCC('D','I','B',' ');
    info.dwScale = 1; info.dwRate = 5; info.dwSuggestedBufferSize = sizeof(pixels);
    SetRect(&info.rcFrame, 0, 0, 16, 16);
    hr = AVIFileCreateStreamW(file, &stream, &info);
    if (FAILED(hr)) goto done_avi;
    bitmap.biSize = sizeof(bitmap); bitmap.biWidth = bitmap.biHeight = 16;
    bitmap.biPlanes = 1; bitmap.biBitCount = 24; bitmap.biSizeImage = sizeof(pixels);
    hr = AVIStreamSetFormat(stream, 0, &bitmap, sizeof(bitmap));
    if (FAILED(hr)) goto done_avi;
    for (i = 0; i < (int)sizeof(pixels); i += 3) { pixels[i] = pixels[i+1] = 0; pixels[i+2] = 255; }
    for (i = 0; i < 10; ++i) {
        hr = AVIStreamWrite(stream, i, 1, pixels, sizeof(pixels), AVIIF_KEYFRAME, NULL, NULL);
        if (FAILED(hr)) goto done_avi;
    }
done_avi:
    if (stream) AVIStreamRelease(stream);
    if (file) AVIFileRelease(file);
    AVIFileExit();
    printf("CreateAVI=%#lx\n", hr);
    if (FAILED(hr)) { DeleteFileW(path); return 1; }
    hr = CoInitialize(NULL);
    if (FAILED(hr)) { DeleteFileW(path); return 1; }
    hr = CoCreateInstance(&CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, &IID_IGraphBuilder, (void **)&graph);
    if (FAILED(hr)) goto done;
    /* Keep the parser test independent of D3D/VMR renderer selection. */
    hr = CoCreateInstance(&CLSID_VideoRenderer, NULL, CLSCTX_INPROC_SERVER, &IID_IBaseFilter, (void **)&renderer);
    if (FAILED(hr)) goto done;
    hr = IGraphBuilder_AddFilter(graph, renderer, L"Video Renderer");
    if (FAILED(hr)) goto done;
    hr = IGraphBuilder_RenderFile(graph, path, NULL);
    printf("RenderFile=%#lx\n", hr);
    if (hr != S_OK) goto done;
#define GET_INTERFACE(id, target) do { hr = IGraphBuilder_QueryInterface(graph, &IID_##id, (void **)&target); if (FAILED(hr)) goto done; } while (0)
    GET_INTERFACE(IMediaSeeking, seeking); GET_INTERFACE(IMediaControl, control);
    GET_INTERFACE(IMediaEvent, events); GET_INTERFACE(IBasicVideo, video); GET_INTERFACE(IVideoWindow, window);
    IVideoWindow_put_AutoShow(window, OAFALSE);
    hr = IMediaSeeking_GetDuration(seeking, &duration);
    printf("Duration=%lld hr=%#lx\n", duration, hr);
    if (hr != S_OK || duration != 20000000) goto done;
    hr = IBasicVideo_GetVideoSize(video, &width, &height);
    if (hr != S_OK || width != 16 || height != 16) goto done;
    hr = IMediaControl_Pause(control);
    if (FAILED(hr)) goto done;
    hr = IMediaControl_GetState(control, 10000, &state);
    if (hr != S_OK || state != State_Paused) goto done;
    /* The graph's IBasicVideo proxy in Wine requires a non-null image buffer.
     * This known-size fixture fits any negotiated RGB16/24/32 format. */
    size = sizeof(BITMAPINFOHEADER) + 16*16*4;
    frame = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
    if (!frame) goto done;
    hr = IBasicVideo_GetCurrentImage(video, &size, (LONG *)frame);
    if (hr != S_OK) goto done;
    {
        BITMAPINFOHEADER *header = (BITMAPINFOHEADER *)frame;
        BYTE *rgb = frame + header->biSize;
        BOOL red = header->biBitCount == 24 || header->biBitCount == 32
            ? rgb[0] == 0 && rgb[1] == 0 && rgb[2] == 255
            : header->biBitCount == 16 && (*(WORD *)rgb == 0x7c00 || *(WORD *)rgb == 0xf800);
        printf("Decoded frame=%ldx%ld bpp=%u red=%d\n", width, height, header->biBitCount, red);
        if (!red) goto done;
    }
    hr = IMediaControl_Run(control);
    if (FAILED(hr)) goto done;
    hr = IMediaEvent_WaitForCompletion(events, 15000, &event);
    printf("Completion=%#lx event=%ld\n", hr, event);
    if (hr != S_OK || event != EC_COMPLETE) goto done;
    failed = 0;
done:
    if (frame) HeapFree(GetProcessHeap(), 0, frame);
    if (control) { IMediaControl_Stop(control); IMediaControl_Release(control); }
    if (window) IVideoWindow_Release(window);
    if (video) IBasicVideo_Release(video);
    if (events) IMediaEvent_Release(events);
    if (seeking) IMediaSeeking_Release(seeking);
    if (graph) IGraphBuilder_Release(graph);
    if (renderer) IBaseFilter_Release(renderer);
    CoUninitialize(); DeleteFileW(path);
    printf("AVI_PLAYBACK_%s (hr=%#lx)\n", failed ? "FAIL" : "PASS", hr);
    return failed;
}
