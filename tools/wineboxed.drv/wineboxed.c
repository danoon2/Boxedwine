/*
 * Boxedwine graphics driver initialisation functions
 *
 * Copyright 1996 Alexandre Julliard
 * Copyright 2011012013 Ken Thomases for CodeWeavers, Inc.
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
 * Foundation, Inc.1 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#if 0
MAKE_DEP_UNIX
#endif
#if BOXED_WINE_VERSION <= 7110
#define WINE_UNIX_LIB
#endif

#include "config.h"
#include "wineboxed.h"
#include "wine/debug.h"
#include "winreg.h"
#include "winternl.h"
#include "winnt.h"
#include "shellapi.h"
#include "wine/server.h"

#if BOXED_WINE_VERSION <= 7110
#define WINE_UNIX_LIB
#undef wcsnicmp
#define wcsnicmp strncmpiW
#undef wcstol
#define wcstol strtolW
#include INCLUDE_UNICODE
#endif

// needed for wine 3 build
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif

#if BOXED_WINE_VERSION >= 7120

static LRESULT send_message_timeout(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam,
    UINT flags, UINT timeout, PDWORD_PTR res_ptr)
{
    struct send_message_timeout_params params = { .flags = flags, .timeout = timeout };
    LRESULT res = NtUserMessageCall(hwnd, msg, wparam, lparam, &params,
        NtUserSendMessageTimeout, FALSE);
    if (res_ptr) *res_ptr = params.result;
    return res;
}

static BOOL get_icon_info(HICON handle, ICONINFOEXW* ret)
{
    UNICODE_STRING module, res_name;
    ICONINFO info;

    module.Buffer = ret->szModName;
    module.MaximumLength = sizeof(ret->szModName) - sizeof(WCHAR);
    res_name.Buffer = ret->szResName;
    res_name.MaximumLength = sizeof(ret->szResName) - sizeof(WCHAR);
    if (!NtUserGetIconInfo(handle, &info, &module, &res_name, NULL, 0)) return FALSE;
    ret->fIcon = info.fIcon;
    ret->xHotspot = info.xHotspot;
    ret->yHotspot = info.yHotspot;
    ret->hbmColor = info.hbmColor;
    ret->hbmMask = info.hbmMask;
    ret->wResID = res_name.Length ? 0 : LOWORD(res_name.Buffer);
    ret->szModName[module.Length] = 0;
    ret->szResName[res_name.Length] = 0;
    return TRUE;
}

static HWND get_focus(void)
{
    GUITHREADINFO info;
    info.cbSize = sizeof(info);
    return NtUserGetGUIThreadInfo(GetCurrentThreadId(), &info) ? info.hwndFocus : 0;
}

#define HeapAlloc(x, y, z) calloc(1, z)
#define HeapFree(x, y, z) free(z)
#define GetWindowLongW NtUserGetWindowLongW
#define GetDesktopWindow NtUserGetDesktopWindow
#define SetWindowPos NtUserSetWindowPos
#define GetForegroundWindow NtUserGetForegroundWindow
#if BOXED_WINE_VERSION >= 8140
#define SendMessageW(hwnd, msg, wparam, lparam) NtUserMessageCall(hwnd, msg, wparam, lparam, NULL, NtUserSendMessage, FALSE)
#elif BOXED_WINE_VERSION >= 7180
#define SendMessageW(hwnd, msg, wparam, lparam) NtUserMessageCall(hwnd, msg, wparam, lparam, NULL, NtUserSendDriverMessage, FALSE)
#else
#define SendMessageW(hwnd, msg, wparam, lparam) NtUserMessageCall(hwnd, msg, wparam, lparam, NULL, NtUserSendMessage, FALSE);
#endif
#define SendMessageTimeoutW send_message_timeout
#define GetTopWindow(hwnd) NtUserGetWindowRelative(hwnd ? hwnd : NtUserGetDesktopWindow(), GW_CHILD)
#define GetObjectW NtGdiExtGetObjectW
#define GetIconInfoExW get_icon_info
#define CreateCompatibleDC NtGdiCreateCompatibleDC
#define GetDIBits(hdc, hbm, start, cLines, lpvBits, lpbmi, usage) NtGdiGetDIBitsInternal(hdc, hbm, start, cLines, lpvBits, lpbmi, usage, 0, 0)
#define DeleteDC NtGdiDeleteObjectApp
#define DeleteObject NtGdiDeleteObjectApp
#define GetAncestor NtUserGetAncestor
#define GetWindow NtUserGetWindowRelative
#define SetForegroundWindow NtUserSetForegroundWindow
#define GetFocus get_focus
#define IsChild NtUserIsChild
#define GetWindowThreadProcessId NtUserGetWindowThread
#define RegCloseKey NtClose
#endif

WINE_DEFAULT_DEBUG_CHANNEL(boxeddrv);

static inline BOOL can_activate_window(HWND hwnd)
{
    LONG style = GetWindowLongW(hwnd, GWL_STYLE);

    if (!(style & WS_VISIBLE)) {
        TRACE("not visible\n");
        return FALSE;
    }
    if ((style & (WS_POPUP|WS_CHILD)) == WS_CHILD) {
        TRACE("child or popup\n");
        return FALSE;
    }
    if (GetWindowLongW(hwnd, GWL_EXSTYLE) & WS_EX_NOACTIVATE) {
        TRACE("not activate\n");
        return FALSE;
    }
    if (hwnd == GetDesktopWindow()) {
        TRACE("desktop\n");
        return FALSE;
    }
    if (style & WS_DISABLED) {
        TRACE("disabled\n");
    }
    TRACE("yes\n");
    return TRUE;
}

struct winZOrder {
    int count;
    HWND windows[1024];
};

BOOL getZOrderCallback(HWND hWnd, LPARAM lParam) {
    struct winZOrder* zorder = (struct winZOrder*)lParam;
    TRACE("hWnd=%p zorder->count=%d\n", hWnd, zorder->count);
    if (zorder->count < 1024) {
        zorder->windows[zorder->count++] = hWnd;
}
    return TRUE;
}
void boxeddrv_FlushSurface(HWND hwnd, void* bits, int xOrg, int yOrg, int width, int height, RECT* rects, int rectCount) {
    struct winZOrder zorder;
    HWND h = GetTopWindow(NULL);

    zorder.count = 0;
    while (h) {
        zorder.windows[zorder.count++] = h;
        h = GetWindow(h, GW_HWNDNEXT);
    }

    // EnumWindows((WNDENUMPROC)getZOrderCallback, (LPARAM)&zorder);
    TRACE("hwnd=%p bits=%p width=%d height=%d rects=%p rectCount=%d hWndCount=%d\n", hwnd, bits, width, height, rects, rectCount, zorder.count);
    CALL_NORETURN_9(BOXED_FLUSH_SURFACE, hwnd, bits, xOrg, yOrg, width, height, &zorder, rects, rectCount);
}

#if BOXED_WINE_VERSION >= 6080
BOOL WINE_CDECL boxeddrv_ActivateKeyboardLayout(HKL hkl, UINT flags) {
    int result;
    CALL_2(BOXED_ACTIVATE_KEYBOARD_LAYOUT, hkl, flags);
    TRACE("hkl=%p flags=0x%08x result=%d\n", hkl, flags, result);
    return result!=0;
}
#else
HKL WINE_CDECL boxeddrv_ActivateKeyboardLayout(HKL hkl, UINT flags) {
    int result;
    CALL_2(BOXED_ACTIVATE_KEYBOARD_LAYOUT, hkl, flags);
    TRACE("hkl=%p flags=0x%08x result=%d\n", hkl, flags, result);
    return (HKL)result;
}
#endif
void WINE_CDECL boxeddrv_Beep(void) {
    TRACE("\n");
    CALL_NORETURN_0(BOXED_BEEP);
}

#if BOXED_WINE_VERSION < 7130
ULONG query_reg_value(HKEY hkey, const WCHAR* name, KEY_VALUE_PARTIAL_INFORMATION* info, ULONG size)
{
    unsigned int name_size = name ? lstrlenW(name) * sizeof(WCHAR) : 0;
    UNICODE_STRING nameW = { name_size, name_size, (WCHAR*)name };

    if (NtQueryValueKey(hkey, &nameW, KeyValuePartialInformation,
        info, size, &size))
        return 0;

    return size - FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);
}

HKEY reg_open_key(HKEY root, const WCHAR* name, ULONG name_len)
{
    UNICODE_STRING nameW = { name_len, name_len, (WCHAR*)name };
    OBJECT_ATTRIBUTES attr;
    HANDLE ret;

    attr.Length = sizeof(attr);
    attr.RootDirectory = root;
    attr.ObjectName = &nameW;
    attr.Attributes = 0;
    attr.SecurityDescriptor = NULL;
    attr.SecurityQualityOfService = NULL;

    return NtOpenKeyEx(&ret, MAXIMUM_ALLOWED, &attr, 0) ? 0 : ret;
}

static inline UINT asciiz_to_unicode(WCHAR* dst, const char* src)
{
    WCHAR* p = dst;
    while ((*p++ = *src++));
    return (p - dst) * sizeof(WCHAR);
}

static HKEY get_display_device_reg_key(const WCHAR* device_name)
{
    static const WCHAR display[] = { '\\','\\','.','\\','D','I','S','P','L','A','Y' };
    static const WCHAR video_key[] = {
        '\\','R','e','g','i','s','t','r','y',
        '\\','M','a','c','h','i','n','e',
        '\\','H','A','R','D','W','A','R','E',
        '\\','D','E','V','I','C','E','M','A','P',
        '\\','V','I','D','E','O' };
    static const WCHAR current_config_key[] = {
        '\\','R','e','g','i','s','t','r','y',
        '\\','M','a','c','h','i','n','e',
        '\\','S','y','s','t','e','m',
        '\\','C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t',
        '\\','H','a','r','d','w','a','r','e',' ','P','r','o','f','i','l','e','s',
        '\\','C','u','r','r','e','n','t' };
    WCHAR value_name[MAX_PATH], buffer[4096], * end_ptr;
    KEY_VALUE_PARTIAL_INFORMATION* value = (void*)buffer;
    DWORD adapter_index, size;
    char adapter_name[100];
    HKEY hkey;

    /* Device name has to be \\.\DISPLAY%d */
    if (wcsnicmp(device_name, display, ARRAY_SIZE(display)))
        return FALSE;

    /* Parse \\.\DISPLAY* */
    adapter_index = wcstol(device_name + ARRAY_SIZE(display), &end_ptr, 10) - 1;
    if (*end_ptr)
        return FALSE;

    /* Open \Device\Video* in HKLM\HARDWARE\DEVICEMAP\VIDEO\ */
    if (!(hkey = reg_open_key(NULL, video_key, sizeof(video_key)))) return FALSE;
    sprintf(adapter_name, "\\Device\\Video%d", (int)adapter_index);
    asciiz_to_unicode(value_name, adapter_name);
    size = query_reg_value(hkey, value_name, value, sizeof(buffer));
    NtClose(hkey);
    if (!size || value->Type != REG_SZ) return FALSE;

    /* Replace \Registry\Machine\ prefix with HKEY_CURRENT_CONFIG */
    memmove(buffer + ARRAY_SIZE(current_config_key), (const WCHAR*)value->Data + 17,
        size - 17 * sizeof(WCHAR));
    memcpy(buffer, current_config_key, sizeof(current_config_key));
    TRACE("display device %s registry settings key %s.\n", wine_dbgstr_w(device_name),
        wine_dbgstr_w(buffer));
    return reg_open_key(NULL, buffer, lstrlenW(buffer) * sizeof(WCHAR));
}

static HANDLE get_display_device_init_mutex(void)
{
    static const WCHAR init_mutexW[] = { 'd','i','s','p','l','a','y','_','d','e','v','i','c','e','_','i','n','i','t' };
    UNICODE_STRING name = { sizeof(init_mutexW), sizeof(init_mutexW), (WCHAR*)init_mutexW };
    OBJECT_ATTRIBUTES attr;
    HANDLE mutex = 0;

    InitializeObjectAttributes(&attr, &name, OBJ_OPENIF, NULL, NULL);
    NtCreateMutant(&mutex, MUTEX_ALL_ACCESS, &attr, FALSE);
    if (mutex) NtWaitForSingleObject(mutex, FALSE, NULL);
    return mutex;
}

static void release_display_device_init_mutex(HANDLE mutex)
{
    NtReleaseMutant(mutex, NULL);
    NtClose(mutex);
}

static BOOL set_setting_value(HKEY hkey, const char* name, DWORD val)
{
    WCHAR nameW[128];
    UNICODE_STRING str = { asciiz_to_unicode(nameW, name) - sizeof(WCHAR), sizeof(nameW), nameW };
    return !NtSetValueKey(hkey, &str, 0, REG_DWORD, &val, sizeof(val));
}

static BOOL write_registry_settings(const WCHAR* device_name, const DEVMODEW* dm)
{
    WCHAR wine_x11_reg_key[MAX_PATH];
    HANDLE mutex;
    HKEY hkey;
    BOOL ret = TRUE;

    mutex = get_display_device_init_mutex();
    if (!(hkey = get_display_device_reg_key(device_name)))
    {
        release_display_device_init_mutex(mutex);
        return FALSE;
    }

    ret &= set_setting_value(hkey, "DefaultSettings.BitsPerPel", dm->dmBitsPerPel);
    ret &= set_setting_value(hkey, "DefaultSettings.XResolution", dm->dmPelsWidth);
    ret &= set_setting_value(hkey, "DefaultSettings.YResolution", dm->dmPelsHeight);
    ret &= set_setting_value(hkey, "DefaultSettings.VRefresh", dm->dmDisplayFrequency);
    ret &= set_setting_value(hkey, "DefaultSettings.Flags", dm->dmDisplayFlags);
    ret &= set_setting_value(hkey, "DefaultSettings.XPanning", dm->dmPosition.x);
    ret &= set_setting_value(hkey, "DefaultSettings.YPanning", dm->dmPosition.y);
    ret &= set_setting_value(hkey, "DefaultSettings.Orientation", dm->dmDisplayOrientation);
    ret &= set_setting_value(hkey, "DefaultSettings.FixedOutput", dm->dmDisplayFixedOutput);

#undef set_value

    RegCloseKey(hkey);
    release_display_device_init_mutex(mutex);
    return ret;
}
#endif

BOOL WINE_CDECL boxeddrv_EnumDisplaySettingsEx(LPCWSTR devname, DWORD mode, LPDEVMODEW devmode, DWORD flags) {
    int result;
    CALL_4(BOXED_ENUM_DISPLAY_SETTINGS_EX, devname, mode, devmode, flags);
    TRACE("devname=%s mode=%d devmode=%p flags=0x%08x result=%d\n", debugstr_w(devname), (int)mode, devmode, (int)flags, result);
    return (BOOL)result;
}

LONG WINE_CDECL boxeddrv_ChangeDisplaySettingsEx(LPCWSTR devname, LPDEVMODEW devmode, HWND hwnd, DWORD flags, LPVOID lpvoid) {
    LONG result;
    LONG cx, cy, bpp;

    TRACE("devname=%s devmode=%p hwnd=%p flags=0x%08x %p &result=%p &cx=%p &cy=%p\n", debugstr_w(devname), devmode, hwnd, (int)flags, lpvoid, &result, &cx, &cy);
    CALL_NORETURN_9(BOXED_CHANGE_DISPLAY_SETTINGS_EX, devname, devmode, hwnd, flags, lpvoid, &result, &cx, &cy, &bpp);
    TRACE("result=%d width=%d height=%d bpp=%d\n", (int)result, (int)cx, (int)cy, (int)bpp);
    if (result==DISP_CHANGE_SUCCESSFUL) {
#if BOXED_WINE_VERSION < 7130
        if (flags & CDS_UPDATEREGISTRY && devname && devmode) {
            if (!write_registry_settings(devname, devmode))
            {
                ERR("Failed to write %s display settings to registry.\n", wine_dbgstr_w(devname));
                return DISP_CHANGE_NOTUPDATED;
            }
        }
#endif
        if (flags & (CDS_TEST | CDS_NORESET)) {
            return result;
        }
#if BOXED_WINE_VERSION <= 7210
        BOXEDDRV_DisplayDevices_Init(TRUE);
        TRACE("SetWindowPos\n");
        SetWindowPos(GetDesktopWindow(), 0, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE | SWP_DEFERERASE );
        TRACE("SendMessageTimeoutW\n");
        SendMessageTimeoutW( HWND_BROADCAST, WM_DISPLAYCHANGE, bpp, MAKELPARAM( cx, cy ), SMTO_ABORTIFHUNG, 2000, NULL );       
        TRACE("SendMessageTimeoutW returned\n");
#endif
    }    
    return result;
}

#if BOXED_WINE_VERSION >= 7210
LONG WINE_CDECL boxeddrv_ChangeDisplaySettings(LPDEVMODEW devmode, LPCWSTR primary_name, HWND hwnd, DWORD flags, LPVOID lpvoid) {
#else
LONG WINE_CDECL boxeddrv_ChangeDisplaySettings(LPDEVMODEW devmode, HWND hwnd, DWORD flags, LPVOID lpvoid) {
#endif
    return boxeddrv_ChangeDisplaySettingsEx(NULL, devmode, hwnd, flags, lpvoid);
}

#if BOXED_WINE_VERSION >= 8100
BOOL WINE_CDECL boxeddrv_ClipCursor(const LPCRECT clip, BOOL reset) {
#else
BOOL WINE_CDECL boxeddrv_ClipCursor(LPCRECT clip) {
#endif
    int result;
    CALL_1(BOXED_CLIP_CURSOR, clip);
    TRACE("clip=%s result=%d\n", wine_dbgstr_rect(clip), result);
    return (BOOL)result;
}

void initDesktop(HWND hwnd) {
    unsigned int width, height;

    TRACE("%p\n", hwnd);

    /* retrieve the real size of the desktop */
    SERVER_START_REQ(get_window_rectangles)
    {
        req->handle = wine_server_user_handle(hwnd);
        req->relative = COORDS_CLIENT;
        wine_server_call(req);
        width  = reply->window.right;
        height = reply->window.bottom;
    }
    SERVER_END_REQ;

    if (!width && !height)  /* not initialized yet */
    {
        width = internal_GetDeviceCaps(DESKTOPHORZRES);
        height = internal_GetDeviceCaps(DESKTOPVERTRES);

        SERVER_START_REQ(set_window_pos)
        {
            req->handle        = wine_server_user_handle(hwnd);
            req->previous      = 0;
            req->swp_flags     = SWP_NOZORDER;
            req->window.left   = 0;
            req->window.top    = 0;
            req->window.right  = width;
            req->window.bottom = height;
            req->client        = req->window;
            wine_server_call(req);
        }
        SERVER_END_REQ;
    }
}
BOOL WINE_CDECL boxeddrv_CreateDesktopWindow(HWND hwnd) {
    int result;
    CALL_1(BOXED_CREATE_DESKTOP_WINDOW, hwnd);
    initDesktop(hwnd);
    return (BOOL)result;
}

void WINE_CDECL boxeddrv_SetDesktopWindow(HWND hwnd) {
    int result;
    CALL_1(BOXED_CREATE_DESKTOP_WINDOW, hwnd);
    initDesktop(hwnd);
}

BOOL WINE_CDECL boxeddrv_CreateWindow(HWND hwnd) {
    int result;
    CALL_1(BOXED_CREATE_WINDOW, hwnd);
    TRACE("hwnd=%p result=%d\n", hwnd, result);
    return (BOOL)result;
}

void WINE_CDECL boxeddrv_DestroyCursorIcon(HCURSOR cursor) {
    TRACE("cursor=%p\n", cursor);
    CALL_NORETURN_1(BOXED_DESTROY_CURSOR_ICON, cursor);
}

void WINE_CDECL boxeddrv_DestroyWindow(HWND hwnd) {
    TRACE("hwnd=%p\n", hwnd);
    CALL_NORETURN_1(BOXED_DESTROY_WINDOW, hwnd);

    if (hwnd == GetForegroundWindow())
    {
        SendMessageW(hwnd, WM_CANCELMODE, 0, 0);
        if (hwnd == GetForegroundWindow())
            SetForegroundWindow(GetDesktopWindow());
    }
}

BOOL WINE_CDECL boxeddrv_GetCursorPos(LPPOINT pos) {
    int result;
    CALL_1(BOXED_GET_CURSOR_POS, pos);
    TRACE("pos=%p(%d,%d) result=%d\n", pos, (int)pos->x, (int)pos->y, result);
    return result;
}

INT WINE_CDECL boxeddrv_GetKeyNameText(LONG lparam, LPWSTR buffer, INT size) {
    INT result;
    CALL_3(BOXED_GET_KEY_NAME, lparam, buffer, size);
    TRACE("lparam=0x%08x buffer=%p size=%d result=%d\n", (int)lparam, buffer, size, result);
    return result;
}

// removed in version 70?
BOOL WINE_CDECL boxeddrv_GetMonitorInfo(HMONITOR monitor, LPMONITORINFO info) {
    static const WCHAR adapter_name[] = { '\\','\\','.','\\','D','I','S','P','L','A','Y','1',0 };

    TRACE("monitor=%p info=%p\n", monitor, info);
    SetRect(&info->rcMonitor, 0, 0, internal_GetDeviceCaps(DESKTOPHORZRES), internal_GetDeviceCaps(DESKTOPVERTRES));
    SetRect(&info->rcWork, 0, 0, internal_GetDeviceCaps(DESKTOPHORZRES), internal_GetDeviceCaps(DESKTOPVERTRES));
    info->dwFlags = MONITORINFOF_PRIMARY;

    if (info->cbSize >= sizeof(MONITORINFOEXW))
        lstrcpyW(((MONITORINFOEXW*)info)->szDevice, adapter_name);
    return TRUE;
    //CALL_2(BOXED_GET_MONITOR_INFO, monitor, info);
}

UINT WINE_CDECL boxeddrv_MapVirtualKeyEx(UINT wCode, UINT wMapType, HKL hkl) {
    UINT result;
    CALL_3(BOXED_MAP_VIRTUAL_KEY_EX, wCode, wMapType, hkl);
    TRACE("wCode=%d wMapType=%d hkl=%p result=%d\n", wCode, wMapType, hkl, result);
    return result;
}

#if WINE_GDI_DRIVER_VERSION >= 78
NTSTATUS WINE_CDECL boxeddrv_MsgWaitForMultipleObjectsEx(DWORD count, const HANDLE* handles, const LARGE_INTEGER* timeout, DWORD mask, DWORD flags) {
#else
DWORD WINE_CDECL boxeddrv_MsgWaitForMultipleObjectsEx(DWORD count, const HANDLE *handles, DWORD timeout, DWORD mask, DWORD flags) {
#endif
    DWORD result;

    TRACE("count=%d handles=%p timeout=0x%08x mask=0x%08x flags=0x%08x\n", (int)count, handles, (int)timeout, (int)mask, (int)flags);
    initEvents();
    CALL_5(BOXED_MSG_WAIT_FOR_MULTIPLE_OBJECTS_EX, count, handles, timeout, mask, flags);
	if (processEvents(mask)) {
		return count - 1;
	}
    if (!count && !timeout) 
        return WAIT_TIMEOUT;  
#if WINE_GDI_DRIVER_VERSION >= 78
    result = NtWaitForMultipleObjects(count, handles, !(flags & MWMO_WAITALL), !!(flags & MWMO_ALERTABLE), timeout);
#else
    result = WaitForMultipleObjectsEx(count, handles, flags & MWMO_WAITALL, timeout, flags & MWMO_ALERTABLE);	
#endif
    if (result == count - 1) {
        processEvents(mask);
    }
    return result;
}

BOOL WINE_CDECL boxeddrv_ProcessEvents(DWORD mask) {
    return processEvents(mask);
}

void WINE_CDECL boxeddrv_SetCapture(HWND hwnd, UINT flags) {
    TRACE("hwnd=%p flags=0x%08x\n", hwnd, flags);
    CALL_NORETURN_2(BOXED_SET_CAPTURE, hwnd, flags);
}

#if BOXED_WINE_VERSION >= 8110
void WINE_CDECL boxeddrv_SetCursor(HWND hwnd, HCURSOR cursor) {
#else
void WINE_CDECL boxeddrv_SetCursor(HCURSOR cursor) {
#endif
    ICONINFOEXW info;
    ICONINFOEXW infoOriginal;    
    DWORD found = 0;

    TRACE("cursor=%p\n", cursor);

    if (cursor==NULL) {
        CALL_NORETURN_5(BOXED_SET_CURSOR, cursor, 0, 0, 0, &found);
        return;
    }
    info.cbSize = sizeof(info);
    if (!GetIconInfoExW(cursor, &info)) {
        WARN("GetIconInfoExW failed\n");
        return;
    }        
    infoOriginal = info;
    TRACE("info->szModName %s info->szResName %s info->wResID %hu\n", debugstr_w(info.szModName), debugstr_w(info.szResName), (int)info.wResID);
    CALL_NORETURN_5(BOXED_SET_CURSOR, cursor, info.szModName, info.szResName, (DWORD)info.wResID, &found);

    if (!found) {
        char buffer[FIELD_OFFSET(BITMAPINFO, bmiColors[256])];
        BITMAPINFO *bmInfo = (BITMAPINFO *)buffer;
        BITMAP bmMask;
        //BITMAP bmColor;    
        LPVOID bits;
        HDC hdc;
        HANDLE mono;
        //BITMAP bm;

        if (info.hbmColor) {
            GetObjectW(info.hbmMask, sizeof(bmMask), &bmMask);
            mono = CopyImage( cursor, IMAGE_CURSOR, bmMask.bmWidth, bmMask.bmHeight, LR_MONOCHROME | LR_COPYFROMRESOURCE );
            if (mono) {
                ICONINFOEXW info2;    
                info2.cbSize = sizeof(info2);
                if (GetIconInfoExW( mono, &info2)) {
                    info = info2;
                }
            }
        }

        GetObjectW(info.hbmMask, sizeof(bmMask), &bmMask);
        //GetObjectW(info.hbmColor, sizeof(bmColor), &bmColor);    
        TRACE("info.hbmColor=%d bmMask.bmWidth=%d bmMask.bmHeight=%d\n", (int)info.hbmColor, (int)bmMask.bmWidth, (int)bmMask.bmHeight);
        bmInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmInfo->bmiHeader.biWidth = bmMask.bmWidth;
        bmInfo->bmiHeader.biHeight = -bmMask.bmHeight;
        bmInfo->bmiHeader.biPlanes = 1;
        bmInfo->bmiHeader.biBitCount = 1;
        bmInfo->bmiHeader.biCompression = BI_RGB;
        bmInfo->bmiHeader.biSizeImage = (bmMask.bmWidth + 31) / 32 * 4 * bmMask.bmHeight;
        bmInfo->bmiHeader.biXPelsPerMeter = 0;
        bmInfo->bmiHeader.biYPelsPerMeter = 0;
        bmInfo->bmiHeader.biClrUsed = 0;
        bmInfo->bmiHeader.biClrImportant = 0;

        bits = HeapAlloc(GetProcessHeap(), 0, bmInfo->bmiHeader.biSizeImage*2);
        TRACE("bits=%p\n", bits);
        hdc = CreateCompatibleDC(0);

        if (!GetDIBits(hdc, info.hbmMask, 0, bmMask.bmHeight, bits, bmInfo, DIB_RGB_COLORS))
        {
            WARN("GetDIBits failed\n");
        } else {
            if (info.hbmColor) {
                GetDIBits(hdc, info.hbmColor, 0, bmMask.bmHeight, (LPVOID)((char*)bits+bmInfo->bmiHeader.biSizeImage), bmInfo, DIB_RGB_COLORS);                
                bmMask.bmHeight=-bmMask.bmHeight;
            }
            CALL_NORETURN_9(BOXED_SET_CURSOR_BITS, cursor, infoOriginal.szModName, infoOriginal.szResName, (DWORD)infoOriginal.wResID, bits, (DWORD)bmMask.bmWidth, (DWORD)bmMask.bmHeight, info.xHotspot, info.yHotspot);
        }
        HeapFree(GetProcessHeap(), 0, bits);
        DeleteDC(hdc);
    }
    if (info.hbmColor)
        DeleteObject(info.hbmColor);
    DeleteObject(info.hbmMask);
}

BOOL WINE_CDECL boxeddrv_SetCursorPos(INT x, INT y) {
    int result;
    CALL_2(BOXED_SET_CURSOR_POS, x, y);
    TRACE("x=%d y=%d result=%d\n", x, y, result);
    return (BOOL)result;
}

void WINE_CDECL boxeddrv_SetFocus(HWND hwnd) {
    LONG style = GetWindowLongW(hwnd, GWL_STYLE);    

    TRACE("hwnd=%p\n", hwnd);	
    if (!(style & WS_MINIMIZE)) {
        BOOL shouldActivate = FALSE;
        HWND parent = GetAncestor(hwnd, GA_ROOT);
        
        CALL_NORETURN_2(BOXED_SET_FOCUS, parent, &shouldActivate);

        TRACE("shouldActivate=%s", shouldActivate?"TRUE":"FALSE");
        if (shouldActivate && can_activate_window(parent))
        {
            // simulate a mouse click on the caption to find out whether the window wants to be activated 
            LRESULT ma = SendMessageW(hwnd, WM_MOUSEACTIVATE, (WPARAM)parent, MAKELONG(HTCAPTION,WM_LBUTTONDOWN));
            if (ma != MA_NOACTIVATEANDEAT && ma != MA_NOACTIVATE)
            {
                TRACE("setting foreground window to %p\n", parent);            
                SetForegroundWindow(parent);            
            }
        }
    }
}

void WINE_CDECL boxeddrv_SetLayeredWindowAttributes(HWND hwnd, COLORREF key, BYTE alpha, DWORD flags) {
    TRACE("hwnd=%p key=0x%08x alpha=0x%02x flags=0x%08x\n", hwnd, (int)key, alpha, (int)flags);
    CALL_NORETURN_4(BOXED_SET_LAYERED_WINDOW_ATTRIBUTES, hwnd, key, alpha, flags);
}

void WINE_CDECL boxeddrv_SetParent(HWND hwnd, HWND parent, HWND old_parent) {
    TRACE("hwnd=%p parent=%p old_parent=%p\n", hwnd, parent, old_parent);
    CALL_NORETURN_3(BOXED_SET_PARENT, hwnd, parent, old_parent);
}

#if BOXED_WINE_VERSION >= 1718
void WINE_CDECL boxeddrv_SetWindowRgn(HWND hwnd, HRGN hrgn, BOOL redraw) {
    int result;
    CALL_3(BOXED_SET_WINDOW_RGN, hwnd, hrgn, redraw);
    TRACE("hwnd=%p hrgn=%p redraw=%d\n", hwnd, hrgn, redraw);
}
#else
int WINE_CDECL boxeddrv_SetWindowRgn(HWND hwnd, HRGN hrgn, BOOL redraw) {
    int result;
    CALL_3(BOXED_SET_WINDOW_RGN, hwnd, hrgn, redraw);
    TRACE("hwnd=%p hrgn=%p redraw=%d result=%d\n", hwnd, hrgn, redraw, result);
    return result;
}
#endif
void WINE_CDECL boxeddrv_SetWindowStyle(HWND hwnd, INT offset, STYLESTRUCT *style) {
    HWND hwndFocus;

    TRACE("hwnd=%p offset=%d style=%p\n", hwnd, offset, style);
    CALL_NORETURN_3(BOXED_SET_WINDOW_STYLE, hwnd, offset, style);
    hwndFocus = GetFocus();
    if (hwndFocus && (hwnd == hwndFocus || IsChild(hwnd, hwndFocus)))
        boxeddrv_SetFocus(hwnd);
}

void WINE_CDECL boxeddrv_SetWindowText(HWND hwnd, LPCWSTR text) {
    TRACE("hwnd=%p text=%s\n", hwnd, debugstr_w(text));
    CALL_NORETURN_2(BOXED_SET_WINDOW_TEXT, hwnd, text);
}

UINT WINE_CDECL boxeddrv_ShowWindow(HWND hwnd, INT cmd, RECT *rect, UINT swp) {
    HWND hwndFocus;
    UINT result;

    TRACE("hwnd=%p cmd=%d rect=%s swp=0x%08x\n", hwnd, cmd, wine_dbgstr_rect(rect), swp);
    if (IsRectEmpty(rect)) return swp;
    CALL_NORETURN_5(BOXED_SHOW_WINDOW, hwnd, cmd, rect, swp, &result);    
    hwndFocus = GetFocus();
    if (hwndFocus && (hwnd == hwndFocus || IsChild(hwnd, hwndFocus)))
        boxeddrv_SetFocus(hwnd);
    return result;
}

LRESULT WINE_CDECL boxeddrv_SysCommand(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    int result;
    CALL_3(BOXED_SYS_COMMAND, hwnd, wparam, lparam);
    TRACE("hwnd=%p wparam=0x%08x lparam=0x%08x result=%d\n", hwnd, (int)wparam, (int)lparam, result);
    return (LRESULT)result;
}

BOOL WINE_CDECL boxeddrv_SystemParametersInfo(UINT action, UINT int_param, void *ptr_param, UINT flags) {
    int result;
    CALL_4(BOXED_SYSTEM_PARAMETERS_INFO, action, int_param, ptr_param, flags);
    TRACE("action=%d int_param=%d ptr_param=%p flags=0x%08x result=%d\n", action, int_param, ptr_param, flags, result);
    return (BOOL)result;
}

INT WINE_CDECL boxeddrv_ToUnicodeEx(UINT virtKey, UINT scanCode, const BYTE *lpKeyState, LPWSTR bufW, int bufW_size, UINT flags, HKL hkl) {
    INT result;
    CALL_7(BOXED_TO_UNICODE_EX, virtKey, scanCode, lpKeyState, bufW, bufW_size, flags, hkl);
    return result;
}

BOOL WINE_CDECL boxeddrv_UpdateLayeredWindow(HWND hwnd, const UPDATELAYEREDWINDOWINFO *info, const RECT *window_rect) {
    int result;
    CALL_3(BOXED_UPDATE_LAYERED_WINDOW, hwnd, info, window_rect);
    return (BOOL)result;
}

SHORT WINE_CDECL boxeddrv_VkKeyScanEx(WCHAR wChar, HKL hkl) {
    int result;
    CALL_2(BOXED_VK_KEY_SCAN_EX, wChar, hkl);
    return (SHORT)result;
}

LRESULT WINE_CDECL boxeddrv_WindowMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    int result;
    CALL_4(BOXED_WINDOW_MESSAGE, hwnd, msg, wp, lp);
    return (LRESULT)result;
}

UINT WINE_CDECL boxeddrv_GetKeyboardLayoutList(INT size, HKL *list) {
    UINT result;
    CALL_2(BOXED_GET_KEYBOARD_LAYOUT_LIST, size, list);
    return result;
}

BOOL WINE_CDECL boxeddrv_RegisterHotKey(HWND hwnd, UINT mod_flags, UINT vkey) {
    int result;
    CALL_3(BOXED_REGISTER_HOT_KEY, hwnd, mod_flags, vkey);
    return (BOOL)result;
}

void WINE_CDECL boxeddrv_UnregisterHotKey(HWND hwnd, UINT modifiers, UINT vkey) {
    CALL_NORETURN_3(BOXED_UNREGISTER_HOT_KEY, hwnd, modifiers, vkey);
}

void WINE_CDECL boxeddrv_ThreadDetach(void)
{
}

void WINE_CDECL boxeddrv_UpdateClipboard(void)
{
}

INT WINE_CDECL boxeddrv_GetDisplayDepth(LPCWSTR name, BOOL is_primary) {
    return internal_GetDeviceCaps(BITSPIXEL);
}

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)
#endif

static NTSTATUS boxedwine_init(void* arg)
{
    TRACE("starting");
    BOXEDDRV_ProcessAttach();
    BOXEDDRV_DisplayDevices_Init(FALSE);
    TRACE("done");
    return STATUS_SUCCESS;
}

#if BOXED_WINE_VERSION >= 7120 && defined(WINE_UNIX_LIB)
const unixlib_entry_t __wine_unix_call_funcs[] =
{
    boxedwine_init
};
#endif
