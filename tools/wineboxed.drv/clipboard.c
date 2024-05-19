#if 0
MAKE_DEP_UNIX
#endif

#include "wineboxed.h"
#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(boxeddrv);

int WINE_CDECL boxeddrv_AcquireClipboard(HWND hwnd) {
    int result;
    CALL_1(BOXED_ACQUIRE_CLIPBOARD, hwnd);
    TRACE("hwnd=%p result=%d\n", hwnd, result);
    return result;
}

INT WINE_CDECL boxeddrv_CountClipboardFormats(void) {
    INT result;
    CALL_0(BOXED_COUNT_CLIPBOARD_FORMATS);
    TRACE("result=%d\n", result);
    return result;
}

void WINE_CDECL boxeddrv_EmptyClipboard(BOOL keepunowned) {
    TRACE("keepunowned=%d\n", keepunowned);
    CALL_NORETURN_1(BOXED_EMPTY_CLIPBOARD, keepunowned);
}

void WINE_CDECL boxeddrv_EndClipboardUpdate(void) {
    TRACE("\n");
    CALL_NORETURN_0(BOXED_END_CLIPBOARD_UPDATE);
}

UINT WINE_CDECL boxeddrv_EnumClipboardFormats(UINT prev_format) {
    UINT result;
    CALL_1(BOXED_ENUM_CLIPBOARD_FORMATS, prev_format);
    TRACE("prev_format=%d result=%d\n", prev_format, result);
    return result;
}

static char tmp64k[64 * 1024];

HANDLE WINE_CDECL boxeddrv_GetClipboardData(UINT desired_format) {
    int result;
    HANDLE h = 0;

    CALL_3(BOXED_GET_CLIPBOARD_DATA, desired_format, tmp64k, sizeof(tmp64k));
    TRACE("desired_format=%d result=%d %s\n", desired_format, result, result > 0 ? tmp64k : "");
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

BOOL WINE_CDECL boxeddrv_IsClipboardFormatAvailable(UINT desired_format) {
    int result;
    CALL_1(BOXED_IS_CLIPBOARD_FORMAT_AVAILABLE, desired_format);
    TRACE("desired_format=%d result=%d\n", desired_format, result);
    return (BOOL)result;
}

BOOL WINE_CDECL boxeddrv_SetClipboardData(UINT format_id, HANDLE data, BOOL owner) {
    int result;
    int len = GlobalSize(data);
    LPVOID src = GlobalLock(data);
    WCHAR buffer[256];

    buffer[0] = 0;
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