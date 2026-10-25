/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
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

#ifndef __SERVER_ENCODING_H__
#define __SERVER_ENCODING_H__

#include "localserver.h"

#ifdef BOXEDWINE_LOCAL_SERVER

#include <string>
#include <vector>

// Small encoders the local server needs, kept free of emulator state so the
// tests can check them against known vectors.
namespace ServerEncoding {
    // 20 byte SHA-1 digest, used only for the WebSocket handshake.
    std::vector<U8> sha1(const U8* data, size_t len);
    std::string base64(const U8* data, size_t len);
    // Sec-WebSocket-Accept value for a client's Sec-WebSocket-Key (RFC 6455 4.2.2).
    std::string webSocketAccept(const std::string& key);

    // PNG of a 32-bit frame whose pixels are 0x00RRGGBB in host U32 order.
    // pitch is in bytes. The alpha byte is ignored and the PNG has no alpha.
    std::vector<U8> encodePng(const U8* pixels, U32 width, U32 height, U32 pitch);

    // 64-bit FNV-1a over the pixels of a frame, row by row, skipping pitch padding.
    U64 hashFrame(const U8* pixels, U32 width, U32 height, U32 pitch);
    std::string hex64(U64 value);
}

#endif

#endif
