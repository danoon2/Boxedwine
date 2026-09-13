#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A control for the Glide probe: exercise the backend without a Glide wrapper.
int main(int argc, char **argv) {
    BOOL testStateBlock = argc > 1 && !strcmp(argv[1], "state-block");
    setvbuf(stdout, NULL, _IONBF, 0);
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "BoxedwineD3D9Probe";
    RegisterClassA(&wc);
    HWND window = CreateWindowA(wc.lpszClassName, "D3D9 control diagnostic",
        WS_OVERLAPPEDWINDOW, 20, 20, 640, 480, NULL, NULL, wc.hInstance, NULL);
    if (!window) return 2;
    ShowWindow(window, SW_SHOW);
    IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d) { puts("D3D9_CREATE_FAILED"); DestroyWindow(window); return 3; }
    D3DPRESENT_PARAMETERS pp = {0};
    pp.Windowed = TRUE;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = window;
    pp.BackBufferCount = 1;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    IDirect3DDevice9 *device = NULL;
    HRESULT hr = IDirect3D9_CreateDevice(d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
        window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &device);
    printf("D3D9_DEVICE hr=%08lx\n", (unsigned long)hr);
    if (FAILED(hr)) { IDirect3D9_Release(d3d); DestroyWindow(window); return 4; }
    DWORD start = GetTickCount();
    unsigned frames = 0;
    MSG message;
    DWORD duration = argc > 2 ? (DWORD)strtoul(argv[2], NULL, 10) : 12000;
    while (GetTickCount() - start < duration) {
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message); DispatchMessageA(&message);
        }
        unsigned phase = ((GetTickCount() - start) / 4000) % 3;
        IDirect3DStateBlock9 *saved = NULL;
        if (testStateBlock) {
            hr = IDirect3DDevice9_CreateStateBlock(device, D3DSBT_ALL, &saved);
            if (FAILED(hr)) { printf("STATE_CAPTURE_FAILED hr=%08lx\n", (unsigned long)hr); break; }
            const float constants[] = {1.0f, 1.0f, 1.0f, 1.0f};
            IDirect3DDevice9_SetPixelShaderConstantF(device, 0, constants, 1);
            IDirect3DDevice9_SetFVF(device, D3DFVF_XYZRHW | D3DFVF_TEX1);
        }
        D3DCOLOR color = phase == 0 ? 0xff2060c0 : phase == 1 ? 0xff40b040 : 0xffc04030;
        hr = IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, color, 1.0f, 0);
        if (FAILED(hr)) {
            if (saved) IDirect3DStateBlock9_Release(saved);
            printf("D3D9_CLEAR_FAILED hr=%08lx\n", (unsigned long)hr); break;
        }
        if (saved) {
            if (frames < 5) printf("STATE_APPLY_BEGIN %u\n", frames);
            hr = IDirect3DStateBlock9_Apply(saved);
            if (frames < 5) printf("STATE_APPLY_END %u hr=%08lx\n", frames, (unsigned long)hr);
            IDirect3DStateBlock9_Release(saved);
            if (FAILED(hr)) break;
        }
        hr = IDirect3DDevice9_Present(device, NULL, NULL, NULL, NULL);
        if (FAILED(hr)) { printf("D3D9_PRESENT_FAILED hr=%08lx\n", (unsigned long)hr); break; }
        ++frames;
        Sleep(16);
    }
    IDirect3DDevice9_Release(device);
    IDirect3D9_Release(d3d);
    DestroyWindow(window);
    printf("%s frames=%u\n", FAILED(hr) ? "D3D9_FAILED" : "D3D9_API_PASS", frames);
    // Successful calls require visual confirmation; drivers can fail asynchronously.
    return FAILED(hr) ? 5 : 0;
}
