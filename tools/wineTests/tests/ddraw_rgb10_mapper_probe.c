/* Compiled with the selected Wine source's actual format conversion functions.
 * buildDDrawRGB10MapperProbe.py generates ddraw_format_mappers.inc.
 * No graphics device or driver is needed to check either mapping direction.
 */
#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include <string.h>

#define WINEMAKEFOURCC MAKEFOURCC
#define TRACE(...) do {} while (0)
#define WARN(...) do {} while (0)
#define FIXME(...) do {} while (0)
#define ERR(...) do {} while (0)
#define TRACE_ON(...) 0
#define DDRAW_dump_pixelformat(...) do {} while (0)
#include "ddraw_format_mappers.inc"

static unsigned checks, failures;
#define CHECK(c) do {++checks; if (!(c)) {++failures; printf("line %u: %s failed\n", __LINE__, #c);}} while (0)

int main(void)
{
    DDPIXELFORMAT format = {0};
    format.dwSize = sizeof(format);
    ddrawformat_from_wined3dformat(&format, WINED3DFMT_B10G10R10A2_UNORM);
    CHECK(format.dwSize == sizeof(format));
    CHECK(format.dwFlags == (DDPF_RGB | DDPF_ALPHAPIXELS));
    CHECK(format.dwRGBBitCount == 32);
    CHECK(format.dwRBitMask == 0x3ff00000);
    CHECK(format.dwGBitMask == 0x000ffc00);
    CHECK(format.dwBBitMask == 0x000003ff);
    CHECK(format.dwRGBAlphaBitMask == 0xc0000000);
    /* Use independent masks for the reverse check: a round trip alone would
     * accept the old pair of mutually consistent but incorrect mappings. */
    format.dwRBitMask = 0x3ff00000;
    format.dwGBitMask = 0x000ffc00;
    format.dwBBitMask = 0x000003ff;
    format.dwRGBAlphaBitMask = 0xc0000000;
    CHECK(wined3dformat_from_ddrawformat(&format) == WINED3DFMT_B10G10R10A2_UNORM);
    format.dwRBitMask = 0xc0000000;
    format.dwGBitMask = 0x3ff00000;
    format.dwBBitMask = 0x000ffc00;
    format.dwRGBAlphaBitMask = 0x000003ff;
    CHECK(wined3dformat_from_ddrawformat(&format) == WINED3DFMT_UNKNOWN);
    ddrawformat_from_wined3dformat(&format, WINED3DFMT_B8G8R8A8_UNORM);
    CHECK(format.dwRBitMask == 0x00ff0000);
    CHECK(format.dwGBitMask == 0x0000ff00);
    CHECK(format.dwBBitMask == 0x000000ff);
    CHECK(format.dwRGBAlphaBitMask == 0xff000000);
    CHECK(wined3dformat_from_ddrawformat(&format) == WINED3DFMT_B8G8R8A8_UNORM);
    printf("DDRAW_MAPPERS checks=%u failures=%u\n", checks, failures);
    return failures ? 1 : 0;
}
