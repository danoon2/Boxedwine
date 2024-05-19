// Ported from the mac driver by James Bryant

/*
 * Mac driver window surface implementation
 *
 * Copyright 1993, 1994, 2011 Alexandre Julliard
 * Copyright 2006 Damjan Jovanovic
 * Copyright 2012, 2013 Ken Thomases for CodeWeavers, Inc.
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
#if 0
MAKE_DEP_UNIX
#endif

#if BOXED_WINE_VERSION <= 7110
#define WINE_UNIX_LIB
#endif
//#include "config.h"

#include "wineboxed.h"

#include <stdlib.h>
#include <limits.h>
#include <pthread.h>

#include "wine/debug.h"
#include "winreg.h"
#include "wine/gdi_driver.h"

//#include "winternl.h"
//#include "winnt.h"
//#include "unixlib.h"


#if BOXED_WINE_VERSION >= 7120
#define OffsetRgn NtGdiOffsetRgn
#define CombineRgn NtGdiCombineRgn
#define CreateRectRgn NtGdiCreateRectRgn
#define DeleteObject NtGdiDeleteObjectApp
#define GetRegionData NtGdiGetRegionData
#define LPtoDP(x, y, z) NtGdiTransformPoints(x, y, y, z, NtGdiLPtoDP);
#define HeapAlloc(x, y, z) calloc(1, z)
#define HeapFree(x, y, z) free(z)
#define GetWindowLongW NtUserGetWindowLongW
#define GetAncestor NtUserGetAncestor
#define GetDesktopWindow NtUserGetDesktopWindow
#define GetWindowRect NtUserGetWindowRect
#define GetWindowThreadProcessId NtUserGetWindowThread
#endif

WINE_DEFAULT_DEBUG_CHANNEL(boxeddrv);


/* only for use on sanitized BITMAPINFO structures */
static inline int get_dib_info_size(const BITMAPINFO *info, UINT coloruse)
{
    if (info->bmiHeader.biCompression == BI_BITFIELDS)
        return sizeof(BITMAPINFOHEADER) + 3 * sizeof(DWORD);
    if (coloruse == DIB_PAL_COLORS)
        return sizeof(BITMAPINFOHEADER) + info->bmiHeader.biClrUsed * sizeof(WORD);
    TRACE("biClrUsed=%d\n", (int)info->bmiHeader.biClrUsed);
    return FIELD_OFFSET(BITMAPINFO, bmiColors[info->bmiHeader.biClrUsed]);
}

static inline int get_dib_stride(int width, int bpp)
{
    return ((width * bpp + 31) >> 3) & ~3;
}

static inline int get_dib_image_size(const BITMAPINFO *info)
{
    return get_dib_stride(info->bmiHeader.biWidth, info->bmiHeader.biBitCount)
        * abs(info->bmiHeader.biHeight);
}

static inline void reset_bounds(RECT *bounds)
{
    bounds->left = bounds->top = INT_MAX;
    bounds->right = bounds->bottom = INT_MIN;
}


struct boxeddrv_window_surface
{
    struct window_surface   header;
    HWND                    window;
    RECT                    bounds;
    HRGN                    region;
    HRGN                    drawn;
    BOOL                    use_alpha;
    RGNDATA                *blit_data;
    BYTE                   *bits;
    pthread_mutex_t         mutex;
    BITMAPINFO              info;   /* variable size, must be last */
};

static struct boxeddrv_window_surface *get_boxed_surface(struct window_surface *surface)
{
    return (struct boxeddrv_window_surface *)surface;
}

RGNDATA *get_region_data(HRGN hrgn, HDC hdc_lptodp)
{
    RGNDATA *data;
    DWORD size;
    int i;
    RECT *rect;

    if (!hrgn || !(size = GetRegionData(hrgn, 0, NULL))) return NULL;
    if (!(data = HeapAlloc(GetProcessHeap(), 0, size))) return NULL;
    if (!GetRegionData(hrgn, size, data))
    {
        HeapFree(GetProcessHeap(), 0, data);
        return NULL;
    }

    rect = (RECT *)data->Buffer;
    if (hdc_lptodp)  /* map to device coordinates */
    {        
        LPtoDP(hdc_lptodp, (POINT *)rect, data->rdh.nCount * 2);
        for (i = 0; i < data->rdh.nCount; i++)
        {
            if (rect[i].right < rect[i].left)
            {
                INT tmp = rect[i].right;
                rect[i].right = rect[i].left;
                rect[i].left = tmp;
            }
            if (rect[i].bottom < rect[i].top)
            {
                INT tmp = rect[i].bottom;
                rect[i].bottom = rect[i].top;
                rect[i].top = tmp;
            }
        }
    }
    return data;
}

/***********************************************************************
 *              update_blit_data
 */
static void update_blit_data(struct boxeddrv_window_surface *surface)
{
    HeapFree(GetProcessHeap(), 0, surface->blit_data);
    surface->blit_data = NULL;

    if (surface->drawn)
    {
        HRGN blit = CreateRectRgn(0, 0, 0, 0);

        if (CombineRgn(blit, surface->drawn, 0, RGN_COPY) > NULLREGION &&
            (!surface->region || CombineRgn(blit, blit, surface->region, RGN_AND) > NULLREGION) &&
            OffsetRgn(blit, surface->header.rect.left, surface->header.rect.top) > NULLREGION)
            surface->blit_data = get_region_data(blit, 0);

        DeleteObject(blit);
    }
}

/***********************************************************************
 *              boxeddrv_surface_lock
 */
static void boxeddrv_surface_lock(struct window_surface *window_surface)
{
    struct boxeddrv_window_surface *surface = get_boxed_surface(window_surface);

    pthread_mutex_lock(&surface->mutex);
}

/***********************************************************************
 *              boxeddrv_surface_unlock
 */
static void boxeddrv_surface_unlock(struct window_surface *window_surface)
{
    struct boxeddrv_window_surface *surface = get_boxed_surface(window_surface);

    pthread_mutex_unlock(&surface->mutex);
}

/***********************************************************************
 *              boxeddrv_surface_get_bitmap_info
 */
static void *boxeddrv_surface_get_bitmap_info(struct window_surface *window_surface,
                                            BITMAPINFO *info)
{
    struct boxeddrv_window_surface *surface = get_boxed_surface(window_surface);

    TRACE("sizeof(BITMAPINFO)=%d get_dib_info_size(&surface->info, DIB_RGB_COLORS)=%d\n", sizeof(BITMAPINFO), get_dib_info_size(&surface->info, DIB_RGB_COLORS));
    memcpy(info, &surface->info, get_dib_info_size(&surface->info, DIB_RGB_COLORS));
    return surface->bits;
}

/***********************************************************************
 *              boxeddrv_surface_get_bounds
 */
static RECT *boxeddrv_surface_get_bounds(struct window_surface *window_surface)
{
    struct boxeddrv_window_surface *surface = get_boxed_surface(window_surface);

    TRACE("surface = %p bounds = %s\n", window_surface, wine_dbgstr_rect(&surface->bounds));
    return &surface->bounds;
}

/***********************************************************************
 *              boxeddrv_surface_set_region
 */
static void boxeddrv_surface_set_region(struct window_surface *window_surface, HRGN region)
{
    struct boxeddrv_window_surface *surface = get_boxed_surface(window_surface);

    TRACE("updating surface %p with %p\n", surface, region);

    window_surface->funcs->lock(window_surface);

    if (region)
    {
        if (!surface->region) surface->region = CreateRectRgn(0, 0, 0, 0);
        CombineRgn(surface->region, region, 0, RGN_COPY);
    }
    else
    {
        if (surface->region) DeleteObject(surface->region);
        surface->region = 0;
    }
    update_blit_data(surface);

    window_surface->funcs->unlock(window_surface);
}

UINT GDI_CDECL boxeddrv_RealizePaletteEntries(DWORD num_entries, PALETTEENTRY* entries);
static void boxeddrv_surface_flush(struct window_surface *window_surface)
{
    struct boxeddrv_window_surface *surface = get_boxed_surface(window_surface);
    HRGN region;
    BOOL isBoundsEmpty;
    window_surface->funcs->lock(window_surface);

    TRACE("flushing %p %s bounds %s bits %p\n", surface, wine_dbgstr_rect(&surface->header.rect),
          wine_dbgstr_rect(&surface->bounds), surface->bits);

    isBoundsEmpty = IsRectEmpty(&surface->bounds);
    if (!isBoundsEmpty && (region = CreateRectRgn(surface->bounds.left, surface->bounds.top,
        surface->bounds.right, surface->bounds.bottom)))
    {
        if (surface->drawn)
        {
            TRACE("drawn += bounds\n");
            CombineRgn(surface->drawn, surface->drawn, region, RGN_OR);
            DeleteObject(region);
        }
        else
        {
            TRACE("drawn = bounds\n");
            surface->drawn = region;
        }
    }
    update_blit_data(surface);
    reset_bounds(&surface->bounds);    

    if (!isBoundsEmpty)
    {
        //RECT r;

        //GetWindowRect(surface->window, &r);
        if (surface->blit_data) { // this can be changed to null sometimes, example: homeworld demo installer with wine 5.0
            boxeddrv_FlushSurface(surface->window, surface->bits, 0, 0, surface->info.bmiHeader.biWidth, surface->info.bmiHeader.biHeight, (RECT*)surface->blit_data->Buffer, surface->blit_data->rdh.nCount);
        }
    }
    window_surface->funcs->unlock(window_surface);
}

/***********************************************************************
 *              boxeddrv_surface_destroy
 */
static void boxeddrv_surface_destroy(struct window_surface *window_surface)
{
    struct boxeddrv_window_surface *surface = get_boxed_surface(window_surface);

    TRACE("freeing %p bits %p\n", surface, surface->bits);
    HeapFree(GetProcessHeap(), 0, surface->bits);
    pthread_mutex_destroy(&surface->mutex);
    HeapFree(GetProcessHeap(), 0, surface);
}

static const struct window_surface_funcs boxeddrv_surface_funcs =
{
    boxeddrv_surface_lock,
    boxeddrv_surface_unlock,
    boxeddrv_surface_get_bitmap_info,
    boxeddrv_surface_get_bounds,
    boxeddrv_surface_set_region,
    boxeddrv_surface_flush,
    boxeddrv_surface_destroy,
};

UINT GDI_CDECL boxeddrv_GetSystemPaletteEntries( PHYSDEV dev, UINT start, UINT count, LPPALETTEENTRY entries );
static void set_color_info(BITMAPINFO *info)
{
    DWORD *colors = (DWORD *)((char *)info + info->bmiHeader.biSize);

    info->bmiHeader.biCompression = BI_RGB;
    info->bmiHeader.biClrUsed = 0;

    TRACE("biBitCount=%d\n", info->bmiHeader.biBitCount);
    switch (info->bmiHeader.biBitCount)
    {
    case 4:
    case 8:
    {
        RGBQUAD *rgb = (RGBQUAD *)colors;
        PALETTEENTRY palette[256];
        UINT i, count;

        info->bmiHeader.biClrUsed = 1 << info->bmiHeader.biBitCount;
        count = boxeddrv_GetSystemPaletteEntries(NULL, 0, info->bmiHeader.biClrUsed, palette);
        for (i = 0; i < count; i++)
        {
            rgb[i].rgbRed   = palette[i].peRed;
            rgb[i].rgbGreen = palette[i].peGreen;
            rgb[i].rgbBlue  = palette[i].peBlue;
            rgb[i].rgbReserved = 0;
        }
        memset( &rgb[count], 0, (info->bmiHeader.biClrUsed - count) * sizeof(*rgb) );
        break;
    }
    case 16:
        colors[0] = 0xF800;
        colors[1] = 0x7E0;
        colors[2] = 0x1F;
        info->bmiHeader.biCompression = BI_BITFIELDS;
        break;
    case 32:
        colors[0] = 0x00ff0000;
        colors[1] = 0x0000ff00;
        colors[2] = 0x000000ff;
        break;
    }
}

/***********************************************************************
 *              create_surface
 */
INT GDI_CDECL boxeddrv_GetDeviceCaps(PHYSDEV dev, INT cap);
struct window_surface *create_surface(HWND window, const RECT *rect, struct window_surface *old_surface, BOOL use_alpha)
{
    struct boxeddrv_window_surface *surface;
    struct boxeddrv_window_surface *old_boxed_surface = get_boxed_surface(old_surface);
    int width = rect->right - rect->left, height = rect->bottom - rect->top;
    pthread_mutexattr_t attr;
    int err;

    surface = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                        FIELD_OFFSET(struct boxeddrv_window_surface, info.bmiColors[256]));
    if (!surface) return NULL;

    err = pthread_mutexattr_init(&attr);
    if (!err)
    {
        err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        if (!err)
            err = pthread_mutex_init(&surface->mutex, &attr);
        pthread_mutexattr_destroy(&attr);
    }
    if (err)
    {
        HeapFree(GetProcessHeap(), 0, surface);
        return NULL;
    }

    surface->info.bmiHeader.biSize        = sizeof(surface->info.bmiHeader);
    surface->info.bmiHeader.biWidth       = width;
    surface->info.bmiHeader.biHeight      = height; /* bottom-up */
    surface->info.bmiHeader.biPlanes      = 1;
    surface->info.bmiHeader.biBitCount    = boxeddrv_GetDeviceCaps(NULL, BITSPIXEL);
    if (surface->info.bmiHeader.biBitCount<=8)
        surface->info.bmiHeader.biBitCount = 32;
    surface->info.bmiHeader.biSizeImage   = get_dib_image_size(&surface->info);
    surface->info.bmiHeader.biCompression = BI_RGB;
    surface->info.bmiHeader.biClrUsed     = 0;
    set_color_info(&surface->info);

    surface->header.funcs = &boxeddrv_surface_funcs;
    surface->header.rect  = *rect;
    surface->header.ref   = 1;
    surface->window = window;
    reset_bounds(&surface->bounds);
    if (old_boxed_surface && old_boxed_surface->drawn)
    {
        surface->drawn = CreateRectRgn(rect->left, rect->top, rect->right, rect->bottom);
        OffsetRgn(surface->drawn, -rect->left, -rect->top);
        if (CombineRgn(surface->drawn, surface->drawn, old_boxed_surface->drawn, RGN_AND) <= NULLREGION)
        {
            DeleteObject(surface->drawn);
            surface->drawn = 0;
        }
    }
    update_blit_data(surface);
    surface->use_alpha = use_alpha;
    surface->bits = HeapAlloc(GetProcessHeap(), 0, surface->info.bmiHeader.biSizeImage);
    if (!surface->bits) goto failed;
    memset(surface->bits, 0x00, surface->info.bmiHeader.biSizeImage);

    TRACE("created %p for %p %s bits %p-%p\n", surface, window, wine_dbgstr_rect(rect),
          surface->bits, surface->bits + surface->info.bmiHeader.biSizeImage);

    return &surface->header;

failed:
    boxeddrv_surface_destroy(&surface->header);
    return NULL;
}


/***********************************************************************
*              surface_clip_to_visible_rect
*
* Intersect the accumulated drawn region with a new visible rect,
* effectively discarding stale drawing in the surface slack area.
*/
void surface_clip_to_visible_rect(struct window_surface *window_surface, const RECT *visible_rect)
{
    struct boxeddrv_window_surface *surface = get_boxed_surface(window_surface);

    window_surface->funcs->lock(window_surface);

    if (surface->drawn)
    {
        RECT rect;
        HRGN region;

        rect = *visible_rect;
        OffsetRect(&rect, -rect.left, -rect.top);

        if ((region = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom)))
        {
            CombineRgn(surface->drawn, surface->drawn, region, RGN_AND);
            DeleteObject(region);

            update_blit_data(surface);
        }
    }

    window_surface->funcs->unlock(window_surface);
}

#if BOXED_WINE_VERSION >= 6090
BOOL WINE_CDECL boxeddrv_WindowPosChanging(HWND hwnd, HWND insert_after, UINT swp_flags, const RECT* window_rect, const RECT* client_rect, RECT* visible_rect, struct window_surface** surface) {
#else
void WINE_CDECL boxeddrv_WindowPosChanging(HWND hwnd, HWND insert_after, UINT swp_flags, const RECT * window_rect, const RECT * client_rect, RECT * visible_rect, struct window_surface** surface) {
#endif
    DWORD style = GetWindowLongW(hwnd, GWL_STYLE);
    struct window_surface* oldSurface = NULL;
    HWND parent = GetAncestor(hwnd, GA_PARENT);

    initEvents();
    TRACE("hwnd=%p (parent=%p) insert_after=%p swp_flags=0x%08x window_rect=%s client_rect=%s visible_rect=%s surface=%p\n", hwnd, parent, insert_after, swp_flags, wine_dbgstr_rect(window_rect), wine_dbgstr_rect(client_rect), wine_dbgstr_rect(visible_rect), surface);

    if (GetWindowThreadProcessId(hwnd, NULL) != GetCurrentThreadId()) {
#if BOXED_WINE_VERSION >= 6090
        return TRUE;
#else
        return;
#endif
    }

    if (!parent)  /* desktop */
    {
#if BOXED_WINE_VERSION >= 6090
        return TRUE;
#else
        return;
#endif
    }

    /* don't create wnd for HWND_MESSAGE windows */
    if (parent != GetDesktopWindow() && !GetAncestor(parent, GA_PARENT)) {
#if BOXED_WINE_VERSION >= 6090
        return TRUE;
#else
        return;
#endif
    }
    if (*surface) {
        oldSurface = *surface;
        TRACE("     setting old surface %p (ref=%d)\n", *surface, (int)(*surface)->ref);
    }
    *surface = NULL;

    *visible_rect = *window_rect;
    if (swp_flags & SWP_HIDEWINDOW) {
#if BOXED_WINE_VERSION >= 6090
        return TRUE;
#else
        return;
#endif
    }
    CALL_NORETURN_7(BOXED_WINDOW_POS_CHANGING, hwnd, insert_after, swp_flags, window_rect, client_rect, visible_rect, surface);

    if (parent != GetDesktopWindow()) {
#if BOXED_WINE_VERSION >= 6090
        return TRUE;
#else
        return;
#endif
    }
    /*
    if (*surface) {
        int surfaceWidth = (*surface)->rect.right - (*surface)->rect.left;
        int surfaceHeight = (*surface)->rect.bottom - (*surface)->rect.top;
        int windowWidth = window_rect->right - window_rect->left;
        int windowHeight = window_rect->bottom - window_rect->top;

        if (oldSurface)  {
            TRACE("     releasing old surface %p (ref=%d)\n", oldSurface, oldSurface->ref);
            window_surface_release(oldSurface);
        }
        TRACE("     checking existing surface %p (ref=%d)\n", *surface, (*surface)->ref);
        if (surfaceWidth==windowWidth && surfaceHeight==windowHeight) {
            // use existing surface
            surface_clip_to_visible_rect(*surface, visible_rect);
            window_surface_add_ref(*surface);
            return;
        }
    }
    */
    if (oldSurface) {
        TRACE("     releasing old surface %p (ref=%d)\n", oldSurface, (int)oldSurface->ref);
        window_surface_release(oldSurface);
        *surface = NULL;
    }
    if ((swp_flags & SWP_SHOWWINDOW) || (style & WS_VISIBLE)) {
        RECT rc;
        rc.left = 0;
        rc.right = window_rect->right - window_rect->left;
        rc.top = 0;
        rc.bottom = window_rect->bottom - window_rect->top;
        if (rc.right && rc.bottom) {
            *surface = create_surface(hwnd, &rc, *surface, FALSE);
            TRACE("     created new surface %p (ref=%d)\n", *surface, (int)(*surface)->ref);
        }
    }
#if BOXED_WINE_VERSION >= 6090
    return TRUE;
#endif
}

BOOL GDI_CDECL boxeddrv_GetDeviceGammaRamp(PHYSDEV dev, LPVOID ramp) {
    int result;
    CALL_2(BOXED_GET_DEVICE_GAMMA_RAMP, dev, ramp);
    TRACE("dev=%p ramp=%p result=%d\n", dev, ramp, result);
    return (BOOL)result;
}

BOOL GDI_CDECL boxeddrv_SetDeviceGammaRamp(PHYSDEV dev, LPVOID ramp) {
    int result;
    CALL_2(BOXED_SET_DEVICE_GAMMA_RAMP, dev, ramp);
    TRACE("dev=%p ramp=%p result=%d\n", dev, ramp, result);
    return (BOOL)result;
}

static void boxeddrv_SetSurface(HWND hwnd, struct window_surface* surface) {
    CALL_NORETURN_2(BOXED_SET_SURFACE, hwnd, surface);
}

static struct window_surface* boxeddrv_GetSurface(HWND hwnd) {
    struct window_surface* result;
    CALL_1(BOXED_GET_SURFACE, hwnd);
    return result;
}

static BOOL boxeddrv_HasWnd(HWND hwnd) {
    int result;
    CALL_1(BOXED_HAS_WND, hwnd);
    return (BOOL)result;
}

void WINE_CDECL boxeddrv_WindowPosChanged(HWND hwnd, HWND insert_after, UINT swp_flags, const RECT* window_rect, const RECT* client_rect, const RECT* visible_rect, const RECT* valid_rects, struct window_surface* surface) {
    DWORD new_style = GetWindowLongW(hwnd, GWL_STYLE);
    struct window_surface* oldSurface = boxeddrv_GetSurface(hwnd);
    RECT r;

    if (!boxeddrv_HasWnd(hwnd))
        return;

    GetWindowRect(hwnd, &r);
    TRACE("hwnd=%p insert_after=%p swp_flags=0x%08x window_rect=%s client_rect=%s visible_rect=%s valid_rects=%s surface=%p style=0x%08x GetWindowRect()=%s\n", hwnd, insert_after, swp_flags, wine_dbgstr_rect(window_rect), wine_dbgstr_rect(client_rect), wine_dbgstr_rect(visible_rect), wine_dbgstr_rect(valid_rects), surface, (int)new_style, wine_dbgstr_rect(&r));
    if (surface) {
        TRACE("     using new surface %p (ref=%d)\n", surface, (int)surface->ref);
        window_surface_add_ref(surface);
    }
    if (oldSurface) {
        TRACE("     releasing old surface %p (ref=%d)\n", oldSurface, (int)oldSurface->ref);
        window_surface_release(oldSurface);
    }
    boxeddrv_SetSurface(hwnd, surface);
    CALL_NORETURN_8(BOXED_WINDOW_POS_CHANGED, hwnd, insert_after, swp_flags, window_rect, client_rect, visible_rect, valid_rects, new_style);
}
