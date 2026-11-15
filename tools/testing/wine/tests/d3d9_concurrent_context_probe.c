#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>

#define WORKERS 2
#define ROUNDS 64
#define COPIES 4
#define SIZE 32

struct vertex { float x, y, z, rhw; DWORD color; };
struct worker
{
    unsigned int index, tests, failures, completed;
    HANDLE thread, ready, go, issued, read_go;
    HWND window;
    IDirect3D9 *d3d;
    IDirect3DDevice9 *device;
    IDirect3DSurface9 *source, *copy, *readback;
    unsigned int round;
};
static HANDLE abort_event;

static int check(struct worker *w, int value, unsigned int line, const char *message)
{
    ++w->tests;
    if (value) return 1;
    ++w->failures;
    printf("concurrent_context.c:%u: Test failed: worker %u round %u: %s.\n",
            line, w->index, w->round, message);
    SetEvent(abort_event);
    return 0;
}
#define CHECK(w,c,m) check(w, !!(c), __LINE__, m)
#define REQUIRE(w,c,m) do { if (!CHECK(w,c,m)) goto done; } while (0)

static int wait_signal(HANDLE event)
{
    HANDLE handles[2] = {abort_event, event};
    return WaitForMultipleObjects(2, handles, FALSE, 60000) == WAIT_OBJECT_0 + 1;
}

static int initialize(struct worker *w)
{
    D3DPRESENT_PARAMETERS pp = {0};
    RECT scissor = {16, 0, SIZE, SIZE};
    HRESULT hr;
    w->window = CreateWindowA("static", "Concurrent Direct3D devices", WS_OVERLAPPEDWINDOW,
            w->index * 100, 0, 96, 96, NULL, NULL, NULL, NULL);
    if (!CHECK(w, w->window, "create owning-thread window")) return 0;
    w->d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!CHECK(w, w->d3d, "create Direct3D")) return 0;
    pp.BackBufferWidth = pp.BackBufferHeight = SIZE;
    pp.BackBufferFormat = D3DFMT_A8R8G8B8;
    pp.BackBufferCount = 1;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = w->window;
    pp.Windowed = TRUE;
    /* Each device and its focus window are owned and used by one thread. */
    hr = IDirect3D9_CreateDevice(w->d3d, 0, D3DDEVTYPE_HAL, w->window,
            D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &w->device);
    if (!CHECK(w, SUCCEEDED(hr), "create device on its owning thread"))
    {
        printf("CREATE_DEVICE_HR worker=%u hr=%#lx\n", w->index, hr);
        return 0;
    }
    if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_CreateRenderTarget(w->device, SIZE, SIZE,
            D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &w->source, NULL)), "create source")) return 0;
    if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_CreateRenderTarget(w->device, SIZE, SIZE,
            D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &w->copy, NULL)), "create copy target")) return 0;
    if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_CreateOffscreenPlainSurface(w->device, SIZE, SIZE,
            D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &w->readback, NULL)), "create readback")) return 0;
    if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_SetRenderTarget(w->device, 0, w->source)), "select source")) return 0;
    if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_SetRenderState(w->device, D3DRS_ZENABLE, FALSE)), "disable depth")) return 0;
    if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_SetRenderState(w->device, D3DRS_LIGHTING, FALSE)), "disable lighting")) return 0;
    if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_SetRenderState(w->device, D3DRS_CULLMODE, D3DCULL_NONE)), "disable culling")) return 0;
    if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_SetRenderState(w->device, D3DRS_ALPHABLENDENABLE, FALSE)), "disable blending")) return 0;
    if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_SetFVF(w->device, D3DFVF_XYZRHW | D3DFVF_DIFFUSE)), "set vertex format")) return 0;
    return CHECK(w, SUCCEEDED(IDirect3DDevice9_SetScissorRect(w->device, &scissor)), "set independent scissor");
}

static int issue_copies(struct worker *w, DWORD color)
{
    struct vertex quad[] = {{0,0,.5f,1,color}, {0,SIZE,.5f,1,color},
            {SIZE,0,.5f,1,color}, {SIZE,SIZE,.5f,1,color}};
    unsigned int iteration;
    for (iteration = 0; iteration < COPIES; ++iteration)
    {
        if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_SetRenderState(w->device, D3DRS_SCISSORTESTENABLE, FALSE)), "disable scissor for clear")) return 0;
        if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_Clear(w->device, 0, NULL, D3DCLEAR_TARGET, 0xff101010, 1, 0)), "clear source")) return 0;
        if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_SetRenderState(w->device, D3DRS_SCISSORTESTENABLE, w->index != 0)), "restore worker scissor")) return 0;
        if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_BeginScene(w->device)), "begin scene")) return 0;
        if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_DrawPrimitiveUP(w->device, D3DPT_TRIANGLESTRIP, 2, quad, sizeof(*quad))), "draw source")) return 0;
        if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_EndScene(w->device)), "end scene")) return 0;
        if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_StretchRect(w->device, w->source, NULL,
                w->copy, NULL, D3DTEXF_NONE)), "copy between distinct surfaces")) return 0;
        Sleep(0);
    }
    return 1;
}

static int verify_pixels(struct worker *w, DWORD color)
{
    const unsigned int positions[2] = {8, 24};
    D3DLOCKED_RECT locked;
    unsigned int x, y;
    int good = 1;
    if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_GetRenderTargetData(w->device, w->copy, w->readback)), "read copied target")) return 0;
    if (!CHECK(w, SUCCEEDED(IDirect3DSurface9_LockRect(w->readback, &locked, NULL, D3DLOCK_READONLY)), "lock readback")) return 0;
    for (y = 0; y < 2; ++y) for (x = 0; x < 2; ++x)
    {
        DWORD actual = ((DWORD *)((BYTE *)locked.pBits + positions[y] * locked.Pitch))[positions[x]] & 0xffffff;
        DWORD expected = w->index && positions[x] < 16 ? 0x101010 : color & 0xffffff;
        if (!CHECK(w, actual == expected, "independent draw/copy pixels"))
        {
            printf("PIXEL worker=%u round=%u x=%u y=%u expected=%06lx actual=%06lx\n",
                    w->index, w->round, positions[x], positions[y], expected, actual);
            good = 0;
        }
    }
    return CHECK(w, SUCCEEDED(IDirect3DSurface9_UnlockRect(w->readback)), "unlock readback") && good;
}

static DWORD WINAPI render_thread(void *parameter)
{
    struct worker *w = parameter;
    DWORD color;
    if (!initialize(w)) goto done;
    for (w->round = 0; w->round < ROUNDS; ++w->round)
    {
        REQUIRE(w, SetEvent(w->ready), "signal ready");
        if (!wait_signal(w->go)) goto done;
        color = 0xff000000 | ((32 + (w->round * 13 + w->index * 83) % 192) << 16)
                | ((32 + (w->round * 29 + w->index * 59) % 192) << 8)
                | (32 + (w->round * 47 + w->index * 37) % 192);
        if (!issue_copies(w, color)) goto done;
        REQUIRE(w, SetEvent(w->issued), "signal batch submitted");
        if (!wait_signal(w->read_go)) goto done;
        if (!verify_pixels(w, color)) goto done;
        ++w->completed;
    }
done:
    if (w->completed != ROUNDS) SetEvent(abort_event);
    if (w->readback) IDirect3DSurface9_Release(w->readback);
    if (w->copy) IDirect3DSurface9_Release(w->copy);
    if (w->source) IDirect3DSurface9_Release(w->source);
    if (w->device) IDirect3DDevice9_Release(w->device);
    if (w->d3d) IDirect3D9_Release(w->d3d);
    if (w->window) CHECK(w, DestroyWindow(w->window), "destroy owning-thread window");
    return w->failures || w->completed != ROUNDS;
}

int main(void)
{
    struct worker workers[WORKERS] = {0};
    unsigned int i, round, count = 0, tests = 0, failures = 0;
    DWORD status;
    HANDLE threads[WORKERS];
    abort_event = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (!abort_event) return 2;
    for (i = 0; i < WORKERS; ++i)
    {
        struct worker *w = &workers[i];
        w->index = i;
        w->ready = CreateEventA(NULL, FALSE, FALSE, NULL);
        w->go = CreateEventA(NULL, FALSE, FALSE, NULL);
        w->issued = CreateEventA(NULL, FALSE, FALSE, NULL);
        w->read_go = CreateEventA(NULL, FALSE, FALSE, NULL);
        if (!w->ready || !w->go || !w->issued || !w->read_go) goto failed;
        w->thread = CreateThread(NULL, 0, render_thread, w, 0, NULL);
        if (!w->thread) goto failed;
        threads[count++] = w->thread;
    }
    for (round = 0; round < ROUNDS; ++round)
    {
        for (i = 0; i < WORKERS; ++i) if (!wait_signal(workers[i].ready)) goto failed;
        for (i = 0; i < WORKERS; ++i) if (!SetEvent(workers[i].go)) goto failed;
        /* Both command batches must be submitted before either readback. */
        for (i = 0; i < WORKERS; ++i) if (!wait_signal(workers[i].issued)) goto failed;
        for (i = 0; i < WORKERS; ++i) if (!SetEvent(workers[i].read_go)) goto failed;
    }
    goto done;
failed:
    ++failures;
    printf("concurrent_context.c:%u: Test failed: coordinator aborted.\n", __LINE__);
    SetEvent(abort_event);
done:
    if (count && WaitForMultipleObjects(count, threads, TRUE, 60000) != WAIT_OBJECT_0)
    {
        printf("concurrent_context.c:%u: Test failed: rendering threads did not exit.\n", __LINE__);
        return 2;
    }
    for (i = 0; i < WORKERS; ++i)
    {
        struct worker *w = &workers[i];
        tests += w->tests;
        failures += w->failures;
        if (w->thread)
        {
            ++tests;
            if (!GetExitCodeThread(w->thread, &status) || status || w->completed != ROUNDS)
            {
                ++failures;
                printf("concurrent_context.c:%u: Test failed: worker %u incomplete.\n", __LINE__, i);
            }
            CloseHandle(w->thread);
        }
        if (w->ready) CloseHandle(w->ready);
        if (w->go) CloseHandle(w->go);
        if (w->issued) CloseHandle(w->issued);
        if (w->read_go) CloseHandle(w->read_go);
    }
    CloseHandle(abort_event);
    printf("CONCURRENT_CONTEXT_PROBE workers=%u rounds=%u copies=%u completed=%u,%u\n",
            WORKERS, ROUNDS, WORKERS * ROUNDS * COPIES, workers[0].completed, workers[1].completed);
    printf("0000:concurrent: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", tests, failures);
    return failures ? 1 : 0;
}
