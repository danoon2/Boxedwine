/* Individual-mask identity, cached shader reuse, and unchanged-mask target switches. */
static void test_mask_transition_cycle(unsigned int cycle)
{
    static const DWORD ps_code[] =
    {
        0xffff0200,
        0x0200001f, 0x80000000, 0x900f0000,
        0x02000001, 0x800f0800, 0x90e40000,
        0x0000ffff,
    };
    static const struct { struct vec3 position; DWORD diffuse; } quad[] =
    {
        {{-1.0f,-1.0f,0.1f},0xffffffff}, {{-1.0f,1.0f,0.1f},0xffffffff},
        {{1.0f,-1.0f,0.1f},0xffffffff}, {{1.0f,1.0f,0.1f},0xffffffff},
    };
    IDirect3DSurface9 *regular = NULL, *resolved = NULL, *ms[2] = {NULL, NULL}, *back = NULL;
    IDirect3DPixelShader9 *ps = NULL;
    IDirect3DDevice9 *device = NULL;
    IDirect3D9 *d3d = NULL;
    struct surface_readback rb;
    unsigned int n, samples, pipe, c, pass, passes, half, all, masks[2], expected, colour;
    BOOL single, set_first, in_scene = FALSE;
    DWORD mask = 0xffffffff, observed_mask;
    ULONG refs;
    HWND window;
    HRESULT hr;

#define MASK_HR(call) do { hr = (call); ok(hr == D3D_OK, "%s returned %#lx.\n", #call, hr); if (FAILED(hr)) goto done; } while (0)
    window = create_window();
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    ok(!!d3d, "Could not create Direct3D9.\n");
    if (!d3d) goto done;
    for (n = 0; n < 2; ++n)
    {
        samples = n ? 4 : 2;
        hr = IDirect3D9_CheckDeviceMultiSampleType(d3d, 0, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, TRUE, samples, NULL);
        trace("MASK_TRANSITION_CAP cycle=%u samples=%u hr=%08lx\n", cycle, samples, hr);
        ok(hr == D3D_OK, "Required sample count %u unavailable, hr %#lx.\n", samples, hr);
        if (FAILED(hr)) goto done;
    }
    device = create_device(d3d, window, window, TRUE);
    ok(!!device, "Could not create HAL device.\n");
    if (!device) goto done;
    MASK_HR(IDirect3DDevice9_GetRenderTarget(device, 0, &back));
    MASK_HR(IDirect3DDevice9_SetDepthStencilSurface(device, NULL));
    MASK_HR(IDirect3DDevice9_CreateRenderTarget(device, 128, 128, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &regular, NULL));
    MASK_HR(IDirect3DDevice9_CreateRenderTarget(device, 128, 128, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &resolved, NULL));
    for (n = 0; n < 2; ++n)
        MASK_HR(IDirect3DDevice9_CreateRenderTarget(device, 128, 128, D3DFMT_A8R8G8B8, n ? 4 : 2, 0, FALSE, &ms[n], NULL));
    MASK_HR(IDirect3DDevice9_CreatePixelShader(device, ps_code, &ps));
    MASK_HR(IDirect3DDevice9_SetFVF(device, D3DFVF_XYZ | D3DFVF_DIFFUSE));
    MASK_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_LIGHTING, FALSE));
    MASK_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, FALSE));
    MASK_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_CULLMODE, D3DCULL_NONE));
    MASK_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHABLENDENABLE, FALSE));
    MASK_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHATESTENABLE, FALSE));
    for (n = 0; n < 2; ++n)
    {
        samples = n ? 4 : 2;
        all = (1u << samples) - 1;
        half = all & 0x55;
        for (pipe = 0; pipe < 2; ++pipe)
        {
            MASK_HR(IDirect3DDevice9_SetPixelShader(device, pipe ? ps : NULL));
            for (c = 0; c < 15; ++c)
            {
                single = c == 2 || c == 5;
                set_first = c != 2 && c != 3 && c != 5 && c != 6;
                passes = c == 7 || c == 8 || c == 9 || c == 13 ? 2 : 1;
                masks[0] = 0xffffffff;
                masks[1] = 0xffffffff;
                if (c == 1) masks[0] = 0;
                if (c == 4 || c == 7 || c == 9 || c == 14) masks[0] = half;
                if (c == 8) masks[0] = all ^ half;
                if (c == 7) masks[1] = all ^ half;
                if (c == 8 || c == 9) masks[1] = half;
                if (c == 10) masks[0] = 0x80000000;
                if (c == 11) masks[0] = all;
                if (c == 13) masks[0] = 0;
                expected = c == 1 || c == 3 || c == 10 ? 0xffff0000
                        : c == 4 || c == 6 || c == 9 || c == 14 ? 0xffff8080 : 0xffffffff;
                MASK_HR(IDirect3DDevice9_SetRenderTarget(device, 0, single ? regular : ms[n]));
                MASK_HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xffff0000, 0.0f, 0));
                MASK_HR(IDirect3DDevice9_BeginScene(device));
                in_scene = TRUE;
                for (pass = 0; pass < passes; ++pass)
                {
                    if (set_first || pass)
                    {
                        mask = masks[pass];
                        MASK_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_MULTISAMPLEMASK, mask));
                    }
                    MASK_HR(IDirect3DDevice9_GetRenderState(device, D3DRS_MULTISAMPLEMASK, &observed_mask));
                    ok(observed_mask == mask, "Mask state %#lx, expected %#lx.\n", observed_mask, mask);
                    MASK_HR(IDirect3DDevice9_DrawPrimitiveUP(device, D3DPT_TRIANGLESTRIP, 2, quad, sizeof(quad[0])));
                }
                MASK_HR(IDirect3DDevice9_EndScene(device));
                in_scene = FALSE;
                if (!single)
                    MASK_HR(IDirect3DDevice9_StretchRect(device, ms[n], NULL, resolved, NULL, D3DTEXF_NONE));
                get_rt_readback(single ? regular : resolved, &rb);
                colour = get_readback_color(&rb, 64, 64);
                trace("MASK_TRANSITION cycle=%u samples=%u pipe=%u case=%u target=%u mask=%08lx passes=%u color=%08x expected=%08x\n",
                        cycle, samples, pipe, c, single ? 0 : samples, mask, passes, colour, expected);
                /* Preserve the upstream native-only NVIDIA half-resolve alternative. */
                ok(color_match(colour, expected, 1) || (expected == 0xffff8080 && broken(color_match(colour, 0xffffbcbc, 1))),
                        "Cycle %u samples %u pipe %u case %u: got %08x expected %08x.\n", cycle, samples, pipe, c, colour, expected);
                release_surface_readback(&rb);
            }
        }
    }
done:
    if (device)
    {
        if (in_scene) IDirect3DDevice9_EndScene(device);
        IDirect3DDevice9_SetPixelShader(device, NULL);
        if (back) IDirect3DDevice9_SetRenderTarget(device, 0, back);
    }
    if (ps) IDirect3DPixelShader9_Release(ps);
    if (regular) IDirect3DSurface9_Release(regular);
    if (resolved) IDirect3DSurface9_Release(resolved);
    for (n = 0; n < 2; ++n) if (ms[n]) IDirect3DSurface9_Release(ms[n]);
    if (back) IDirect3DSurface9_Release(back);
    if (device)
    {
        refs = IDirect3DDevice9_Release(device);
        ok(!refs, "Device has %lu references left.\n", refs);
    }
    if (d3d) IDirect3D9_Release(d3d);
    DestroyWindow(window);
#undef MASK_HR
}
