/* Native-referenced coverage for CPU-backed mapped draw snapshots. */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <string.h>

struct vertex { float x, y, z, rhw; DWORD color; };
static unsigned tests, failures, cases, samples;
static char phase[80] = "setup";
static int check(int value, unsigned line, const char *message)
{
    ++tests;
    if (value) return 1;
    ++failures;
    printf("mapped_buffer_probe.c:%u: Test failed: %s: %s.\n",line,phase,message);
    return 0;
}
#define CHECK(c,m) check(!!(c),__LINE__,m)
#define REQUIRE(c,m) do { if (!CHECK(c,m)) goto done; } while (0)
#define HR(c) REQUIRE(SUCCEEDED(c),#c)

static void quad(struct vertex *v, unsigned tile, DWORD color)
{
    float x = tile == 4 ? 0.0f : (float)(tile % 2) * 32.0f;
    float y = tile == 4 ? 0.0f : (float)(tile / 2) * 32.0f;
    float side = tile == 4 ? 64.0f : 32.0f;
    struct vertex q[] = {{x,y,0,1,color},{x,y+side,0,1,color},
                         {x+side,y,0,1,color},{x+side,y+side,0,1,color}};
    memcpy(v,q,sizeof(q));
}

static void indices(void *data, unsigned bits, unsigned base)
{
    unsigned i;
    for (i=0;i<4;++i)
        if (bits == 16) ((WORD *)data)[i]=(WORD)(base+i);
        else ((DWORD *)data)[i]=base+i;
}

static HRESULT draw(IDirect3DDevice9 *device, unsigned bits, unsigned base)
{
    if (bits) return IDirect3DDevice9_DrawIndexedPrimitive(device,D3DPT_TRIANGLESTRIP,0,0,16,base,2);
    return IDirect3DDevice9_DrawPrimitive(device,D3DPT_TRIANGLESTRIP,base,2);
}

static int pixels(IDirect3DDevice9 *device,IDirect3DSurface9 *target,IDirect3DSurface9 *readback,int final)
{
    static const DWORD colors[] = {0xff0000,0x00ff00,0x0000ff,0xffff00};
    D3DLOCKED_RECT lock;
    unsigned x,y;
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_GetRenderTargetData(device,target,readback)),"read pixels")) return 0;
    if (!CHECK(SUCCEEDED(IDirect3DSurface9_LockRect(readback,&lock,NULL,D3DLOCK_READONLY)),"lock pixels")) return 0;
    for (y=8;y<64;y+=16) for (x=8;x<64;x+=16)
    {
        DWORD actual = ((DWORD *)((BYTE *)lock.pBits+y*lock.Pitch))[x] & 0xffffff;
        DWORD expected = final ? 0xffffff : colors[(y/32)*2+x/32];
        ++samples;
        printf("MAPPED_PIXEL case=%s final=%d x=%u y=%u actual=%06lx expected=%06lx\n",phase,final,x,y,actual,expected);
        CHECK(actual == expected,"pixel color");
    }
    return CHECK(SUCCEEDED(IDirect3DSurface9_UnlockRect(readback)),"unlock pixels");
}

static void run_case(IDirect3DDevice9 *device,IDirect3DSurface9 *target,IDirect3DSurface9 *readback,
        unsigned dynamic,unsigned bits,unsigned nested)
{
    static const DWORD colors[] = {0xffff0000,0xff00ff00,0xff0000ff,0xffffff00};
    IDirect3DVertexBuffer9 *vb=NULL;
    IDirect3DIndexBuffer9 *ib=NULL;
    struct vertex *v=NULL;
    void *idx=NULL,*second=NULL;
    unsigned vb_locks=0,ib_locks=0,tile,base;
    int scene=0;
    DWORD usage=D3DUSAGE_WRITEONLY | (dynamic ? D3DUSAGE_DYNAMIC : 0);
    DWORD flags=dynamic ? D3DLOCK_DISCARD : 0;
    sprintf(phase,"%s-ib%u-nested%u",dynamic ? "dynamic" : "static",bits,nested);
    printf("MAPPED_CASE %s\n",phase);
    HR(IDirect3DDevice9_CreateVertexBuffer(device,16*sizeof(*v),usage,D3DFVF_XYZRHW|D3DFVF_DIFFUSE,D3DPOOL_DEFAULT,&vb,NULL));
    HR(IDirect3DDevice9_SetStreamSource(device,0,vb,0,sizeof(*v)));
    if (bits)
    {
        HR(IDirect3DDevice9_CreateIndexBuffer(device,16*(bits/8),usage,bits == 16 ? D3DFMT_INDEX16 : D3DFMT_INDEX32,D3DPOOL_DEFAULT,&ib,NULL));
        HR(IDirect3DDevice9_SetIndices(device,ib));
        HR(IDirect3DIndexBuffer9_Lock(ib,0,0,&idx,flags)); ++ib_locks;
        if (nested)
        {
            HR(IDirect3DIndexBuffer9_Lock(ib,0,0,&second,0)); ++ib_locks;
            CHECK(second == idx,"nested index pointer stable"); idx=second;
        }
    }
    HR(IDirect3DVertexBuffer9_Lock(vb,0,0,(void **)&v,flags)); ++vb_locks;
    if (nested)
    {
        HR(IDirect3DVertexBuffer9_Lock(vb,0,0,&second,0)); ++vb_locks;
        CHECK(second == v,"nested vertex pointer stable"); v=second;
    }
    memset(v,0,16*sizeof(*v));
    HR(IDirect3DDevice9_Clear(device,0,NULL,D3DCLEAR_TARGET,0xff000000,1,0));
    HR(IDirect3DDevice9_BeginScene(device)); scene=1;
    for (tile=0;tile<4;++tile)
    {
        /* Append to disjoint regions while earlier draws may still be in flight. */
        base=tile*4;
        quad(v+base,tile,colors[tile]);
        if (bits) indices((BYTE *)idx + base*(bits/8),bits,base);
        HR(draw(device,bits,base));
        if (!tile && nested)
        {
            HR(IDirect3DVertexBuffer9_Unlock(vb)); --vb_locks;
            if (bits) { HR(IDirect3DIndexBuffer9_Unlock(ib)); --ib_locks; }
        }
    }
    HR(IDirect3DDevice9_EndScene(device)); scene=0;
    pixels(device,target,readback,0);
    /* These writes happen after the final mapped draw. */
    quad(v,4,0xffffffff);
    if (bits) indices(idx,bits,0);
    HR(IDirect3DVertexBuffer9_Unlock(vb)); --vb_locks;
    if (bits) { HR(IDirect3DIndexBuffer9_Unlock(ib)); --ib_locks; }
    HR(IDirect3DDevice9_Clear(device,0,NULL,D3DCLEAR_TARGET,0xff000000,1,0));
    HR(IDirect3DDevice9_BeginScene(device)); scene=1;
    HR(draw(device,bits,0));
    HR(IDirect3DDevice9_EndScene(device)); scene=0;
    pixels(device,target,readback,1);
    ++cases;
    printf("MAPPED_END %s\n",phase);
done:
    if (scene) IDirect3DDevice9_EndScene(device);
    while (vb_locks) { IDirect3DVertexBuffer9_Unlock(vb); --vb_locks; }
    while (ib_locks) { IDirect3DIndexBuffer9_Unlock(ib); --ib_locks; }
    CHECK(SUCCEEDED(IDirect3DDevice9_SetStreamSource(device,0,NULL,0,0)),"unbind vertices");
    CHECK(SUCCEEDED(IDirect3DDevice9_SetIndices(device,NULL)),"unbind indices");
    if (vb) CHECK(!IDirect3DVertexBuffer9_Release(vb),"vertex references released");
    if (ib) CHECK(!IDirect3DIndexBuffer9_Release(ib),"index references released");
}

int main(void)
{
    IDirect3D9 *d3d=NULL;
    IDirect3DDevice9 *device=NULL;
    IDirect3DSurface9 *target=NULL,*readback=NULL;
    HWND window=NULL;
    D3DPRESENT_PARAMETERS pp={0};
    unsigned dynamic,bits,nested;
    char module[MAX_PATH]; DWORD length;
    length=GetModuleFileNameA(GetModuleHandleA("d3d9.dll"),module,sizeof(module));
    printf("MAPPED_MODULE length=%lu path=%s\n",length,module);
    printf("MAPPED_BEGIN\n");
    d3d=Direct3DCreate9(D3D_SDK_VERSION); REQUIRE(d3d,"D3D9 create");
    window=CreateWindowA("static","Mapped buffer probe",WS_OVERLAPPEDWINDOW,0,0,96,96,NULL,NULL,NULL,NULL);
    REQUIRE(window,"window create");
    pp.Windowed=TRUE; pp.hDeviceWindow=window; pp.BackBufferWidth=pp.BackBufferHeight=64;
    pp.BackBufferFormat=D3DFMT_A8R8G8B8; pp.BackBufferCount=1; pp.SwapEffect=D3DSWAPEFFECT_DISCARD;
    HR(IDirect3D9_CreateDevice(d3d,0,D3DDEVTYPE_HAL,window,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&device));
    HR(IDirect3DDevice9_GetRenderTarget(device,0,&target));
    HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device,64,64,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&readback,NULL));
    HR(IDirect3DDevice9_SetFVF(device,D3DFVF_XYZRHW|D3DFVF_DIFFUSE));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_LIGHTING,FALSE));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_ZENABLE,FALSE));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_CULLMODE,D3DCULL_NONE));
    for (dynamic=0;dynamic<2;++dynamic) for (bits=0;bits<=32;bits+=16) for (nested=0;nested<2;++nested)
        run_case(device,target,readback,dynamic,bits,nested);
done:
    strcpy(phase,"cleanup");
    if (readback) IDirect3DSurface9_Release(readback);
    if (target) IDirect3DSurface9_Release(target);
    if (device) CHECK(!IDirect3DDevice9_Release(device),"device references released");
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
    CHECK(cases == 12,"complete cases"); CHECK(samples == 384,"complete pixel comparisons");
    printf("MAPPED_COVERAGE cases=%u samples=%u\n",cases,samples);
    printf("0000:mappedbuffers: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n",tests,failures);
    return failures ? 1 : 0;
}
