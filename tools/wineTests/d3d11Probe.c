/* Bounded D3D11/DXGI rendering, compute, query and resize checks.
 * Build: i686-w64-mingw32-gcc -O2 -Wall -Wextra -Werror d3d11Probe.c -luser32 -ldxguid
 */
#define COBJMACROS
#include <windows.h>
#include <d3d11.h>
#include <stdio.h>
#include <string.h>
#include "d3d11ProbeShaders.h"

static unsigned assertions, failures;
#define EXPECT(condition, ...) do { ++assertions; if (!(condition)) { ++failures; \
    printf("d3d11Probe.c:%d: Test failed: ", __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)
#define HR(call) do { HRESULT result = (call); EXPECT(SUCCEEDED(result), "%s -> %#lx", #call, (unsigned long)result); \
    if (FAILED(result)) goto cleanup; } while (0)
#define RELEASE(value, type) do { if (value) { type##_Release(value); value = NULL; } } while (0)

static void second_window(PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN create)
{
    ID3D11Device* device = NULL;
    ID3D11DeviceContext* context = NULL;
    IDXGISwapChain* swapchain = NULL;
    ID3D11Texture2D* back = NULL;
    ID3D11RenderTargetView* target = NULL;
    HWND window = CreateWindowA("STATIC", "Second Vulkan surface", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        240, 30, 128, 128, NULL, NULL, GetModuleHandleA(NULL), NULL);
    EXPECT(window != NULL, "second window unavailable");
    if (!window) goto cleanup;
    DXGI_SWAP_CHAIN_DESC swap = {0};
    swap.BufferDesc.Width = swap.BufferDesc.Height = 48;
    swap.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap.SampleDesc.Count = 1;
    swap.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap.BufferCount = 1;
    swap.OutputWindow = window;
    swap.Windowed = TRUE;
    D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_0;
    HR(create(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &level, 1, D3D11_SDK_VERSION,
        &swap, &swapchain, &device, NULL, &context));
    HR(IDXGISwapChain_GetBuffer(swapchain, 0, &IID_ID3D11Texture2D, (void**)&back));
    HR(ID3D11Device_CreateRenderTargetView(device, (ID3D11Resource*)back, NULL, &target));
    const FLOAT green[4] = {0, 1, 0, 1};
    ID3D11DeviceContext_ClearRenderTargetView(context, target, green);
    HR(IDXGISwapChain_Present(swapchain, 0, 0));
cleanup:
    if (context) { ID3D11DeviceContext_ClearState(context); ID3D11DeviceContext_Flush(context); }
    RELEASE(target, ID3D11RenderTargetView);
    RELEASE(back, ID3D11Texture2D);
    RELEASE(swapchain, IDXGISwapChain);
    RELEASE(context, ID3D11DeviceContext);
    if (device) EXPECT(ID3D11Device_Release(device) == 0, "second device references remain");
    if (window) DestroyWindow(window);
}

int main(void)
{
    HMODULE d3d = LoadLibraryA("d3d11.dll");
    PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN create = d3d ?
        (void*)GetProcAddress(d3d, "D3D11CreateDeviceAndSwapChain") : NULL;
    EXPECT(create != NULL, "D3D11 unavailable");
    ID3D11Device* device = NULL;
    ID3D11DeviceContext* context = NULL;
    IDXGISwapChain* swapchain = NULL;
    ID3D11Texture2D *back = NULL, *staging = NULL;
    ID3D11RenderTargetView* target = NULL;
    ID3D11VertexShader* vs = NULL;
    ID3D11PixelShader* ps = NULL;
    ID3D11ComputeShader* cs = NULL;
    ID3D11Buffer *buffer = NULL, *readback = NULL;
    ID3D11UnorderedAccessView* uav = NULL;
    ID3D11Query* query = NULL;
    HWND window = NULL;
    if (!create) goto cleanup;
    window = CreateWindowA("STATIC", "BoxedWine D3D11 probe", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        30, 30, 160, 160, NULL, NULL, GetModuleHandleA(NULL), NULL);
    EXPECT(window != NULL, "window creation failed");
    if (!window) goto cleanup;
    DXGI_SWAP_CHAIN_DESC swap = {0};
    swap.BufferDesc.Width = swap.BufferDesc.Height = 32;
    swap.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap.SampleDesc.Count = 1;
    swap.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap.BufferCount = 1;
    swap.OutputWindow = window;
    swap.Windowed = TRUE;
    swap.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_0, actual;
    HR(create(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &level, 1, D3D11_SDK_VERSION,
        &swap, &swapchain, &device, &actual, &context));
    EXPECT(actual == level, "unexpected feature level %#x", actual);
    HR(ID3D11Device_CreateVertexShader(device, probeVS, sizeof(probeVS), NULL, &vs));
    HR(ID3D11Device_CreatePixelShader(device, probePS, sizeof(probePS), NULL, &ps));
    HR(ID3D11Device_CreateComputeShader(device, probeCS, sizeof(probeCS), NULL, &cs));
    D3D11_BUFFER_DESC bufferInfo = {0};
    bufferInfo.ByteWidth = 128;
    bufferInfo.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    bufferInfo.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferInfo.StructureByteStride = 4;
    HR(ID3D11Device_CreateBuffer(device, &bufferInfo, NULL, &buffer));
    HR(ID3D11Device_CreateUnorderedAccessView(device, (ID3D11Resource*)buffer, NULL, &uav));
    bufferInfo.BindFlags = bufferInfo.MiscFlags = bufferInfo.StructureByteStride = 0;
    bufferInfo.Usage = D3D11_USAGE_STAGING;
    bufferInfo.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    HR(ID3D11Device_CreateBuffer(device, &bufferInfo, NULL, &readback));
    ID3D11DeviceContext_CSSetShader(context, cs, NULL, 0);
    ID3D11DeviceContext_CSSetUnorderedAccessViews(context, 0, 1, &uav, NULL);
    ID3D11DeviceContext_Dispatch(context, 1, 1, 1);
    ID3D11DeviceContext_CopyResource(context, (ID3D11Resource*)readback, (ID3D11Resource*)buffer);
    D3D11_MAPPED_SUBRESOURCE mapped;
    HR(ID3D11DeviceContext_Map(context, (ID3D11Resource*)readback, 0, D3D11_MAP_READ, 0, &mapped));
    unsigned i, frame;
    for (i = 0; i < 32; ++i)
        EXPECT(((const UINT*)mapped.pData)[i] == (0x12345678u ^ (i * 79)), "compute word %u: %#x", i, ((const UINT*)mapped.pData)[i]);
    ID3D11DeviceContext_Unmap(context, (ID3D11Resource*)readback, 0);
    D3D11_QUERY_DESC queryInfo = {D3D11_QUERY_EVENT, 0};
    HR(ID3D11Device_CreateQuery(device, &queryInfo, &query));
    ID3D11DeviceContext_IASetPrimitiveTopology(context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ID3D11DeviceContext_VSSetShader(context, vs, NULL, 0);
    ID3D11DeviceContext_PSSetShader(context, ps, NULL, 0);
    for (frame = 0; frame < 3; ++frame) {
        UINT size = 32 * (frame + 1);
        if (frame == 1) second_window(create);
        if (frame == 2) {
            ShowWindow(window, SW_MINIMIZE);
            ShowWindow(window, SW_RESTORE);
        }
        if (frame) {
            SetWindowPos(window, NULL, 30, 30, size + 40, size + 60, SWP_NOZORDER);
            HR(IDXGISwapChain_ResizeBuffers(swapchain, 1, size, size, DXGI_FORMAT_UNKNOWN, 0));
        }
        HR(IDXGISwapChain_GetBuffer(swapchain, 0, &IID_ID3D11Texture2D, (void**)&back));
        HR(ID3D11Device_CreateRenderTargetView(device, (ID3D11Resource*)back, NULL, &target));
        D3D11_TEXTURE2D_DESC texture;
        ID3D11Texture2D_GetDesc(back, &texture);
        EXPECT(texture.Width == size && texture.Height == size, "backbuffer size %ux%u", texture.Width, texture.Height);
        texture.Usage = D3D11_USAGE_STAGING;
        texture.BindFlags = texture.MiscFlags = 0;
        texture.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        HR(ID3D11Device_CreateTexture2D(device, &texture, NULL, &staging));
        ID3D11DeviceContext_OMSetRenderTargets(context, 1, &target, NULL);
        D3D11_VIEWPORT viewport = {0, 0, (FLOAT)size, (FLOAT)size, 0, 1};
        ID3D11DeviceContext_RSSetViewports(context, 1, &viewport);
        const FLOAT red[4] = {1, 0, 0, 1};
        ID3D11DeviceContext_ClearRenderTargetView(context, target, red);
        ID3D11DeviceContext_Draw(context, 3, 0);
        ID3D11DeviceContext_CopyResource(context, (ID3D11Resource*)staging, (ID3D11Resource*)back);
        HR(ID3D11DeviceContext_Map(context, (ID3D11Resource*)staging, 0, D3D11_MAP_READ, 0, &mapped));
        UINT x, y;
        for (y = 0; y < size; ++y) for (x = 0; x < size; ++x) {
            UINT pixel = *(const UINT*)((const char*)mapped.pData + y * mapped.RowPitch + x * 4);
            EXPECT(pixel == 0xffffff00, "frame %u pixel %u,%u: %#x", frame, x, y, pixel);
        }
        ID3D11DeviceContext_Unmap(context, (ID3D11Resource*)staging, 0);
        ID3D11DeviceContext_End(context, (ID3D11Asynchronous*)query);
        ID3D11DeviceContext_Flush(context);
        BOOL complete = FALSE;
        HRESULT status = S_FALSE;
        for (i = 0; i < 10000 && status == S_FALSE; ++i) {
            status = ID3D11DeviceContext_GetData(context, (ID3D11Asynchronous*)query, &complete, sizeof(complete), 0);
            if (status == S_FALSE) Sleep(1);
        }
        EXPECT(status == S_OK && complete, "event query did not finish: %#lx", (unsigned long)status);
        HR(IDXGISwapChain_Present(swapchain, 0, 0));
        ID3D11DeviceContext_OMSetRenderTargets(context, 0, NULL, NULL);
        RELEASE(staging, ID3D11Texture2D);
        RELEASE(target, ID3D11RenderTargetView);
        RELEASE(back, ID3D11Texture2D);
    }
cleanup:
    if (context) { ID3D11DeviceContext_ClearState(context); ID3D11DeviceContext_Flush(context); }
    RELEASE(query, ID3D11Query);
    RELEASE(staging, ID3D11Texture2D);
    RELEASE(target, ID3D11RenderTargetView);
    RELEASE(back, ID3D11Texture2D);
    RELEASE(uav, ID3D11UnorderedAccessView);
    RELEASE(readback, ID3D11Buffer);
    RELEASE(buffer, ID3D11Buffer);
    RELEASE(cs, ID3D11ComputeShader);
    RELEASE(ps, ID3D11PixelShader);
    RELEASE(vs, ID3D11VertexShader);
    RELEASE(swapchain, IDXGISwapChain);
    RELEASE(context, ID3D11DeviceContext);
    if (device) EXPECT(ID3D11Device_Release(device) == 0, "device references remain");
    if (window) DestroyWindow(window);
    if (d3d) FreeLibrary(d3d);
    printf("0000:probe: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", assertions, failures);
    return failures > 255 ? 255 : failures;
}
