#if 0
MAKE_DEP_UNIX
#endif

#include "wineboxed.h"
#include "shellapi.h"
#include "boxed_imm.h"

BOOL WINAPI boxeddrv_create_desktop(UINT width, UINT height)
{
    int result;
    CALL_2(BOXED_CREATE_DESKTOP, width, height);
    return (BOOL)result;
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

#if BOXED_WINE_VERSION >= 8020
BOOL WINAPI ImeInquire(LPIMEINFO lpIMEInfo, LPWSTR lpszUIClass, DWORD lpszOption) {
#else
BOOL WINAPI ImeInquire(LPIMEINFO lpIMEInfo, LPWSTR lpszUIClass, LPCWSTR lpszOption) {
#endif
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

#if BOXED_WINE_VERSION >= 8020
UINT WINAPI ImeToAsciiEx(UINT uVKey, UINT uScanCode, const LPBYTE lpbKeyState, LPTRANSMSGLIST lpdwTransKey, UINT fuState, HIMC hIMC) {
#else
UINT WINAPI ImeToAsciiEx(UINT uVKey, UINT uScanCode, const LPBYTE lpbKeyState, LPDWORD lpdwTransKey, UINT fuState, HIMC hIMC) {
#endif
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

int CDECL wine_notify_icon(DWORD msg, NOTIFYICONDATAW* data) {
    int result;
    CALL_2(BOXED_WINE_NOTIFY_ICON, msg, data);
    return result;
}