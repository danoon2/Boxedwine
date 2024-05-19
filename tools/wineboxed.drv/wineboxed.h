#ifndef WINE_BOXED_H

#include <stdarg.h>

#include "windef.h"
#include "winbase.h"
#include "wingdi.h"
#include "winuser.h"
#if BOXED_WINE_VERSION < 8100 || defined(WINE_UNIX_LIB)
#if BOXED_WINE_VERSION >= 7000
#include "ntgdi.h"
#endif
#include "wine/gdi_driver.h"
#else
#include "winternl.h"
#define WINE_GDI_DRIVER_VERSION 83
#endif
#include "unixlib.h"

#if BOXED_WINE_VERSION >= 4120 && BOXED_WINE_VERSION <= 7050
#define WINE_CDECL
#else
#define WINE_CDECL
#endif

#if BOXED_WINE_VERSION >= 8100
#define GDI_CDECL
#else
#define GDI_CDECL
#endif

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

#define CALL_0(index) __asm__("push %1\n\tint $0x98\n\taddl $4, %%esp": "=a" (result):"i"(index):); 
#define CALL_1(index, arg1) __asm__("push %2\n\tpush %1\n\tint $0x98\n\taddl $8, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1): "esp"); 
#define CALL_2(index, arg1,arg2) __asm__("push %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $12, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2): "esp");
#define CALL_3(index, arg1,arg2,arg3) __asm__("push %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $16, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3): "esp");
#define CALL_4(index, arg1,arg2,arg3,arg4) __asm__("push %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $20, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4): "esp");
#define CALL_5(index, arg1,arg2,arg3,arg4,arg5) __asm__("push %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $24, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5): "esp");
#define CALL_6(index, arg1,arg2,arg3,arg4,arg5,arg6) __asm__("push %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $28, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5), "g"((DWORD)arg6): "esp");
#define CALL_7(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7) __asm__("push %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $32, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5), "g"((DWORD)arg6), "g"((DWORD)arg7): "esp");

#define CALL_NORETURN_0(index) __asm__("push %0\n\tint $0x98\n\taddl $4, %%esp"::"i"(index)); 
#define CALL_NORETURN_1(index, arg1) __asm__("push %1\n\tpush %0\n\tint $0x98\n\taddl $8, %%esp"::"i"(index), "g"((DWORD)arg1): "esp"); 
#define CALL_NORETURN_2(index, arg1, arg2) __asm__("push %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $12, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2): "esp"); 
#define CALL_NORETURN_3(index, arg1, arg2, arg3) __asm__("push %3\n\rpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $16, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3): "esp"); 
#define CALL_NORETURN_4(index, arg1, arg2, arg3, arg4) __asm__("push %4\n\tpush %3\n\rpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $20, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4): "esp"); 
#define CALL_NORETURN_5(index, arg1, arg2, arg3, arg4, arg5) __asm__("push %5\n\tpush %4\n\tpush %3\n\rpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $24, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5): "esp"); 
#define CALL_NORETURN_7(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7) __asm__("push %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $32, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5), "g"((DWORD)arg6), "g"((DWORD)arg7): "esp");
#define CALL_NORETURN_8(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) __asm__("push %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $36, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5), "g"((DWORD)arg6), "g"((DWORD)arg7), "g"((DWORD)arg8): "esp");
#define CALL_NORETURN_9(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) __asm__("push %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $40, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5), "g"((DWORD)arg6), "g"((DWORD)arg7), "g"((DWORD)arg8), "g"((DWORD)arg9): "esp");

void BOXEDDRV_ProcessAttach(void);
BOOL processEvents(DWORD mask);
void initEvents(void);
void BOXEDDRV_DisplayDevices_Init(BOOL force);
void boxeddrv_FlushSurface(HWND hwnd, void* bits, int xOrg, int yOrg, int width, int height, RECT* rects, int rectCount);

void WINE_CDECL boxeddrv_UpdateClipboard(void);

void WINE_CDECL boxeddrv_UnregisterHotKey(HWND hwnd, UINT modifiers, UINT vkey);
BOOL WINE_CDECL boxeddrv_RegisterHotKey(HWND hwnd, UINT mod_flags, UINT vkey);
UINT WINE_CDECL boxeddrv_GetKeyboardLayoutList(INT size, HKL* list);
LRESULT WINE_CDECL boxeddrv_WindowMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
SHORT WINE_CDECL boxeddrv_VkKeyScanEx(WCHAR wChar, HKL hkl);
BOOL WINE_CDECL boxeddrv_UpdateLayeredWindow(HWND hwnd, const UPDATELAYEREDWINDOWINFO* info, const RECT* window_rect);
INT WINE_CDECL boxeddrv_ToUnicodeEx(UINT virtKey, UINT scanCode, const BYTE* lpKeyState, LPWSTR bufW, int bufW_size, UINT flags, HKL hkl);
BOOL WINE_CDECL boxeddrv_SystemParametersInfo(UINT action, UINT int_param, void* ptr_param, UINT flags);
LRESULT WINE_CDECL boxeddrv_SysCommand(HWND hwnd, WPARAM wparam, LPARAM lparam);
UINT WINE_CDECL boxeddrv_ShowWindow(HWND hwnd, INT cmd, RECT* rect, UINT swp);
void WINE_CDECL boxeddrv_SetWindowText(HWND hwnd, LPCWSTR text);
void WINE_CDECL boxeddrv_SetWindowStyle(HWND hwnd, INT offset, STYLESTRUCT* style);
void WINE_CDECL boxeddrv_SetParent(HWND hwnd, HWND parent, HWND old_parent);
void WINE_CDECL boxeddrv_SetLayeredWindowAttributes(HWND hwnd, COLORREF key, BYTE alpha, DWORD flags);
void WINE_CDECL boxeddrv_SetFocus(HWND hwnd);
BOOL WINE_CDECL boxeddrv_SetCursorPos(INT x, INT y);
#if BOXED_WINE_VERSION >= 8110
void WINE_CDECL boxeddrv_SetCursor(HWND hwnd, HCURSOR cursor);
#else
void WINE_CDECL boxeddrv_SetCursor(HCURSOR cursor);
#endif
void WINE_CDECL boxeddrv_SetCapture(HWND hwnd, UINT flags);
UINT WINE_CDECL boxeddrv_MapVirtualKeyEx(UINT wCode, UINT wMapType, HKL hkl);
BOOL WINE_CDECL boxeddrv_GetMonitorInfo(HMONITOR monitor, LPMONITORINFO info);
INT WINE_CDECL boxeddrv_GetKeyNameText(LONG lparam, LPWSTR buffer, INT size);
BOOL WINE_CDECL boxeddrv_GetCursorPos(LPPOINT pos);
BOOL WINE_CDECL boxeddrv_EnumDisplayMonitors(HDC hdc, LPRECT rect, MONITORENUMPROC proc, LPARAM lparam);
void WINE_CDECL boxeddrv_DestroyWindow(HWND hwnd);
void WINE_CDECL boxeddrv_DestroyCursorIcon(HCURSOR cursor);
BOOL WINE_CDECL boxeddrv_CreateWindow(HWND hwnd);
BOOL WINE_CDECL boxeddrv_CreateDesktopWindow(HWND hwnd);
void WINE_CDECL boxeddrv_SetDesktopWindow(HWND hwnd);
#if BOXED_WINE_VERSION >= 8100
BOOL WINE_CDECL boxeddrv_ClipCursor(const LPCRECT clip, BOOL reset);
#else
BOOL WINE_CDECL boxeddrv_ClipCursor(LPCRECT clip);
#endif
LONG WINE_CDECL boxeddrv_ChangeDisplaySettingsEx(LPCWSTR devname, LPDEVMODEW devmode, HWND hwnd, DWORD flags, LPVOID lpvoid);
void WINE_CDECL boxeddrv_Beep(void);

#if BOXED_WINE_VERSION >= 7210
LONG WINE_CDECL boxeddrv_ChangeDisplaySettings(LPDEVMODEW devmode, LPCWSTR primary_name, HWND hwnd, DWORD flags, LPVOID lpvoid);
#else
LONG WINE_CDECL boxeddrv_ChangeDisplaySettings(LPDEVMODEW devmode, HWND hwnd, DWORD flags, LPVOID lpvoid);
#endif

#if BOXED_WINE_VERSION >= 1718
void WINE_CDECL boxeddrv_SetWindowRgn(HWND hwnd, HRGN hrgn, BOOL redraw);
#else
int WINE_CDECL boxeddrv_SetWindowRgn(HWND hwnd, HRGN hrgn, BOOL redraw);
#endif

#if WINE_GDI_DRIVER_VERSION >= 78
NTSTATUS WINE_CDECL boxeddrv_MsgWaitForMultipleObjectsEx(DWORD count, const HANDLE* handles, const LARGE_INTEGER* timeout, DWORD mask, DWORD flags);
#else
DWORD WINE_CDECL boxeddrv_MsgWaitForMultipleObjectsEx(DWORD count, const HANDLE * handles, DWORD timeout, DWORD mask, DWORD flags);
#endif

BOOL WINE_CDECL boxeddrv_ProcessEvents(DWORD mask);

#if BOXED_WINE_VERSION >= 6080
BOOL WINE_CDECL boxeddrv_ActivateKeyboardLayout(HKL hkl, UINT flags);
#else
HKL WINE_CDECL boxeddrv_ActivateKeyboardLayout(HKL hkl, UINT flags);
#endif

#if BOXED_WINE_VERSION >= 7210
BOOL WINE_CDECL boxeddrv_GetCurrentDisplaySettings(LPCWSTR name, BOOL is_primary, LPDEVMODEW devmode);
#else
BOOL WINE_CDECL boxeddrv_GetCurrentDisplaySettings(LPCWSTR name, LPDEVMODEW devmode);
#endif

BOOL GDI_CDECL boxeddrv_UnrealizePalette(HPALETTE hpal);
void WINE_CDECL boxeddrv_ThreadDetach(void);
INT WINE_CDECL internal_GetDeviceCaps(INT cap);
INT WINE_CDECL boxeddrv_GetDisplayDepth(LPCWSTR name, BOOL is_primary);

#endif
