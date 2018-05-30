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

#include <SDL.h>
#include "wnd.h"

static int modesInitialized;

PixelFormat pfs[512];
U32 numberOfPfs;

void initDisplayModes() {
    if (!modesInitialized) {
        modesInitialized = 1;
        numberOfPfs = getPixelFormats(pfs, sizeof(pfs)/sizeof(PixelFormat));            
    }
}

U32 sdl_wglDescribePixelFormat(KThread* thread, U32 hdc, U32 fmt, U32 size, U32 descr)
{
    initDisplayModes();

    if (!descr) return numberOfPfs;
    if (size < 40) return 0;
    if (fmt>numberOfPfs) {
        return 0;
    }

    writew(descr, pfs[fmt].nSize); descr+=2;
    writew(descr, pfs[fmt].nVersion); descr+=2;
    writed(descr, pfs[fmt].dwFlags); descr+=4;
    writeb(descr, pfs[fmt].iPixelType); descr++;
    writeb(descr, pfs[fmt].cColorBits); descr++;
    writeb(descr, pfs[fmt].cRedBits); descr++;
    writeb(descr, pfs[fmt].cRedShift); descr++;
    writeb(descr, pfs[fmt].cGreenBits); descr++;
    writeb(descr, pfs[fmt].cGreenShift); descr++;
    writeb(descr, pfs[fmt].cBlueBits); descr++;
    writeb(descr, pfs[fmt].cBlueShift); descr++;
    writeb(descr, pfs[fmt].cAlphaBits); descr++;
    writeb(descr, pfs[fmt].cAlphaShift); descr++;
    writeb(descr, pfs[fmt].cAccumBits); descr++;
    writeb(descr, pfs[fmt].cAccumRedBits); descr++;
    writeb(descr, pfs[fmt].cAccumGreenBits); descr++;
    writeb(descr, pfs[fmt].cAccumBlueBits); descr++;
    writeb(descr, pfs[fmt].cAccumAlphaBits); descr++;
    writeb(descr, pfs[fmt].cDepthBits); descr++;
    writeb(descr, pfs[fmt].cStencilBits); descr++;
    writeb(descr, pfs[fmt].cAuxBuffers); descr++;
    writeb(descr, pfs[fmt].iLayerType); descr++;
    writeb(descr, pfs[fmt].bReserved); descr++;
    writed(descr, pfs[fmt].dwLayerMask); descr+=4;
    writed(descr, pfs[fmt].dwVisibleMask); descr+=4;
    writed(descr, pfs[fmt].dwDamageMask);

    return numberOfPfs;
}

void writePixelFormat(KThread* thread, PixelFormat* pf, U32 descr) {
    pf->nSize = readw(descr); descr+=2;
    pf->nVersion = readw(descr); descr+=2;
    pf->dwFlags = readd(descr); descr+=4;
    pf->iPixelType = readb(descr); descr++;
    pf->cColorBits = readb(descr); descr++;
    pf->cRedBits = readb(descr); descr++;
    pf->cRedShift = readb(descr); descr++;
    pf->cGreenBits = readb(descr); descr++;
    pf->cGreenShift = readb(descr); descr++;
    pf->cBlueBits = readb(descr); descr++;
    pf->cBlueShift = readb(descr); descr++;
    pf->cAlphaBits = readb(descr); descr++;
    pf->cAlphaShift = readb(descr); descr++;
    pf->cAccumBits = readb(descr); descr++;
    pf->cAccumRedBits = readb(descr); descr++;
    pf->cAccumGreenBits = readb(descr); descr++;
    pf->cAccumBlueBits = readb(descr); descr++;
    pf->cAccumAlphaBits = readb(descr); descr++;
    pf->cDepthBits = readb(descr); descr++;
    pf->cStencilBits = readb(descr); descr++;
    pf->cAuxBuffers = readb(descr); descr++;
    pf->iLayerType = readb(descr); descr++;
    pf->bReserved = readb(descr); descr++;
    pf->dwLayerMask = readd(descr); descr+=4;
    pf->dwVisibleMask = readd(descr); descr+=4;
    pf->dwDamageMask = readd(descr);
}