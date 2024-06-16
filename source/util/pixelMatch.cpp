// code from https://github.com/mapbox/pixelmatch
// cpp port from https://github.com/mapbox/pixelmatch-cpp

// ISC License
// 
// Copyright(c) 2019, Mapbox
// 
// Permission to use, copy, modify, and /or distribute this software for any purpose
// with or without fee is hereby granted, provided that the above copyright notice
// and this permission notice appear in all copies.
// 
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS.IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
// OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
// TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
// THIS SOFTWARE.

#include "boxedwine.h"

#include <cstdint>
#include <cstddef>
#include <algorithm>

// blend semi-transparent color with white
U8 blend(U8 c, double a) {
    return (U8)(255 + (c - 255) * a);
}

double rgb2y(U8 r, U8 g, U8 b) { return r * 0.29889531 + g * 0.58662247 + b * 0.11448223; }
double rgb2i(U8 r, U8 g, U8 b) { return r * 0.59597799 - g * 0.27417610 - b * 0.32180189; }
double rgb2q(U8 r, U8 g, U8 b) { return r * 0.21147017 - g * 0.52261711 + b * 0.31114694; }

// calculate color difference according to the paper "Measuring perceived color difference
// using YIQ NTSC transmission color space in mobile applications" by Y. Kotsarenko and F. Ramos

double colorDelta(const U8* img1, const U8* img2, U32 k, U32 m, bool yOnly = false) {
    double a1 = 1.0;
    double a2 = 1.0;

    U8 r1 = blend(img1[k + 0], a1);
    U8 g1 = blend(img1[k + 1], a1);
    U8 b1 = blend(img1[k + 2], a1);

    U8 r2 = blend(img2[m + 0], a2);
    U8 g2 = blend(img2[m + 1], a2);
    U8 b2 = blend(img2[m + 2], a2);

    double y = rgb2y(r1, g1, b1) - rgb2y(r2, g2, b2);

    if (yOnly) return y; // brightness difference only

    double i = rgb2i(r1, g1, b1) - rgb2i(r2, g2, b2);
    double q = rgb2q(r1, g1, b1) - rgb2q(r2, g2, b2);

    return 0.5053 * y * y + 0.299 * i * i + 0.1957 * q * q;
}

void drawPixel(U8* output, U32 pos, U8 r, U8 g, U8 b) {
    output[pos + 0] = r;
    output[pos + 1] = g;
    output[pos + 2] = b;
    output[pos + 3] = 255;
}

double grayPixel(const U8* img, U32 i) {
    double a = double(img[i + 3]) / 255;
    U8 r = blend(img[i + 0], a);
    U8 g = blend(img[i + 1], a);
    U8 b = blend(img[i + 2], a);
    return rgb2y(r, g, b);
}

// check if a pixel is likely a part of anti-aliasing;
// based on "Anti-aliased Pixel and Intensity Slope Detector" paper by V. Vysniauskas, 2009

bool antialiased(const U8* img, U32 x1, U32 y1, U32 width, U32 height, const U8* img2 = nullptr) {
    U32 x0 = x1 > 0 ? x1 - 1 : 0;
    U32 y0 = y1 > 0 ? y1 - 1 : 0;
    U32 x2 = std::min(x1 + 1, width - 1);
    U32 y2 = std::min(y1 + 1, height - 1);
    U32 pos = (y1 * width + x1) * 4;
    U32 zeroes = 0;
    U32 positives = 0;
    U32 negatives = 0;
    double min = 0;
    double max = 0;
    U32 minX = 0, minY = 0, maxX = 0, maxY = 0;

    // go through 8 adjacent pixels
    for (U32 x = x0; x <= x2; x++) {
        for (U32 y = y0; y <= y2; y++) {
            if (x == x1 && y == y1) continue;

            // brightness delta between the center pixel and adjacent one
            double delta = colorDelta(img, img, pos, (y * width + x) * 4, true);

            // count the number of equal, darker and brighter adjacent pixels
            if (delta == 0) zeroes++;
            else if (delta < 0) negatives++;
            else if (delta > 0) positives++;

            // if found more than 2 equal siblings, it's definitely not anti-aliasing
            if (zeroes > 2) return false;

            if (!img2) continue;

            // remember the darkest pixel
            if (delta < min) {
                min = delta;
                minX = x;
                minY = y;
            }
            // remember the brightest pixel
            if (delta > max) {
                max = delta;
                maxX = x;
                maxY = y;
            }
        }
    }

    if (!img2) return true;

    // if there are no both darker and brighter pixels among siblings, it's not anti-aliasing
    if (negatives == 0 || positives == 0) return false;

    // if either the darkest or the brightest pixel has more than 2 equal siblings in both images
    // (definitely not anti-aliased), this pixel is anti-aliased
    return (!antialiased(img, minX, minY, width, height) && !antialiased(img2, minX, minY, width, height)) ||
        (!antialiased(img, maxX, maxY, width, height) && !antialiased(img2, maxX, maxY, width, height));
}

U32 pixelmatch(const U8* img1, U32 stride1, const U8* img2, U32 stride2, U32 width, U32 height, U8* output = nullptr, double threshold = 0.1, bool includeAA = false) {
    // maximum acceptable square distance between two colors;
    // 35215 is the maximum possible value for the YIQ difference metric
    double maxDelta = 35215 * threshold * threshold;
    U32 diff = 0;

    // compare each pixel of one image against the other one
    for (U32 y = 0; y < height; y++) {
        for (U32 x = 0; x < width; x++) {

            // allow input images to include different padding in their strides
            U32 pos1 = y * stride1 + x * 4;
            U32 pos2 = y * stride2 + x * 4;

            // but write the output as tightly-packed
            U32 posOut = (y * width + x) * 4;

            // squared YUV distance between colors at this pixel position
            double delta = colorDelta(img1, img2, pos1, pos2);

            // the color difference is above the threshold
            if (delta > maxDelta) {
                // check it's a real rendering difference or just anti-aliasing
                if (!includeAA && (antialiased(img1, x, y, width, height, img2) ||
                    antialiased(img2, x, y, width, height, img1))) {
                    // one of the pixels is anti-aliasing; draw as yellow and do not count as difference
                    if (output) drawPixel(output, posOut, 255, 255, 0);

                } else {
                    // found substantial difference not caused by anti-aliasing; draw it as red
                    if (output) drawPixel(output, posOut, 255, 0, 0);
                    diff++;
                }

            } else if (output) {
                // pixels are similar; draw background as grayscale image blended with white
                U8 val = blend((U8)grayPixel(img1, posOut), 0.1);
                drawPixel(output, posOut, val, val, val);
            }
        }
    }

    // return the number of different pixels
    return diff;
}
