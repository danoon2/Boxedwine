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

#include "boxedwine.h"
#include "encoding.h"

#ifdef BOXEDWINE_LOCAL_SERVER

#ifdef BOXEDWINE_ZLIB
// Boxedwine has its own OF macro, see fszip.h.
#undef OF
extern "C" {
#include <zlib.h>
}
#undef OF
#endif

namespace ServerEncoding {

static U32 rol(U32 value, U32 bits) {
    return (value << bits) | (value >> (32 - bits));
}

std::vector<U8> sha1(const U8* data, size_t len) {
    U32 h[5] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0 };
    std::vector<U8> message(data, data + len);
    U64 bitLen = (U64)len * 8;
    message.push_back(0x80);
    while (message.size() % 64 != 56) {
        message.push_back(0);
    }
    for (int i = 7; i >= 0; i--) {
        message.push_back((U8)(bitLen >> (i * 8)));
    }
    for (size_t chunk = 0; chunk < message.size(); chunk += 64) {
        U32 w[80];
        for (int i = 0; i < 16; i++) {
            const U8* p = &message[chunk + i * 4];
            w[i] = ((U32)p[0] << 24) | ((U32)p[1] << 16) | ((U32)p[2] << 8) | p[3];
        }
        for (int i = 16; i < 80; i++) {
            w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
        }
        U32 a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
        for (int i = 0; i < 80; i++) {
            U32 f, k;
            if (i < 20) {
                f = (b & c) | (~b & d);
                k = 0x5A827999;
            } else if (i < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            U32 temp = rol(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = rol(b, 30);
            b = a;
            a = temp;
        }
        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
    }
    std::vector<U8> result;
    for (int i = 0; i < 5; i++) {
        result.push_back((U8)(h[i] >> 24));
        result.push_back((U8)(h[i] >> 16));
        result.push_back((U8)(h[i] >> 8));
        result.push_back((U8)h[i]);
    }
    return result;
}

std::string base64(const U8* data, size_t len) {
    static const char* table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    result.reserve((len + 2) / 3 * 4);
    for (size_t i = 0; i < len; i += 3) {
        U32 n = (U32)data[i] << 16;
        if (i + 1 < len) n |= (U32)data[i + 1] << 8;
        if (i + 2 < len) n |= data[i + 2];
        result.push_back(table[(n >> 18) & 63]);
        result.push_back(table[(n >> 12) & 63]);
        result.push_back(i + 1 < len ? table[(n >> 6) & 63] : '=');
        result.push_back(i + 2 < len ? table[n & 63] : '=');
    }
    return result;
}

std::string webSocketAccept(const std::string& key) {
    std::string text = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    std::vector<U8> digest = sha1((const U8*)text.data(), text.size());
    return base64(digest.data(), digest.size());
}

static void putU32BE(std::vector<U8>& out, U32 value) {
    out.push_back((U8)(value >> 24));
    out.push_back((U8)(value >> 16));
    out.push_back((U8)(value >> 8));
    out.push_back((U8)value);
}

static U32 crc32Png(const U8* data, size_t len, U32 crc = 0) {
    static U32 table[256];
    static std::once_flag once;
    std::call_once(once, []() {
        for (U32 n = 0; n < 256; n++) {
            U32 c = n;
            for (int k = 0; k < 8; k++) {
                c = (c & 1) ? 0xEDB88320 ^ (c >> 1) : c >> 1;
            }
            table[n] = c;
        }
    });
    crc = ~crc;
    for (size_t i = 0; i < len; i++) {
        crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }
    return ~crc;
}

static void putChunk(std::vector<U8>& out, const char* type, const U8* data, size_t len) {
    putU32BE(out, (U32)len);
    size_t start = out.size();
    out.insert(out.end(), type, type + 4);
    if (len) {
        out.insert(out.end(), data, data + len);
    }
    putU32BE(out, crc32Png(&out[start], out.size() - start));
}

#ifndef BOXEDWINE_ZLIB
static U32 adler32Png(const U8* data, size_t len) {
    U32 a = 1, b = 0;
    for (size_t i = 0; i < len; i++) {
        a = (a + data[i]) % 65521;
        b = (b + a) % 65521;
    }
    return (b << 16) | a;
}

// A zlib stream of uncompressed (stored) deflate blocks.
static std::vector<U8> storeZlib(const std::vector<U8>& raw) {
    std::vector<U8> out;
    out.push_back(0x78);
    out.push_back(0x01);
    size_t pos = 0;
    do {
        size_t blockLen = std::min<size_t>(raw.size() - pos, 65535);
        bool last = pos + blockLen == raw.size();
        out.push_back(last ? 1 : 0);
        out.push_back((U8)blockLen);
        out.push_back((U8)(blockLen >> 8));
        out.push_back((U8)~blockLen);
        out.push_back((U8)(~blockLen >> 8));
        out.insert(out.end(), raw.begin() + pos, raw.begin() + pos + blockLen);
        pos += blockLen;
    } while (pos < raw.size());
    putU32BE(out, adler32Png(raw.data(), raw.size()));
    return out;
}
#endif

std::vector<U8> encodePng(const U8* pixels, U32 width, U32 height, U32 pitch) {
    // Each row: filter byte 0 (None), then RGB triples.
    std::vector<U8> raw;
    raw.resize((size_t)height * (1 + (size_t)width * 3));
    U8* dst = raw.data();
    for (U32 y = 0; y < height; y++) {
        const U32* src = (const U32*)(pixels + (size_t)y * pitch);
        *dst++ = 0;
        for (U32 x = 0; x < width; x++) {
            U32 p = src[x];
            *dst++ = (U8)(p >> 16);
            *dst++ = (U8)(p >> 8);
            *dst++ = (U8)p;
        }
    }

    std::vector<U8> compressed;
#ifdef BOXEDWINE_ZLIB
    uLongf compressedLen = compressBound((uLong)raw.size());
    compressed.resize(compressedLen);
    if (compress2(compressed.data(), &compressedLen, raw.data(), (uLong)raw.size(), 1) != Z_OK) {
        return std::vector<U8>();
    }
    compressed.resize(compressedLen);
#else
    compressed = storeZlib(raw);
#endif

    std::vector<U8> out;
    static const U8 signature[8] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n' };
    out.insert(out.end(), signature, signature + 8);
    std::vector<U8> header;
    putU32BE(header, width);
    putU32BE(header, height);
    header.push_back(8); // bit depth
    header.push_back(2); // color type: RGB
    header.push_back(0); // compression
    header.push_back(0); // filter
    header.push_back(0); // no interlace
    putChunk(out, "IHDR", header.data(), header.size());
    putChunk(out, "IDAT", compressed.data(), compressed.size());
    putChunk(out, "IEND", nullptr, 0);
    return out;
}

U64 hashFrame(const U8* pixels, U32 width, U32 height, U32 pitch) {
    U64 hash = 0xcbf29ce484222325ull;
    hash = (hash ^ width) * 0x100000001b3ull;
    hash = (hash ^ height) * 0x100000001b3ull;
    for (U32 y = 0; y < height; y++) {
        const U32* row = (const U32*)(pixels + (size_t)y * pitch);
        for (U32 x = 0; x < width; x++) {
            hash = (hash ^ (row[x] & 0xFFFFFF)) * 0x100000001b3ull;
        }
    }
    return hash;
}

std::string hex64(U64 value) {
    char buffer[17];
    snprintf(buffer, sizeof(buffer), "%016llx", (unsigned long long)value);
    return buffer;
}

}

#endif
