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

#ifndef __SDLWINDOW_H__
#define __SDLWINDOW_H__

#include "platform.h"
#include "wnd.h"

#ifdef BOXEDWINE_VM

struct SdlCallback {
    SDL_Event sdlEvent;
    SDL_mutex* mutex;
    SDL_cond* cond;
    struct KThread* thread;
    U32 iArg1;
    U32 iArg2;
    U32 iArg3;
    U32 iArg4;
    U32 iArg5;
    U32 iArg6;
    U32 iArg7;
    void* pArg1;
    void* pArg2;
    void* pArg3;
    void* pArg4;
    void (*func)(struct SdlCallback*);
    U32 result;
    struct SdlCallback* next;
};

#endif

int sdlMouseMouse(int x, int y);
int sdlMouseButton(U32 down, U32 button, int x, int y);
int sdlMouseWheel(int amount, int x, int y);
int sdlKey(U32 key, U32 down);
U32 sdlToUnicodeEx(KThread* thread, U32 virtKey, U32 scanCode, U32 lpKeyState, U32 bufW, U32 bufW_size, U32 flags, U32 hkl);
void sdlSwapBuffers(KThread* thread);
void sdlGetPalette(KThread* thread, U32 start, U32 count, U32 entries);
U32 sdlGetNearestColor(KThread* thread, U32 color);
U32 sdlRealizePalette(KThread* thread, U32 start, U32 numberOfEntries, U32 entries);
void sdlRealizeDefaultPalette();
U32 sdlSetCursor(KThread* thread, char* moduleName, char* resourceName, int resource);
void sdlCreateAndSetCursor(KThread* thread, char* moduleName, char* resourceName, int resource, U8* bits, U8* mask, int width, int height, int hotX, int hotY);
U32 sdlMakeCurrent(KThread* thread, U32 context);
U32 sdlCreateOpenglWindow(KThread* thread, Wnd* wnd, int major, int minor, int profile, int flags);
U32 sdlShareLists(KThread* thread, U32 srcContext, U32 destContext);
void sdlDeleteContext(KThread* thread, U32 context);
void sdlScreenResized(KThread* thread);
void wndBlt(KThread* thread, U32 hwnd, U32 bits, S32 xOrg, S32 yOrg, U32 width, U32 height, U32 rect);
void sdlDrawAllWindows(KThread* thread, U32 hWnd, int count);
void sdlShowWnd(KThread* thread, Wnd* wnd, U32 bShow);
U32 sdlGetGammaRamp(KThread* thread, U32 ramp);
unsigned int sdlGetMouseState(KThread* thread,int* x, int* y);
bool isBoxedWineDriverActive();
U32 sdlVirtualKeyToChar(U32 virtKey);
U32 sdlVirtualKeyToScanCode(U32 virtKey);
U32 sdlVirtualKeyToScanCodeEx(U32 virtKey);
U32 sdlScanCodeToVirtualKey(U32 code);
U32 sdlScanCodeToVirtualKeyEx(U32 code);
bool sdlScreenShot(std::string filepath, U32* crc);
bool sdlPartialScreenShot(std::string filepath, U32 x, U32 y, U32 w, U32 h, U32* crc);
void sdlPushWindowSurface();
void sdlPopWindowSurface();
void sdlDrawRectOnPushedSurfaceAndDisplay(U32 x, U32 y, U32 w, U32 h, U8 r, U8 g, U8 b, U8 a);
#endif
