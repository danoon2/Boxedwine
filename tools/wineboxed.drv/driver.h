#ifndef DRIVER_H
UINT GDI_CDECL boxeddrv_RealizePalette(PHYSDEV dev, HPALETTE hpal, BOOL primary);
UINT GDI_CDECL boxeddrv_GetSystemPaletteEntries(PHYSDEV dev, UINT start, UINT count, LPPALETTEENTRY entries);
COLORREF GDI_CDECL boxeddrv_GetNearestColor(PHYSDEV dev, COLORREF color);
UINT GDI_CDECL boxeddrv_RealizeDefaultPalette(PHYSDEV dev);

#if BOXED_WINE_VERSION >= 6090
BOOL WINE_CDECL boxeddrv_WindowPosChanging(HWND hwnd, HWND insert_after, UINT swp_flags, const RECT* window_rect, const RECT* client_rect, RECT* visible_rect, struct window_surface** surface);
#else
void WINE_CDECL boxeddrv_WindowPosChanging(HWND hwnd, HWND insert_after, UINT swp_flags, const RECT* window_rect, const RECT* client_rect, RECT* visible_rect, struct window_surface** surface);
#endif

INT GDI_CDECL boxeddrv_GetDeviceCaps(PHYSDEV dev, INT cap);
BOOL GDI_CDECL boxeddrv_SetDeviceGammaRamp(PHYSDEV dev, LPVOID ramp);
BOOL GDI_CDECL boxeddrv_GetDeviceGammaRamp(PHYSDEV dev, LPVOID ramp);
void WINE_CDECL boxeddrv_WindowPosChanged(HWND hwnd, HWND insert_after, UINT swp_flags, const RECT* window_rect, const RECT* client_rect, const RECT* visible_rect, const RECT* valid_rects, struct window_surface* surface);
BOOL GDI_CDECL boxeddrv_DeleteDC(PHYSDEV dev);

#if WINE_GDI_DRIVER_VERSION >= 75
#if WINE_WGL_DRIVER_VERSION >= 22
struct opengl_funcs* boxeddrv_wine_get_wgl_driver(UINT version);
#else
struct opengl_funcs* CDECL boxeddrv_wine_get_wgl_driver(UINT version);
#endif
#else
struct opengl_funcs* CDECL boxeddrv_wine_get_wgl_driver(PHYSDEV hdc, UINT version);
#endif

#if WINE_VULKAN_DRIVER_VERSION >= 11
const struct vulkan_funcs* boxeddrv_wine_get_vulkan_driver(UINT version);
#elif WINE_GDI_DRIVER_VERSION >= 74
const struct vulkan_funcs* CDECL boxeddrv_wine_get_vulkan_driver(UINT version);
#else
const struct vulkan_funcs* CDECL boxeddrv_wine_get_vulkan_driver(PHYSDEV hdc, UINT version);
#endif

BOOL WINE_CDECL boxeddrv_EnumDisplaySettingsEx(LPCWSTR devname, DWORD mode, LPDEVMODEW devmode, DWORD flags);

#if WINE_GDI_DRIVER_VERSION >= 81
BOOL WINE_CDECL boxedwine_UpdateDisplayDevices(const struct gdi_device_manager* device_manager, BOOL force, void* param);
#elif WINE_GDI_DRIVER_VERSION >= 70
void WINE_CDECL boxedwine_UpdateDisplayDevices(const struct gdi_device_manager* device_manager, BOOL force, void* param);
#endif

#endif