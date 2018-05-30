#ifndef __PIXELFORMAT_H__
#define __PIXELFORMAT_H__

#include "platformtypes.h"

#define K_PFD_DOUBLEBUFFER          0x00000001
#define K_PFD_STEREO                0x00000002
#define K_PFD_DRAW_TO_WINDOW        0x00000004
#define K_PFD_DRAW_TO_BITMAP        0x00000008
#define K_PFD_SUPPORT_GDI           0x00000010
#define K_PFD_SUPPORT_OPENGL        0x00000020
#define K_PFD_GENERIC_FORMAT        0x00000040
#define K_PFD_NEED_PALETTE          0x00000080
#define K_PFD_NEED_SYSTEM_PALETTE   0x00000100
#define K_PFD_SWAP_EXCHANGE         0x00000200
#define K_PFD_SWAP_COPY             0x00000400
#define K_PFD_SWAP_LAYER_BUFFERS    0x00000800
#define K_PFD_GENERIC_ACCELERATED   0x00001000
#define K_PFD_SUPPORT_COMPOSITION   0x00008000 /* Vista stuff */

#define K_PFD_DEPTH_DONTCARE        0x20000000
#define K_PFD_DOUBLEBUFFER_DONTCARE 0x40000000
#define K_PFD_STEREO_DONTCARE       0x80000000

#define K_PFD_TYPE_RGBA        0
#define K_PFD_TYPE_COLORINDEX  1

typedef struct tagPixelFormat {
    U16  nSize;
    U16  nVersion;
    U32 dwFlags;
    U8  iPixelType;
    U8  cColorBits;
    U8  cRedBits;
    U8  cRedShift;
    U8  cGreenBits;
    U8  cGreenShift;
    U8  cBlueBits;
    U8  cBlueShift;
    U8  cAlphaBits;
    U8  cAlphaShift;
    U8  cAccumBits;
    U8  cAccumRedBits;
    U8  cAccumGreenBits;
    U8  cAccumBlueBits;
    U8  cAccumAlphaBits;
    U8  cDepthBits;
    U8  cStencilBits;
    U8  cAuxBuffers;
    U8  iLayerType;
    U8  bReserved;
    U32 dwLayerMask;
    U32 dwVisibleMask;
    U32 dwDamageMask;
} PixelFormat;

int getPixelFormats(PixelFormat* pfd, int maxPfs);

#endif