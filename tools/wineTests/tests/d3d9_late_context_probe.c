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
    unsigned int index, tests, failures, completed, copies;
    HANDLE thread, ready, go, issued, read_go;
    HWND window;
    IDirect3D9 *d3d;
    IDirect3DDevice9 *device;
    IDirect3DVertexBuffer9 *vb;
    IDirect3DSurface9 *source, *copy, *readback;
    unsigned int round;
};
static HANDLE abort_event, late_create;
static int use_vertex_buffer;
static unsigned int total_tests, total_failures;

static int check(struct worker *w, int value, unsigned int line, const char *message)
{
    ++w->tests;
    if (value) return 1;
    ++w->failures;
    printf("late_context.c:%u: Test failed: worker %u round %u: %s.\n",
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
    void *mapped;
    struct vertex vertices[] = {{0,0,.5f,1,0xff20e060}, {0,SIZE,.5f,1,0xff20e060},
            {SIZE,0,.5f,1,0xff20e060}, {SIZE,SIZE,.5f,1,0xff20e060}};
    w->window = CreateWindowA("static", "Late Direct3D device initialization", WS_OVERLAPPEDWINDOW,
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
    if (use_vertex_buffer)
    {
        if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_CreateVertexBuffer(w->device, sizeof(vertices),
                D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &w->vb, NULL)), "create stable vertex buffer")) return 0;
        if (!CHECK(w, SUCCEEDED(IDirect3DVertexBuffer9_Lock(w->vb, 0, sizeof(vertices), &mapped, 0)), "lock vertex buffer")) return 0;
        memcpy(mapped, vertices, sizeof(vertices));
        if (!CHECK(w, SUCCEEDED(IDirect3DVertexBuffer9_Unlock(w->vb)), "unlock vertex buffer")) return 0;
        if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_SetStreamSource(w->device, 0, w->vb, 0, sizeof(*vertices))), "bind vertex buffer once")) return 0;
    }
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
        if (!CHECK(w, SUCCEEDED((use_vertex_buffer ? IDirect3DDevice9_DrawPrimitive(w->device, D3DPT_TRIANGLESTRIP, 0, 2)
                : IDirect3DDevice9_DrawPrimitiveUP(w->device, D3DPT_TRIANGLESTRIP, 2, quad, sizeof(*quad)))), "draw source")) return 0;
        if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_EndScene(w->device)), "end scene")) return 0;
        if (!CHECK(w, SUCCEEDED(IDirect3DDevice9_StretchRect(w->device, w->source, NULL,
                w->copy, NULL, D3DTEXF_NONE)), "copy between distinct surfaces")) return 0;
        ++w->copies;
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
    if (w->index && !wait_signal(late_create)) goto done;
    if (!initialize(w)) goto done;
    if (!w->index)
    {
        if (!issue_copies(w, 0xff20e060) || !verify_pixels(w, 0xff20e060)) goto done;
        REQUIRE(w, SetEvent(late_create), "release late device creation after completed first draw");
    }
    if (w->index)
    {
        REQUIRE(w, SetEvent(w->ready), "idle secondary device initialized");
        if (!wait_signal(w->go)) goto done;
        w->completed = 1;
        goto done;
    }
    for (w->round = 0; w->round < ROUNDS; ++w->round)
    {
        REQUIRE(w, SetEvent(w->ready), "signal ready");
        if (!wait_signal(w->go)) goto done;
        color = 0xff000000 | ((32 + (w->round * 13 + w->index * 83) % 192) << 16)
                | ((32 + (w->round * 29 + w->index * 59) % 192) << 8)
                | (32 + (w->round * 47 + w->index * 37) % 192);
        if (use_vertex_buffer) color = 0xff20e060;
        if (!issue_copies(w, color)) goto done;
        REQUIRE(w, SetEvent(w->issued), "signal batch submitted");
        if (!wait_signal(w->read_go)) goto done;
        if (!verify_pixels(w, color)) goto done;
        ++w->completed;
    }
done:
    if (w->completed != (w->index ? 1 : ROUNDS)) SetEvent(abort_event);
    if (w->readback) IDirect3DSurface9_Release(w->readback);
    if (w->copy) IDirect3DSurface9_Release(w->copy);
    if (w->source) IDirect3DSurface9_Release(w->source);
    if (w->vb) IDirect3DVertexBuffer9_Release(w->vb);
    if (w->device) IDirect3DDevice9_Release(w->device);
    if (w->d3d) IDirect3D9_Release(w->d3d);
    if (w->window) CHECK(w, DestroyWindow(w->window), "destroy owning-thread window");
    return w->failures || w->completed != (w->index ? 1 : ROUNDS);
}

static int run_phase(void)
{
    struct worker workers[WORKERS] = {0};
    unsigned int i, round, count = 0, tests = 0, failures = 0;
    DWORD status;
    HANDLE threads[WORKERS];
    abort_event = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (!abort_event) return 2;
    late_create = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (!late_create) return 2;
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
    if (!wait_signal(workers[1].ready)) goto failed;
    for (round = 0; round < ROUNDS; ++round)
    {
        for (i = 0; i < 1; ++i) if (!wait_signal(workers[i].ready)) goto failed;
        for (i = 0; i < 1; ++i) if (!SetEvent(workers[i].go)) goto failed;
        /* Keep the secondary device idle while the first submits and reads back. */
        for (i = 0; i < 1; ++i) if (!wait_signal(workers[i].issued)) goto failed;
        for (i = 0; i < 1; ++i) if (!SetEvent(workers[i].read_go)) goto failed;
    }
    SetEvent(workers[1].go);
    goto done;
failed:
    ++failures;
    printf("late_context.c:%u: Test failed: coordinator aborted.\n", __LINE__);
    SetEvent(abort_event);
done:
    if (count && WaitForMultipleObjects(count, threads, TRUE, 60000) != WAIT_OBJECT_0)
    {
        printf("late_context.c:%u: Test failed: rendering threads did not exit.\n", __LINE__);
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
            if (!GetExitCodeThread(w->thread, &status) || status || w->completed != (w->index ? 1 : ROUNDS))
            {
                ++failures;
                printf("late_context.c:%u: Test failed: worker %u incomplete.\n", __LINE__, i);
            }
            CloseHandle(w->thread);
        }
        if (w->ready) CloseHandle(w->ready);
        if (w->go) CloseHandle(w->go);
        if (w->issued) CloseHandle(w->issued);
        if (w->read_go) CloseHandle(w->read_go);
    }
    CloseHandle(abort_event);
    CloseHandle(late_create);
    printf("LATE_CONTEXT_PHASE vertex_buffer=%u rounds=%u completed=%u copies=%u idle_secondary_stopped=%u tests=%u failures=%u\n",
            use_vertex_buffer, ROUNDS, workers[0].completed, workers[0].copies, workers[1].completed, tests, failures);
    total_tests += tests;
    total_failures += failures;
    return failures ? 1 : 0;
}

int main(void)
{
    int status = 0;
    /* The second device stays idle after initialization. Drawing on it would
     * hide the stale-state path that follows its capability probes and setup. */
    for (use_vertex_buffer = 0; use_vertex_buffer < 2; ++use_vertex_buffer)
    {
        int result = run_phase();
        if (result == 2) return result;
        status |= result;
    }
    printf("0000:latecontext: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n",
            total_tests, total_failures);
    return status;
}
