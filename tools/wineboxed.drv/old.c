#include "wineboxed.h"
#include "winnls.h"
#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(boxeddrv);

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
        layout |= 0xe0010000; /* IME */
    else
        layout |= layout << 16;

    return (HKL)layout;
}

// removed in version 70?
HKL CDECL boxeddrv_LoadKeyboardLayout(LPCWSTR name, UINT flags)
{
    FIXME("%s, %04x: semi-stub! Returning default layout.\n", debugstr_w(name), flags);
    return get_locale_kbd_layout();
}

// removed in version 70?
BOOL CDECL boxeddrv_UnloadKeyboardLayout(HKL hkl)
{
    FIXME("%p: stub!\n", hkl);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

void WINE_CDECL boxeddrv_SetWindowIcon(HWND hwnd, UINT type, HICON icon)
{
}

BOOL WINE_CDECL boxeddrv_ScrollDC(HDC hdc, INT dx, INT dy, HRGN update)
{
    return FALSE;
}

void WINE_CDECL boxeddrv_ReleaseDC(HWND hwnd, HDC hdc)
{
}

void WINE_CDECL boxeddrv_GetDC(HDC hdc, HWND hwnd, HWND top, const RECT* win_rect, const RECT* top_rect, DWORD flags)
{
}

void WINE_CDECL boxeddrv_FlashWindowEx(PFLASHWINFO pfinfo)
{
}

// removed in version 70?
BOOL WINE_CDECL boxeddrv_GetKeyboardLayoutName(LPWSTR name) {
    static const WCHAR formatW[] = { '%','0','8','x',0 };
    DWORD layout;

    layout = HandleToUlong(get_locale_kbd_layout());
    if (HIWORD(layout) == LOWORD(layout)) layout = LOWORD(layout);
    wsprintfW(name, formatW, layout);
    TRACE("returning %s\n", debugstr_w(name));
    return TRUE;
}

HKL WINE_CDECL boxeddrv_GetKeyboardLayout(DWORD thread_id) {
    return get_locale_kbd_layout();
}