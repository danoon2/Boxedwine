/* Compare advertised maskable support with actual resource creation and ordinary drawing. */
static void test_mask_capability_bound(void)
{
    static const D3DMULTISAMPLE_TYPE types[] = {D3DMULTISAMPLE_NONE, D3DMULTISAMPLE_NONMASKABLE, D3DMULTISAMPLE_2_SAMPLES, D3DMULTISAMPLE_4_SAMPLES, D3DMULTISAMPLE_15_SAMPLES};
    static const D3DFORMAT formats[] = {D3DFMT_A8R8G8B8, D3DFMT_D24S8};
    static const struct { struct vec3 position; DWORD diffuse; } quad[] =
    {
        {{-1.0f,-1.0f,0.1f},0xffffffff}, {{-1.0f,1.0f,0.1f},0xffffffff},
        {{1.0f,-1.0f,0.1f},0xffffffff}, {{1.0f,1.0f,0.1f},0xffffffff},
    };
    IDirect3DSurface9 *surface = NULL, *resolved = NULL, *back = NULL;
    IDirect3DDevice9 *device = NULL;
    IDirect3D9 *d3d = NULL;
    struct surface_readback rb;
    unsigned int f, t, colour;
    DWORD quality;
    BOOL absent, in_scene = FALSE;
    char option[8];
    ULONG refs;
    HWND window;
    HRESULT hr, expected;
#define BOUND_HR(call) do { hr=(call); ok(hr==D3D_OK,"%s returned %#lx.\n",#call,hr); if(FAILED(hr))goto done; } while(0)
    absent = GetEnvironmentVariableA("MASK_EXTENSION_ABSENT", option, sizeof(option)) == 1 && option[0] == '1';
    trace("MASK_BOUND_BEGIN absent=%u\n", absent);
    window = create_window();
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    ok(!!d3d,"Could not create Direct3D9.\n");
    if(!d3d)goto done;
    device = create_device(d3d,window,window,TRUE);
    ok(!!device,"Could not create ordinary HAL device.\n");
    if(!device)goto done;
    BOUND_HR(IDirect3DDevice9_GetRenderTarget(device,0,&back));
    BOUND_HR(IDirect3DDevice9_SetDepthStencilSurface(device,NULL));
    BOUND_HR(IDirect3DDevice9_CreateRenderTarget(device,128,128,D3DFMT_A8R8G8B8,D3DMULTISAMPLE_NONE,0,FALSE,&resolved,NULL));
    BOUND_HR(IDirect3DDevice9_SetFVF(device,D3DFVF_XYZ|D3DFVF_DIFFUSE));
    BOUND_HR(IDirect3DDevice9_SetRenderState(device,D3DRS_LIGHTING,FALSE));
    BOUND_HR(IDirect3DDevice9_SetRenderState(device,D3DRS_ZENABLE,FALSE));
    BOUND_HR(IDirect3DDevice9_SetRenderState(device,D3DRS_CULLMODE,D3DCULL_NONE));
    for(f=0;f<ARRAY_SIZE(formats);++f)
    {
        for(t=0;t<ARRAY_SIZE(types);++t)
        {
            quality=0xdeadbeef;
            expected=(types[t]==D3DMULTISAMPLE_15_SAMPLES || (absent && types[t]>D3DMULTISAMPLE_NONMASKABLE)) ? D3DERR_NOTAVAILABLE : D3D_OK;
            hr=IDirect3D9_CheckDeviceMultiSampleType(d3d,0,D3DDEVTYPE_HAL,formats[f],TRUE,types[t],&quality);
            trace("MASK_BOUND_CAP format=%u samples=%u hr=%08lx quality=%lu\n",formats[f],types[t],hr,quality);
            ok(hr==expected,"Format %u sample type %u query returned %#lx expected %#lx.\n",formats[f],types[t],hr,expected);
            if(SUCCEEDED(hr))ok(quality>0 && quality!=0xdeadbeef,"Successful query left invalid quality count %#lx.\n",quality);
            /* Creation validates the unavailable sample type as an invalid call. */
            if(expected==D3DERR_NOTAVAILABLE)expected=D3DERR_INVALIDCALL;
            if(f)
                hr=IDirect3DDevice9_CreateDepthStencilSurface(device,128,128,formats[f],types[t],0,FALSE,&surface,NULL);
            else
                hr=IDirect3DDevice9_CreateRenderTarget(device,128,128,formats[f],types[t],0,FALSE,&surface,NULL);
            trace("MASK_BOUND_CREATE format=%u samples=%u hr=%08lx\n",formats[f],types[t],hr);
            ok(hr==expected,"Format %u sample type %u creation returned %#lx expected %#lx.\n",formats[f],types[t],hr,expected);
            if(SUCCEEDED(hr) && !f)
            {
                BOUND_HR(IDirect3DDevice9_SetRenderTarget(device,0,surface));
                BOUND_HR(IDirect3DDevice9_Clear(device,0,NULL,D3DCLEAR_TARGET,0xffff0000,0.0f,0));
                /* Only the single-sample case requires ignoring a zero mask.
                 * NONMASKABLE is tested with the ordinary all-enabled state. */
                BOUND_HR(IDirect3DDevice9_SetRenderState(device,D3DRS_MULTISAMPLEMASK,types[t] ? 0xffffffff : 0));
                BOUND_HR(IDirect3DDevice9_BeginScene(device));
                in_scene=TRUE;
                BOUND_HR(IDirect3DDevice9_DrawPrimitiveUP(device,D3DPT_TRIANGLESTRIP,2,quad,sizeof(quad[0])));
                BOUND_HR(IDirect3DDevice9_EndScene(device));
                in_scene=FALSE;
                if(types[t])BOUND_HR(IDirect3DDevice9_StretchRect(device,surface,NULL,resolved,NULL,D3DTEXF_NONE));
                get_rt_readback(types[t] ? resolved : surface,&rb);
                colour=get_readback_color(&rb,64,64);
                trace("MASK_BOUND_PIXEL samples=%u color=%08x\n",types[t],colour);
                ok(colour==0xffffffff,"Sample type %u ordinary rendering returned %08x.\n",types[t],colour);
                release_surface_readback(&rb);
                BOUND_HR(IDirect3DDevice9_SetRenderTarget(device,0,back));
            }
            if(surface)IDirect3DSurface9_Release(surface);
            surface=NULL;
        }
    }
done:
    if(device)
    {
        if(in_scene)IDirect3DDevice9_EndScene(device);
        if(back)IDirect3DDevice9_SetRenderTarget(device,0,back);
    }
    if(surface)IDirect3DSurface9_Release(surface);
    if(resolved)IDirect3DSurface9_Release(resolved);
    if(back)IDirect3DSurface9_Release(back);
    if(device){refs=IDirect3DDevice9_Release(device);ok(!refs,"Device has %lu references left.\n",refs);}
    if(d3d)IDirect3D9_Release(d3d);
    DestroyWindow(window);
    trace("MASK_BOUND_END absent=%u\n",absent);
#undef BOUND_HR
}
