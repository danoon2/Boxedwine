/*
 *  Copyright (C) 2016  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"

#include "knativewindow.h"
#include "knativesystem.h"

void notImplemented(const char* s) {
    kwarn(s);
}

#define ARG1 cpu->peek32(1)
#define ARG2 cpu->peek32(2)
#define ARG3 cpu->peek32(3)
#define ARG4 cpu->peek32(4)
#define ARG5 cpu->peek32(5)
#define ARG6 cpu->peek32(6)
#define ARG7 cpu->peek32(7)
#define ARG8 cpu->peek32(8)
#define ARG9 cpu->peek32(9)


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

# define __MSABI_LONG(x)         x

#define WS_OVERLAPPED          __MSABI_LONG(0x00000000)
#define WS_POPUP               __MSABI_LONG(0x80000000)
#define WS_CHILD               __MSABI_LONG(0x40000000)
#define WS_MINIMIZE            __MSABI_LONG(0x20000000)
#define WS_VISIBLE             __MSABI_LONG(0x10000000)
#define WS_DISABLED            __MSABI_LONG(0x08000000)
#define WS_CLIPSIBLINGS        __MSABI_LONG(0x04000000)
#define WS_CLIPCHILDREN        __MSABI_LONG(0x02000000)
#define WS_MAXIMIZE            __MSABI_LONG(0x01000000)
#define WS_BORDER              __MSABI_LONG(0x00800000)
#define WS_DLGFRAME            __MSABI_LONG(0x00400000)
#define WS_VSCROLL             __MSABI_LONG(0x00200000)
#define WS_HSCROLL             __MSABI_LONG(0x00100000)
#define WS_SYSMENU             __MSABI_LONG(0x00080000)
#define WS_THICKFRAME          __MSABI_LONG(0x00040000)
#define WS_GROUP               __MSABI_LONG(0x00020000)
#define WS_TABSTOP             __MSABI_LONG(0x00010000)
#define WS_MINIMIZEBOX         __MSABI_LONG(0x00020000)
#define WS_MAXIMIZEBOX         __MSABI_LONG(0x00010000)
#define WS_CAPTION             (WS_BORDER | WS_DLGFRAME)
#define WS_TILED               WS_OVERLAPPED
#define WS_ICONIC              WS_MINIMIZE
#define WS_SIZEBOX             WS_THICKFRAME
#define WS_OVERLAPPEDWINDOW    (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME| WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
#define WS_POPUPWINDOW         (WS_POPUP | WS_BORDER | WS_SYSMENU)
#define WS_CHILDWINDOW         WS_CHILD
#define WS_TILEDWINDOW         WS_OVERLAPPEDWINDOW


/*** Window extended styles ***/
#define WS_EX_DLGMODALFRAME    __MSABI_LONG(0x00000001)
#define WS_EX_DRAGDETECT       __MSABI_LONG(0x00000002) /* Undocumented */
#define WS_EX_NOPARENTNOTIFY   __MSABI_LONG(0x00000004)
#define WS_EX_TOPMOST          __MSABI_LONG(0x00000008)
#define WS_EX_ACCEPTFILES      __MSABI_LONG(0x00000010)
#define WS_EX_TRANSPARENT      __MSABI_LONG(0x00000020)
#define WS_EX_MDICHILD         __MSABI_LONG(0x00000040)
#define WS_EX_TOOLWINDOW       __MSABI_LONG(0x00000080)
#define WS_EX_WINDOWEDGE       __MSABI_LONG(0x00000100)
#define WS_EX_CLIENTEDGE       __MSABI_LONG(0x00000200)
#define WS_EX_CONTEXTHELP      __MSABI_LONG(0x00000400)
#define WS_EX_RIGHT            __MSABI_LONG(0x00001000)
#define WS_EX_LEFT             __MSABI_LONG(0x00000000)
#define WS_EX_RTLREADING       __MSABI_LONG(0x00002000)
#define WS_EX_LTRREADING       __MSABI_LONG(0x00000000)
#define WS_EX_LEFTSCROLLBAR    __MSABI_LONG(0x00004000)
#define WS_EX_RIGHTSCROLLBAR   __MSABI_LONG(0x00000000)
#define WS_EX_CONTROLPARENT    __MSABI_LONG(0x00010000)
#define WS_EX_STATICEDGE       __MSABI_LONG(0x00020000)
#define WS_EX_APPWINDOW        __MSABI_LONG(0x00040000)
#define WS_EX_LAYERED          __MSABI_LONG(0x00080000)
#define WS_EX_NOINHERITLAYOUT  __MSABI_LONG(0x00100000)
#define WS_EX_LAYOUTRTL        __MSABI_LONG(0x00400000)
#define WS_EX_COMPOSITED       __MSABI_LONG(0x02000000)
#define WS_EX_NOACTIVATE       __MSABI_LONG(0x08000000)

#define WS_EX_OVERLAPPEDWINDOW (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)
#define WS_EX_PALETTEWINDOW    (WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST)

#define SWP_NOSIZE          0x0001
#define SWP_NOMOVE          0x0002
#define SWP_NOZORDER        0x0004
#define SWP_NOREDRAW        0x0008
#define SWP_NOACTIVATE      0x0010
#define SWP_FRAMECHANGED    0x0020  /* The frame changed: send WM_NCCALCSIZE */
#define SWP_SHOWWINDOW      0x0040
#define SWP_HIDEWINDOW      0x0080
#define SWP_NOCOPYBITS      0x0100
#define SWP_NOOWNERZORDER   0x0200  /* Don't do owner Z ordering */

#define SWP_DRAWFRAME       SWP_FRAMECHANGED
#define SWP_NOREPOSITION    SWP_NOOWNERZORDER

#define SWP_NOSENDCHANGING  0x0400
#define SWP_DEFERERASE      0x2000
#define SWP_ASYNCWINDOWPOS  0x4000

/* undocumented SWP flags - from SDK 3.1 */
#define SWP_NOCLIENTSIZE    0x0800
#define SWP_NOCLIENTMOVE    0x1000
#define SWP_STATECHANGED    0x8000

#define CF_UNICODETEXT 0xd
#define CF_TEXT 0x1

void boxeddrv_AcquireClipboard(CPU* cpu) {
    
}

// HKL CDECL drv_ActivateKeyboardLayout(HKL hkl, UINT flags)
void boxeddrv_ActivateKeyboardLayout(CPU* cpu) {
    notImplemented("boxeddrv_ActivateKeyboardLayout not implemented");
    EAX = ARG1;
}

// void CDECL drv_Beep(void)
void boxeddrv_Beep(CPU* cpu) {
    notImplemented("boxeddrv_Beep not implemented");
}

#define DISP_CHANGE_SUCCESSFUL 0
#define DISP_CHANGE_RESTART    1
#define DISP_CHANGE_FAILED     (-1)
#define DISP_CHANGE_BADMODE    (-2)
#define DISP_CHANGE_NOTUPDATED (-3)
#define DISP_CHANGE_BADFLAGS   (-4)
#define DISP_CHANGE_BADPARAM   (-5)
#define DISP_CHANGE_BADDUALVIEW (-6)

#define DM_ORIENTATION          __MSABI_LONG(0x00000001)
#define DM_PAPERSIZE            __MSABI_LONG(0x00000002)
#define DM_PAPERLENGTH          __MSABI_LONG(0x00000004)
#define DM_PAPERWIDTH           __MSABI_LONG(0x00000008)
#define DM_SCALE                __MSABI_LONG(0x00000010)
#define DM_POSITION             __MSABI_LONG(0x00000020)
#define DM_NUP                  __MSABI_LONG(0x00000040)
#define DM_DISPLAYORIENTATION   __MSABI_LONG(0x00000080)
#define DM_COPIES               __MSABI_LONG(0x00000100)
#define DM_DEFAULTSOURCE        __MSABI_LONG(0x00000200)
#define DM_PRINTQUALITY         __MSABI_LONG(0x00000400)
#define DM_COLOR                __MSABI_LONG(0x00000800)
#define DM_DUPLEX               __MSABI_LONG(0x00001000)
#define DM_YRESOLUTION          __MSABI_LONG(0x00002000)
#define DM_TTOPTION             __MSABI_LONG(0x00004000)
#define DM_COLLATE              __MSABI_LONG(0x00008000)
#define DM_FORMNAME             __MSABI_LONG(0x00010000)
#define DM_LOGPIXELS            __MSABI_LONG(0x00020000)
#define DM_BITSPERPEL           __MSABI_LONG(0x00040000)
#define DM_PELSWIDTH            __MSABI_LONG(0x00080000)
#define DM_PELSHEIGHT           __MSABI_LONG(0x00100000)
#define DM_DISPLAYFLAGS         __MSABI_LONG(0x00200000)
#define DM_DISPLAYFREQUENCY     __MSABI_LONG(0x00400000)
#define DM_ICMMETHOD            __MSABI_LONG(0x00800000)
#define DM_ICMINTENT            __MSABI_LONG(0x01000000)
#define DM_MEDIATYPE            __MSABI_LONG(0x02000000)
#define DM_DITHERTYPE           __MSABI_LONG(0x04000000)
#define DM_PANNINGWIDTH         __MSABI_LONG(0x08000000)
#define DM_PANNINGHEIGHT        __MSABI_LONG(0x10000000)
#define DM_DISPLAYFIXEDOUTPUT   __MSABI_LONG(0x20000000)

// LONG CDECL drv_ChangeDisplaySettingsEx(LPCWSTR devname, LPDEVMODEW devmode, HWND hwnd, DWORD flags, LPVOID lpvoid)
void boxeddrv_ChangeDisplaySettingsEx(CPU* cpu) {
    U32 devmode = ARG2;
    if (devmode)
    {
        U32 dmFields;
        U32 dmSize = readw( devmode + 68);
        U32 width = KNativeWindow::defaultScreenWidth;
        U32 height = KNativeWindow::defaultScreenHeight;
        U32 bpp = KNativeWindow::defaultScreenBpp;

        /* this is the minimal dmSize that XP accepts */
        if (dmSize < 44) {
            writed(ARG6, DISP_CHANGE_FAILED);	
            return;
        }

        dmFields = readd(devmode + 72);

        if (dmFields & DM_BITSPERPEL) {
            bpp = readd(devmode + 168);
            if (bpp < 8)
                bpp = 32; // let the dib driver handle it
        }
        if (dmFields & DM_PELSWIDTH) {
            width = readd(devmode + 172);
        }
        if (dmFields & DM_PELSHEIGHT) {
            height = readd(devmode + 176);
        }
        KNativeWindow::getNativeWindow()->screenChanged(cpu->thread, width, height, bpp);
    }	    
    writed(ARG6, DISP_CHANGE_SUCCESSFUL);	
    writed(ARG7, KNativeWindow::getNativeWindow()->screenWidth());
    writed(ARG8, KNativeWindow::getNativeWindow()->screenHeight());
    writed(ARG9, KNativeWindow::getNativeWindow()->screenBpp());
}

// BOOL CDECL drv_ClipCursor(LPCRECT clip)
void boxeddrv_ClipCursor(CPU* cpu) {
    notImplemented("boxeddrv_ClipCursor not implemented");
    EAX = 1;
}

// INT CDECL drv_CountClipboardFormats(void)
void boxeddrv_CountClipboardFormats(CPU* cpu) {
    if (KNativeSystem::clipboardHasText()) {
        EAX = 2; // CF_UNICODETEXT & CF_TEXT
    } else {
        EAX = 0;
    }
}

// BOOL CDECL drv_CreateDesktopWindow(HWND hwnd)
void boxeddrv_CreateDesktopWindow(CPU* cpu) {
    // setting up window pos was handled in driver
    EAX = 1;
}

// BOOL CDECL drv_CreateWindow(HWND hwnd)
void boxeddrv_CreateWindow(CPU* cpu) {
    EAX = 1;
}

// void CDECL drv_DestroyCursorIcon(HCURSOR cursor)
void boxeddrv_DestroyCursorIcon(CPU* cpu) {
    
}

// void CDECL drv_DestroyWindow(HWND hwnd)
void boxeddrv_DestroyWindow(CPU* cpu) {
    std::shared_ptr<Wnd> wnd = KNativeWindow::getNativeWindow()->getWnd(ARG1);
    if (wnd) {
        wnd->destroy();
    }
}

// void CDECL drv_EmptyClipboard(void)
void boxeddrv_EmptyClipboard(CPU* cpu) {
    KNativeSystem::clipboardSetText("");
}

//void CDECL drv_EndClipboardUpdate(void)
void boxeddrv_EndClipboardUpdate(CPU* cpu) {
    
}

// UINT CDECL drv_EnumClipboardFormats(UINT prev_format)
void boxeddrv_EnumClipboardFormats(CPU* cpu) {
    U32 prevFormat = ARG1;
    if (KNativeSystem::clipboardHasText()) {
        if (prevFormat == 0)
            EAX = CF_TEXT;
        else if (prevFormat == CF_TEXT)
            EAX = CF_UNICODETEXT;
        else
            EAX = 0;
    } else {
        EAX = 0;
    }
}

// BOOL CDECL drv_EnumDisplayMonitors(HDC hdc, LPRECT rect, MONITORENUMPROC proc, LPARAM lparam)
void boxeddrv_EnumDisplayMonitors(CPU* cpu) {
    // handled in driver
    notImplemented("drv_EnumDisplayMonitors not implemented");
    EAX = 0;
}

#define DMDFO_DEFAULT           0
#define DMDFO_STRETCH           1
#define DMDFO_CENTER            2

#define ENUM_CURRENT_SETTINGS  ((U32) -1)
#define ENUM_REGISTRY_SETTINGS ((U32) -2)

struct DisplayModes {
    int bpp;
    int cx;
    int cy;
};

// BOOL CDECL macdrv_EnumDisplaySettingsEx(LPCWSTR devname, DWORD mode, LPDEVMODEW devmode, DWORD flags)
void boxeddrv_EnumDisplaySettingsEx(CPU* cpu) {
    U32 devmode = ARG3;
    static const U16 dev_name[32] = { 'B','o','x','e','d','W','i','n','e',' ','d','r','i','v','e','r',0 };
    static DisplayModes* displayModes;
    static int displayModesCount;
    int i;

    if (!displayModesCount) {
        U32 desktopCx = 0;
        U32 desktopCy = 0;
        
        KNativeSystem::getScreenDimensions(&desktopCx, &desktopCy);
        displayModes = new DisplayModes[18];
        displayModes[displayModesCount].bpp = 32;
        displayModes[displayModesCount].cx = 1024;
        displayModes[displayModesCount++].cy = 768;
        displayModes[displayModesCount].bpp = 32;
        displayModes[displayModesCount].cx = 800;
        displayModes[displayModesCount++].cy = 600;
        displayModes[displayModesCount].bpp = 32;
        displayModes[displayModesCount].cx = 640;
        displayModes[displayModesCount++].cy = 480;

        displayModes[displayModesCount].bpp = 16;
        displayModes[displayModesCount].cx = 1024;
        displayModes[displayModesCount++].cy = 768;
        displayModes[displayModesCount].bpp = 16;
        displayModes[displayModesCount].cx = 800;
        displayModes[displayModesCount++].cy = 600;
        displayModes[displayModesCount].bpp = 16;
        displayModes[displayModesCount].cx = 640;
        displayModes[displayModesCount++].cy = 480;

        displayModes[displayModesCount].bpp = 8;
        displayModes[displayModesCount].cx = 1024;
        displayModes[displayModesCount++].cy = 768;
        displayModes[displayModesCount].bpp = 8;
        displayModes[displayModesCount].cx = 800;
        displayModes[displayModesCount++].cy = 600;
        displayModes[displayModesCount].bpp = 8;
        displayModes[displayModesCount].cx = 640;
        displayModes[displayModesCount++].cy = 480;

        if (desktopCx && desktopCx>1280 && desktopCy>1024) {
            displayModes[displayModesCount].bpp = 32;
            displayModes[displayModesCount].cx = 1280;
            displayModes[displayModesCount++].cy = 1024;
            displayModes[displayModesCount].bpp = 16;
            displayModes[displayModesCount].cx = 1280;
            displayModes[displayModesCount++].cy = 1024;
            displayModes[displayModesCount].bpp = 8;
            displayModes[displayModesCount].cx = 1280;
            displayModes[displayModesCount++].cy = 1024;
        }

        if (desktopCx && desktopCx>1600 && desktopCy>1200) {
            displayModes[displayModesCount].bpp = 32;
            displayModes[displayModesCount].cx = 1600;
            displayModes[displayModesCount++].cy = 1200;
            displayModes[displayModesCount].bpp = 16;
            displayModes[displayModesCount].cx = 1600;
            displayModes[displayModesCount++].cy = 1200;
            displayModes[displayModesCount].bpp = 8;
            displayModes[displayModesCount].cx = 1600;
            displayModes[displayModesCount++].cy = 1200;
        }

        if (desktopCx) {
            displayModes[displayModesCount].bpp = 32;
            displayModes[displayModesCount].cx = desktopCx;
            displayModes[displayModesCount++].cy = desktopCy;
            displayModes[displayModesCount].bpp = 16;
            displayModes[displayModesCount].cx = desktopCx;
            displayModes[displayModesCount++].cy = desktopCy;
            displayModes[displayModesCount].bpp = 8;
            displayModes[displayModesCount].cx = desktopCx;
            displayModes[displayModesCount++].cy = desktopCy;
        }
    }
    zeroMemory(devmode, 188);

    writew(devmode + 64, 0x401); // dmSpecVersion
    writew(devmode + 66, 0x401); // dmDriverVersion
    writew(devmode + 68, 188); // dmSize
    for (i=0;i<17;i++) {
        writew(devmode+i*2, dev_name[i]);
    }
    writed(devmode + 72, DM_POSITION | DM_DISPLAYORIENTATION | DM_DISPLAYFIXEDOUTPUT | DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFLAGS | DM_DISPLAYFREQUENCY);

    writed(devmode + 76, 0); // dmPosition.x
    writed(devmode + 80, 0); // dmPosition.y
    writed(devmode + 84, 0); // dmDisplayOrientation
    writed(devmode + 88, DMDFO_CENTER); // dmDisplayFixedOutput
    
    
    if (ARG2 == ENUM_REGISTRY_SETTINGS) {
        writed(devmode + 168, KNativeWindow::defaultScreenBpp);
        writed(devmode + 172, KNativeWindow::defaultScreenWidth);
        writed(devmode + 176, KNativeWindow::defaultScreenHeight);
    }
    else if (ARG2 == ENUM_CURRENT_SETTINGS) {
        writed(devmode + 168, KNativeWindow::getNativeWindow()->screenBpp());
        writed(devmode + 172, KNativeWindow::getNativeWindow()->screenWidth());
        writed(devmode + 176, KNativeWindow::getNativeWindow()->screenHeight());
    } else if (ARG2>=0 && ARG2<(U32)displayModesCount) {
        writed(devmode + 168, displayModes[ARG2].bpp);
        writed(devmode + 172, displayModes[ARG2].cx);
        writed(devmode + 176, displayModes[ARG2].cy);
    } else {
        EAX = 0;
        return;
    }
   
    writed(devmode + 180, 0); // dmDisplayFlags
    writed(devmode + 184, 60); // dmDisplayFrequency
    EAX = 1;
}

// int CDECL drv_GetClipboardData(UINT desired_format, char* buffer, int bufferLen)
void boxeddrv_GetClipboardData(CPU* cpu) {
    U32 format = ARG1;

    if ((format == CF_TEXT || format == CF_UNICODETEXT) && KNativeSystem::clipboardHasText()) {
        std::string text = KNativeSystem::clipboardGetText();
        int len = (int)text.length();
        if (format == CF_TEXT) {
            if (len+1>(int)ARG3)
                len = ARG3 - 1;
            memcopyFromNative(ARG2, text.c_str(), len+1);
            EAX = len+1;
        } else {
            writeNativeStringW(ARG2, text.c_str());
            EAX = 2*(len+1);
        }        
    } else {
        EAX = 0;
    }
}

// BOOL CDECL drv_GetCursorPos(LPPOINT pos)
void boxeddrv_GetCursorPos(CPU* cpu) {
    U32 pos = ARG1;
    int x = 0;
    int y = 0;

    if (KNativeWindow::getNativeWindow()->getMousePos(&x, &y)) {
        writed(pos, (U32)x);
        writed(pos + 4, (U32)y);
        EAX = 1;
    } else {
        EAX = 0;
    }
}

// HKL CDECL drv_GetKeyboardLayout(DWORD thread_id)
void boxeddrv_GetKeyboardLayout(CPU* cpu) {
    notImplemented("boxeddrv_GetKeyboardLayout not implemented");
    EAX = 0x1409; 
}

// BOOL CDECL drv_GetKeyboardLayoutName(LPWSTR name)
void boxeddrv_GetKeyboardLayoutName(CPU* cpu) {
    writeNativeString(ARG1, "Unknown");
    EAX = 1;
    notImplemented("boxeddrv_GetKeyboardLayoutName not implemented");
}

// INT CDECL drv_GetKeyNameText(LONG lparam, LPWSTR buffer, INT size)
void boxeddrv_GetKeyNameText(CPU* cpu) {
    notImplemented("boxeddrv_GetKeyNameText not implemented");
    EAX = 6;
    writeNativeString(ARG2, "bogus");
}

// BOOL CDECL drv_GetMonitorInfo(HMONITOR monitor, LPMONITORINFO info)
void boxeddrv_GetMonitorInfo(CPU* cpu) {
    notImplemented("boxeddrv_GetMonitorInfo not implemented");
    EAX = 0;
}

// BOOL CDECL drv_IsClipboardFormatAvailable(UINT desired_format)
void boxeddrv_IsClipboardFormatAvailable(CPU* cpu) {
    U32 format = ARG1;
    if ((format == CF_TEXT || format == CF_UNICODETEXT) && KNativeSystem::clipboardHasText()) {
        EAX = 1;
    } else {
        EAX = 0;
    }
}

static U8 vkToChar[] = {
    0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0x0,
    0x8, 0x9, 0x0, 0x0, 0x0, 0xD, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x1B, 0x0, 0x0, 0x0, 0x0,
    0x20, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x2A, 0x2B, 0x0, 0x2D, 0x2E, 0x2F,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x3B, 0x3D, 0x2C, 0x2D, 0x2E, 0x2F,
    0x60, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x5B, 0x5C, 0x5D, 0x27, 0x0,
    0x0, 0x0, 0x5C, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

static U32 virtualKeyToChar(U32 virtKey) {
    U32 c = 0;
    if (virtKey < sizeof(vkToChar)) {
        c = vkToChar[virtKey];
    }
    return c;
}

static U8 vkToScanCode[] = {
    0x0, 0x0, 0x0, 0x46, 0x0, 0x0, 0x0, 0x0,
    0xE, 0xF, 0x0, 0x0, 0x4C, 0x1C, 0x0, 0x0,
    0x2A, 0x1D, 0x38, 0x0, 0x3A, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0,
    0x39, 0x49, 0x51, 0x4F, 0x47, 0x4B, 0x48, 0x4D,
    0x50, 0x0, 0x0, 0x0, 0x54, 0x52, 0x53, 0x63,
    0xB, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,
    0x9, 0xA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22,
    0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
    0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11,
    0x2D, 0x15, 0x2C, 0x5B, 0x5C, 0x5D, 0x0, 0x5F,
    0x52, 0x4F, 0x50, 0x51, 0x4B, 0x4C, 0x4D, 0x47,
    0x48, 0x49, 0x37, 0x4E, 0x0, 0x4A, 0x53, 0x35,
    0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42,
    0x43, 0x44, 0x57, 0x58, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x76,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x45, 0x46, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x2A, 0x36, 0x1D, 0x1D, 0x38, 0x38, 0x6A, 0x69,
    0x67, 0x68, 0x65, 0x66, 0x32, 0x20, 0x2E, 0x30,
    0x19, 0x10, 0x24, 0x22, 0x6C, 0x6D, 0x6B, 0x21,
    0x0, 0x0, 0x27, 0xD, 0x33, 0xC, 0x34, 0x35,
    0x29, 0x73, 0x7E, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x1A, 0x2B, 0x1B, 0x28, 0x0,
    0x0, 0x0, 0x56, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x71, 0x5C, 0x7B, 0x0, 0x6F, 0x5A, 0x0,
    0x0, 0x5B, 0x0, 0x5F, 0x0, 0x5E, 0x0, 0x0,
    0x0, 0x5D, 0x0, 0x62, 0x0, 0x0, 0x0, 0x0
};

static U32 virtualKeyToScanCode(U32 virtKey) {
    U32 c = 0;
    if (virtKey < sizeof(vkToScanCode)) {
        c = vkToScanCode[virtKey];
    }
    return c;
}

static U16 vkToScanCodeEx[] = {
    0x0, 0x0, 0x0, 0xE046, 0x0, 0x0, 0x0, 0x0,
    0xE, 0xF, 0x0, 0x0, 0x4C, 0x1C, 0x0, 0x0,
    0x2A, 0x1D, 0x38, 0xE11D, 0x3A, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0,
    0x39, 0x49, 0x51, 0x4F, 0x47, 0x4B, 0x48, 0x4D,
    0x50, 0x0, 0x0, 0x0, 0x54, 0x52, 0x53, 0x63,
    0xB, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,
    0x9, 0xA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22,
    0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
    0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11,
    0x2D, 0x15, 0x2C, 0xE05B, 0xE05C, 0xE05D, 0x0, 0xE05F,
    0x52, 0x4F, 0x50, 0x51, 0x4B, 0x4C, 0x4D, 0x47,
    0x48, 0x49, 0x37, 0x4E, 0x0, 0x4A, 0x53, 0xE035,
    0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42,
    0x43, 0x44, 0x57, 0x58, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x76,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x45, 0x46, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x2A, 0x36, 0x1D, 0xE01D, 0x38, 0xE038, 0xE06A, 0xE069,
    0xE067, 0xE068, 0xE065, 0xE066, 0xE032, 0xE020, 0xE02E, 0xE030,
    0xE019, 0xE010, 0xE024, 0xE022, 0xE06C, 0xE06D, 0xE06B, 0xE021,
    0x0, 0x0, 0x27, 0xD, 0x33, 0xC, 0x34, 0x35,
    0x29, 0x73, 0x7E, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x1A, 0x2B, 0x1B, 0x28, 0x0,
    0x0, 0x0, 0x56, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x71, 0x5C, 0x7B, 0x0, 0x6F, 0x5A, 0x0,
    0x0, 0x5B, 0x0, 0x5F, 0x0, 0x5E, 0x0, 0x0,
    0x0, 0x5D, 0x0, 0x62, 0x0, 0x0, 0x0, 0x0
};

static U32 virtualKeyToScanCodeEx(U32 virtKey) {
    U32 c = 0;
    if (virtKey < sizeof(vkToScanCodeEx) / sizeof(vkToScanCodeEx[0])) {
        c = vkToScanCodeEx[virtKey];
    }
    return c;
}

static U8 scanCodeToVK[] = {
    0x0, 0x1B, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x30, 0xBD, 0xBB, 0x8, 0x9,
    0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49,
    0x4F, 0x50, 0xDB, 0xDD, 0xD, 0x11, 0x41, 0x53,
    0x44, 0x46, 0x47, 0x48, 0x4A, 0x4B, 0x4C, 0xBA,
    0xDE, 0xC0, 0x10, 0xDC, 0x5A, 0x58, 0x43, 0x56,
    0x42, 0x4E, 0x4D, 0xBC, 0xBE, 0xBF, 0x10, 0x6A,
    0x12, 0x20, 0x14, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x78, 0x79, 0x90, 0x91, 0x24,
    0x26, 0x21, 0x6D, 0x25, 0xC, 0x27, 0x6B, 0x23,
    0x28, 0x22, 0x2D, 0x2E, 0x2C, 0x0, 0xE2, 0x7A,
    0x7B, 0xC, 0xEE, 0xF1, 0xEA, 0xF9, 0xF5, 0xF3,
    0x0, 0x0, 0xFB, 0x2F, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0xED,
    0x0, 0xE9, 0x0, 0xC1, 0x0, 0x0, 0x87, 0x0,
    0x0, 0x0, 0x0, 0xEB, 0x9, 0x0, 0xC2, 0x0,
};

static U32 scanCodeToVirtualKey(U32 code) {
    U32 c = 0;
    if (code < sizeof(scanCodeToVK)) {
        c = scanCodeToVK[code];
    }
    return c;
}

static U8 scanCodeToVkEx[] = {
    0x0, 0x1B, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x30, 0xBD, 0xBB, 0x8, 0x9,
    0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49,
    0x4F, 0x50, 0xDB, 0xDD, 0xD, 0x11, 0x41, 0x53,
    0x44, 0x46, 0x47, 0x48, 0x4A, 0x4B, 0x4C, 0xBA,
    0xDE, 0xC0, 0x10, 0xDC, 0x5A, 0x58, 0x43, 0x56,
    0x42, 0x4E, 0x4D, 0xBC, 0xBE, 0xBF, 0x10, 0x6A,
    0x12, 0x20, 0x14, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x78, 0x79, 0x90, 0x91, 0x24,
    0x26, 0x21, 0x6D, 0x25, 0xC, 0x27, 0x6B, 0x23,
    0x28, 0x22, 0x2D, 0x2E, 0x2C, 0x0, 0xE2, 0x7A,
    0x7B, 0xC, 0xEE, 0xF1, 0xEA, 0xF9, 0xF5, 0xF3,
    0x0, 0x0, 0xFB, 0x2F, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0xED,
    0x0, 0xE9, 0x0, 0xC1, 0x0, 0x0, 0x87, 0x0,
    0x0, 0x0, 0x0, 0xEB, 0x9, 0x0, 0xC2, 0x0,
};

/*
static U32 scanCodeToVirtualKeyEx(U32 code) {
    U32 c = 0;
    if (code < sizeof(scanCodeToVkEx)) {
        c = scanCodeToVkEx[code];
    }
    return c;
}
*/

// UINT CDECL drv_MapVirtualKeyEx(UINT wCode, UINT wMapType, HKL hkl)
void boxeddrv_MapVirtualKeyEx(CPU* cpu) {
    U32 wCode = ARG1;
    U32 wMapType = ARG2;
    //U32 hkl = ARG3;

    if (wMapType == 2) { // MAPVK_VK_TO_CHAR
        EAX = virtualKeyToChar(wCode);
        return;
    }
    if (wMapType == 4) { // MAPVK_VK_TO_VSC_EX
        EAX = virtualKeyToScanCodeEx(wCode);
        return;
    }
    if (wMapType == 0) { // MAPVK_VK_TO_VSC
        EAX = virtualKeyToScanCode(wCode);
        return;
    }
    if (wMapType == 1) { // MAPVK_VSC_TO_VK
        EAX = scanCodeToVirtualKey(wCode);
        return;
    }
    if (wMapType == 3) { // MAPVK_VSC_TO_VK_EX
        EAX = scanCodeToVirtualKey(wCode);
        return;
    }
    klog("boxeddrv_MapVirtualKeyEx not implemented");
    EAX = 0;
}

// DWORD CDECL drv_MsgWaitForMultipleObjectsEx(DWORD count, const HANDLE *handles, DWORD timeout, DWORD mask, DWORD flags)
void boxeddrv_MsgWaitForMultipleObjectsEx(CPU* cpu) {
    // notImplemented("boxeddrv_MsgWaitForMultipleObjectsEx not implemented");
    // :TODO: ?
    EAX = 0;
}

// void CDECL drv_SetCapture(HWND hwnd, UINT flags)
void boxeddrv_SetCapture(CPU* cpu) {
    notImplemented("boxeddrv_SetCapture not implemented");
}

// BOOL CDECL drv_SetClipboardData(UINT format_id, char* data, int len, BOOL owner)
void boxeddrv_SetClipboardData(CPU* cpu) {
    U32 format = ARG1;
    char* text = 0;
    //int len = ARG3;
    char tmp[MAX_FILEPATH_LEN];

    if (format == CF_TEXT) {
        text = getNativeString(ARG2, tmp, sizeof(tmp));
    } else if (format == CF_UNICODETEXT) {
        text = getNativeStringW(ARG2, tmp, sizeof(tmp));
    }
    if (text) {
        if (KNativeSystem::clipboardSetText(text))
            EAX = 1;
        else
            EAX = 0;
    } else {
        EAX = 0;
    }
}

// void CDECL drv_SetCursor(HCURSOR cursor)
void boxeddrv_SetCursor(CPU* cpu) {
    U32 hCursor = ARG1;
    U32 wModuleName = ARG2;
    U32 wResName = ARG3;
    U32 resId = ARG4;
    U32 pFound = ARG5;
    char tmp[MAX_FILEPATH_LEN];
    char tmp2[MAX_FILEPATH_LEN];

    if (!hCursor && !wModuleName && !wResName && !resId) {
        KNativeWindow::getNativeWindow()->setCursor(NULL, NULL, 0);
        if (pFound) {
            writed(ARG5, 1);
        }
    } else if (KNativeWindow::getNativeWindow()->setCursor(getNativeStringW(wModuleName, tmp, sizeof(tmp)), getNativeStringW(wResName, tmp2, sizeof(tmp2)), resId)) {
        writed(ARG5, 1);
    } else {
        writed(ARG5, 0);    
    }
}

void boxeddrv_SetCursorBits(CPU* cpu) {
    //U32 hCursor = ARG1;
    U32 wModuleName = ARG2;
    U32 wResName = ARG3;
    U32 resId = ARG4;
    U32 bits = ARG5;
    U32 width = ARG6;
    S32 height = ARG7;
    U32 hotX = ARG8;
    U32 hotY = ARG9;
    int pitch = (width+31) / 32 *4;
    U8 data[64*64/8];
    U8 mask[64*64/8];
    char tmp[MAX_FILEPATH_LEN];
    char tmp2[MAX_FILEPATH_LEN];
    int size;

    if (height>0) {
        height/=2;
    } else {
        height=-height;
    }
    size = pitch*height;
    if (size>(int)sizeof(data)) {
        klog("boxeddrv_SetCursorBits too large of cursor\n");
        return;
    }
    memcopyToNative(bits, data, size);
    memcopyToNative(bits+size, mask, size);
    KNativeWindow::getNativeWindow()->createAndSetCursor(getNativeStringW(wModuleName, tmp, sizeof(tmp)), getNativeStringW(wResName, tmp2, sizeof(tmp2)), resId, data, mask, width, height, hotX, hotY);
}

// BOOL CDECL drv_SetCursorPos(INT x, INT y)
void boxeddrv_SetCursorPos(CPU* cpu) {
    KNativeWindow::getNativeWindow()->setMousePos((int)ARG1, (int)ARG2);
    EAX = 1;
}

// void CDECL drv_SetFocus(HWND hwnd, BOOL* canSetFocus)
void boxeddrv_SetFocus(CPU* cpu) {
    std::shared_ptr<Wnd> wnd = KNativeWindow::getNativeWindow()->getWnd(ARG1);
    if (wnd && wnd->setFocus()) {
        writed(ARG2, 1);
    }
}

// void CDECL drv_SetLayeredWindowAttributes(HWND hwnd, COLORREF key, BYTE alpha, DWORD flags)
void boxeddrv_SetLayeredWindowAttributes(CPU* cpu) {
    notImplemented("boxeddrv_SetLayeredWindowAttributes not implemented");
}

// void CDECL drv_SetParent(HWND hwnd, HWND parent, HWND old_parent)
void boxeddrv_SetParent(CPU* cpu) {
    notImplemented("boxeddrv_SetParent not implemented");
}

// void CDECL drv_SetWindowRgn(HWND hwnd, HRGN hrgn, BOOL redraw)
void boxeddrv_SetWindowRgn(CPU* cpu) {
    notImplemented("boxeddrv_SetWindowRgn not implemented");	
}

// void CDECL drv_SetWindowStyle(HWND hwnd, INT offset, STYLESTRUCT *style)
void boxeddrv_SetWindowStyle(CPU* cpu) {
    notImplemented("boxeddrv_SetWindowStyle not implemented");
}

// void CDECL drv_SetWindowText(HWND hwnd, LPCWSTR text)
void boxeddrv_SetWindowText(CPU* cpu) {
    std::shared_ptr<Wnd> wnd = KNativeWindow::getNativeWindow()->getWnd(ARG1);
    if (wnd) {
        char tmp[1024];
        wnd->setText(getNativeStringW(ARG2, tmp, sizeof(tmp)));
    }
}

// void CDECL drv_ShowWindow(HWND hwnd, INT cmd, RECT *rect, UINT swp, UINT* result)
void boxeddrv_ShowWindow(CPU* cpu) {
    U32 swp = ARG4;
    notImplemented("boxeddrv_ShowWindow not implemented");
    writed(ARG5, swp);
}

// LRESULT CDECL macdrv_SysCommand(HWND hwnd, WPARAM wparam, LPARAM lparam)
void boxeddrv_SysCommand(CPU* cpu) {
    EAX = -1;
}

#define SPI_GETSCREENSAVEACTIVE   16
#define SPI_SETSCREENSAVEACTIVE   17

// BOOL CDECL SystemParametersInfo(UINT action, UINT int_param, void *ptr_param, UINT flags)
void boxeddrv_SystemParametersInfo(CPU* cpu) {
    switch (ARG1)
    {
    case SPI_GETSCREENSAVEACTIVE:
        if (ARG3)
            writed(ARG3, false);
        EAX = 1;
        return;
    case SPI_SETSCREENSAVEACTIVE:
        break;
    }
    EAX = 0;
}

static U32 toUnicodeEx(KThread* thread, U32 virtKey, U32 scanCode, U32 lpKeyState, U32 bufW, U32 bufW_size, U32 flags, U32 hkl) {
    U32 ret = 0;
    U8 c = 0;
    U32 shift = readb(lpKeyState + BOXED_VK_SHIFT) & 0x80;
    U32 ctrl = readb(lpKeyState + BOXED_VK_CONTROL) & 0x80;

    if (!virtKey)
        goto done;

    /* UCKeyTranslate, below, terminates a dead-key sequence if passed a
       modifier key press.  We want it to effectively ignore modifier key
       presses.  I think that one isn't supposed to call it at all for modifier
       events (e.g. NSFlagsChanged or kEventRawKeyModifiersChanged), since they
       are different event types than key up/down events. */
    switch (virtKey)
    {
    case BOXED_VK_SHIFT:
    case BOXED_VK_CONTROL:
    case BOXED_VK_MENU:
    case BOXED_VK_CAPITAL:
    case BOXED_VK_LSHIFT:
    case BOXED_VK_RSHIFT:
    case BOXED_VK_LCONTROL:
    case BOXED_VK_RCONTROL:
    case BOXED_VK_LMENU:
    case BOXED_VK_RMENU:
        goto done;
    }

    /* There are a number of key combinations for which Windows does not
       produce characters, but Mac keyboard layouts may.  Eat them.  Do this
       here to avoid the expense of UCKeyTranslate() but also because these
       keys shouldn't terminate dead key sequences. */
    if ((BOXED_VK_PRIOR <= virtKey && virtKey <= BOXED_VK_HELP) || (BOXED_VK_F1 <= virtKey && virtKey <= BOXED_VK_F24))
        goto done;

    /* Shift + <non-digit keypad keys>. */
    if (shift && BOXED_VK_MULTIPLY <= virtKey && virtKey <= BOXED_VK_DIVIDE)
        goto done;

    if (ctrl)
    {
        /* Control-Tab, with or without other modifiers. */
        if (virtKey == BOXED_VK_TAB)
            goto done;

        /* Control-Shift-<key>, Control-Alt-<key>, and Control-Alt-Shift-<key>
           for these keys. */
        if (shift || (readb(lpKeyState + BOXED_VK_MENU)))
        {
            switch (virtKey)
            {
            case BOXED_VK_CANCEL:
            case BOXED_VK_BACK:
            case BOXED_VK_ESCAPE:
            case BOXED_VK_SPACE:
            case BOXED_VK_RETURN:
                goto done;
            }
        }
    }

    if (shift) {
        if (virtKey >= 'A' && virtKey <= 'Z') {
            c = virtKey;
        } else {
            switch (virtKey) {
            case '1': c = '!'; break;
            case '2': c = '@'; break;
            case '3': c = '#'; break;
            case '4': c = '$'; break;
            case '5': c = '%'; break;
            case '6': c = '^'; break;
            case '7': c = '&'; break;
            case '8': c = '*'; break;
            case '9': c = '('; break;
            case '0': c = ')'; break;
            case BOXED_VK_OEM_MINUS: c = '_'; break;
            case BOXED_VK_OEM_PLUS: c = '+'; break;
            case BOXED_VK_TAB: c = '\t'; break;
            case BOXED_VK_OEM_4: c = '{'; break;
            case BOXED_VK_OEM_6: c = '}'; break;
            case BOXED_VK_OEM_1: c = ':'; break;
            case BOXED_VK_OEM_7: c = '\"'; break;
            case BOXED_VK_OEM_3: c = '~'; break;
            case BOXED_VK_OEM_5: c = '|'; break;
            case BOXED_VK_OEM_COMMA: c = '<'; break;
            case BOXED_VK_OEM_PERIOD: c = '>'; break;
            case BOXED_VK_OEM_2: c = '?'; break;
            case BOXED_VK_SPACE: c = ' '; break;
            case BOXED_VK_RETURN: c = 13; break;
            case BOXED_VK_BACK: c = 8; break;
            case BOXED_VK_ADD: c = '+'; break;
            default:
                kwarn("Unhandled key: %d", virtKey);
                break;
            }
        }
    } else {
        if (virtKey >= '0' && virtKey <= '9') {
            c = virtKey;
        } else if (virtKey >= 'A' && virtKey <= 'Z') {
            c = virtKey - 'A' + 'a';
        } else {
            switch (virtKey) {
            case BOXED_VK_OEM_MINUS: c = '-'; break;
            case BOXED_VK_OEM_PLUS: c = '='; break;
            case BOXED_VK_TAB: c = '\t'; break;
            case BOXED_VK_OEM_4: c = '['; break;
            case BOXED_VK_OEM_6: c = ']'; break;
            case BOXED_VK_OEM_1: c = ';'; break;
            case BOXED_VK_OEM_7: c = '\''; break;
            case BOXED_VK_OEM_3: c = '`'; break;
            case BOXED_VK_OEM_5: c = '\\'; break;
            case BOXED_VK_OEM_COMMA: c = ','; break;
            case BOXED_VK_OEM_PERIOD: c = '.'; break;
            case BOXED_VK_OEM_2: c = '/'; break;
            case BOXED_VK_SPACE: c = ' '; break;
            case BOXED_VK_RETURN: c = 13; break;
            case BOXED_VK_BACK: c = 8; break;
            case BOXED_VK_ADD: c = '+'; break;
            default:
                kwarn("Unhandled key: %d", virtKey);
                break;
            }
        }
    }
    if (c && ctrl) {
        if (c == '@') {
            writew(bufW, 0);
            ret = 1;
        } else if (c >= 'a' && c <= 'z') {
            c = c - 'a' + 1;
        } else if (c >= 'A' && c <= 'Z') {
            c = c - 'A' + 1;
        } else if (c == '[') {
            c = 27;
        } else if (c == '\\') {
            c = 28;
        } else if (c == ']') {
            c = 29;
        } else if (c == '^') {
            c = 30;
        } else if (c == '_') {
            c = 31;
        }
    }

    if (c) {
        writew(bufW, c);
        ret = 1;
    }
done:
    /* Null-terminate the buffer, if there's room.  MSDN clearly states that the
       caller must not assume this is done, but some programs (e.g. Audiosurf) do. */
    if (1 <= ret && ret < bufW_size)
        writew(bufW + ret * 2, 0);

    return ret;
}

// INT CDECL drv_ToUnicodeEx(UINT virtKey, UINT scanCode, const BYTE *lpKeyState, LPWSTR bufW, int bufW_size, UINT flags, HKL hkl)
void boxeddrv_ToUnicodeEx(CPU* cpu) {
    EAX = toUnicodeEx(cpu->thread, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
}

// BOOL CDECL drv_UpdateLayeredWindow(HWND hwnd, const UPDATELAYEREDWINDOWINFO *info, const RECT *window_rect)
void boxeddrv_UpdateLayeredWindow(CPU* cpu) {
    notImplemented("boxeddrv_UpdateLayeredWindow not implemented");
    EAX = 0;
}

// SHORT CDECL drv_VkKeyScanEx(WCHAR wChar, HKL hkl)
void boxeddrv_VkKeyScanEx(CPU* cpu) {
    notImplemented("boxeddrv_VkKeyScanEx not implemented");
    EAX = 0;
}

// LRESULT CDECL drv_WindowMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
void boxeddrv_WindowMessage(CPU* cpu) {
    notImplemented("boxeddrv_WindowMessage not implemented");
    EAX = 0;
}

static void writeRect(U32 address, wRECT* rect) {
    if (address) {
        writed(address, rect->left);
        writed(address + 4, rect->top);
        writed(address + 8, rect->right);
        writed(address + 12, rect->bottom);
    }
}

// void CDECL drv_WindowPosChanged(HWND hwnd, HWND insert_after, UINT swp_flags, const RECT *window_rect, const RECT *client_rect, const RECT *visible_rect, const RECT *valid_rects, DWORD style)
void boxeddrv_WindowPosChanged(CPU* cpu) {
    std::shared_ptr<Wnd> wnd = KNativeWindow::getNativeWindow()->getWnd(ARG1);
    U32 style = ARG8;
    U32 swp_flags = ARG3;

    if (!wnd)
        return;
    writeRect(ARG4, &wnd->windowRect);
    writeRect(ARG5, &wnd->clientRect);
    //wnd->surface = 0;
    if ((swp_flags & SWP_HIDEWINDOW) && !(style & WS_VISIBLE)) {
        wnd->show(false);
    } else if (style & WS_VISIBLE) {
        wnd->show(true);
    }
}

// void boxeddrv_SetSurface(HWND wnd, struct window_surface *surface) {
void boxeddrv_SetSurface(CPU* cpu) {
    std::shared_ptr<Wnd> wnd = KNativeWindow::getNativeWindow()->getWnd(ARG1);
    if (wnd)
        wnd->surface = ARG2;
}

// struct window_surface* boxeddrv_GetSurface(HWND wnd)
void boxeddrv_GetSurface(CPU* cpu) {
    std::shared_ptr<Wnd> wnd = KNativeWindow::getNativeWindow()->getWnd(ARG1);
    if (wnd)
        EAX = wnd->surface;
    else
        EAX = 0;
}

// void CDECL drv_WindowPosChanging(HWND hwnd, HWND insert_after, UINT swp_flags, const RECT *window_rect, const RECT *client_rect, RECT *visible_rect, struct window_surface **surface)
void boxeddrv_WindowPosChanging(CPU* cpu) {
    std::shared_ptr<Wnd> wnd = KNativeWindow::getNativeWindow()->getWnd(ARG1);
    wRECT rect;

    if (!wnd) {
        wnd = KNativeWindow::getNativeWindow()->createWnd(cpu->thread, cpu->thread->process->id, ARG1, ARG4, ARG5);
    } else {
        wnd->windowRect.readRect(ARG4);
        wnd->clientRect.readRect(ARG5);
    }

    // *visible_rect = *window_rect;
    rect.readRect(ARG4);
    writeRect(ARG6, &rect);

    // *surface = wnd->surface;
    writed(ARG7, wnd->surface);
}

// BOOL drv_GetDeviceGammaRamp(PHYSDEV dev, LPVOID ramp)
void boxeddrv_GetDeviceGammaRamp(CPU* cpu) {
    EAX = KNativeWindow::getNativeWindow()->getGammaRamp(ARG2);
}

// BOOL drv_SetDeviceGammaRamp(PHYSDEV dev, LPVOID ramp)
void boxeddrv_SetDeviceGammaRamp(CPU* cpu) {
    //notImplemented("boxeddrv_SetDeviceGammaRamp not implemented");
    EAX = 0;
}

/* CURVECAPS */
#define CC_NONE           0x0000
#define CC_CIRCLES        0x0001
#define CC_PIE            0x0002
#define CC_CHORD         0x0004
#define CC_ELLIPSES       0x0008
#define CC_WIDE           0x0010
#define CC_STYLED         0x0020
#define CC_WIDESTYLED     0x0040
#define CC_INTERIORS      0x0080
#define CC_ROUNDRECT      0x0100

/* LINECAPS */
#define LC_NONE           0x0000
#define LC_POLYLINE       0x0002
#define LC_MARKER         0x0004
#define LC_POLYMARKER     0x0008
#define LC_WIDE           0x0010
#define LC_STYLED         0x0020
#define LC_WIDESTYLED     0x0040
#define LC_INTERIORS      0x0080

/* POLYGONALCAPS */
#define PC_NONE           0x0000
#define PC_POLYGON        0x0001
#define PC_RECTANGLE      0x0002
#define PC_WINDPOLYGON    0x0004
#define PC_TRAPEZOID      0x0004
#define PC_SCANLINE       0x0008
#define PC_WIDE           0x0010
#define PC_STYLED         0x0020
#define PC_WIDESTYLED     0x0040
#define PC_INTERIORS      0x0080
#define PC_POLYPOLYGON    0x0100
#define PC_PATHS          0x0200

/* TEXTCAPS */
#define TC_OP_CHARACTER   0x0001
#define TC_OP_STROKE      0x0002
#define TC_CP_STROKE      0x0004
#define TC_CR_90          0x0008
#define TC_CR_ANY         0x0010
#define TC_SF_X_YINDEP    0x0020
#define TC_SA_DOUBLE      0x0040
#define TC_SA_INTEGER     0x0080
#define TC_SA_CONTIN      0x0100
#define TC_EA_DOUBLE      0x0200
#define TC_IA_ABLE        0x0400
#define TC_UA_ABLE        0x0800
#define TC_SO_ABLE        0x1000
#define TC_RA_ABLE        0x2000
#define TC_VA_ABLE        0x4000
#define TC_RESERVED       0x8000
#define TC_SCROLLBLT      0x00010000

/* CLIPCAPS */
#define CP_NONE           0x0000
#define CP_RECTANGLE      0x0001
#define CP_REGION         0x0002

/* RASTERCAPS */
#define RC_NONE           0x0000
#define RC_BITBLT         0x0001
#define RC_BANDING        0x0002
#define RC_SCALING        0x0004
#define RC_BITMAP64       0x0008
#define RC_GDI20_OUTPUT   0x0010
#define RC_GDI20_STATE    0x0020
#define RC_SAVEBITMAP     0x0040
#define RC_DI_BITMAP      0x0080
#define RC_PALETTE        0x0100
#define RC_DIBTODEV       0x0200
#define RC_BIGFONT        0x0400
#define RC_STRETCHBLT     0x0800
#define RC_FLOODFILL      0x1000
#define RC_STRETCHDIB     0x2000
#define RC_OP_DX_OUTPUT   0x4000
#define RC_DEVBITS        0x8000

/* SHADEBLENDCAPS */
#define SB_NONE           0x0000
#define SB_CONST_ALPHA    0x0001
#define SB_PIXEL_ALPHA    0x0002
#define SB_PREMULT_ALPHA  0x0004
#define SB_GRAD_RECT      0x0010
#define SB_GRAD_TRI       0x0020

/* Device parameters for GetDeviceCaps() */
#define DRIVERVERSION     0
#define TECHNOLOGY        2
#define HORZSIZE          4
#define VERTSIZE          6
#define HORZRES           8
#define VERTRES           10
#define BITSPIXEL         12
#define PLANES            14
#define NUMBRUSHES        16
#define NUMPENS           18
#define NUMMARKERS        20
#define NUMFONTS          22
#define NUMCOLORS         24
#define PDEVICESIZE       26
#define CURVECAPS         28
#define LINECAPS          30
#define POLYGONALCAPS     32
#define TEXTCAPS          34
#define CLIPCAPS          36
#define RASTERCAPS        38
#define ASPECTX           40
#define ASPECTY           42
#define ASPECTXY          44
#define LOGPIXELSX        88
#define LOGPIXELSY        90
#define CAPS1             94
#define SIZEPALETTE       104
#define NUMRESERVED       106
#define COLORRES          108

#define PHYSICALWIDTH     110
#define PHYSICALHEIGHT    111
#define PHYSICALOFFSETX   112
#define PHYSICALOFFSETY   113
#define SCALINGFACTORX    114
#define SCALINGFACTORY    115
#define VREFRESH          116
#define DESKTOPVERTRES    117
#define DESKTOPHORZRES    118
#define BLTALIGNMENT      119
#define SHADEBLENDCAPS    120
#define COLORMGMTCAPS     121

// Args: PHYSDEV dev, INT cap
// return: INT
void boxeddrv_GetDeviceCaps(CPU* cpu) {
    S32 ret;

    switch (ARG2) {
    case DRIVERVERSION:
        ret = 0x300;
        break;
    case TECHNOLOGY:
        ret = 1; // DT_RASDISPLAY;
        break;
    case HORZSIZE:
        ret = 320; // 17 inch monitor?
        break;
    case VERTSIZE:
        ret = 240; // 17 inch monitor?
        break;
    case HORZRES:
        ret = KNativeWindow::getNativeWindow()->screenWidth();
        break;
    case VERTRES:
        ret = KNativeWindow::getNativeWindow()->screenHeight();
        break;
    case DESKTOPHORZRES:
        ret = KNativeWindow::getNativeWindow()->screenWidth();
        break;
    case DESKTOPVERTRES:
        ret = KNativeWindow::getNativeWindow()->screenHeight();
        break;
    case BITSPIXEL:
        ret = KNativeWindow::getNativeWindow()->screenBpp();
        break;
    case PLANES:
        ret = 1;
        break;
    case NUMBRUSHES:
        ret = -1;
        break;
    case NUMPENS:
        ret = -1;
        break;
    case NUMMARKERS:
        ret = 0;
        break;
    case NUMFONTS:
        ret = 0;
        break;
    case NUMCOLORS:
        /* MSDN: Number of entries in the device's color table, if the device has
        * a color depth of no more than 8 bits per pixel.For devices with greater
        * color depths, -1 is returned. */
        ret = (KNativeWindow::getNativeWindow()->screenBpp() > 8) ? -1 : (1 << KNativeWindow::getNativeWindow()->screenBpp());
        break;
    case CURVECAPS:
        ret = (CC_CIRCLES | CC_PIE | CC_CHORD | CC_ELLIPSES | CC_WIDE |
            CC_STYLED | CC_WIDESTYLED | CC_INTERIORS | CC_ROUNDRECT);
        break;
    case LINECAPS:
        ret = (LC_POLYLINE | LC_MARKER | LC_POLYMARKER | LC_WIDE |
            LC_STYLED | LC_WIDESTYLED | LC_INTERIORS);
        break;
    case POLYGONALCAPS:
        ret = (PC_POLYGON | PC_RECTANGLE | PC_WINDPOLYGON | PC_SCANLINE |
            PC_WIDE | PC_STYLED | PC_WIDESTYLED | PC_INTERIORS);
        break;
    case TEXTCAPS:
        ret = (TC_OP_CHARACTER | TC_OP_STROKE | TC_CP_STROKE |
            TC_CR_ANY | TC_SF_X_YINDEP | TC_SA_DOUBLE | TC_SA_INTEGER |
            TC_SA_CONTIN | TC_UA_ABLE | TC_SO_ABLE | TC_RA_ABLE | TC_VA_ABLE);
        break;
    case CLIPCAPS:
        ret = CP_REGION;
        break;
    case COLORRES:
        /* The observed correspondence between BITSPIXEL and COLORRES is:
        * BITSPIXEL: 8  -> COLORRES: 18
        * BITSPIXEL: 16 -> COLORRES: 16
        * BITSPIXEL: 24 -> COLORRES: 24
        * (note that screenBpp is never 24)
        * BITSPIXEL: 32 -> COLORRES: 24 */
        ret = (KNativeWindow::getNativeWindow()->screenBpp() <= 8) ? 18 : (KNativeWindow::getNativeWindow()->screenBpp() == 32) ? 24 : KNativeWindow::getNativeWindow()->screenBpp();
        break;
    case RASTERCAPS:
        ret = (RC_BITBLT | RC_BANDING | RC_SCALING | RC_BITMAP64 | RC_DI_BITMAP |
            RC_DIBTODEV | RC_BIGFONT | RC_STRETCHBLT | RC_STRETCHDIB | RC_DEVBITS |
            (KNativeWindow::getNativeWindow()->screenBpp() <= 8 ? RC_PALETTE : 0));
        break;
    case SHADEBLENDCAPS:
        ret = (SB_GRAD_RECT | SB_GRAD_TRI | SB_CONST_ALPHA | SB_PIXEL_ALPHA);
        break;
    case ASPECTX:
    case ASPECTY:
        ret = 36;
        break;
    case ASPECTXY:
        ret = 51;
        break;
    case LOGPIXELSX:
        ret = 96;
        break;
    case LOGPIXELSY:
        ret = 96;
        break;
    case CAPS1:
        kwarn("CAPS1 is unimplemented, will return 0\n");
        /* please see wingdi.h for the possible bit-flag values that need
        to be returned. */
        ret = 0;
        break;
    case SIZEPALETTE:
        ret = KNativeWindow::getNativeWindow()->screenBpp() <= 8 ? 1 << KNativeWindow::getNativeWindow()->screenBpp() : 0;
        break;
    case NUMRESERVED:
    case PHYSICALWIDTH:
    case PHYSICALHEIGHT:
    case PHYSICALOFFSETX:
    case PHYSICALOFFSETY:
    case SCALINGFACTORX:
    case SCALINGFACTORY:
    case VREFRESH:
    case BLTALIGNMENT:
        ret = 0;
        break;
    default:
        kwarn("unsupported capability %d %d %d %d, will return 0\n", ARG1, ARG2, ARG3, ARG4);
        ret = 0;
        break;
    }
    EAX = ret;
}

// int CDECL wine_notify_icon(DWORD msg, NOTIFYICONDATAW *data)
void wine_notify_icon(CPU* cpu) {
    notImplemented("wine_notify_icon not implemented");
    EAX = 0;
}

// BOOL WINAPI ImeConfigure(HKL hKL, HWND hWnd, DWORD dwMode, LPVOID lpData)
void boxeddrv_ImeConfigure(CPU* cpu) {
    notImplemented("boxeddrv_ImeConfigure not implemented");
    EAX = 0;
}

// DWORD WINAPI ImeConversionList(HIMC hIMC, LPCWSTR lpSource, LPCANDIDATELIST lpCandList, DWORD dwBufLen, UINT uFlag)
void boxeddrv_ImeConversionList(CPU* cpu) {
    notImplemented("boxeddrv_ImeConversionList not implemented");
    EAX = 0;
}

// BOOL WINAPI ImeDestroy(UINT uForce)
void boxeddrv_ImeDestroy(CPU* cpu) {
    notImplemented("boxeddrv_ImeDestroy not implemented");
    EAX = 1;
}

// UINT WINAPI ImeEnumRegisterWord(REGISTERWORDENUMPROCW lpfnEnumProc, LPCWSTR lpszReading, DWORD dwStyle, LPCWSTR lpszRegister, LPVOID lpData)
void boxeddrv_ImeEnumRegisterWord(CPU* cpu) {
    notImplemented("boxeddrv_ImeEnumRegisterWord not implemented");
    EAX = 0;
}

// LRESULT WINAPI ImeEscape(HIMC hIMC, UINT uSubFunc, LPVOID lpData)
void boxeddrv_ImeEscape(CPU* cpu) {
    notImplemented("boxeddrv_ImeEscape not implemented");
    EAX = 0;
}

// DWORD WINAPI ImeGetImeMenuItems(HIMC hIMC, DWORD dwFlags, DWORD dwType, LPIMEMENUITEMINFOW lpImeParentMenu, LPIMEMENUITEMINFOW lpImeMenu, DWORD dwSize)
void boxeddrv_ImeGetImeMenuItems(CPU* cpu) {
    notImplemented("boxeddrv_ImeGetImeMenuItems not implemented");
    EAX = 0;
}

// UINT WINAPI ImeGetRegisterWordStyle(UINT nItem, LPSTYLEBUFW lpStyleBuf)
void boxeddrv_ImeGetRegisterWordStyle(CPU* cpu) {
    notImplemented("boxeddrv_ImeGetRegisterWordStyle not implemented");
    EAX = 0;
}

// BOOL WINAPI ImeInquire(LPIMEINFO lpIMEInfo, LPWSTR lpszUIClass, LPCWSTR lpszOption)
void boxeddrv_ImeInquire(CPU* cpu) {
    notImplemented("boxeddrv_ImeInquire not implemented");
    EAX = 0;
}

// BOOL WINAPI ImeProcessKey(HIMC hIMC, UINT vKey, LPARAM lKeyData, const LPBYTE lpbKeyState)
void boxeddrv_ImeProcessKey(CPU* cpu) {
    notImplemented("boxeddrv_ImeProcessKey not implemented");
    EAX = 0;
}

// BOOL WINAPI ImeRegisterWord(LPCWSTR lpszReading, DWORD dwStyle, LPCWSTR lpszRegister)
void boxeddrv_ImeRegisterWord(CPU* cpu) {
    notImplemented("boxeddrv_ImeRegisterWord not implemented");
    EAX = 0;
}

// BOOL WINAPI ImeSelect(HIMC hIMC, BOOL fSelect)
void boxeddrv_ImeSelect(CPU* cpu) {
    notImplemented("boxeddrv_ImeSelect not implemented");
    EAX = 0;
}

// BOOL WINAPI ImeSetActiveContext(HIMC hIMC, BOOL fFlag)
void boxeddrv_ImeSetActiveContext(CPU* cpu) {
    notImplemented("boxeddrv_ImeSetActiveContext not implemented");
    EAX = 1;
}

// BOOL WINAPI ImeSetCompositionString(HIMC hIMC, DWORD dwIndex, LPCVOID lpComp, DWORD dwCompLen, LPCVOID lpRead, DWORD dwReadLen)
void boxeddrv_ImeSetCompositionString(CPU* cpu) {
    notImplemented("boxeddrv_ImeSetCompositionString not implemented");
    EAX = 0;
}

// UINT WINAPI ImeToAsciiEx(UINT uVKey, UINT uScanCode, const LPBYTE lpbKeyState, LPDWORD lpdwTransKey, UINT fuState, HIMC hIMC)
void boxeddrv_ImeToAsciiEx(CPU* cpu) {
    notImplemented("boxeddrv_ImeToAsciiEx not implemented");
    EAX = 0;
}

// BOOL WINAPI ImeUnregisterWord(LPCWSTR lpszReading, DWORD dwStyle, LPCWSTR lpszUnregister)
void boxeddrv_ImeUnregisterWord(CPU* cpu) {
    notImplemented("boxeddrv_ImeUnregisterWord not implemented");
    EAX = 0;
}

// BOOL WINAPI NotifyIME(HIMC hIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue)
void boxeddrv_NotifyIME(CPU* cpu) {
    notImplemented("boxeddrv_NotifyIME not implemented");
    EAX = 0;
}

void boxeddrv_wglCopyContext(CPU* cpu) {
    notImplemented("boxeddrv_wglCopyContext not implemented");
}

// HWND hwnd, int major, int minor, int profile, int flags
void boxeddrv_wglCreateContext(CPU* cpu) {
    std::shared_ptr<Wnd> wnd = KNativeWindow::getNativeWindow()->getWnd(ARG1);
    if (!wnd) {
        EAX = 0;
    } else {
        EAX = KNativeWindow::getNativeWindow()->glCreateContext(cpu->thread, wnd, ARG2, ARG3, ARG4, ARG5);
    }
}

void boxeddrv_wglDeleteContext(CPU* cpu) {
    KNativeWindow::getNativeWindow()->glDeleteContext(cpu->thread, ARG1);
}

// HDC hdc, int fmt, UINT size, PIXELFORMATDESCRIPTOR *descr
void boxeddrv_wglDescribePixelFormat(CPU* cpu) {
    EAX = KSystem::describePixelFormat(cpu->thread, ARG1, ARG2, ARG3, ARG4);
}

void boxeddrv_wglGetPixelFormat(CPU* cpu) {
    std::shared_ptr<Wnd> wnd = KNativeWindow::getNativeWindow()->getWnd(ARG1);
    if (wnd)
        EAX = wnd->glGetPixelFormat();
    else
        EAX = 0;
}

#if defined(BOXEDWINE_OPENGL_SDL) || defined(BOXEDWINE_OPENGL_ES)
std::unordered_map<std::string, void*> glFunctionMap;
void boxeddrv_wglGetProcAddress(CPU* cpu) {
    char tmp[1024];
    char* name = getNativeString(ARG1, tmp, sizeof(tmp));

    if (glFunctionMap.count(name))
        EAX = 1;
    else
        EAX = 0;
}
#else
void boxeddrv_wglGetProcAddress(CPU* cpu) {
    EAX = 0;
}
#endif

// HwND hwnd, void* context
void boxeddrv_wglMakeCurrent(CPU* cpu) {
    EAX = KNativeWindow::getNativeWindow()->glMakeCurrent(cpu->thread, ARG2);
}

// HWND hwnd, int fmt, const PIXELFORMATDESCRIPTOR *descr
void boxeddrv_wglSetPixelFormat(CPU* cpu) {
    std::shared_ptr<Wnd> wnd = KNativeWindow::getNativeWindow()->getWnd(ARG1);
    if (wnd) {
        EAX = wnd->glSetPixelFormat(ARG2);
    } else {
        EAX = 0;
    }
}

void boxeddrv_HasWnd(CPU* cpu) {
    if (KNativeWindow::getNativeWindow()->getWnd(ARG1)) {
        EAX = 1;
    } else {
        EAX = 0;
    }
}

void boxeddrv_GetVersion(CPU* cpu) {
    EAX = 3;
}

void boxeddrv_wglShareLists(CPU* cpu) {
    EAX = KNativeWindow::getNativeWindow()->glShareLists(cpu->thread, ARG1, ARG2);
}

void boxeddrv_wglSwapBuffers(CPU* cpu) {
    KNativeWindow::getNativeWindow()->glSwapBuffers(cpu->thread);
    EAX = 1;
}

void boxeddrv_GetKeyboardLayoutList(CPU* cpu) {
    
}

void boxeddrv_RegisterHotKey(CPU* cpu) {
    
}

void boxeddrv_UnregisterHotKey(CPU* cpu) {
    
}

// void boxeddrv_FlushSurface(HWND hwnd, void* bits, int xOrg, int yOrg, int width, int height, zOrder, RECT* rects, int rectCount)
void boxeddrv_FlushSurface(CPU* cpu) {
    U32 i;

    for (i=0;i<ARG9;i++) {
        KNativeWindow::getNativeWindow()->bltWnd(cpu->thread, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG8+16*i);
    }
    KNativeWindow::getNativeWindow()->drawAllWindows(cpu->thread, ARG7+4, readd(ARG7));
}

void boxeddrv_CreateDC(CPU* cpu) {
    //U32 physdev = ARG1;
}

void boxeddrv_GetSystemPalette(CPU* cpu) {
    klog("GetSystemPalette not implemented");
    EAX = 0;
}

void boxeddrv_GetNearestColor(CPU* cpu) {
    //klog("GetNearestColor not implemented");
    EAX = ARG1;
}

void boxeddrv_RealizePalette(CPU* cpu) {
    //klog("RealizePalette not implemented");
    EAX = 0;
}

void boxeddrv_RealizeDefaultPalette(CPU* cpu) {
    //klog("RealizeDefaultPalette not implemented");
    EAX = 0;
}

void boxeddrv_SetEventFD(CPU* cpu) {
    cpu->thread->process->eventQueueFD = ARG1;
}

void boxeddrv_CreateDesktop(CPU* cpu) {
    //horz_res = ARG1;
    //vert_res = ARG2;
    //default_horz_res = ARG1;
    //default_vert_res = ARG2;
    //displayChanged();
    EAX = 1;
}

Int99Callback* wine_callback;
U32 wine_callbackSize;

void initWine() {
	if (!wine_callback) {
		wine_callback = new Int99Callback[89];
		wine_callback[BOXED_ACQUIRE_CLIPBOARD] = boxeddrv_AcquireClipboard;
		wine_callback[BOXED_ACTIVATE_KEYBOARD_LAYOUT] = boxeddrv_ActivateKeyboardLayout;
		wine_callback[BOXED_BEEP] = boxeddrv_Beep;
		wine_callback[BOXED_CHANGE_DISPLAY_SETTINGS_EX] = boxeddrv_ChangeDisplaySettingsEx;
		wine_callback[BOXED_CLIP_CURSOR] = boxeddrv_ClipCursor;
		wine_callback[BOXED_COUNT_CLIPBOARD_FORMATS] = boxeddrv_CountClipboardFormats;
		wine_callback[BOXED_CREATE_DESKTOP_WINDOW] = boxeddrv_CreateDesktopWindow;
		wine_callback[BOXED_CREATE_WINDOW] = boxeddrv_CreateWindow;
		wine_callback[BOXED_DESTROY_CURSOR_ICON] = boxeddrv_DestroyCursorIcon;
		wine_callback[BOXED_DESTROY_WINDOW] = boxeddrv_DestroyWindow;
		wine_callback[BOXED_EMPTY_CLIPBOARD] = boxeddrv_EmptyClipboard;
		wine_callback[BOXED_END_CLIPBOARD_UPDATE] = boxeddrv_EndClipboardUpdate;
		wine_callback[BOXED_ENUM_CLIPBOARD_FORMATS] = boxeddrv_EnumClipboardFormats;
		wine_callback[BOXED_ENUM_DISPLAY_MONITORS] = boxeddrv_EnumDisplayMonitors;
		wine_callback[BOXED_ENUM_DISPLAY_SETTINGS_EX] = boxeddrv_EnumDisplaySettingsEx;
		wine_callback[BOXED_GET_CLIPBOARD_DATA] = boxeddrv_GetClipboardData;
		wine_callback[BOXED_GET_CURSOR_POS] = boxeddrv_GetCursorPos;
		wine_callback[BOXED_GET_KEYBOARD_LAYOUT] = boxeddrv_GetKeyboardLayout;
		wine_callback[BOXED_GET_KEYBOARD_LAYOUT_NAME] = boxeddrv_GetKeyboardLayoutName;
		wine_callback[BOXED_GET_KEY_NAME] = boxeddrv_GetKeyNameText;
		wine_callback[BOXED_GET_MONITOR_INFO] = boxeddrv_GetMonitorInfo;
		wine_callback[BOXED_IS_CLIPBOARD_FORMAT_AVAILABLE] = boxeddrv_IsClipboardFormatAvailable;
		wine_callback[BOXED_MAP_VIRTUAL_KEY_EX] = boxeddrv_MapVirtualKeyEx;
		wine_callback[BOXED_MSG_WAIT_FOR_MULTIPLE_OBJECTS_EX] = boxeddrv_MsgWaitForMultipleObjectsEx;
		wine_callback[BOXED_SET_CAPTURE] = boxeddrv_SetCapture;
		wine_callback[BOXED_SET_CLIPBOARD_DATA] = boxeddrv_SetClipboardData;
		wine_callback[BOXED_SET_CURSOR] = boxeddrv_SetCursor;
		wine_callback[BOXED_SET_CURSOR_POS] = boxeddrv_SetCursorPos;
		wine_callback[BOXED_SET_FOCUS] = boxeddrv_SetFocus;
		wine_callback[BOXED_SET_LAYERED_WINDOW_ATTRIBUTES] = boxeddrv_SetLayeredWindowAttributes;
		wine_callback[BOXED_SET_PARENT] = boxeddrv_SetParent;
		wine_callback[BOXED_SET_WINDOW_RGN] = boxeddrv_SetWindowRgn;
		wine_callback[BOXED_SET_WINDOW_STYLE] = boxeddrv_SetWindowStyle;
		wine_callback[BOXED_SET_WINDOW_TEXT] = boxeddrv_SetWindowText;
		wine_callback[BOXED_SHOW_WINDOW] = boxeddrv_ShowWindow;
		wine_callback[BOXED_SYS_COMMAND] = boxeddrv_SysCommand;
		wine_callback[BOXED_SYSTEM_PARAMETERS_INFO] = boxeddrv_SystemParametersInfo;
		wine_callback[BOXED_TO_UNICODE_EX] = boxeddrv_ToUnicodeEx;
		wine_callback[BOXED_UPDATE_LAYERED_WINDOW] = boxeddrv_UpdateLayeredWindow;
		wine_callback[BOXED_VK_KEY_SCAN_EX] = boxeddrv_VkKeyScanEx;
		wine_callback[BOXED_WINDOW_MESSAGE] = boxeddrv_WindowMessage;
		wine_callback[BOXED_WINDOW_POS_CHANGED] = boxeddrv_WindowPosChanged;
		wine_callback[BOXED_WINDOW_POS_CHANGING] = boxeddrv_WindowPosChanging;
		wine_callback[BOXED_GET_DEVICE_GAMMA_RAMP] = boxeddrv_GetDeviceGammaRamp;
		wine_callback[BOXED_SET_DEVICE_GAMMA_RAMP] = boxeddrv_SetDeviceGammaRamp;
		wine_callback[BOXED_GET_DEVICE_CAPS] = boxeddrv_GetDeviceCaps;
		wine_callback[BOXED_WINE_NOTIFY_ICON] = wine_notify_icon;
		wine_callback[BOXED_IME_CONFIGURE] = boxeddrv_ImeConfigure;
		wine_callback[BOXED_IME_CONVERSION_LIST] = boxeddrv_ImeConversionList;
		wine_callback[BOXED_IME_DESTROY] = boxeddrv_ImeDestroy;
		wine_callback[BOXED_IME_ENUM_REGISTER_WORD] = boxeddrv_ImeEnumRegisterWord;
		wine_callback[BOXED_IME_ESCAPE] = boxeddrv_ImeEscape;
		wine_callback[BOXED_IME_GET_IME_MENU_ITEMS] = boxeddrv_ImeGetImeMenuItems;
		wine_callback[BOXED_IME_GET_REGISTER_WORD_STYLE] = boxeddrv_ImeGetRegisterWordStyle;
		wine_callback[BOXED_IME_INQUIRE] = boxeddrv_ImeInquire;
		wine_callback[BOXED_IME_PROCESS_KEY] = boxeddrv_ImeProcessKey;
		wine_callback[BOXED_IME_REGISTER_WORD] = boxeddrv_ImeRegisterWord;
		wine_callback[BOXED_IME_SELECT] = boxeddrv_ImeSelect;
		wine_callback[BOXED_IME_SET_ACTIVE_CONTEXT] = boxeddrv_ImeSetActiveContext;
		wine_callback[BOXED_IME_SET_COMPOSITION_STRING] = boxeddrv_ImeSetCompositionString;
		wine_callback[BOXED_IME_TO_ASCII_EX] = boxeddrv_ImeToAsciiEx;
		wine_callback[BOXED_IME_UNREGISTER_WORD] = boxeddrv_ImeUnregisterWord;
		wine_callback[BOXED_NOTIFY_IME] = boxeddrv_NotifyIME;
		wine_callback[BOXED_GL_COPY_CONTEXT] = boxeddrv_wglCopyContext;
		wine_callback[BOXED_GL_CREATE_CONTEXT] = boxeddrv_wglCreateContext;
		wine_callback[BOXED_GL_DELETE_CONTEXT] = boxeddrv_wglDeleteContext;
		wine_callback[BOXED_GL_DESCRIBE_PIXEL_FORMAT] = boxeddrv_wglDescribePixelFormat;
		wine_callback[BOXED_GL_GET_PIXEL_FORMAT] = boxeddrv_wglGetPixelFormat;
		wine_callback[BOXED_GL_GET_PROC_ADDRESS] = boxeddrv_wglGetProcAddress;
		wine_callback[BOXED_GL_MAKE_CURRENT] = boxeddrv_wglMakeCurrent;
		wine_callback[BOXED_GL_SET_PIXEL_FORMAT] = boxeddrv_wglSetPixelFormat;
		wine_callback[BOXED_GL_SHARE_LISTS] = boxeddrv_wglShareLists;
		wine_callback[BOXED_GL_SWAP_BUFFERS] = boxeddrv_wglSwapBuffers;
		wine_callback[BOXED_GET_KEYBOARD_LAYOUT_LIST] = boxeddrv_GetKeyboardLayoutList;
		wine_callback[BOXED_REGISTER_HOT_KEY] = boxeddrv_RegisterHotKey;
		wine_callback[BOXED_UNREGISTER_HOT_KEY] = boxeddrv_UnregisterHotKey;
		wine_callback[BOXED_SET_SURFACE] = boxeddrv_SetSurface;
		wine_callback[BOXED_GET_SURFACE] = boxeddrv_GetSurface;
		wine_callback[BOXED_FLUSH_SURFACE] = boxeddrv_FlushSurface;
		wine_callback[BOXED_CREATE_DC] = boxeddrv_CreateDC;
		wine_callback[BOXED_GET_SYSTEM_PALETTE] = boxeddrv_GetSystemPalette;
		wine_callback[BOXED_GET_NEAREST_COLOR] = boxeddrv_GetNearestColor;
		wine_callback[BOXED_REALIZE_PALETTE] = boxeddrv_RealizePalette;
		wine_callback[BOXED_REALIZE_DEFAULT_PALETTE] = boxeddrv_RealizeDefaultPalette;
		wine_callback[BOXED_SET_EVENT_FD] = boxeddrv_SetEventFD;
		wine_callback[BOXED_SET_CURSOR_BITS] = boxeddrv_SetCursorBits;
		wine_callback[BOXED_CREATE_DESKTOP] = boxeddrv_CreateDesktop;
		wine_callback[BOXED_HAS_WND] = boxeddrv_HasWnd;
		wine_callback[BOXED_GET_VERSION] = boxeddrv_GetVersion;
		wine_callbackSize = 89;
	}
}
