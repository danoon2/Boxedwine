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

#ifndef __WND_H__
#define __WND_H__

#include "platform.h"
#include "pixelformat.h"

class wRECT {
public:
    wRECT() : left(0), top(0), right(0), bottom(0) {}
    S32 left;
    S32 top;
    S32 right;
    S32 bottom;
};

class Wnd {
public:
    Wnd() : surface(0), pixelFormat(NULL), pixelFormatIndex(0), openGlContext(NULL), activated(0), processId(0), hwnd(0)
#ifdef SDL2
        , sdlTexture(NULL), sdlTextureHeight(0), sdlTextureWidth(0)
#else
        , sdlSurface(NULL)
#endif
    {}

    U32 surface;
    wRECT windowRect;
    wRECT clientRect;
    std::string text;
    PixelFormat* pixelFormat;
    U32 pixelFormatIndex;
    void* openGlContext;
    U32 activated;
    U32 processId;
    U32 hwnd;
#ifdef SDL2
    void* sdlTexture;
    int sdlTextureHeight;
    int sdlTextureWidth;
#else
    void* sdlSurface;
#endif
};

void writeRect(KThread* thread, U32 address, wRECT* rect);
void readRect(KThread* thread, U32 address, wRECT* rect);

Wnd* getWnd(U32 hwnd);
Wnd* wndCreate(KThread* thread, U32 processId, U32 hwnd, U32 windowRect, U32 clientRect);
void wndDestroy(U32 hwnd);
void setWndText(Wnd* wnd, const char* text);
void updateScreen();

#endif
