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
#pragma GCC diagnostic ignored "-Wreturn-type"
#include "config.h"

#include <stdarg.h>

#include "windef.h"
#include "winbase.h"
#include "wingdi.h"
#include "wine/debug.h"
#include "wine/gdi_driver.h"
#include "winreg.h"

#include "winuser.h"
#include "winternl.h"
#include "winnt.h"
#include "shellapi.h"
#include "imm.h"
#include "ddk/imm.h"
#ifndef BOXED_IGNORE_WINELIB
#include "wine/library.h"
#endif
#include "wine/wgl.h"
#include "wine/wgl_driver.h"
#ifdef BOXED_NEED_WGLEXT
#include "wine/wglext.h"
#endif
#include "wine/unicode.h"
#include "wine/server.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>

#if WINE_VULKAN_DRIVER_VERSION >= 7
#define VK_NO_PROTOTYPES
#define WINE_VK_HOST

#include "wine/vulkan.h"
#include "wine/vulkan_driver.h"
#endif

WINE_DEFAULT_DEBUG_CHANNEL(boxeddrv);

#if WINE_GDI_DRIVER_VERSION >= 50
#define WINE_CDECL CDECL
#else
#define WINE_CDECL
#endif

#if WINE_WGL_DRIVER_VERSION >= 21
#define BOXED_GLAPI WINAPI
#else
#define BOXED_GLAPI
#endif

#define USE_GL_FUNC(name) #name,
static const char *opengl_func_names[] = { ALL_WGL_FUNCS };
#undef USE_GL_FUNC

typedef struct
{
    struct gdi_physdev  dev;
} BOXEDDRV_PDEVICE;

#define BOXED_BASE 0

#define BOXED_ACQUIRE_CLIPBOARD						(BOXED_BASE)
#define BOXED_ACTIVATE_KEYBOARD_LAYOUT				(BOXED_BASE+1)
#define BOXED_BEEP									(BOXED_BASE+2)
#define BOXED_CHANGE_DISPLAY_SETTINGS_EX			(BOXED_BASE+3)
#define BOXED_CLIP_CURSOR							(BOXED_BASE+4)
#define BOXED_COUNT_CLIPBOARD_FORMATS				(BOXED_BASE+5)
#define BOXED_CREATE_DESKTOP_WINDOW					(BOXED_BASE+6)
#define BOXED_CREATE_WINDOW							(BOXED_BASE+7)
#define BOXED_DESTROY_CURSOR_ICON					(BOXED_BASE+8)
#define BOXED_DESTROY_WINDOW						(BOXED_BASE+9)
#define BOXED_EMPTY_CLIPBOARD						(BOXED_BASE+10)
#define BOXED_END_CLIPBOARD_UPDATE					(BOXED_BASE+11)
#define BOXED_ENUM_CLIPBOARD_FORMATS				(BOXED_BASE+12)
#define BOXED_ENUM_DISPLAY_MONITORS					(BOXED_BASE+13)
#define BOXED_ENUM_DISPLAY_SETTINGS_EX				(BOXED_BASE+14)
#define BOXED_GET_CLIPBOARD_DATA					(BOXED_BASE+15)
#define BOXED_GET_CURSOR_POS						(BOXED_BASE+16)
#define BOXED_GET_KEYBOARD_LAYOUT					(BOXED_BASE+17)
#define BOXED_GET_KEYBOARD_LAYOUT_NAME				(BOXED_BASE+18)
#define BOXED_GET_KEY_NAME							(BOXED_BASE+19)
#define BOXED_GET_MONITOR_INFO						(BOXED_BASE+20)
#define BOXED_IS_CLIPBOARD_FORMAT_AVAILABLE			(BOXED_BASE+21)
#define BOXED_MAP_VIRTUAL_KEY_EX					(BOXED_BASE+22)
#define BOXED_MSG_WAIT_FOR_MULTIPLE_OBJECTS_EX		(BOXED_BASE+23)
#define BOXED_SET_CAPTURE							(BOXED_BASE+24)
#define BOXED_SET_CLIPBOARD_DATA					(BOXED_BASE+25)
#define BOXED_SET_CURSOR							(BOXED_BASE+26)
#define BOXED_SET_CURSOR_POS						(BOXED_BASE+27)
#define BOXED_SET_FOCUS								(BOXED_BASE+28)
#define BOXED_SET_LAYERED_WINDOW_ATTRIBUTES			(BOXED_BASE+29)
#define BOXED_SET_PARENT							(BOXED_BASE+30)
#define BOXED_SET_WINDOW_RGN						(BOXED_BASE+31)
#define BOXED_SET_WINDOW_STYLE						(BOXED_BASE+32)
#define BOXED_SET_WINDOW_TEXT						(BOXED_BASE+33)
#define BOXED_SHOW_WINDOW							(BOXED_BASE+34)
#define BOXED_SYS_COMMAND							(BOXED_BASE+35)
#define BOXED_SYSTEM_PARAMETERS_INFO				(BOXED_BASE+36)
#define BOXED_TO_UNICODE_EX							(BOXED_BASE+37)
#define BOXED_UPDATE_LAYERED_WINDOW					(BOXED_BASE+38)
#define BOXED_VK_KEY_SCAN_EX						(BOXED_BASE+39)
#define BOXED_WINDOW_MESSAGE						(BOXED_BASE+40)
#define BOXED_WINDOW_POS_CHANGED					(BOXED_BASE+41)
#define BOXED_WINDOW_POS_CHANGING					(BOXED_BASE+42)

#define BOXED_GET_DEVICE_GAMMA_RAMP					(BOXED_BASE+43)
#define BOXED_SET_DEVICE_GAMMA_RAMP					(BOXED_BASE+44)
#define BOXED_GET_DEVICE_CAPS						(BOXED_BASE+45)

#define BOXED_WINE_NOTIFY_ICON						(BOXED_BASE+46)

#define BOXED_IME_CONFIGURE							(BOXED_BASE+47)
#define BOXED_IME_CONVERSION_LIST					(BOXED_BASE+48)
#define BOXED_IME_DESTROY							(BOXED_BASE+49)
#define BOXED_IME_ENUM_REGISTER_WORD				(BOXED_BASE+50)
#define BOXED_IME_ESCAPE							(BOXED_BASE+51)
#define BOXED_IME_GET_IME_MENU_ITEMS				(BOXED_BASE+52)
#define BOXED_IME_GET_REGISTER_WORD_STYLE			(BOXED_BASE+53)
#define BOXED_IME_INQUIRE							(BOXED_BASE+54)
#define BOXED_IME_PROCESS_KEY						(BOXED_BASE+55)
#define BOXED_IME_REGISTER_WORD						(BOXED_BASE+56)
#define BOXED_IME_SELECT							(BOXED_BASE+57)
#define BOXED_IME_SET_ACTIVE_CONTEXT				(BOXED_BASE+58)
#define BOXED_IME_SET_COMPOSITION_STRING			(BOXED_BASE+59)
#define BOXED_IME_TO_ASCII_EX						(BOXED_BASE+60)
#define BOXED_IME_UNREGISTER_WORD					(BOXED_BASE+61)
#define BOXED_NOTIFY_IME							(BOXED_BASE+62)

#define BOXED_GL_COPY_CONTEXT						(BOXED_BASE+63)
#define BOXED_GL_CREATE_CONTEXT						(BOXED_BASE+64)
#define BOXED_GL_DELETE_CONTEXT						(BOXED_BASE+65)
#define BOXED_GL_DESCRIBE_PIXEL_FORMAT				(BOXED_BASE+66)
#define BOXED_GL_GET_PIXEL_FORMAT					(BOXED_BASE+67)
#define BOXED_GL_GET_PROC_ADDRESS					(BOXED_BASE+68)
#define BOXED_GL_MAKE_CURRENT						(BOXED_BASE+69)
#define BOXED_GL_SET_PIXEL_FORMAT					(BOXED_BASE+70)
#define BOXED_GL_SHARE_LISTS						(BOXED_BASE+71)
#define BOXED_GL_SWAP_BUFFERS						(BOXED_BASE+72)

#define BOXED_GET_KEYBOARD_LAYOUT_LIST				(BOXED_BASE+73)
#define BOXED_REGISTER_HOT_KEY						(BOXED_BASE+74)
#define BOXED_UNREGISTER_HOT_KEY					(BOXED_BASE+75)
#define BOXED_SET_SURFACE							(BOXED_BASE+76)
#define BOXED_GET_SURFACE							(BOXED_BASE+77)
#define BOXED_FLUSH_SURFACE							(BOXED_BASE+78)

#define BOXED_CREATE_DC                             (BOXED_BASE+79)
#define BOXED_GET_SYSTEM_PALETTE                    (BOXED_BASE+80)
#define BOXED_GET_NEAREST_COLOR                     (BOXED_BASE+81)
#define BOXED_REALIZE_PALETTE                       (BOXED_BASE+82)
#define BOXED_REALIZE_DEFAULT_PALETTE               (BOXED_BASE+83)
#define BOXED_SET_EVENT_FD                          (BOXED_BASE+84)
#define BOXED_SET_CURSOR_BITS                       (BOXED_BASE+85)
#define BOXED_CREATE_DESKTOP                        (BOXED_BASE+86)
#define BOXED_HAS_WND                               (BOXED_BASE+87)
#define BOXED_GET_VERSION                           (BOXED_BASE+88)

#define BOXED_VK_CREATE_INSTANCE                    (BOXED_BASE+89)
#define BOXED_VK_CREATE_SWAPCHAIN                   (BOXED_BASE+90)
#define BOXED_VK_CREATE_SURFACE                     (BOXED_BASE+91)
#define BOXED_VK_DESTROY_INSTANCE                   (BOXED_BASE+92)
#define BOXED_VK_DESTROY_SURFACE                    (BOXED_BASE+93)
#define BOXED_VK_DESTROY_SWAPCHAIN                  (BOXED_BASE+94)
#define BOXED_VK_ENUMERATE_INSTANCE_EXTENSION_PROPERTIES (BOXED_BASE+95)
#define BOXED_VK_GET_DEVICE_GROUP_SURFACE_PRESENT_MODES  (BOXED_BASE+96)
#define BOXED_VK_GET_PHYSICAL_DEVICE_PRESENT_RECTANGLES  (BOXED_BASE+97)
#define BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_CAPABILITIES   (BOXED_BASE+98)
#define BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_FORMATS (BOXED_BASE+99)
#define BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_PRESENT_MODES (BOXED_BASE+100)
#define BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_SUPPORT (BOXED_BASE+101)
#define BOXED_VK_GET_PHYSICAL_DEVICE_WIN32_PRESENTATION_SUPPORT (BOXED_BASE+102)
#define BOXED_VK_GET_SWAPCHAIN_IMAGES                (BOXED_BASE+103)
#define BOXED_VK_QUEUE_PRESENT                       (BOXED_BASE+104)
#define BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_CAPABILITIES2   (BOXED_BASE+105)
#define BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_FORMATS2 (BOXED_BASE+106)
#define BOXED_VK_GET_NATIVE_SURFACE                  (BOXED_BASE+107)

#define CALL_0(index) __asm__("push %1\n\tint $0x98\n\taddl $4, %%esp": "=a" (result):"i"(index):); 
#define CALL_1(index, arg1) __asm__("push %2\n\tpush %1\n\tint $0x98\n\taddl $8, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1):); 
#define CALL_2(index, arg1,arg2) __asm__("push %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $12, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2):);
#define CALL_3(index, arg1,arg2,arg3) __asm__("push %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $16, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3):);
#define CALL_4(index, arg1,arg2,arg3,arg4) __asm__("push %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $20, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4):);
#define CALL_5(index, arg1,arg2,arg3,arg4,arg5) __asm__("push %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $24, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5):);
#define CALL_6(index, arg1,arg2,arg3,arg4,arg5,arg6) __asm__("push %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $28, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5), "g"((DWORD)arg6):);
#define CALL_7(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7) __asm__("push %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $32, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5), "g"((DWORD)arg6), "g"((DWORD)arg7):);

#define CALL_NORETURN_0(index) __asm__("push %0\n\tint $0x98\n\taddl $4, %%esp"::"i"(index)); 
#define CALL_NORETURN_1(index, arg1) __asm__("push %1\n\tpush %0\n\tint $0x98\n\taddl $8, %%esp"::"i"(index), "g"((DWORD)arg1)); 
#define CALL_NORETURN_2(index, arg1, arg2) __asm__("push %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $12, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2)); 
#define CALL_NORETURN_3(index, arg1, arg2, arg3) __asm__("push %3\n\rpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $16, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3)); 
#define CALL_NORETURN_4(index, arg1, arg2, arg3, arg4) __asm__("push %4\n\tpush %3\n\rpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $20, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4)); 
#define CALL_NORETURN_5(index, arg1, arg2, arg3, arg4, arg5) __asm__("push %5\n\tpush %4\n\tpush %3\n\rpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $24, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5)); 
#define CALL_NORETURN_7(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7) __asm__("push %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $32, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5), "g"((DWORD)arg6), "g"((DWORD)arg7));
#define CALL_NORETURN_8(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) __asm__("push %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $36, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5), "g"((DWORD)arg6), "g"((DWORD)arg7), "g"((DWORD)arg8));
#define CALL_NORETURN_9(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) __asm__("push %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $40, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5), "g"((DWORD)arg6), "g"((DWORD)arg7), "g"((DWORD)arg8), "g"((DWORD)arg9));

INT WINE_CDECL boxeddrv_GetDeviceCaps(PHYSDEV dev, INT cap);

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


int CDECL boxeddrv_AcquireClipboard(HWND hwnd) {
    int result;
    CALL_1(BOXED_ACQUIRE_CLIPBOARD, hwnd);
    TRACE("hwnd=%p result=%d\n", hwnd, result);
    return result;
}

HKL CDECL boxeddrv_ActivateKeyboardLayout(HKL hkl, UINT flags) {
    int result;
    CALL_2(BOXED_ACTIVATE_KEYBOARD_LAYOUT, hkl, flags);
    TRACE("hkl=%p flags=0x%08x result=%d\n", hkl, flags, result);
    return (HKL)result;
}

void CDECL boxeddrv_Beep(void) {
    TRACE("\n");
    CALL_NORETURN_0(BOXED_BEEP);
}

LONG CDECL boxeddrv_ChangeDisplaySettingsEx(LPCWSTR devname, LPDEVMODEW devmode, HWND hwnd, DWORD flags, LPVOID lpvoid) {
    LONG result;
    LONG cx, cy, bpp;

    TRACE("devname=%s devmode=%p hwnd=%p flags=0x%08x %p &result=%p &cx=%p &cy=%p\n", debugstr_w(devname), devmode, hwnd, flags, lpvoid, &result, &cx, &cy);
    CALL_NORETURN_9(BOXED_CHANGE_DISPLAY_SETTINGS_EX, devname, devmode, hwnd, flags, lpvoid, &result, &cx, &cy, &bpp);
    TRACE("result=%d width=%d height=%d bpp=%d\n", result, cx, cy, bpp);
    if (result==DISP_CHANGE_SUCCESSFUL) {
        TRACE("SetWindowPos\n");
        SetWindowPos(GetDesktopWindow(), 0, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE | SWP_DEFERERASE );
        TRACE("SendMessageTimeoutW\n");
        SendMessageTimeoutW( HWND_BROADCAST, WM_DISPLAYCHANGE, bpp, MAKELPARAM( cx, cy ), SMTO_ABORTIFHUNG, 2000, NULL );       
        TRACE("SendMessageTimeoutW returned\n");
    }    
    return result;
}

BOOL CDECL boxeddrv_ClipCursor(LPCRECT clip) {
    int result;
    CALL_1(BOXED_CLIP_CURSOR, clip);
    TRACE("clip=%s result=%d\n", wine_dbgstr_rect(clip), result);
    return (BOOL)result;
}

INT CDECL boxeddrv_CountClipboardFormats(void) {
    INT result;
    CALL_0(BOXED_COUNT_CLIPBOARD_FORMATS);
    TRACE("result=%d\n", result);
    return result;
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
        width = boxeddrv_GetDeviceCaps(NULL, DESKTOPHORZRES); 
        height = boxeddrv_GetDeviceCaps(NULL, DESKTOPVERTRES);

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
BOOL CDECL boxeddrv_CreateDesktopWindow(HWND hwnd) {
    int result;
    CALL_1(BOXED_CREATE_DESKTOP_WINDOW, hwnd);
    initDesktop(hwnd);
    return (BOOL)result;
}

BOOL CDECL boxeddrv_CreateWindow(HWND hwnd) {
    int result;
    CALL_1(BOXED_CREATE_WINDOW, hwnd);
    TRACE("hwnd=%p result=%d\n", hwnd, result);
    return (BOOL)result;
}

void CDECL boxeddrv_DestroyCursorIcon(HCURSOR cursor) {
    TRACE("cursor=%p\n", cursor);
    CALL_NORETURN_1(BOXED_DESTROY_CURSOR_ICON, cursor);
}

void CDECL boxeddrv_DestroyWindow(HWND hwnd) {
    TRACE("hwnd=%p\n", hwnd);
    CALL_NORETURN_1(BOXED_DESTROY_WINDOW, hwnd);

    if (hwnd == GetForegroundWindow())
    {
        SendMessageW(hwnd, WM_CANCELMODE, 0, 0);
        if (hwnd == GetForegroundWindow())
            SetForegroundWindow(GetDesktopWindow());
    }
}

void CDECL boxeddrv_EmptyClipboard(BOOL keepunowned) {
    TRACE("keepunowned=%d\n", keepunowned);
    CALL_NORETURN_1(BOXED_EMPTY_CLIPBOARD, keepunowned);
}

void CDECL boxeddrv_EndClipboardUpdate(void) {
    TRACE("\n");
    CALL_NORETURN_0(BOXED_END_CLIPBOARD_UPDATE);
}

UINT CDECL boxeddrv_EnumClipboardFormats(UINT prev_format) {
    UINT result;
    CALL_1(BOXED_ENUM_CLIPBOARD_FORMATS, prev_format);
    TRACE("prev_format=%d result=%d\n", prev_format, result);
    return result;
}

INT WINE_CDECL boxeddrv_GetDeviceCaps(PHYSDEV dev, INT cap);
BOOL CDECL boxeddrv_EnumDisplayMonitors(HDC hdc, LPRECT rect, MONITORENUMPROC proc, LPARAM lparam) {
    RECT r;
    r.left = 0;
    r.right = boxeddrv_GetDeviceCaps(0, HORZRES);
    r.top = 0;
    r.bottom = boxeddrv_GetDeviceCaps(0, VERTRES);

    TRACE("hdc=%p rect=%s proc=%p lparam=0x%08x\n", hdc, wine_dbgstr_rect(rect), proc, (int)lparam);
    if (hdc) {
        POINT origin;
        RECT limit;
        RECT monrect = r;

        if (!GetDCOrgEx(hdc, &origin)) return FALSE;
        if (GetClipBox(hdc, &limit) == ERROR) return FALSE;

        if (rect && !IntersectRect(&limit, &limit, rect)) return TRUE;

        if (IntersectRect(&monrect, &monrect, &limit)) {
            if (!proc((HMONITOR)1, hdc, &monrect, lparam))
                return FALSE;
        }
    }
    else {
        RECT monrect = r;
        RECT unused;

        if (!rect || IntersectRect(&unused, &monrect, rect)) {
            TRACE("calling proc hdc=%p monrect=%s proc=%p lparam=0x%08x\n", hdc, wine_dbgstr_rect(&monrect), proc, (int)lparam);
            if (!proc((HMONITOR)1, hdc, &monrect, lparam))
                return FALSE;
        }
    }
    
    return TRUE;
}

BOOL CDECL boxeddrv_EnumDisplaySettingsEx(LPCWSTR devname, DWORD mode, LPDEVMODEW devmode, DWORD flags) {
    int result;
    CALL_4(BOXED_ENUM_DISPLAY_SETTINGS_EX, devname, mode, devmode, flags);
    TRACE("devname=%s mode=%d devmode=%p flags=0x%08x result=%d\n", debugstr_w(devname), mode, devmode, flags, result);
    return (BOOL)result;
}

char tmp64k[64*1024];

HANDLE CDECL boxeddrv_GetClipboardData(UINT desired_format) {
    int result;
    HANDLE h = 0;

    CALL_3(BOXED_GET_CLIPBOARD_DATA, desired_format, tmp64k, sizeof(tmp64k));
    TRACE("desired_format=%d result=%d %s\n", desired_format, result, result>0?tmp64k:"");
    if (result) {
        LPVOID p;

        h = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, result);
        if (!h)
            return NULL;
        p = GlobalLock(h);
        if (!p) {
            GlobalFree(h);
            return NULL;
        }
        memcpy(p, tmp64k, result);
        GlobalUnlock(h);
    }
    return h;
}

BOOL CDECL boxeddrv_GetCursorPos(LPPOINT pos) {
    int result;
    CALL_1(BOXED_GET_CURSOR_POS, pos);
    TRACE("pos=%p(%d,%d) result=%d\n", pos, pos->x, pos->y, result);
    return result;
}

static HKL get_locale_kbd_layout(void)
{
    ULONG_PTR layout;
    LANGID langid;

    /* FIXME:
     *
     * layout = main_key_tab[kbd_layout].lcid;
     *
     * Winword uses return value of GetKeyboardLayout as a codepage
     * to translate ANSI keyboard messages to unicode. But we have
     * a problem with it: for instance Polish keyboard layout is
     * identical to the US one, and therefore instead of the Polish
     * locale id we return the US one.
     */

    layout = GetUserDefaultLCID();

    /*
     * Microsoft Office expects this value to be something specific
     * for Japanese and Korean Windows with an IME the value is 0xe001
     * We should probably check to see if an IME exists and if so then
     * set this word properly.
     */
    langid = PRIMARYLANGID(LANGIDFROMLCID(layout));
    if (langid == LANG_CHINESE || langid == LANG_JAPANESE || langid == LANG_KOREAN)
        layout |= (ULONG_PTR)0xe001 << 16; /* IME */
    else
        layout |= layout << 16;

    return (HKL)layout;
}

HKL CDECL boxeddrv_GetKeyboardLayout(DWORD thread_id) {
     return get_locale_kbd_layout();
}

BOOL CDECL boxeddrv_GetKeyboardLayoutName(LPWSTR name) {
    static const WCHAR formatW[] = {'%','0','8','x',0};
    DWORD layout;

    layout = HandleToUlong( get_locale_kbd_layout() );
    if (HIWORD(layout) == LOWORD(layout)) layout = LOWORD(layout);
    sprintfW(name, formatW, layout);
    TRACE("returning %s\n", debugstr_w(name));
    return TRUE;
}

INT CDECL boxeddrv_GetKeyNameText(LONG lparam, LPWSTR buffer, INT size) {
    INT result;
    CALL_3(BOXED_GET_KEY_NAME, lparam, buffer, size);
    TRACE("lparam=0x%08x buffer=%p size=%d result=%d\n", lparam, buffer, size, result);
    return result;
}

BOOL CDECL boxeddrv_GetMonitorInfo(HMONITOR monitor, LPMONITORINFO info) {
    static const WCHAR adapter_name[] = { '\\','\\','.','\\','D','I','S','P','L','A','Y','1',0 };

    TRACE("monitor=%p info=%p\n", monitor, info);
    SetRect(&info->rcMonitor, 0, 0, boxeddrv_GetDeviceCaps(NULL, DESKTOPHORZRES), boxeddrv_GetDeviceCaps(NULL, DESKTOPVERTRES));
    SetRect(&info->rcWork, 0, 0, boxeddrv_GetDeviceCaps(NULL, DESKTOPHORZRES), boxeddrv_GetDeviceCaps(NULL, DESKTOPVERTRES));
    info->dwFlags = MONITORINFOF_PRIMARY;

    if (info->cbSize >= sizeof(MONITORINFOEXW))
        lstrcpyW(((MONITORINFOEXW*)info)->szDevice, adapter_name);
    return TRUE;
    //CALL_2(BOXED_GET_MONITOR_INFO, monitor, info);
}

BOOL CDECL boxeddrv_IsClipboardFormatAvailable(UINT desired_format) {
    int result;
    CALL_1(BOXED_IS_CLIPBOARD_FORMAT_AVAILABLE, desired_format);
    TRACE("desired_format=%d result=%d\n", desired_format, result);
    return (BOOL)result;
}

UINT CDECL boxeddrv_MapVirtualKeyEx(UINT wCode, UINT wMapType, HKL hkl) {
    UINT result;
    CALL_3(BOXED_MAP_VIRTUAL_KEY_EX, wCode, wMapType, hkl);
    TRACE("wCode=%d wMapType=%d hkl=%p result=%d\n", wCode, wMapType, hkl, result);
    return result;
}

int eventsInitialized = 0;
int eventQueueFD;

void initEvents(void) 
{
    HANDLE handle;
    int ret;
    int fds[2];

    if (eventsInitialized)
        return;
    eventsInitialized = 1;
    pipe(fds);
    fcntl(fds[0], F_SETFD, 1);
    fcntl(fds[0], F_SETFL, O_NONBLOCK);
    fcntl(fds[1], F_SETFD, 1);
    fcntl(fds[1], F_SETFL, O_NONBLOCK);

    eventQueueFD = fds[0];
    CALL_NORETURN_1(BOXED_SET_EVENT_FD, fds[1]);

    if (wine_server_fd_to_handle(fds[0], GENERIC_READ | SYNCHRONIZE, 0, &handle))
    {
        MESSAGE("wineboxed.drv: Can't allocate handle for event queue fd\n");
        ExitProcess(1);
    }
    SERVER_START_REQ(set_queue_fd)
    {
        req->handle = wine_server_obj_handle(handle);
        ret = wine_server_call(req);
    }
    SERVER_END_REQ;
    if (ret)
    {
        MESSAGE("wineboxed.drv: Can't store handle for event queue fd\n");
        ExitProcess(1);
    } else {
        TRACE("event queue read fd=%d\n", eventQueueFD);
    }
    CloseHandle(handle);
}

BOOL processEvents(void) {
    HWND hwnd;
    INPUT input;
    int r;
	BOOL result = FALSE;
    while (1) {
        if ((r=read(eventQueueFD, &input, sizeof(INPUT)))==-1) {
            return result;
        }
        TRACE("read event: type=");
        if (input.type == 0) {
            const char* type = "rel";
            if (input.mi.dwFlags & MOUSEEVENTF_WHEEL) {
                hwnd = GetForegroundWindow();
            } else if (input.mi.dwFlags & MOUSEEVENTF_ABSOLUTE) {
                POINT p;
                p.x = input.mi.dx;
                p.y = input.mi.dy;
                hwnd = WindowFromPoint(p);
                if (!hwnd) {
                    continue;
                }
                hwnd = GetAncestor(hwnd, GA_ROOT);                
            } else {
                hwnd = GetForegroundWindow();
            }
            if (input.mi.dwFlags & MOUSEEVENTF_ABSOLUTE) {
                type = "abs";
            }
            TRACE("mouse %s %s hwnd=%p dx=%d dy=%d dwFlags=%X time=%X\n", ((input.mi.dwFlags & MOUSEEVENTF_WHEEL)?"wheel":""), type, hwnd, input.mi.dx, input.mi.dy, input.mi.dwFlags, input.mi.time);
        } else {
            hwnd = GetForegroundWindow();
        }
    
        TRACE("hwnd=%p GetFocus()=%p GetForegroundWindow()=%p\n", hwnd, GetFocus(), GetForegroundWindow());
        __wine_send_input(hwnd, &input);
		result = TRUE;
    }
}

DWORD CDECL boxeddrv_MsgWaitForMultipleObjectsEx(DWORD count, const HANDLE *handles, DWORD timeout, DWORD mask, DWORD flags) {
    DWORD result;

    TRACE("count=%d handles=%p timeout=0x%08x mask=0x%08x flags=0x%08x\n", count, handles, timeout, mask, flags);
    initEvents();
    CALL_5(BOXED_MSG_WAIT_FOR_MULTIPLE_OBJECTS_EX, count, handles, timeout, mask, flags);
	if (processEvents()) {
		return count - 1;
	}
    if (!count && !timeout) 
        return WAIT_TIMEOUT;    
    result = WaitForMultipleObjectsEx(count, handles, flags & MWMO_WAITALL, timeout, flags & MWMO_ALERTABLE);	
    return result;
}

void CDECL boxeddrv_SetCapture(HWND hwnd, UINT flags) {
    TRACE("hwnd=%p flags=0x%08x\n", hwnd, flags);
    CALL_NORETURN_2(BOXED_SET_CAPTURE, hwnd, flags);
}

BOOL CDECL boxeddrv_SetClipboardData(UINT format_id, HANDLE data, BOOL owner) {
    int result;
    int len = GlobalSize(data);
    LPVOID src = GlobalLock(data);
    WCHAR buffer[256];

    buffer[0]=0;
    GetClipboardFormatNameW(format_id, buffer, sizeof(buffer) / sizeof(buffer[0]));
    CALL_4(BOXED_SET_CLIPBOARD_DATA, format_id, src, len, owner);
    GlobalUnlock(data);
    TRACE("format_id=%d (%s) data=%p dataLen=%d owner=%d\n", format_id, debugstr_w(buffer), data, len, owner);
    if (len && format_id != CF_TEXT && format_id != CF_UNICODETEXT) {
        HWND clipboardOwner = GetClipboardOwner();
        SendMessageW(clipboardOwner, WM_RENDERFORMAT, CF_UNICODETEXT, 0);
        TRACE("Requesting HWND=%x to convert %s to CF_UNICODETEXT\n", (int)clipboardOwner, debugstr_w(buffer));
    }
    return (BOOL)result;
}

void CDECL boxeddrv_SetCursor(HCURSOR cursor) {
    ICONINFOEXW info;    
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

    TRACE("info->szModName %s info->szResName %s info->wResID %hu\n", debugstr_w(info.szModName), debugstr_w(info.szResName), (DWORD)info.wResID);
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
        TRACE("info.hbmColor=%d bmMask.bmWidth=%d bmMask.bmHeight=%d\n", (DWORD)info.hbmColor, (DWORD)bmMask.bmWidth, (DWORD)bmMask.bmHeight);
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
            CALL_NORETURN_9(BOXED_SET_CURSOR_BITS, cursor, info.szModName, info.szResName, (DWORD)info.wResID, bits, (DWORD)bmMask.bmWidth, (DWORD)bmMask.bmHeight, info.xHotspot, info.yHotspot);
        }
        HeapFree(GetProcessHeap(), 0, bits);
        DeleteDC(hdc);
    }
    if (info.hbmColor)
        DeleteObject(info.hbmColor);
    DeleteObject(info.hbmMask);
}

BOOL CDECL boxeddrv_SetCursorPos(INT x, INT y) {
    int result;
    CALL_2(BOXED_SET_CURSOR_POS, x, y);
    TRACE("x=%d y=%d result=%d\n", x, y, result);
    return (BOOL)result;
}

void CDECL boxeddrv_SetFocus(HWND hwnd) {
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

void CDECL boxeddrv_SetLayeredWindowAttributes(HWND hwnd, COLORREF key, BYTE alpha, DWORD flags) {
    TRACE("hwnd=%p key=0x%08x alpha=0x%02x flags=0x%08x\n", hwnd, key, alpha, flags);
    CALL_NORETURN_4(BOXED_SET_LAYERED_WINDOW_ATTRIBUTES, hwnd, key, alpha, flags);
}

void CDECL boxeddrv_SetParent(HWND hwnd, HWND parent, HWND old_parent) {
    TRACE("hwnd=%p parent=%p old_parent=%p\n", hwnd, parent, old_parent);
    CALL_NORETURN_3(BOXED_SET_PARENT, hwnd, parent, old_parent);
}

int CDECL boxeddrv_SetWindowRgn(HWND hwnd, HRGN hrgn, BOOL redraw) {
    int result;
    CALL_3(BOXED_SET_WINDOW_RGN, hwnd, hrgn, redraw);
    TRACE("hwnd=%p hrgn=%p redraw=%d result=%d\n", hwnd, hrgn, redraw, result);
    return result;
}

void CDECL boxeddrv_SetWindowStyle(HWND hwnd, INT offset, STYLESTRUCT *style) {
    HWND hwndFocus;

    TRACE("hwnd=%p offset=%d style=%p\n", hwnd, offset, style);
    CALL_NORETURN_3(BOXED_SET_WINDOW_STYLE, hwnd, offset, style);
    hwndFocus = GetFocus();
    if (hwndFocus && (hwnd == hwndFocus || IsChild(hwnd, hwndFocus)))
        boxeddrv_SetFocus(hwnd);
}

void CDECL boxeddrv_SetWindowText(HWND hwnd, LPCWSTR text) {
    TRACE("hwnd=%p text=%s\n", hwnd, debugstr_w(text));
    CALL_NORETURN_2(BOXED_SET_WINDOW_TEXT, hwnd, text);
}

UINT CDECL boxeddrv_ShowWindow(HWND hwnd, INT cmd, RECT *rect, UINT swp) {
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

LRESULT CDECL boxeddrv_SysCommand(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    int result;
    CALL_3(BOXED_SYS_COMMAND, hwnd, wparam, lparam);
    TRACE("hwnd=%p wparam=0x%08x lparam=0x%08x result=%d\n", hwnd, (int)wparam, (int)lparam, result);
    return (LRESULT)result;
}

BOOL CDECL boxeddrv_SystemParametersInfo(UINT action, UINT int_param, void *ptr_param, UINT flags) {
    int result;
    CALL_4(BOXED_SYSTEM_PARAMETERS_INFO, action, int_param, ptr_param, flags);
    TRACE("action=%d int_param=%d ptr_param=%p flags=0x%08x result=%d\n", action, int_param, ptr_param, flags, result);
    return (BOOL)result;
}

INT CDECL boxeddrv_ToUnicodeEx(UINT virtKey, UINT scanCode, const BYTE *lpKeyState, LPWSTR bufW, int bufW_size, UINT flags, HKL hkl) {
    INT result;
    CALL_7(BOXED_TO_UNICODE_EX, virtKey, scanCode, lpKeyState, bufW, bufW_size, flags, hkl);
    return result;
}

BOOL CDECL boxeddrv_UpdateLayeredWindow(HWND hwnd, const UPDATELAYEREDWINDOWINFO *info, const RECT *window_rect) {
    int result;
    CALL_3(BOXED_UPDATE_LAYERED_WINDOW, hwnd, info, window_rect);
    return (BOOL)result;
}

SHORT CDECL boxeddrv_VkKeyScanEx(WCHAR wChar, HKL hkl) {
    int result;
    CALL_2(BOXED_VK_KEY_SCAN_EX, wChar, hkl);
    return (SHORT)result;
}

LRESULT CDECL boxeddrv_WindowMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    int result;
    CALL_4(BOXED_WINDOW_MESSAGE, hwnd, msg, wp, lp);
    return (LRESULT)result;
}

void boxeddrv_SetSurface(HWND hwnd, struct window_surface *surface) {
    CALL_NORETURN_2(BOXED_SET_SURFACE, hwnd, surface);
}

struct window_surface* boxeddrv_GetSurface(HWND hwnd) {
    struct window_surface* result;
    CALL_1(BOXED_GET_SURFACE, hwnd);
    return result;
}

BOOL boxeddrv_HasWnd(HWND hwnd) {
    int result;
    CALL_1(BOXED_HAS_WND, hwnd);
    return (BOOL)result;
}

void CDECL boxeddrv_WindowPosChanged(HWND hwnd, HWND insert_after, UINT swp_flags, const RECT *window_rect, const RECT *client_rect, const RECT *visible_rect, const RECT *valid_rects, struct window_surface *surface) {
    DWORD new_style = GetWindowLongW(hwnd, GWL_STYLE);
    struct window_surface* oldSurface = boxeddrv_GetSurface(hwnd);
    RECT r;

    if (!boxeddrv_HasWnd(hwnd))
        return;

    GetWindowRect(hwnd, &r);
    TRACE("hwnd=%p insert_after=%p swp_flags=0x%08x window_rect=%s client_rect=%s visible_rect=%s valid_rects=%s surface=%p style=0x%08x GetWindowRect()=%s\n", hwnd, insert_after, swp_flags, wine_dbgstr_rect(window_rect), wine_dbgstr_rect(client_rect), wine_dbgstr_rect(visible_rect), wine_dbgstr_rect(valid_rects), surface, new_style, wine_dbgstr_rect(&r));
    if (surface) {
        TRACE("     using new surface %p (ref=%d)\n", surface, surface->ref);
        window_surface_add_ref(surface);
    }
    if (oldSurface) {
        TRACE("     releasing old surface %p (ref=%d)\n", oldSurface, oldSurface->ref);
        window_surface_release(oldSurface);
    }
    boxeddrv_SetSurface(hwnd, surface);	
    CALL_NORETURN_8(BOXED_WINDOW_POS_CHANGED, hwnd, insert_after, swp_flags, window_rect, client_rect, visible_rect, valid_rects, new_style);
}

void surface_clip_to_visible_rect(struct window_surface *window_surface, const RECT *visible_rect);
struct window_surface *create_surface(HWND window, const RECT *rect, struct window_surface *old_surface, BOOL use_alpha);
void CDECL boxeddrv_WindowPosChanging(HWND hwnd, HWND insert_after, UINT swp_flags, const RECT *window_rect, const RECT *client_rect, RECT *visible_rect, struct window_surface **surface) {
    DWORD style = GetWindowLongW(hwnd, GWL_STYLE); 
    struct window_surface *oldSurface = NULL;
    HWND parent = GetAncestor(hwnd, GA_PARENT);

    initEvents();
    TRACE("hwnd=%p (parent=%p) insert_after=%p swp_flags=0x%08x window_rect=%s client_rect=%s visible_rect=%s surface=%p\n", hwnd, parent, insert_after, swp_flags, wine_dbgstr_rect(window_rect), wine_dbgstr_rect(client_rect), wine_dbgstr_rect(visible_rect), surface);     

    if (GetWindowThreadProcessId(hwnd, NULL) != GetCurrentThreadId()) return;

    if (!parent)  /* desktop */
    {
        return;
    }

    /* don't create wnd for HWND_MESSAGE windows */
    if (parent != GetDesktopWindow() && !GetAncestor(parent, GA_PARENT)) return;
    
    if (*surface)  {
        oldSurface = *surface;
        TRACE("     setting old surface %p (ref=%d)\n", *surface, (*surface)->ref);
    }
    *surface = NULL;
    
    *visible_rect = *window_rect;
    if (swp_flags & SWP_HIDEWINDOW) return;        
    CALL_NORETURN_7(BOXED_WINDOW_POS_CHANGING, hwnd, insert_after, swp_flags, window_rect, client_rect, visible_rect, surface);

    if (parent != GetDesktopWindow()) {
        return; // don't create surface
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
    if (oldSurface)  {
        TRACE("     releasing old surface %p (ref=%d)\n", oldSurface, oldSurface->ref);
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
            TRACE("     created new surface %p (ref=%d)\n", *surface, (*surface)->ref);
        }
    }
}

struct winZOrder {
    int count;
    HWND windows[1024];
};

BOOL getZOrderCallback(HWND hWnd, LPARAM lParam) {
    struct winZOrder* zorder = (struct winZOrder*)lParam;
    TRACE("hWnd=%p zorder->count=%d\n", hWnd, zorder->count);
    if (zorder->count<1024) {
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

BOOL WINE_CDECL boxeddrv_GetDeviceGammaRamp(PHYSDEV dev, LPVOID ramp) {
    int result;
    CALL_2(BOXED_GET_DEVICE_GAMMA_RAMP, dev, ramp);
    TRACE("dev=%p ramp=%p result=%d\n", dev, ramp, result);
    return (BOOL)result;
}

BOOL WINE_CDECL boxeddrv_SetDeviceGammaRamp(PHYSDEV dev, LPVOID ramp) {
    int result;
    CALL_2(BOXED_SET_DEVICE_GAMMA_RAMP, dev, ramp);
    TRACE("dev=%p ramp=%p result=%d\n", dev, ramp, result);
    return (BOOL)result;
}

INT WINE_CDECL boxeddrv_GetDeviceCaps(PHYSDEV dev, INT cap) {
    INT result;
    switch (cap) {
    case PDEVICESIZE:
        return sizeof(BOXEDDRV_PDEVICE);
    }
    TRACE("BOXED_GET_DEVICE_CAPS=%d\n", BOXED_GET_DEVICE_CAPS);
    CALL_2(BOXED_GET_DEVICE_CAPS, dev, cap);
    TRACE("dev=%p cap=%d result=%d\n", dev, cap, result);
    return result;
}

int CDECL wine_notify_icon(DWORD msg, NOTIFYICONDATAW *data) {
    int result;
    CALL_2(BOXED_WINE_NOTIFY_ICON, msg, data);
    return result;
}

BOOL WINAPI ImeConfigure(HKL hKL, HWND hWnd, DWORD dwMode, LPVOID lpData) {
    int result;
    CALL_4(BOXED_IME_CONFIGURE, hKL, hWnd, dwMode, lpData);
    return (BOOL)result;
}

DWORD WINAPI ImeConversionList(HIMC hIMC, LPCWSTR lpSource, LPCANDIDATELIST lpCandList, DWORD dwBufLen, UINT uFlag) {
    DWORD result;
    CALL_5(BOXED_IME_CONVERSION_LIST, hIMC, lpSource, lpCandList, dwBufLen, uFlag);
    return result;
}

BOOL WINAPI ImeDestroy(UINT uForce) {
    int result;
    CALL_1(BOXED_IME_DESTROY, uForce);
    return (BOOL)result;
}

UINT WINAPI ImeEnumRegisterWord(REGISTERWORDENUMPROCW lpfnEnumProc, LPCWSTR lpszReading, DWORD dwStyle, LPCWSTR lpszRegister, LPVOID lpData) {
    UINT result;
    CALL_5(BOXED_IME_ENUM_REGISTER_WORD, lpfnEnumProc, lpszReading, dwStyle, lpszRegister, lpData);
    return result;
}

LRESULT WINAPI ImeEscape(HIMC hIMC, UINT uSubFunc, LPVOID lpData) {
    LRESULT result;
    CALL_3(BOXED_IME_ESCAPE, hIMC, uSubFunc, lpData);
    return result;
}

DWORD WINAPI ImeGetImeMenuItems(HIMC hIMC, DWORD dwFlags, DWORD dwType, LPIMEMENUITEMINFOW lpImeParentMenu, LPIMEMENUITEMINFOW lpImeMenu, DWORD dwSize) {
    DWORD result;
    CALL_6(BOXED_IME_GET_IME_MENU_ITEMS, hIMC, dwFlags, dwType, lpImeParentMenu, lpImeMenu, dwSize);
    return result;
}

UINT WINAPI ImeGetRegisterWordStyle(UINT nItem, LPSTYLEBUFW lpStyleBuf) {
    UINT result;
    CALL_2(BOXED_IME_GET_REGISTER_WORD_STYLE, nItem, lpStyleBuf);
    return result;
}

BOOL WINAPI ImeInquire(LPIMEINFO lpIMEInfo, LPWSTR lpszUIClass, LPCWSTR lpszOption) {
    int result;
    CALL_3(BOXED_IME_INQUIRE, lpIMEInfo, lpszUIClass, lpszOption);
    return (BOOL)result;
}

BOOL WINAPI ImeProcessKey(HIMC hIMC, UINT vKey, LPARAM lKeyData, const LPBYTE lpbKeyState) {
    int result;
    CALL_4(BOXED_IME_PROCESS_KEY, hIMC, vKey, lKeyData, lpbKeyState);
    return (BOOL)result;
}

BOOL WINAPI ImeRegisterWord(LPCWSTR lpszReading, DWORD dwStyle, LPCWSTR lpszRegister) {
    int result;
    CALL_3(BOXED_IME_REGISTER_WORD, lpszReading, dwStyle, lpszRegister);
    return (BOOL)result;
}

BOOL WINAPI ImeSelect(HIMC hIMC, BOOL fSelect) {
    int result;
    CALL_2(BOXED_IME_SELECT, hIMC, fSelect);
    return (BOOL)result;
}

BOOL WINAPI ImeSetActiveContext(HIMC hIMC, BOOL fFlag) {
    int result;
    CALL_2(BOXED_IME_SET_ACTIVE_CONTEXT, hIMC, fFlag);
    return (BOOL)result;
}

BOOL WINAPI ImeSetCompositionString(HIMC hIMC, DWORD dwIndex, LPCVOID lpComp, DWORD dwCompLen, LPCVOID lpRead, DWORD dwReadLen) {
    int result;
    CALL_6(BOXED_IME_SET_COMPOSITION_STRING, hIMC, dwIndex, lpComp, dwCompLen, lpRead, dwReadLen);
    return (BOOL)result;
}

UINT WINAPI ImeToAsciiEx(UINT uVKey, UINT uScanCode, const LPBYTE lpbKeyState, LPDWORD lpdwTransKey, UINT fuState, HIMC hIMC) {
    UINT result;
    CALL_6(BOXED_IME_TO_ASCII_EX, uVKey, uScanCode, lpbKeyState, lpdwTransKey, fuState, hIMC);
    return (UINT)result;
}

BOOL WINAPI ImeUnregisterWord(LPCWSTR lpszReading, DWORD dwStyle, LPCWSTR lpszUnregister) {
    int result;
    CALL_3(BOXED_IME_UNREGISTER_WORD, lpszReading, dwStyle, lpszUnregister);
    return (BOOL)result;
}

BOOL WINAPI NotifyIME(HIMC hIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue) {
    int result;
    CALL_4(BOXED_NOTIFY_IME, hIMC, dwAction, dwIndex, dwValue);
    return (BOOL)result;
}

static BOOL BOXED_GLAPI boxeddrv_wglCopyContext(struct wgl_context *src, struct wgl_context *dst, UINT mask) {
    int result;
    CALL_3(BOXED_GL_COPY_CONTEXT, src, dst, mask);
    TRACE("boxeddrv_wglCopyContext src=%p dst=%p mask=%X result=%d\n", src, dst, mask, result);
    return (BOOL)result;
}

static struct wgl_context * BOXED_GLAPI boxeddrv_wglCreateContext(HDC hdc) {
    struct wgl_context* result;
    CALL_5(BOXED_GL_CREATE_CONTEXT, WindowFromDC(hdc), 0, 0, 0, 0);
    TRACE("boxeddrv_wglCreateContext hdc=%X result=%p\n", (int)hdc, result);
    return result;
}

static BOOL BOXED_GLAPI boxeddrv_wglDeleteContext(struct wgl_context *context) {
    TRACE("boxeddrv_wglDeleteContext context=%p\n", context);
    CALL_NORETURN_1(BOXED_GL_DELETE_CONTEXT, context);
    return TRUE;
}

static int BOXED_GLAPI boxeddrv_wglDescribePixelFormat(HDC hdc, int fmt, UINT size, PIXELFORMATDESCRIPTOR *descr) {
    int result;
    CALL_4(BOXED_GL_DESCRIBE_PIXEL_FORMAT, hdc, fmt, size, descr);
    TRACE("boxeddrv_wglDescribePixelFormat hdc=%X fmt=%d size=%d descr=%p result=%d\n", (int)hdc, fmt, size, descr, result);
    return result;
}

static int BOXED_GLAPI boxeddrv_wglGetPixelFormat(HDC hdc) {
    int result;
    CALL_1(BOXED_GL_GET_PIXEL_FORMAT, WindowFromDC(hdc));
    TRACE("boxeddrv_wglGetPixelFormat hdc=%X result=%d\n", (int)hdc, result);
    return result;
}

static struct wgl_context *boxeddrv_wglCreateContextAttribsARB(HDC hdc, struct wgl_context *share_context, const int *attrib_list);
static void* glModule;

static PROC BOXED_GLAPI boxeddrv_wglGetProcAddress(const char *proc) {
    TRACE("boxeddrv_wglGetProcAddress %s\n", proc);
    if (!strcmp(proc, "wglCreateContextAttribsARB"))
        return (PROC)boxeddrv_wglCreateContextAttribsARB;

    if (!glModule) {
        glModule = dlopen("/lib/libGL.so.1", RTLD_LAZY);        
    } 
    if (glModule) {
        int result = 0;
        PROC pfn;

        CALL_1(BOXED_GL_GET_PROC_ADDRESS, proc);
        if (!result) {
            TRACE("    %s not found\n", proc);
            return 0;
        }
        pfn = dlsym(glModule, proc);
        TRACE("glModule=%p result=%p\n", glModule, pfn);
        return pfn;
    }
    TRACE("could not find /lib/libGL.so.1\n");
    return NULL;
}

static BOOL BOXED_GLAPI boxeddrv_wglMakeCurrent(HDC hdc, struct wgl_context *context) {
    int result;
    CALL_2(BOXED_GL_MAKE_CURRENT, WindowFromDC(hdc), context);
    TRACE("boxeddrv_wglMakeCurrent hdc=%X context=%p result=%d\n",(int)hdc, context, result);
    return (BOOL)result;
}

static BOOL BOXED_GLAPI boxeddrv_wglSetPixelFormat(HDC hdc, int fmt, const PIXELFORMATDESCRIPTOR *descr) {
    int result;
    CALL_3(BOXED_GL_SET_PIXEL_FORMAT, WindowFromDC(hdc), fmt, descr);
    TRACE("boxeddrv_wglSetPixelFormat hdc=%X fmt=%d descr=%p result=%d\n", (int)hdc, fmt, descr, result);
    return (BOOL)result;
}

static BOOL BOXED_GLAPI boxeddrv_wglShareLists(struct wgl_context *org, struct wgl_context *dest) {
    int result;
    CALL_2(BOXED_GL_SHARE_LISTS, org, dest);
    TRACE("boxeddrv_wglShareLists org=%p dest=%p result=%d\n", org, dest, result);
    return (BOOL)result;
}

static BOOL BOXED_GLAPI boxeddrv_wglSwapBuffers(HDC hdc) {
    int result;
    CALL_1(BOXED_GL_SWAP_BUFFERS, hdc);
    return (BOOL)result;
}

UINT CDECL boxeddrv_GetKeyboardLayoutList(INT size, HKL *list) {
    UINT result;
    CALL_2(BOXED_GET_KEYBOARD_LAYOUT_LIST, size, list);
    return result;
}

BOOL CDECL boxeddrv_RegisterHotKey(HWND hwnd, UINT mod_flags, UINT vkey) {
    int result;
    CALL_3(BOXED_REGISTER_HOT_KEY, hwnd, mod_flags, vkey);
    return (BOOL)result;
}

void CDECL boxeddrv_UnregisterHotKey(HWND hwnd, UINT modifiers, UINT vkey) {
    CALL_NORETURN_3(BOXED_UNREGISTER_HOT_KEY, hwnd, modifiers, vkey);
}

static struct opengl_funcs opengl_funcs =
{
    {
        boxeddrv_wglCopyContext,          /* p_wglCopyContext */
        boxeddrv_wglCreateContext,        /* p_wglCreateContext */
        boxeddrv_wglDeleteContext,        /* p_wglDeleteContext */
        boxeddrv_wglDescribePixelFormat,  /* p_wglDescribePixelFormat */
        boxeddrv_wglGetPixelFormat,       /* p_wglGetPixelFormat */
        boxeddrv_wglGetProcAddress,       /* p_wglGetProcAddress */
        boxeddrv_wglMakeCurrent,          /* p_wglMakeCurrent */
        boxeddrv_wglSetPixelFormat,       /* p_wglSetPixelFormat */
        boxeddrv_wglShareLists,           /* p_wglShareLists */
        boxeddrv_wglSwapBuffers,          /* p_wglSwapBuffers */
    }
};

#if WINE_VULKAN_DRIVER_VERSION >= 7
static void* (*pvkGetDeviceProcAddr)(VkDevice, const char*);
static void* (*pvkGetInstanceProcAddr)(VkInstance, const char*);

static void* vulkan_handle;

static BOOL wine_vk_init(void)
{
    if (!(vulkan_handle = dlopen("/lib/libvulkan.so.1", RTLD_NOW)))
    {
        ERR("Failed to load libvulkan.so.1.\n");
        return TRUE;
    }
#define LOAD_FUNCPTR(f) p##f = dlsym(vulkan_handle, #f);
        LOAD_FUNCPTR(vkGetDeviceProcAddr)
        LOAD_FUNCPTR(vkGetInstanceProcAddr)
#undef LOAD_FUNCPTR
#undef LOAD_OPTIONAL_FUNCPTR
    return TRUE;

}

static VkResult boxedwine_vkCreateInstance(const VkInstanceCreateInfo* create_info, const VkAllocationCallbacks* allocator, VkInstance* instance)
{
    int result;
    TRACE("create_info %p, allocator %p, instance %p\n", create_info, allocator, instance);
    CALL_3(BOXED_VK_CREATE_INSTANCE, create_info, allocator, instance);    
    return (VkResult)result;
}

static VkResult boxedwine_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* create_info, const VkAllocationCallbacks* allocator, VkSwapchainKHR* swapchain)
{
    int result;
    TRACE("%p %p %p %p\n", device, create_info, allocator, swapchain);
    CALL_4(BOXED_VK_CREATE_SWAPCHAIN, device, create_info, allocator, swapchain);
    return (VkResult)result;
}

static VkResult boxedwine_vkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* create_info, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface)
{
    int result;
    TRACE("%p %p %p %p\n", instance, create_info, allocator, surface);
    CALL_4(BOXED_VK_CREATE_SURFACE, instance, create_info, allocator, surface);
    return (VkResult)result;
}

static void boxedwine_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* allocator)
{
    TRACE("%p %p\n", instance, allocator);
    CALL_NORETURN_2(BOXED_VK_DESTROY_INSTANCE, instance, allocator);
}

static void boxedwine_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* allocator)
{
    TRACE("%p 0x%s %p\n", instance, wine_dbgstr_longlong(surface), allocator);
    CALL_NORETURN_3(BOXED_VK_DESTROY_SURFACE, instance, &surface, allocator);
}

static void boxedwine_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* allocator)
{
    TRACE("%p, 0x%s %p\n", device, wine_dbgstr_longlong(swapchain), allocator);
    CALL_NORETURN_3(BOXED_VK_DESTROY_SWAPCHAIN, device, &swapchain, allocator);
}

static VkResult boxedwine_vkEnumerateInstanceExtensionProperties(const char* layer_name, uint32_t* count, VkExtensionProperties* properties)
{
    int result;
    TRACE("layer_name %s, count %p, properties %p\n", debugstr_a(layer_name), count, properties);
    CALL_3(BOXED_VK_ENUMERATE_INSTANCE_EXTENSION_PROPERTIES, layer_name, count, properties);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* flags)
{
    int result;
    TRACE("%p, 0x%s, %p\n", device, wine_dbgstr_longlong(surface), flags);
    CALL_3(BOXED_VK_GET_DEVICE_GROUP_SURFACE_PRESENT_MODES, device, &surface, flags);
    return (VkResult)result;
}

static void* boxedwine_vkGetDeviceProcAddr(VkDevice device, const char* name)
{
    TRACE("%p, %s\n", device, debugstr_a(name));
    return pvkGetDeviceProcAddr(device, name);
}

static void* boxedwine_vkGetInstanceProcAddr(VkInstance instance, const char* name)
{
    TRACE("%p, %s\n", instance, debugstr_a(name));
    return pvkGetInstanceProcAddr(instance, name);
}

static VkResult boxedwine_vkGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice phys_dev, VkSurfaceKHR surface, uint32_t* count, VkRect2D* rects)
{
    int result;
    TRACE("%p, 0x%s, %p, %p\n", phys_dev, wine_dbgstr_longlong(surface), count, rects);
    CALL_4(BOXED_VK_GET_PHYSICAL_DEVICE_PRESENT_RECTANGLES, phys_dev, &surface, count, rects);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice phys_dev, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* capabilities)
{
    int result;
    TRACE("%p, 0x%s, %p\n", phys_dev, wine_dbgstr_longlong(surface), capabilities);
    CALL_3(BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_CAPABILITIES, phys_dev, &surface, capabilities);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice phys_dev, const VkPhysicalDeviceSurfaceInfo2KHR* surface_info, VkSurfaceCapabilities2KHR* capabilities)
{
    int result;
    TRACE("%p, %p, %p\n", phys_dev, surface_info, capabilities);
    CALL_3(BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_CAPABILITIES2, phys_dev, surface_info, capabilities);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice phys_dev, VkSurfaceKHR surface, uint32_t* count, VkSurfaceFormatKHR* formats)
{
    int result;
    TRACE("%p, 0x%s, %p, %p\n", phys_dev, wine_dbgstr_longlong(surface), count, formats);
    CALL_4(BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_FORMATS, phys_dev, &surface, count, formats);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice phys_dev, const VkPhysicalDeviceSurfaceInfo2KHR* surface_info, uint32_t* count, VkSurfaceFormat2KHR* formats)
{
    int result;
    TRACE("%p, %p, %p, %p\n", phys_dev, surface_info, count, formats);
    CALL_4(BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_FORMATS2, phys_dev, surface_info, count, formats);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice phys_dev, VkSurfaceKHR surface, uint32_t* count, VkPresentModeKHR* modes)
{
    int result;
    TRACE("%p, 0x%s, %p, %p\n", phys_dev, wine_dbgstr_longlong(surface), count, modes);
    CALL_4(BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_PRESENT_MODES, phys_dev, &surface, count, modes);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice phys_dev, uint32_t index, VkSurfaceKHR surface, VkBool32* supported)
{
    int result;
    TRACE("%p, %u, 0x%s, %p\n", phys_dev, index, wine_dbgstr_longlong(surface), supported);
    CALL_4(BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_SUPPORT, phys_dev, index, &surface, supported);
    return (VkResult)result;
}

static VkBool32 boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice phys_dev, uint32_t index)
{
    int result;
    TRACE("%p %u\n", phys_dev, index);
    CALL_2(BOXED_VK_GET_PHYSICAL_DEVICE_WIN32_PRESENTATION_SUPPORT, phys_dev, index);
    return (VkBool32)result;
}

static VkResult boxedwine_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* count, VkImage* images)
{
    int result;
    TRACE("%p, 0x%s %p %p\n", device, wine_dbgstr_longlong(swapchain), count, images);
    CALL_4(BOXED_VK_GET_SWAPCHAIN_IMAGES, device, &swapchain, count, images);
    return (VkResult)result;
}

static VkResult boxedwine_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* present_info)
{
    int result;
    TRACE("%p, %p\n", queue, present_info);
    CALL_2(BOXED_VK_QUEUE_PRESENT, queue, present_info);
    return (VkResult)result;
}

#if WINE_VULKAN_DRIVER_VERSION >= 10
static VkSurfaceKHR boxedwine_wine_get_native_surface(VkSurfaceKHR surface)
{
    VkSurfaceKHR result;
    TRACE("0x%s\n", wine_dbgstr_longlong(surface));
    CALL_NORETURN_2(BOXED_VK_GET_NATIVE_SURFACE, &surface, &result);
    return result;
}
#endif

#if WINE_VULKAN_DRIVER_VERSION == 7
static const struct vulkan_funcs vulkan_funcs =
{
    boxeddrv_vkCreateInstance,
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroyInstance,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkEnumerateInstanceExtensionProperties,
    boxedwine_vkGetDeviceGroupSurfacePresentModesKHR,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDevicePresentRectanglesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormatsKHR,
    boxedwine_vkGetPhysicalDeviceSurfacePresentModesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceSupportKHR,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,
};
#elif WINE_VULKAN_DRIVER_VERSION == 8
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateInstance,
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroyInstance,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkEnumerateInstanceExtensionProperties,
    boxedwine_vkGetDeviceGroupSurfacePresentModesKHR,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDevicePresentRectanglesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceCapabilities2KHR,
    boxedwine_vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormats2KHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormatsKHR,
    boxedwine_vkGetPhysicalDeviceSurfacePresentModesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceSupportKHR,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,
};
#elif WINE_VULKAN_DRIVER_VERSION == 10
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateInstance,
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroyInstance,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkEnumerateInstanceExtensionProperties,
    boxedwine_vkGetDeviceGroupSurfacePresentModesKHR,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDevicePresentRectanglesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceCapabilities2KHR,
    boxedwine_vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormats2KHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormatsKHR,
    boxedwine_vkGetPhysicalDeviceSurfacePresentModesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceSupportKHR,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,
};
#else
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateInstance,
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroyInstance,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkEnumerateInstanceExtensionProperties,
    boxedwine_vkGetDeviceGroupSurfacePresentModesKHR,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDevicePresentRectanglesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceCapabilities2KHR,
    boxedwine_vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormats2KHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormatsKHR,
    boxedwine_vkGetPhysicalDeviceSurfacePresentModesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceSupportKHR,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,

    boxedwine_wine_get_native_surface,
};
#endif

// WINE_VULKAN_DRIVER_VERSION
// 7  Jul 16, 2018 (Wine 3.13)
// 8  Apr 7, 2020 (Wine 6.0)
// 9  Jan 22, 2021 (Wine 6.1)
// 10 Jan 26, 2021 (Wine 6.1)

const struct vulkan_funcs* WINE_CDECL boxeddrv_wine_get_vulkan_driver(PHYSDEV dev, UINT version)
{
    TRACE("version %d\n", version);
    if (version != WINE_VULKAN_DRIVER_VERSION)
    {
        ERR("version mismatch, vulkan wants %u but boxeddrv has %u\n", version, WINE_WGL_DRIVER_VERSION);
        return NULL;
    }

    if (wine_vk_init())
        return &vulkan_funcs;
    return NULL;
}
#endif

int initOpengl(void) {
    static int init_done;
    static void *opengl_handle;

    unsigned int i;

    if (init_done) return (opengl_handle != NULL);
    init_done = 1;

    /* No need to load any other libraries as according to the ABI, libGL should be self-sufficient
       and include all dependencies */
    opengl_handle = dlopen("libGL.so.1", RTLD_NOW|RTLD_GLOBAL);
    if (opengl_handle == NULL)
    {
        ERR( "Failed to load libGL\n");
        ERR( "OpenGL support is disabled.\n");
        return FALSE;
    }

    for (i = 0; i < sizeof(opengl_func_names)/sizeof(opengl_func_names[0]); i++)
    {
        if (!(((void **)&opengl_funcs.gl)[i] = dlsym( opengl_handle, opengl_func_names[i])))
        {
            ERR( "%s not found in libGL, disabling OpenGL.\n", opengl_func_names[i] );
            goto failed;
        }
    }
    return TRUE;

failed:
    dlclose(opengl_handle);
    opengl_handle = NULL;
    return FALSE;
}

struct opengl_funcs* WINE_CDECL boxeddrv_wine_get_wgl_driver(PHYSDEV dev, UINT version)
{
    if (version != WINE_WGL_DRIVER_VERSION)
    {
        ERR("version mismatch, opengl32 wants %u but boxeddrv has %u\n", version, WINE_WGL_DRIVER_VERSION);
        return NULL;
    }

    if (initOpengl())
        return &opengl_funcs;
    return NULL;
}

static inline BOXEDDRV_PDEVICE *get_boxeddrv_dev(PHYSDEV dev)
{
    return (BOXEDDRV_PDEVICE*)dev;
}

static BOXEDDRV_PDEVICE *create_boxed_physdev(void)
{
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BOXEDDRV_PDEVICE));
}


/**********************************************************************
*              DeleteDC (BOXEDDRV.@)
*/
static BOOL WINE_CDECL boxeddrv_DeleteDC(PHYSDEV dev)
{
    BOXEDDRV_PDEVICE *physDev = get_boxeddrv_dev(dev);

    TRACE("hdc %p\n", dev->hdc);

    HeapFree(GetProcessHeap(), 0, physDev);
    return TRUE;
}

UINT WINE_CDECL boxeddrv_RealizePalette( PHYSDEV dev, HPALETTE hpal, BOOL primary ) {
    PALETTEENTRY entries[256];
    WORD num_entries;
    UINT result;

    TRACE("dev=%p hpal=%p primary=%d\n", dev, hpal, primary);
    if (!GetObjectW( hpal, sizeof(num_entries), &num_entries )) return 0;

     if (num_entries > 256)
    {
        FIXME( "more than 256 entries not supported\n" );
        num_entries = 256;
    }
    if (!(num_entries = GetPaletteEntries( hpal, 0, num_entries, entries ))) return 0;
    CALL_2(BOXED_REALIZE_PALETTE, (DWORD)num_entries, entries);
    TRACE("num_entries=%d entries=%p result=%d\n", num_entries, entries, result);
    return result;
}

BOOL WINE_CDECL boxeddrv_UnrealizePalette( HPALETTE hpal )
{
    return TRUE;
}

UINT WINE_CDECL boxeddrv_GetSystemPaletteEntries( PHYSDEV dev, UINT start, UINT count, LPPALETTEENTRY entries )
{
    UINT result;
    CALL_3(BOXED_GET_SYSTEM_PALETTE, start, count, entries);
    TRACE("dev=%p start=%d count=%d entries=%p result=%d\n", dev, start, count, entries, result);
    return result;
}

COLORREF WINE_CDECL boxeddrv_GetNearestColor( PHYSDEV dev, COLORREF color )
{
    COLORREF result;
    CALL_1(BOXED_GET_NEAREST_COLOR, color);
    return result;
}

UINT WINE_CDECL boxeddrv_RealizeDefaultPalette( PHYSDEV dev )
{
    PALETTEENTRY entries[256];
    int count;
    UINT result;

    count = GetPaletteEntries( GetStockObject(DEFAULT_PALETTE), 0, 256, entries );
    CALL_2(BOXED_REALIZE_DEFAULT_PALETTE, count, entries);
    TRACE("dev=%p result=%d\n",dev, result);
    return result;
}

static BOOL WINE_CDECL boxeddrv_CreateDC(PHYSDEV *pdev, LPCWSTR driver, LPCWSTR device, LPCWSTR output, const DEVMODEW* initData);
static BOOL WINE_CDECL boxeddrv_CreateCompatibleDC(PHYSDEV orig, PHYSDEV *pdev);

// Dec 18, 2012, wine-1.5.20 
#if WINE_GDI_DRIVER_VERSION == 46
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExcludeClipRect */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtSelectClipRgn */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGdiRealizationInfo */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pIntersectClipRect */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pModifyWorldTransform */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pOffsetClipRgn */
    NULL,                                   /* pOffsetViewportOrg */
    NULL,                                   /* pOffsetWindowOrg */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolygon */
    NULL,                                   /* pPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSaveDC */
    NULL,                                   /* pScaleViewportExt */
    NULL,                                   /* pScaleWindowExt */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPalette */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetArcDirection */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBkMode */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetLayout */
    NULL,                                   /* pSetMapMode */
    NULL,                                   /* pSetMapperFlags */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetPolyFillMode */
    NULL,                                   /* pSetROP2 */
    NULL,                                   /* pSetRelAbs */
    NULL,                                   /* pSetStretchBltMode */
    NULL,                                   /* pSetTextAlign */
    NULL,                                   /* pSetTextCharacterExtra */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pSetTextJustification */
    NULL,                                   /* pSetViewportExt */
    NULL,                                   /* pSetViewportOrg */
    NULL,                                   /* pSetWindowExt */
    NULL,                                   /* pSetWindowOrg */
    NULL,                                   /* pSetWorldTransform */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Sep 1, 2015, wine-1.7.51
#if WINE_GDI_DRIVER_VERSION == 47
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExcludeClipRect */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtSelectClipRgn */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pIntersectClipRect */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pModifyWorldTransform */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pOffsetClipRgn */
    NULL,                                   /* pOffsetViewportOrg */
    NULL,                                   /* pOffsetWindowOrg */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolygon */
    NULL,                                   /* pPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSaveDC */
    NULL,                                   /* pScaleViewportExt */
    NULL,                                   /* pScaleWindowExt */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPalette */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetArcDirection */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBkMode */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetLayout */
    NULL,                                   /* pSetMapMode */
    NULL,                                   /* pSetMapperFlags */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetPolyFillMode */
    NULL,                                   /* pSetROP2 */
    NULL,                                   /* pSetRelAbs */
    NULL,                                   /* pSetStretchBltMode */
    NULL,                                   /* pSetTextAlign */
    NULL,                                   /* pSetTextCharacterExtra */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pSetTextJustification */
    NULL,                                   /* pSetViewportExt */
    NULL,                                   /* pSetViewportOrg */
    NULL,                                   /* pSetWindowExt */
    NULL,                                   /* pSetWindowOrg */
    NULL,                                   /* pSetWorldTransform */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Feb 27, 2018 wine-3.3
#if WINE_GDI_DRIVER_VERSION == 48
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExcludeClipRect */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtSelectClipRgn */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pIntersectClipRect */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pModifyWorldTransform */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pOffsetClipRgn */
    NULL,                                   /* pOffsetViewportOrg */
    NULL,                                   /* pOffsetWindowOrg */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolygon */
    NULL,                                   /* pPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSaveDC */
    NULL,                                   /* pScaleViewportExt */
    NULL,                                   /* pScaleWindowExt */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPalette */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetArcDirection */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBkMode */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetLayout */
    NULL,                                   /* pSetMapMode */
    NULL,                                   /* pSetMapperFlags */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetPolyFillMode */
    NULL,                                   /* pSetROP2 */
    NULL,                                   /* pSetRelAbs */
    NULL,                                   /* pSetStretchBltMode */
    NULL,                                   /* pSetTextAlign */
    NULL,                                   /* pSetTextCharacterExtra */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pSetTextJustification */
    NULL,                                   /* pSetViewportExt */
    NULL,                                   /* pSetViewportOrg */
    NULL,                                   /* pSetWindowExt */
    NULL,                                   /* pSetWindowOrg */
    NULL,                                   /* pSetWorldTransform */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    NULL,                                   /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Apr 9, 2019, wine-4.6
#if WINE_GDI_DRIVER_VERSION == 49 || WINE_GDI_DRIVER_VERSION == 50
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExcludeClipRect */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtSelectClipRgn */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pIntersectClipRect */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pModifyWorldTransform */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pOffsetClipRgn */
    NULL,                                   /* pOffsetViewportOrg */
    NULL,                                   /* pOffsetWindowOrg */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolygon */
    NULL,                                   /* pPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSaveDC */
    NULL,                                   /* pScaleViewportExt */
    NULL,                                   /* pScaleWindowExt */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPalette */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetArcDirection */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBkMode */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetLayout */
    NULL,                                   /* pSetMapMode */
    NULL,                                   /* pSetMapperFlags */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetPolyFillMode */
    NULL,                                   /* pSetROP2 */
    NULL,                                   /* pSetRelAbs */
    NULL,                                   /* pSetStretchBltMode */
    NULL,                                   /* pSetTextAlign */
    NULL,                                   /* pSetTextCharacterExtra */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pSetTextJustification */
    NULL,                                   /* pSetViewportExt */
    NULL,                                   /* pSetViewportOrg */
    NULL,                                   /* pSetWindowExt */
    NULL,                                   /* pSetWindowOrg */
    NULL,                                   /* pSetWorldTransform */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    NULL,                                   /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Jul 6, 2019 wine-4.12.1
#if WINE_GDI_DRIVER_VERSION == 50
// added CDECL
#endif

// Oct 22, 2019 wine-4.19
#if WINE_GDI_DRIVER_VERSION == 51
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExcludeClipRect */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtSelectClipRgn */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pIntersectClipRect */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pModifyWorldTransform */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pOffsetClipRgn */
    NULL,                                   /* pOffsetViewportOrg */
    NULL,                                   /* pOffsetWindowOrg */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolygon */
    NULL,                                   /* pPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSaveDC */
    NULL,                                   /* pScaleViewportExt */
    NULL,                                   /* pScaleWindowExt */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPalette */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetArcDirection */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBkMode */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetLayout */
    NULL,                                   /* pSetMapMode */
    NULL,                                   /* pSetMapperFlags */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetPolyFillMode */
    NULL,                                   /* pSetROP2 */
    NULL,                                   /* pSetRelAbs */
    NULL,                                   /* pSetStretchBltMode */
    NULL,                                   /* pSetTextAlign */
    NULL,                                   /* pSetTextCharacterExtra */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pSetTextJustification */
    NULL,                                   /* pSetViewportExt */
    NULL,                                   /* pSetViewportOrg */
    NULL,                                   /* pSetWindowExt */
    NULL,                                   /* pSetWindowOrg */
    NULL,                                   /* pSetWorldTransform */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,        /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

/**********************************************************************
*              CreateDC (BOXEDDRV.@)
*/
static BOOL WINE_CDECL boxeddrv_CreateDC(PHYSDEV *pdev, LPCWSTR driver, LPCWSTR device,
    LPCWSTR output, const DEVMODEW* initData)
{
    BOXEDDRV_PDEVICE *physDev = create_boxed_physdev();

    TRACE("pdev %p hdc %p driver %s device %s output %s initData %p\n", pdev,
        (*pdev)->hdc, debugstr_w(driver), debugstr_w(device), debugstr_w(output),
        initData);

    if (!physDev) return FALSE;

    push_dc_driver(pdev, &physDev->dev, &boxeddrv_funcs);
    CALL_NORETURN_1(BOXED_CREATE_DC, physDev);
    TRACE("priority=%d\n", boxeddrv_funcs.priority);
    return TRUE;
}


/**********************************************************************
*              CreateCompatibleDC (BOXEDDRV.@)
*/
static BOOL WINE_CDECL boxeddrv_CreateCompatibleDC(PHYSDEV orig, PHYSDEV *pdev)
{
    BOXEDDRV_PDEVICE *physDev = create_boxed_physdev();

    TRACE("orig %p orig->hdc %p pdev %p pdev->hdc %p\n", orig, (orig ? orig->hdc : NULL), pdev,
        ((pdev && *pdev) ? (*pdev)->hdc : NULL));

    if (!physDev) return FALSE;

    push_dc_driver(pdev, &physDev->dev, &boxeddrv_funcs);
    CALL_NORETURN_1(BOXED_CREATE_DC, physDev);
    return TRUE;
}

/******************************************************************************
 *              boxeddrv_get_gdi_driver
 */
const struct gdi_dc_funcs * CDECL boxeddrv_get_gdi_driver(unsigned int version)
{
    int result;

    if (version != WINE_GDI_DRIVER_VERSION)
    {
        ERR("version mismatch, gdi32 wants %u but wineboxed has %u\n", version, WINE_GDI_DRIVER_VERSION);
        return NULL;
    }
    CALL_0(BOXED_GET_VERSION)
    if (result != 3) {
        ERR("version mismatch, boxedwine wants %u but winex11.drv has %u\n", result, 3);
        return NULL;
    }
    return &boxeddrv_funcs;
}

static struct wgl_context *boxeddrv_wglCreateContextAttribsARB(HDC hdc, struct wgl_context *share_context, const int *attrib_list)
{
    const int *iptr;
    int major = 1, minor = 0, profile = WGL_CONTEXT_CORE_PROFILE_BIT_ARB, flags = 0;
    struct wgl_context* result;

    TRACE("boxeddrv_wglCreateContextAttribsARB hdc=%p share_context=%p attrib_list=%p\n", hdc, share_context, attrib_list);

    for (iptr = attrib_list; iptr && *iptr; iptr += 2)
    {
        int attr = iptr[0];
        int value = iptr[1];

        TRACE("attribute %d.%d\n", attr, value);

        switch (attr)
        {
            case WGL_CONTEXT_MAJOR_VERSION_ARB:
                major = value;
                break;

            case WGL_CONTEXT_MINOR_VERSION_ARB:
                minor = value;
                break;

            case WGL_CONTEXT_LAYER_PLANE_ARB:
                WARN("WGL_CONTEXT_LAYER_PLANE_ARB attribute ignored\n");
                break;

            case WGL_CONTEXT_FLAGS_ARB:
                flags = value;
                if (flags & ~WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB)
                    WARN("WGL_CONTEXT_FLAGS_ARB attributes %#x ignored\n", flags & ~WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB);
                break;

            case WGL_CONTEXT_PROFILE_MASK_ARB:
                if (value != WGL_CONTEXT_CORE_PROFILE_BIT_ARB &&
                    value != WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB)
                {
                    WARN("WGL_CONTEXT_PROFILE_MASK_ARB bits %#x invalid\n", value);
                    SetLastError(ERROR_INVALID_PROFILE_ARB);
                    return NULL;
                }
                profile = value;
                break;            
            default:
                WARN("Unknown attribute %d.%d\n", attr, value);
                SetLastError(ERROR_INVALID_PARAMETER);
                return NULL;
        }
    }

    CALL_5(BOXED_GL_CREATE_CONTEXT, WindowFromDC(hdc), major, minor, profile, flags);
    return result;
}

BOOL CDECL boxeddrv_create_desktop( UINT width, UINT height )
{
    int result;
    CALL_2(BOXED_CREATE_DESKTOP, width, height);
    return (BOOL)result;
}

HKL CDECL boxeddrv_LoadKeyboardLayout(LPCWSTR name, UINT flags)
{
    FIXME("%s, %04x: semi-stub! Returning default layout.\n", debugstr_w(name), flags);
    return get_locale_kbd_layout();
}

BOOL CDECL boxeddrv_UnloadKeyboardLayout(HKL hkl)
{
    FIXME("%p: stub!\n", hkl);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

void CDECL boxeddrv_FlashWindowEx( PFLASHWINFO pfinfo )
{
}

void CDECL boxeddrv_GetDC( HDC hdc, HWND hwnd, HWND top, const RECT *win_rect, const RECT *top_rect, DWORD flags ) 
{
}

void CDECL boxeddrv_ReleaseDC( HWND hwnd, HDC hdc )
{
}

BOOL CDECL boxeddrv_ScrollDC( HDC hdc, INT dx, INT dy, HRGN update )
{
    return FALSE;
}

void CDECL boxeddrv_SetWindowIcon( HWND hwnd, UINT type, HICON icon )
{
}

void CDECL boxeddrv_UpdateClipboard(void)
{
}

void CDECL boxeddrv_ThreadDetach(void) 
{
}
