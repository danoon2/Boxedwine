# GDI driver

@ cdecl wine_get_gdi_driver(long) boxeddrv_get_gdi_driver

# USER driver

@ cdecl ActivateKeyboardLayout(long long) boxeddrv_ActivateKeyboardLayout
@ cdecl Beep() boxeddrv_Beep
@ cdecl GetKeyNameText(long ptr long) boxeddrv_GetKeyNameText
@ cdecl GetKeyboardLayout(long) boxeddrv_GetKeyboardLayout
@ cdecl GetKeyboardLayoutName(ptr) boxeddrv_GetKeyboardLayoutName
@ cdecl LoadKeyboardLayout(wstr long) boxeddrv_LoadKeyboardLayout
@ cdecl MapVirtualKeyEx(long long long) boxeddrv_MapVirtualKeyEx
@ cdecl ToUnicodeEx(long long ptr ptr long long long) boxeddrv_ToUnicodeEx
@ cdecl UnloadKeyboardLayout(long) boxeddrv_UnloadKeyboardLayout
@ cdecl VkKeyScanEx(long long) boxeddrv_VkKeyScanEx
@ cdecl DestroyCursorIcon(long) boxeddrv_DestroyCursorIcon
@ cdecl SetCursor(long) boxeddrv_SetCursor
@ cdecl GetCursorPos(ptr) boxeddrv_GetCursorPos
@ cdecl SetCursorPos(long long) boxeddrv_SetCursorPos
@ cdecl ClipCursor(ptr) boxeddrv_ClipCursor
@ cdecl ChangeDisplaySettingsEx(ptr ptr long long long) boxeddrv_ChangeDisplaySettingsEx
@ cdecl EnumDisplayMonitors(long ptr ptr long) boxeddrv_EnumDisplayMonitors
@ cdecl EnumDisplaySettingsEx(ptr long ptr long) boxeddrv_EnumDisplaySettingsEx
@ cdecl GetMonitorInfo(long ptr) boxeddrv_GetMonitorInfo
@ cdecl CreateDesktopWindow(long) boxeddrv_CreateDesktopWindow
@ cdecl CreateWindow(long) boxeddrv_CreateWindow
@ cdecl DestroyWindow(long) boxeddrv_DestroyWindow
@ cdecl FlashWindowEx(ptr) boxeddrv_FlashWindowEx
@ cdecl GetDC(long long long ptr ptr long) boxeddrv_GetDC
@ cdecl MsgWaitForMultipleObjectsEx(long ptr long long long) boxeddrv_MsgWaitForMultipleObjectsEx
@ cdecl ReleaseDC(long long) boxeddrv_ReleaseDC
@ cdecl ScrollDC(long long long long) boxeddrv_ScrollDC
@ cdecl SetCapture(long long) boxeddrv_SetCapture
@ cdecl SetFocus(long) boxeddrv_SetFocus
@ cdecl SetLayeredWindowAttributes(long long long long) boxeddrv_SetLayeredWindowAttributes
@ cdecl SetParent(long long long) boxeddrv_SetParent
@ cdecl SetWindowIcon(long long long) boxeddrv_SetWindowIcon
@ cdecl SetWindowRgn(long long long) boxeddrv_SetWindowRgn
@ cdecl SetWindowStyle(ptr long ptr) boxeddrv_SetWindowStyle
@ cdecl SetWindowText(long wstr) boxeddrv_SetWindowText
@ cdecl ShowWindow(long long ptr long) boxeddrv_ShowWindow
@ cdecl SysCommand(long long long) boxeddrv_SysCommand
@ cdecl UpdateClipboard() boxeddrv_UpdateClipboard
@ cdecl UpdateLayeredWindow(long ptr ptr) boxeddrv_UpdateLayeredWindow
@ cdecl WindowMessage(long long long long) boxeddrv_WindowMessage
@ cdecl WindowPosChanging(long long long ptr ptr ptr ptr) boxeddrv_WindowPosChanging
@ cdecl WindowPosChanged(long long long ptr ptr ptr ptr ptr) boxeddrv_WindowPosChanged
@ cdecl SystemParametersInfo(long long ptr long) boxeddrv_SystemParametersInfo
@ cdecl ThreadDetach() boxeddrv_ThreadDetach

# Desktop
@ cdecl wine_create_desktop(long long) boxeddrv_create_desktop

# System tray
@ cdecl wine_notify_icon(long ptr)

#IME Interface
@ stdcall ImeInquire(ptr ptr wstr)
@ stdcall ImeConfigure(long long long ptr)
@ stdcall ImeDestroy(long)
@ stdcall ImeEscape(long long ptr)
@ stdcall ImeSelect(long long)
@ stdcall ImeSetActiveContext(long long)
@ stdcall ImeToAsciiEx(long long ptr ptr long long)
@ stdcall NotifyIME(long long long long)
@ stdcall ImeRegisterWord(wstr long wstr)
@ stdcall ImeUnregisterWord(wstr long wstr)
@ stdcall ImeEnumRegisterWord(ptr wstr long wstr ptr)
@ stdcall ImeSetCompositionString(long long ptr long ptr long)
@ stdcall ImeConversionList(long wstr ptr long long)
@ stdcall ImeProcessKey(long long long ptr)
@ stdcall ImeGetRegisterWordStyle(long ptr)
@ stdcall ImeGetImeMenuItems(long long long ptr ptr long)
