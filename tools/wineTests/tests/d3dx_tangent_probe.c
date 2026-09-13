/* Native-first diagnostics for D3DX tangent-frame semantics.
 * D3DX entry points are loaded from d3dx9_43.dll; no local shim is linked.
 * Compile after the active graphics matrix finishes.
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

typedef HRESULT (WINAPI *create_mesh_fn)(DWORD, DWORD, DWORD,
        const D3DVERTEXELEMENT9 *, IDirect3DDevice9 *, ID3DXMesh **);
typedef HRESULT (WINAPI *tangent_fn)(ID3DXMesh *, DWORD, DWORD, DWORD, DWORD,
        DWORD, DWORD, DWORD, DWORD, DWORD, const DWORD *, FLOAT, FLOAT, FLOAT,
        ID3DXMesh **, ID3DXBuffer **);
struct vertex {float position[3], normal[3], uv[2], tangent[3], binormal[3];};
static unsigned tests, failures, cases;
static const char *phase = "setup";
static int check(int ok, unsigned line, const char *message)
{
    ++tests;
    if (ok) return 1;
    ++failures;
    printf("d3dx_tangent_probe.c:%u: Test failed: %s: %s.\n", line, phase, message);
    return 0;
}
#define CHECK(c,m) check(!!(c), __LINE__, m)
#define REQUIRE(c,m) do {if (!CHECK(c,m)) goto done;} while (0)
#define HR(c) do {HRESULT hr_=(c); if (!CHECK(SUCCEEDED(hr_),#c)) { \
    printf("HRESULT %08lx\n",(unsigned long)hr_); goto done;}} while (0)

static float dot(const float *a, const float *b)
{
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}
static int float_near(float a, float b)
{
    return isfinite(a) && fabsf(a-b) <= 0.0002f;
}
static void module_path(const char *name, HMODULE module)
{
    char path[32768];
    DWORD length = GetModuleFileNameA(module, path, sizeof(path));
    if (CHECK(length && length < sizeof(path), "loaded module identity"))
        printf("TANGENT_MODULE %s %s\n", name, path);
}
static void run_case(IDirect3DDevice9 *device, create_mesh_fn create_mesh,
        tangent_fn compute, unsigned which, unsigned bits)
{
    static const char *names[] = {"orthogonal", "skew", "normal", "clone"};
    static const D3DVERTEXELEMENT9 declaration[] = {
        {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
        {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
        {0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
        {0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0},
        {0, 44, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0},
        D3DDECL_END()};
    static const WORD indices16[] = {0, 1, 2};
    static const DWORD indices32[] = {0, 1, 2};
    struct vertex original[3] = {
        {{0,0,0}, {0,0,1}, {0,0}, {13,17,19}, {23,29,31}},
        {{1,0,0}, {0,0,1}, {1,0}, {13,17,19}, {23,29,31}},
        {{0,1,0}, {0,0,1}, {0,1}, {13,17,19}, {23,29,31}}};
    struct vertex after[3], result[3];
    ID3DXMesh *mesh = NULL, *output = NULL, *target;
    IUnknown *input_identity = NULL, *output_identity = NULL;
    void *locked = NULL;
    DWORD flags = which == 3 ? 0 : D3DXTANGENT_GENERATE_IN_PLACE;
    HRESULT hr;
    unsigned i;
    char label[64];
    snprintf(label, sizeof(label), "%s-%u", names[which], bits);
    phase = label;
    printf("TANGENT_CASE %s\n", label);
    ++cases;
    if (which == 1) original[2].uv[0] = -1;
    if (which == 2)
    {
        flags |= D3DXTANGENT_CALCULATE_NORMALS;
        for (i = 0; i < 3; ++i) original[i].normal[2] = 0;
    }
    HR(create_mesh(1, 3, D3DXMESH_SYSTEMMEM | (bits == 32 ? D3DXMESH_32BIT : 0),
            declaration, device, &mesh));
    REQUIRE(mesh->lpVtbl->GetNumBytesPerVertex(mesh) == sizeof(struct vertex), "vertex layout");
    HR(mesh->lpVtbl->LockVertexBuffer(mesh, 0, &locked));
    memcpy(locked, original, sizeof(original));
    HR(mesh->lpVtbl->UnlockVertexBuffer(mesh));
    HR(mesh->lpVtbl->LockIndexBuffer(mesh, 0, &locked));
    memcpy(locked, bits == 32 ? (const void *)indices32 : indices16,
            bits == 32 ? sizeof(indices32) : sizeof(indices16));
    HR(mesh->lpVtbl->UnlockIndexBuffer(mesh));
    /* One triangle has no shared-edge or singular-vertex splitting requirement. */
    hr = compute(mesh, D3DDECLUSAGE_TEXCOORD, 0, D3DDECLUSAGE_TANGENT, 0,
            D3DDECLUSAGE_BINORMAL, 0, D3DDECLUSAGE_NORMAL, 0, flags,
            NULL, -1.f, 0.f, -1.f, which == 3 ? &output : NULL, NULL);
    printf("TANGENT_RESULT %s hr=%08lx\n", label, (unsigned long)hr);
    REQUIRE(SUCCEEDED(hr), "tangent frame computation succeeds");
    HR(mesh->lpVtbl->LockVertexBuffer(mesh, D3DLOCK_READONLY, &locked));
    memcpy(after, locked, sizeof(after));
    HR(mesh->lpVtbl->UnlockVertexBuffer(mesh));
    if (which == 3)
    {
        REQUIRE(output, "cloning returns an output mesh");
        HR(mesh->lpVtbl->QueryInterface(mesh, &IID_IUnknown, (void **)&input_identity));
        HR(output->lpVtbl->QueryInterface(output, &IID_IUnknown, (void **)&output_identity));
        CHECK(input_identity != output_identity, "cloning returns a distinct object");
        CHECK(!memcmp(original, after, sizeof(original)), "cloning preserves all source vertex bytes");
        printf("TANGENT_CLONE %s distinct=%u input_unchanged=%u\n", label,
                input_identity != output_identity, !memcmp(original, after, sizeof(original)));
    }
    target = which == 3 ? output : mesh;
    REQUIRE(target->lpVtbl->GetNumVertices(target) == 3 && target->lpVtbl->GetNumFaces(target) == 1,
            "single triangle needs no topology changes");
    REQUIRE(target->lpVtbl->GetNumBytesPerVertex(target) == sizeof(struct vertex), "output vertex layout");
    HR(target->lpVtbl->LockVertexBuffer(target, D3DLOCK_READONLY, &locked));
    memcpy(result, locked, sizeof(result));
    HR(target->lpVtbl->UnlockVertexBuffer(target));
    for (i = 0; i < 3; ++i)
    {
        const struct vertex *v = &result[i];
        CHECK(!memcmp(v->position, original[i].position, sizeof(v->position)), "position preserved");
        CHECK(!memcmp(v->uv, original[i].uv, sizeof(v->uv)), "UV preserved");
        CHECK(float_near(dot(v->tangent, v->tangent), 1), "unit tangent");
        CHECK(float_near(dot(v->binormal, v->binormal), 1), "unit binormal");
        CHECK(float_near(dot(v->normal, v->normal), 1), "unit normal");
        CHECK(float_near(dot(v->tangent, v->binormal), 0), "orthogonal tangent and binormal");
        CHECK(float_near(dot(v->tangent, v->normal), 0), "tangent perpendicular to normal");
        CHECK(float_near(dot(v->binormal, v->normal), 0), "binormal perpendicular to normal");
        CHECK(float_near(v->normal[0], 0) && float_near(v->normal[1], 0) && float_near(v->normal[2], 1),
                "CCW triangle has positive-Z normal");
        printf("TANGENT_VERTEX %s vertex=%u normal=%.9g,%.9g,%.9g tangent=%.9g,%.9g,%.9g binormal=%.9g,%.9g,%.9g\n",
                label, i, v->normal[0], v->normal[1], v->normal[2], v->tangent[0], v->tangent[1],
                v->tangent[2], v->binormal[0], v->binormal[1], v->binormal[2]);
    }
done:
    if (output_identity) IUnknown_Release(output_identity);
    if (input_identity) IUnknown_Release(input_identity);
    if (output) output->lpVtbl->Release(output);
    if (mesh) mesh->lpVtbl->Release(mesh);
    phase = "setup";
}
int main(void)
{
    IDirect3D9 *d3d = NULL;
    IDirect3DDevice9 *device = NULL;
    D3DPRESENT_PARAMETERS pp = {0};
    HWND window = NULL;
    HMODULE module = NULL;
    create_mesh_fn create_mesh;
    tangent_fn compute;
    unsigned bits, which;
    setvbuf(stdout, NULL, _IONBF, 0);
    puts("D3DX_TANGENT_BEGIN");
    module = LoadLibraryA("d3dx9_43.dll");
    REQUIRE(module, "load D3DX");
    module_path("d3dx9_43.dll", module);
    create_mesh = (create_mesh_fn)GetProcAddress(module, "D3DXCreateMesh");
    compute = (tangent_fn)GetProcAddress(module, "D3DXComputeTangentFrameEx");
    REQUIRE(create_mesh && compute, "D3DX entry points");
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    REQUIRE(d3d, "create D3D9");
    module_path("d3d9.dll", GetModuleHandleA("d3d9.dll"));
    window = CreateWindowA("static", "D3DX tangent diagnostic", WS_OVERLAPPEDWINDOW,
            0, 0, 64, 64, NULL, NULL, NULL, NULL);
    REQUIRE(window, "create window");
    pp.Windowed = TRUE;
    pp.hDeviceWindow = window;
    pp.BackBufferWidth = pp.BackBufferHeight = 16;
    pp.BackBufferFormat = D3DFMT_A8R8G8B8;
    pp.BackBufferCount = 1;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    HR(IDirect3D9_CreateDevice(d3d, 0, D3DDEVTYPE_HAL, window,
            D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &device));
    for (bits = 16; bits <= 32; bits += 16)
        for (which = 0; which < 4; ++which) run_case(device, create_mesh, compute, which, bits);
    REQUIRE(cases == 8, "all tangent cases reached");
done:
    if (device) IDirect3DDevice9_Release(device);
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
    if (module) FreeLibrary(module);
    printf("D3DX_TANGENT_COVERAGE cases=%u\n", cases);
    printf("0000:d3dxtangent: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", tests, failures);
    return failures ? 1 : 0;
}
