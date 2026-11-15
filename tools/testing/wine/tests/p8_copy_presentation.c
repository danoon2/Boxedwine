/*
 * Copyright 2005 Antoine Chavasse (a.chavasse@gmail.com)
 * Copyright 2006, 2008, 2011, 2012-2014 Stefan Dösinger for CodeWeavers
 * Copyright 2011-2014 Henri Verbeet for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#define COBJMACROS
#include "wine/test.h"
#include "d3d.h"
#include <string.h>

static HRESULT (WINAPI *pDirectDrawCreateEx)(GUID *guid, void **ddraw, REFIID iid, IUnknown *outer_unknown);

static BOOL compare_uint(unsigned int x, unsigned int y, unsigned int max_diff)
{
    unsigned int diff = x > y ? x - y : y - x;

    return diff <= max_diff;
}

static BOOL compare_color(D3DCOLOR c1, D3DCOLOR c2, BYTE max_diff)
{
    return compare_uint(c1 & 0xff, c2 & 0xff, max_diff)
            && compare_uint((c1 >> 8) & 0xff, (c2 >> 8) & 0xff, max_diff)
            && compare_uint((c1 >> 16) & 0xff, (c2 >> 16) & 0xff, max_diff)
            && compare_uint((c1 >> 24) & 0xff, (c2 >> 24) & 0xff, max_diff);
}

static BOOL ddraw_get_identifier(IDirectDraw7 *ddraw, DDDEVICEIDENTIFIER2 *identifier)
{
    HRESULT hr;

    hr = IDirectDraw7_GetDeviceIdentifier(ddraw, identifier, 0);
    ok(SUCCEEDED(hr), "Failed to get device identifier, hr %#lx.\n", hr);

    return SUCCEEDED(hr);
}

static BOOL ddraw_is_warp(IDirectDraw7 *ddraw)
{
    DDDEVICEIDENTIFIER2 identifier;

    return strcmp(winetest_platform, "wine")
            && ddraw_get_identifier(ddraw, &identifier)
            && strstr(identifier.szDriver, "warp");
}

static HWND create_window(void)
{
    RECT r = {0, 0, 640, 480};

    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW | WS_VISIBLE, FALSE);

    return CreateWindowA("static", "ddraw_test", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top, NULL, NULL, NULL, NULL);
}

static D3DCOLOR get_surface_color(IDirectDrawSurface7 *surface, UINT x, UINT y)
{
    RECT rect = {x, y, x + 1, y + 1};
    DDSURFACEDESC2 surface_desc;
    D3DCOLOR color;
    HRESULT hr;

    memset(&surface_desc, 0, sizeof(surface_desc));
    surface_desc.dwSize = sizeof(surface_desc);

    hr = IDirectDrawSurface7_Lock(surface, &rect, &surface_desc, DDLOCK_READONLY, NULL);
    ok(SUCCEEDED(hr), "Failed to lock surface, hr %#lx.\n", hr);
    if (FAILED(hr))
        return 0xdeadbeef;

    trace("P8_RGB_RAW x=%u y=%u color=%08lx\n", x, y, *((DWORD *)surface_desc.lpSurface));
    color = *((DWORD *)surface_desc.lpSurface) & 0x00ffffff;

    hr = IDirectDrawSurface7_Unlock(surface, &rect);
    ok(SUCCEEDED(hr), "Failed to unlock surface, hr %#lx.\n", hr);

    return color;
}

static IDirectDraw7 *create_ddraw(void)
{
    IDirectDraw7 *ddraw;

    if (FAILED(pDirectDrawCreateEx(NULL, (void **)&ddraw, &IID_IDirectDraw7, NULL)))
        return NULL;

    return ddraw;
}

static void test_p8_blit(void)
{
    IDirectDrawSurface7 *src, *dst, *dst_p8;
    DDSURFACEDESC2 surface_desc;
    unsigned int color, x;
    IDirectDraw7 *ddraw;
    IDirectDrawPalette *palette, *palette2;
    ULONG refcount;
    HWND window;
    HRESULT hr;
    PALETTEENTRY palette_entries[256];
    DDBLTFX fx;
    BOOL is_warp;
    static const BYTE src_data[] = {0x10, 0x1, 0x2, 0x3, 0x4, 0x5, 0xff, 0x80};
    static const BYTE src_data2[] = {0x10, 0x5, 0x4, 0x3, 0x2, 0x1, 0xff, 0x80};
    static const BYTE expected_p8[] = {0x10, 0x1, 0x4, 0x3, 0x4, 0x5, 0xff, 0x80};
    static const unsigned int expected[] =
    {
        0x00101010, 0x00010101, 0x00020202, 0x00030303,
        0x00040404, 0x00050505, 0x00ffffff, 0x00808080,
    };

    window = create_window();
    ddraw = create_ddraw();
    ok(!!ddraw, "Failed to create a ddraw object.\n");
    hr = IDirectDraw7_SetCooperativeLevel(ddraw, window, DDSCL_NORMAL);
    ok(SUCCEEDED(hr), "Failed to set cooperative level, hr %#lx.\n", hr);
    is_warp = ddraw_is_warp(ddraw);
    trace("P8_DRIVER warp=%u\n", is_warp);

    memset(palette_entries, 0, sizeof(palette_entries));
    palette_entries[1].peGreen = 0xff;
    palette_entries[2].peBlue = 0xff;
    palette_entries[3].peFlags = 0xff;
    palette_entries[4].peRed = 0xff;
    hr = IDirectDraw7_CreatePalette(ddraw, DDPCAPS_8BIT | DDPCAPS_ALLOW256,
            palette_entries, &palette, NULL);
    ok(SUCCEEDED(hr), "Failed to create palette, hr %#lx.\n", hr);
    palette_entries[1].peBlue = 0xff;
    palette_entries[2].peGreen = 0xff;
    palette_entries[3].peRed = 0xff;
    palette_entries[4].peFlags = 0x0;
    hr = IDirectDraw7_CreatePalette(ddraw, DDPCAPS_8BIT | DDPCAPS_ALLOW256,
            palette_entries, &palette2, NULL);
    ok(SUCCEEDED(hr), "Failed to create palette, hr %#lx.\n", hr);

    memset(&surface_desc, 0, sizeof(surface_desc));
    surface_desc.dwSize = sizeof(surface_desc);
    surface_desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    surface_desc.dwWidth = 8;
    surface_desc.dwHeight = 1;
    surface_desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    surface_desc.ddpfPixelFormat.dwSize = sizeof(surface_desc.ddpfPixelFormat);
    surface_desc.ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED8 | DDPF_RGB;
    surface_desc.ddpfPixelFormat.dwRGBBitCount = 8;
    hr = IDirectDraw7_CreateSurface(ddraw, &surface_desc, &src, NULL);
    ok(SUCCEEDED(hr), "Failed to create surface, hr %#lx.\n", hr);
    hr = IDirectDraw7_CreateSurface(ddraw, &surface_desc, &dst_p8, NULL);
    ok(SUCCEEDED(hr), "Failed to create surface, hr %#lx.\n", hr);
    hr = IDirectDrawSurface7_SetPalette(dst_p8, palette2);
    ok(SUCCEEDED(hr), "Failed to set palette, hr %#lx.\n", hr);

    memset(&surface_desc, 0, sizeof(surface_desc));
    surface_desc.dwSize = sizeof(surface_desc);
    surface_desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    surface_desc.dwWidth = 8;
    surface_desc.dwHeight = 1;
    surface_desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    surface_desc.ddpfPixelFormat.dwSize = sizeof(surface_desc.ddpfPixelFormat);
    surface_desc.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    surface_desc.ddpfPixelFormat.dwRGBBitCount = 32;
    surface_desc.ddpfPixelFormat.dwRBitMask = 0x00ff0000;
    surface_desc.ddpfPixelFormat.dwGBitMask = 0x0000ff00;
    surface_desc.ddpfPixelFormat.dwBBitMask = 0x000000ff;
    surface_desc.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;
    hr = IDirectDraw7_CreateSurface(ddraw, &surface_desc, &dst, NULL);
    ok(SUCCEEDED(hr), "Failed to create surface, hr %#lx.\n", hr);

    memset(&surface_desc, 0, sizeof(surface_desc));
    surface_desc.dwSize = sizeof(surface_desc);
    hr = IDirectDrawSurface7_Lock(src, NULL, &surface_desc, DDLOCK_WAIT, NULL);
    ok(SUCCEEDED(hr), "Failed to lock source surface, hr %#lx.\n", hr);
    memcpy(surface_desc.lpSurface, src_data, sizeof(src_data));
    hr = IDirectDrawSurface7_Unlock(src, NULL);
    ok(SUCCEEDED(hr), "Failed to unlock source surface, hr %#lx.\n", hr);

    hr = IDirectDrawSurface7_Lock(dst_p8, NULL, &surface_desc, DDLOCK_WAIT, NULL);
    ok(SUCCEEDED(hr), "Failed to lock destination surface, hr %#lx.\n", hr);
    memcpy(surface_desc.lpSurface, src_data2, sizeof(src_data2));
    hr = IDirectDrawSurface7_Unlock(dst_p8, NULL);
    ok(SUCCEEDED(hr), "Failed to unlock destination surface, hr %#lx.\n", hr);

    fx.dwSize = sizeof(fx);
    fx.dwFillColor = 0xdeadbeef;
    hr = IDirectDrawSurface7_Blt(dst, NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &fx);
    ok(SUCCEEDED(hr), "Got hr %#lx.\n", hr);

    hr = IDirectDrawSurface7_SetPalette(src, palette);
    ok(SUCCEEDED(hr), "Failed to set palette, hr %#lx.\n", hr);
    hr = IDirectDrawSurface7_Blt(dst, NULL, src, NULL, DDBLT_WAIT, NULL);
    trace("P8_RGB_RESULT hr=%08lx\n", hr);
    /* The r500 Windows 7 driver returns E_NOTIMPL. r200 on Windows XP works.
     * The Geforce 7 driver on Windows Vista returns E_FAIL. Newer Nvidia GPUs work. */
    ok(SUCCEEDED(hr) || broken(hr == E_NOTIMPL) || broken(hr == E_FAIL),
            "Failed to blit, hr %#lx.\n", hr);

    if (SUCCEEDED(hr))
    {
        for (x = 0; x < ARRAY_SIZE(expected); ++x)
        {
            color = get_surface_color(dst, x, 0);
            trace("P8_RGB_PIXEL x=%u color=%08x expected=%08x\n", x, color, expected[x]);
            /* WARP on 1709 and newer write zeroes on non-colorkeyed P8 -> RGB blits. For ckey
             * blits see below. */
            ok(compare_color(color, expected[x], 0)
                    || broken(is_warp && compare_color(color, 0x00000000, 0)),
                    "Pixel %u: Got color %#x, expected %#x.\n",
                    x, color, expected[x]);
        }
    }

    fx.ddckSrcColorkey.dwColorSpaceHighValue = 0x2;
    fx.ddckSrcColorkey.dwColorSpaceLowValue = 0x2;
    hr = IDirectDrawSurface7_Blt(dst_p8, NULL, src, NULL, DDBLT_WAIT | DDBLT_KEYSRCOVERRIDE, &fx);
    trace("P8_KEY_RESULT hr=%08lx\n", hr);
    ok(SUCCEEDED(hr), "Failed to blit, hr %#lx.\n", hr);

    hr = IDirectDrawSurface7_Lock(dst_p8, NULL, &surface_desc, DDLOCK_READONLY | DDLOCK_WAIT, NULL);
    ok(SUCCEEDED(hr), "Failed to lock destination surface, hr %#lx.\n", hr);
    for (x = 0; x < ARRAY_SIZE(expected_p8); ++x)
        trace("P8_KEY_BYTE x=%u actual=%02x expected=%02x original=%02x\n", x,
                ((const BYTE *)surface_desc.lpSurface)[x], expected_p8[x], src_data2[x]);
    /* A color keyed P8 blit doesn't do anything on WARP - it just leaves the data in the destination
     * surface untouched. Error checking (DDBLT_KEYSRC without a key
     * for example) also works as expected.
     *
     * Using DDBLT_KEYSRC instead of DDBLT_KEYSRCOVERRIDE doesn't change this. Doing this blit with
     * the display mode set to P8 doesn't help either. */
    ok(!memcmp(surface_desc.lpSurface, expected_p8, sizeof(expected_p8))
            || broken(is_warp && !memcmp(surface_desc.lpSurface, src_data2, sizeof(src_data2))),
            "Got unexpected P8 color key blit result.\n");
    hr = IDirectDrawSurface7_Unlock(dst_p8, NULL);
    ok(SUCCEEDED(hr), "Failed to unlock destination surface, hr %#lx.\n", hr);

    IDirectDrawSurface7_Release(src);
    IDirectDrawSurface7_Release(dst);
    IDirectDrawSurface7_Release(dst_p8);
    IDirectDrawPalette_Release(palette);
    IDirectDrawPalette_Release(palette2);

    refcount = IDirectDraw7_Release(ddraw);
    ok(!refcount, "Got unexpected refcount %lu.\n", refcount);
    DestroyWindow(window);
}

static void test_p8_display(void)
{
    unsigned int life, phase, x, y, i;
    DDSURFACEDESC2 desc, lock;
    IDirectDrawSurface7 *src, *dst, *primary;
    DDBLTFX fx;
    RECT fill;
    BOOL display_changed;
    IDirectDrawPalette *palette;
    PALETTEENTRY entries[256];
    IDirectDraw7 *ddraw;
    DWORD color, expected;
    RECT rect = {2, 2, 18, 18};
    HWND window;
    HRESULT hr;
    ULONG refs;

    for (life = 0; life < 2; ++life)
    {
        src = dst = primary = NULL;
        display_changed = FALSE;
        palette = NULL;
        window = create_window();
        ddraw = create_ddraw();
        ok(!!ddraw, "Could not create DirectDraw.\n");
        if (!ddraw) { DestroyWindow(window); return; }
        hr = IDirectDraw7_SetCooperativeLevel(ddraw, window, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
        ok(hr == DD_OK, "SetCooperativeLevel returned %#lx.\n", hr);
        if (FAILED(hr)) goto done;
        hr = IDirectDraw7_SetDisplayMode(ddraw, 640, 480, 8, 0, 0);
        ok(hr == DD_OK, "SetDisplayMode returned %#lx.\n", hr);
        if (FAILED(hr)) goto done;
        display_changed = TRUE;
        memset(&desc, 0, sizeof(desc));
        desc.dwSize = sizeof(desc);
        desc.dwFlags = DDSD_CAPS;
        desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
        hr = IDirectDraw7_CreateSurface(ddraw, &desc, &primary, NULL);
        ok(hr == DD_OK, "CreatePrimary returned %#lx.\n", hr);
        if (FAILED(hr)) goto done;
        memset(entries, 0, sizeof(entries));
        hr = IDirectDraw7_CreatePalette(ddraw, DDPCAPS_8BIT | DDPCAPS_ALLOW256, entries, &palette, NULL);
        ok(hr == DD_OK, "CreatePalette returned %#lx.\n", hr);
        if (FAILED(hr)) goto done;
        memset(&desc, 0, sizeof(desc));
        desc.dwSize = sizeof(desc);
        desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
        desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
        desc.dwWidth = desc.dwHeight = 16;
        desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
        desc.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;
        desc.ddpfPixelFormat.dwRGBBitCount = 8;
        hr = IDirectDraw7_CreateSurface(ddraw, &desc, &src, NULL);
        ok(hr == DD_OK, "Create P8 surface returned %#lx.\n", hr);
        if (FAILED(hr)) goto done;
        desc.dwWidth = desc.dwHeight = 20;
        desc.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
        desc.ddpfPixelFormat.dwRGBBitCount = 32;
        desc.ddpfPixelFormat.dwRBitMask = 0x00ff0000;
        desc.ddpfPixelFormat.dwGBitMask = 0x0000ff00;
        desc.ddpfPixelFormat.dwBBitMask = 0x000000ff;
        desc.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;
        hr = IDirectDraw7_CreateSurface(ddraw, &desc, &dst, NULL);
        ok(hr == DD_OK, "Create RGB surface returned %#lx.\n", hr);
        if (FAILED(hr)) goto done;

        for (phase = 0; phase < 4; ++phase)
        {
            trace("P8_MATRIX_BEGIN life=%u phase=%u\n", life, phase);
            for (i = 0; i < 256; ++i)
            {
                entries[i].peRed = i ^ (0x55 + phase + life * 7);
                entries[i].peGreen = 255 - i;
                entries[i].peBlue = i * 3 + phase;
                entries[i].peFlags = 0;
            }
            hr = IDirectDrawPalette_SetEntries(palette, 0, 0, 256, entries);
            ok(hr == DD_OK, "SetEntries returned %#lx.\n", hr);
            hr = IDirectDrawSurface7_SetPalette(primary, palette);
            ok(hr == DD_OK, "Set primary palette returned %#lx.\n", hr);
            hr = IDirectDrawSurface7_SetPalette(src, phase == 2 ? NULL : palette);
            ok(hr == DD_OK, "SetPalette returned %#lx.\n", hr);
            memset(&lock, 0, sizeof(lock));
            lock.dwSize = sizeof(lock);
            hr = IDirectDrawSurface7_Lock(src, NULL, &lock, DDLOCK_WAIT, NULL);
            ok(hr == DD_OK, "Lock source returned %#lx.\n", hr);
            if (FAILED(hr)) goto done;
            for (y = 0; y < 16; ++y)
                for (x = 0; x < 16; ++x)
                    ((BYTE *)lock.lpSurface)[y * lock.lPitch + x] = x + y * 16 + phase * 37 + life * 13;
            hr = IDirectDrawSurface7_Unlock(src, NULL);
            ok(hr == DD_OK, "Unlock source returned %#lx.\n", hr);
            hr = IDirectDrawSurface7_Lock(dst, NULL, &lock, DDLOCK_WAIT, NULL);
            ok(hr == DD_OK, "Lock destination returned %#lx.\n", hr);
            if (FAILED(hr)) goto done;
            for (y = 0; y < 20; ++y)
                for (x = 0; x < 20; ++x)
                    ((DWORD *)((BYTE *)lock.lpSurface + y * lock.lPitch))[x] = 0xff2468ac;
            hr = IDirectDrawSurface7_Unlock(dst, NULL);
            ok(hr == DD_OK, "Unlock destination returned %#lx.\n", hr);
            hr = IDirectDrawSurface7_Blt(dst, &rect, src, NULL, DDBLT_WAIT, NULL);
            ok(hr == DD_OK, "Partial P8 conversion returned %#lx.\n", hr);
            trace("P8_MATRIX_BLT life=%u phase=%u hr=%08lx\n", life, phase, hr);
            hr = IDirectDrawSurface7_Lock(dst, NULL, &lock, DDLOCK_WAIT | DDLOCK_READONLY, NULL);
            ok(hr == DD_OK, "Readback returned %#lx.\n", hr);
            if (FAILED(hr)) goto done;
            for (y = 0; y < 20; ++y)
                for (x = 0; x < 20; ++x)
                {
                    expected = 0xff2468ac;
                    if (x >= 2 && x < 18 && y >= 2 && y < 18)
                    {
                        i = (x - 2 + (y - 2) * 16 + phase * 37 + life * 13) & 255;
                        expected = 0xff000000 | i * 0x00010101;
                    }
                    color = ((DWORD *)((BYTE *)lock.lpSurface + y * lock.lPitch))[x];
                    trace("P8_MATRIX_PIXEL life=%u phase=%u x=%u y=%u color=%08lx expected=%08lx\n",
                            life, phase, x, y, color, expected);
                    ok(color == expected, "Life %u phase %u pixel %u,%u got %#lx expected %#lx.\n",
                            life, phase, x, y, color, expected);
                }
            hr = IDirectDrawSurface7_Unlock(dst, NULL);
            ok(hr == DD_OK, "Readback unlock returned %#lx.\n", hr);
            trace("P8_MATRIX_END life=%u phase=%u\n", life, phase);
            /* The capture signature appears only after the offscreen copy. */
            for (i = 0; i < 256; ++i) entries[i].peBlue ^= 0x80;
            hr = IDirectDrawPalette_SetEntries(palette, 0, 0, 256, entries);
            ok(hr == DD_OK, "Post-copy palette update returned %#lx.\n", hr);
            memset(&fx, 0, sizeof(fx));
            fx.dwSize = sizeof(fx);
            fx.dwFillColor = 23;
            SetRect(&fill, 0, 0, 320, 480);
            hr = IDirectDrawSurface7_Blt(primary, &fill, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &fx);
            ok(hr == DD_OK, "Fill primary left returned %#lx.\n", hr);
            fx.dwFillColor = 129;
            SetRect(&fill, 320, 0, 640, 480);
            hr = IDirectDrawSurface7_Blt(primary, &fill, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &fx);
            ok(hr == DD_OK, "Fill primary right returned %#lx.\n", hr);
            trace("P8_DISPLAY_SHOW life=%u phase=%u\n", life, phase);
            Sleep(250);

        }
done:
        if (display_changed)
        {
            hr = IDirectDraw7_RestoreDisplayMode(ddraw);
            ok(hr == DD_OK, "RestoreDisplayMode returned %#lx.\n", hr);
        }
        if (primary) { refs = IDirectDrawSurface7_Release(primary); ok(!refs, "Primary refs %lu.\n", refs); }
        if (src) { refs = IDirectDrawSurface7_Release(src); ok(!refs, "Source refs %lu.\n", refs); }
        if (dst) { refs = IDirectDrawSurface7_Release(dst); ok(!refs, "Destination refs %lu.\n", refs); }
        if (palette) { refs = IDirectDrawPalette_Release(palette); ok(!refs, "Palette refs %lu.\n", refs); }
        refs = IDirectDraw7_Release(ddraw);
        ok(!refs, "DirectDraw refs %lu.\n", refs);
        DestroyWindow(window);
    }
}

START_TEST(ddraw7)
{
    HMODULE module = GetModuleHandleA("ddraw.dll");
    char module_path[MAX_PATH];
    DWORD module_length;

    if (!(pDirectDrawCreateEx = (void *)GetProcAddress(module, "DirectDrawCreateEx")))
    {
        win_skip("DirectDrawCreateEx not available, skipping tests.\n");
        return;
    }
    module_length = GetModuleFileNameA(module, module_path, sizeof(module_path));
    trace("P8_MODULE ddraw.dll length=%lu path=%s\n", module_length, module_length ? module_path : "unavailable");
    trace("P8_SELECTED_BEGIN test_p8_blit\n");
    test_p8_blit();
    test_p8_display();
    trace("P8_SELECTED_END test_p8_blit\n");
}
