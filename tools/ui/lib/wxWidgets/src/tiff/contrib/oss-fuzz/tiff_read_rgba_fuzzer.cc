/* Copyright (c) 1988-1997 Sam Leffler
 * Copyright (c) 1991-1997 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include <cstdint>
#include <sstream>
#include <tiffio.h>
#include <tiffio.hxx>


/* stolen from tiffiop.h, which is a private header so we can't just include it */
/* safe multiply returns either the multiplied value or 0 if it overflowed */
#define __TIFFSafeMultiply(t,v,m) ((((t)(m) != (t)0) && (((t)(((v)*(m))/(m))) == (t)(v))) ? (t)((v)*(m)) : (t)0)

const uint64 MAX_SIZE = 500000000;

extern "C" void handle_error(const char *unused, const char *unused2, va_list unused3) {
    return;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  TIFFSetErrorHandler(handle_error);
  TIFFSetWarningHandler(handle_error);
  std::istringstream s(std::string(Data,Data+Size));
  TIFF* tif = TIFFStreamOpen("MemTIFF", &s);
  if (!tif) {
      return 0;
  }
  uint32 w, h;
  uint32* raster;

  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
  /* don't continue if file size is ludicrous */
  if (TIFFTileSize64(tif) > MAX_SIZE) {
      TIFFClose(tif);
      return 0;
  }
  uint64 bufsize = TIFFTileSize64(tif);
  /* don't continue if the buffer size greater than the max allowed by the fuzzer */
  if (bufsize > MAX_SIZE || bufsize == 0) {
      TIFFClose(tif);
      return 0;
  }
  /* another hack to work around an OOM in tif_fax3.c */
  uint32 tilewidth = 0;
  uint32 imagewidth = 0;
  TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tilewidth);
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &imagewidth);
  tilewidth = __TIFFSafeMultiply(uint32, tilewidth, 2);
  imagewidth = __TIFFSafeMultiply(uint32, imagewidth, 2);
  if (tilewidth * 2 > MAX_SIZE || imagewidth * 2 > MAX_SIZE || tilewidth == 0 || imagewidth == 0) {
      TIFFClose(tif);
      return 0;
  }
  uint32 size = __TIFFSafeMultiply(uint32, w, h);
  if (size > MAX_SIZE || size == 0) {
      TIFFClose(tif);
      return 0;
  }
  raster = (uint32*) _TIFFmalloc(size * sizeof (uint32));
  if (raster != NULL) {
      TIFFReadRGBAImage(tif, w, h, raster, 0);
      _TIFFfree(raster);
  }
  TIFFClose(tif);

  return 0;
}
