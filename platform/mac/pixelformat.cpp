
/*
 * Mac driver OpenGL support
 *
 * Copyright 2012 Alexandre Julliard
 * Copyright 2012, 2013 Ken Thomases for CodeWeavers Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

// ported and adapted from Wine, dlls/winemac.drv/opengl.c
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CGDirectDisplay.h>
#include "boxedwine.h"
#define GL_SILENCE_DEPRECATION
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#include <OpenGL/CGLRenderers.h>
//#include <SDL_opengl.h>
#include "pixelformat.h"
#include <string.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

static bool allow_software_rendering = false;

struct color_mode {
    GLint   mode;
    int     bits_per_pixel;
    GLint   color_bits; /* including alpha_bits */
    int     red_bits, red_shift;
    int     green_bits, green_shift;
    int     blue_bits, blue_shift;
    GLint   alpha_bits, alpha_shift;
    bool    is_float;
    int     color_ordering;
};

/* The value of "color_ordering" is somewhat arbitrary.  It incorporates some
   observations of the behavior of Windows systems, but also subjective judgments
   about what color formats are more "normal" than others.

   On at least some Windows systems, integer color formats are listed before
   floating-point formats.  Within the integer formats, higher color bits were
   usually listed before lower color bits, while for floating-point formats it
   was the reverse.  However, that leads D3D to select 64-bit integer formats in
   preference to 32-bit formats when the latter would be sufficient.  It seems
   that a 32-bit format is much more likely to be normally used in that case.

   Also, there are certain odd color formats supported on the Mac which seem like
   they would be less appropriate than more common ones.  For instance, the color
   formats with alpha in a separate byte (e.g. kCGLRGB888A8Bit with R8G8B8 in one
   32-bit value and A8 in a separate 8-bit value) and the formats with 10-bit RGB
   components.

   For two color formats which differ only in whether or not they have alpha bits,
   we use the same ordering.  pixel_format_comparator() gives alpha bits a
   different weight than color formats.
 */
static const struct color_mode color_modes[] = {
    { kCGLRGB444Bit,        16,     12,     4,  8,      4,  4,      4,  0,      0,  0,  false,  5 },
    { kCGLARGB4444Bit,      16,     16,     4,  8,      4,  4,      4,  0,      4,  12, false,  5 },
    { kCGLRGB444A8Bit,      24,     20,     4,  8,      4,  4,      4,  0,      8,  16, false,  10 },
    { kCGLRGB555Bit,        16,     15,     5,  10,     5,  5,      5,  0,      0,  0,  false,  4 },
    { kCGLARGB1555Bit,      16,     16,     5,  10,     5,  5,      5,  0,      1,  15, false,  4 },
    { kCGLRGB555A8Bit,      24,     23,     5,  10,     5,  5,      5,  0,      8,  16, false,  9 },
    { kCGLRGB565Bit,        16,     16,     5,  11,     6,  5,      5,  0,      0,  0,  false,  3 },
    { kCGLRGB565A8Bit,      24,     24,     5,  11,     6,  5,      5,  0,      8,  16, false,  8 },
    { kCGLRGB888Bit,        32,     24,     8,  16,     8,  8,      8,  0,      0,  0,  false,  0 },
    { kCGLARGB8888Bit,      32,     32,     8,  16,     8,  8,      8,  0,      8,  24, false,  0 },
    { kCGLRGB888A8Bit,      40,     32,     8,  16,     8,  8,      8,  0,      8,  32, false,  7 },
    { kCGLRGB101010Bit,     32,     30,     10, 20,     10, 10,     10, 0,      0,  0,  false,  6 },
    { kCGLARGB2101010Bit,   32,     32,     10, 20,     10, 10,     10, 0,      2,  30, false,  6 },
    { kCGLRGB101010_A8Bit,  40,     38,     10, 20,     10, 10,     10, 0,      8,  32, false,  11 },
    { kCGLRGB121212Bit,     48,     36,     12, 24,     12, 12,     12, 0,      0,  0,  false,  2 },
    { kCGLARGB12121212Bit,  48,     48,     12, 24,     12, 12,     12, 0,      12, 36, false,  2 },
    { kCGLRGB161616Bit,     64,     48,     16, 48,     16, 32,     16, 16,     0,  0,  false,  1 },
    { kCGLRGBA16161616Bit,  64,     64,     16, 48,     16, 32,     16, 16,     16, 0,  false,  1 },
    { kCGLRGBFloat64Bit,    64,     48,     16, 32,     16, 16,     16, 0,      0,  0,  true,   12 },
    { kCGLRGBAFloat64Bit,   64,     64,     16, 48,     16, 32,     16, 16,     16, 0,  true,   12 },
    { kCGLRGBFloat128Bit,   128,    96,     32, 96,     32, 64,     32, 32,     0,  0,  true,   13 },
    { kCGLRGBAFloat128Bit,  128,    128,    32, 96,     32, 64,     32, 32,     32, 0,  true,   13 },
    { kCGLRGBFloat256Bit,   256,    192,    64, 192,    64, 128,    64, 64,     0,  0,  true,   14 },
    { kCGLRGBAFloat256Bit,  256,    256,    64, 192,    64, 128,    64, 64,     64, 0,  true,   15 },
};


static const struct {
    GLint   mode;
    int     bits;
} depth_stencil_modes[] = {
    { kCGL0Bit,     0 },
    { kCGL1Bit,     1 },
    { kCGL2Bit,     2 },
    { kCGL3Bit,     3 },
    { kCGL4Bit,     4 },
    { kCGL5Bit,     5 },
    { kCGL6Bit,     6 },
    { kCGL8Bit,     8 },
    { kCGL10Bit,    10 },
    { kCGL12Bit,    12 },
    { kCGL16Bit,    16 },
    { kCGL24Bit,    24 },
    { kCGL32Bit,    32 },
    { kCGL48Bit,    48 },
    { kCGL64Bit,    64 },
    { kCGL96Bit,    96 },
    { kCGL128Bit,   128 },
};


typedef struct {
    GLint   renderer_id;
    GLint   buffer_modes;
    GLint   color_modes;
    GLint   accum_modes;
    GLint   depth_modes;
    GLint   stencil_modes;
    GLint   max_aux_buffers;
    GLint   max_sample_buffers;
    GLint   max_samples;
    bool    offscreen;
    bool    accelerated;
    bool    backing_store;
    bool    window;
    bool    online;
} renderer_properties;


typedef struct {
    unsigned int window:1;
    unsigned int pbuffer:1;
    unsigned int accelerated:1;
    unsigned int color_mode:5; /* index into color_modes table */
    unsigned int aux_buffers:3;
    unsigned int depth_bits:8;
    unsigned int stencil_bits:8;
    unsigned int accum_mode:5; /* 1 + index into color_modes table (0 means no accum buffer) */
    unsigned int double_buffer:1;
    unsigned int stereo:1;
    unsigned int sample_buffers:1;
    unsigned int samples:5;
    unsigned int backing_store:1;
} pixel_format;


typedef union
{
    pixel_format    format;
    U64             code;
} pixel_format_or_code;

static pixel_format *pixel_formats;
static int nb_formats, nb_displayable_formats;

static bool get_renderer_property(CGLRendererInfoObj renderer_info, GLint renderer_index, CGLRendererProperty property, GLint *value) {
    CGLError err = CGLDescribeRenderer(renderer_info, renderer_index, property, value);
    if (err != kCGLNoError) {
        kwarn("CGLDescribeRenderer failed for property %d: %d %s\n", property, err, CGLErrorString(err));
    }
    return (err == kCGLNoError);
}


static void get_renderer_properties(CGLRendererInfoObj renderer_info, int renderer_index, renderer_properties* properties) {
    GLint value;

    memset(properties, 0, sizeof(*properties));

    if (get_renderer_property(renderer_info, renderer_index, kCGLRPRendererID, &value)) {
        properties->renderer_id = value & kCGLRendererIDMatchingMask;
    }
    if (get_renderer_property(renderer_info, renderer_index, kCGLRPBufferModes, &value)) {
        properties->buffer_modes = value;
    }
    if (get_renderer_property(renderer_info, renderer_index, kCGLRPColorModes, &value)) {
        properties->color_modes = value;
    }
    if (get_renderer_property(renderer_info, renderer_index, kCGLRPAccumModes, &value)) {
        properties->accum_modes = value;
    }
    if (get_renderer_property(renderer_info, renderer_index, kCGLRPDepthModes, &value)) {
        properties->depth_modes = value;
    }
    if (get_renderer_property(renderer_info, renderer_index, kCGLRPStencilModes, &value)) {
        properties->stencil_modes = value;
    }
    if (get_renderer_property(renderer_info, renderer_index, kCGLRPMaxAuxBuffers, &value)) {
        properties->max_aux_buffers = value;
    }
    if (get_renderer_property(renderer_info, renderer_index, kCGLRPMaxSampleBuffers, &value)) {
        properties->max_sample_buffers = value;
    }
    if (get_renderer_property(renderer_info, renderer_index, kCGLRPMaxSamples, &value)) {
        properties->max_samples = value;
    }
    if (get_renderer_property(renderer_info, renderer_index, kCGLRPOffScreen, &value)) {
        properties->offscreen = (value != 0);
    }
    if (get_renderer_property(renderer_info, renderer_index, kCGLRPAccelerated, &value)) {
        properties->accelerated = (value != 0);
    }
    if (get_renderer_property(renderer_info, renderer_index, kCGLRPBackingStore, &value)) {
        properties->backing_store = (value != 0);
    }
    if (get_renderer_property(renderer_info, renderer_index, kCGLRPWindow, &value)) {
        properties->window = (value != 0);
    }
    if (get_renderer_property(renderer_info, renderer_index, kCGLRPOnline, &value)) {
        properties->online = (value != 0);
    }
}

static inline U64 code_for_pixel_format(const pixel_format* format) {
    pixel_format_or_code pfc;

    pfc.code = 0;
    pfc.format = *format;
    return pfc.code;
}


static inline pixel_format pixel_format_for_code(U64 code) {
    pixel_format_or_code pfc;

    pfc.code = code;
    return pfc.format;
}

static unsigned int best_color_mode(GLint modes, GLint color_size, GLint alpha_size, GLint color_float) {
    int best = -1;

    for (int i = 0; i < ARRAY_SIZE(color_modes); i++) {
        if ((modes & color_modes[i].mode) && color_modes[i].color_bits >= color_size && color_modes[i].alpha_bits >= alpha_size && !color_modes[i].is_float == !color_float) {
            if (best < 0) { /* no existing best choice */
                best = i;
            } else if (color_modes[i].color_bits == color_size && color_modes[i].alpha_bits == alpha_size) { /* candidate is exact match */
                /* prefer it over a best which isn't exact or which has a higher bpp */
                if (color_modes[best].color_bits != color_size || color_modes[best].alpha_bits != alpha_size || color_modes[i].bits_per_pixel < color_modes[best].bits_per_pixel) {
                    best = i;
                }
            } else if (color_modes[i].color_bits < color_modes[best].color_bits || (color_modes[i].color_bits == color_modes[best].color_bits && color_modes[i].alpha_bits < color_modes[best].alpha_bits)) { /* prefer closer */
                best = i;
            }
        }
    }

    if (best < 0) {
        /* Couldn't find a match.  Return first one that renderer supports. */
        for (int i = 0; i < ARRAY_SIZE(color_modes); i++) {
            if (modes & color_modes[i].mode) {
                return i;
            }
        }
    }

    return best;
}


static unsigned int best_accum_mode(GLint modes, GLint accum_size) {
    int best = -1;

    for (int i = 0; i < ARRAY_SIZE(color_modes); i++) {
        if ((modes & color_modes[i].mode) && color_modes[i].color_bits >= accum_size) {
            /* Prefer the fewest color bits, then prefer more alpha bits, then
               prefer more bits per pixel. */
            if (best < 0) {
                best = i;
            } else if (color_modes[i].color_bits < color_modes[best].color_bits) {
                best = i;
            } else if (color_modes[i].color_bits == color_modes[best].color_bits) {
                if (color_modes[i].alpha_bits > color_modes[best].alpha_bits) {
                    best = i;
                } else if (color_modes[i].alpha_bits == color_modes[best].alpha_bits && color_modes[i].bits_per_pixel > color_modes[best].bits_per_pixel) {
                    best = i;
                }
            }
        }
    }

    if (best < 0) {
        /* Couldn't find a match.  Return last one that renderer supports. */
        for (int i = ARRAY_SIZE(color_modes) - 1; i >= 0; i--) {
            if (modes & color_modes[i].mode) {
                return i;
            }
        }
    }

    return best;
}


static void enum_renderer_pixel_formats(renderer_properties renderer, CFMutableArrayRef pixel_format_array, CFMutableSetRef pixel_format_set)
{
    CGLPixelFormatAttribute attribs[64];
    attribs[0] = kCGLPFAMinimumPolicy;
    attribs[1] = kCGLPFAClosestPolicy;
    attribs[2] = kCGLPFARendererID;
    attribs[3] = (CGLPixelFormatAttribute)renderer.renderer_id;
    attribs[4] = kCGLPFASingleRenderer;
    int n = 5, n_stack[16], n_stack_idx = -1;
    unsigned int tried_pixel_formats = 0, failed_pixel_formats = 0, dupe_pixel_formats = 0,
                 new_pixel_formats = 0;
    pixel_format request;
    unsigned int double_buffer;
    unsigned int accelerated = renderer.accelerated;

    if (accelerated) {
        attribs[n++] = kCGLPFAAccelerated;
        attribs[n++] = kCGLPFANoRecovery;
    } else if (!allow_software_rendering) {
        klog("ignoring software renderer because AllowSoftwareRendering is off\n");
        return;
    }

    n_stack[++n_stack_idx] = n;
    for (double_buffer = 0; double_buffer <= 1; double_buffer++) {
        unsigned int aux;

        n = n_stack[n_stack_idx];

        if ((!double_buffer && !(renderer.buffer_modes & kCGLSingleBufferBit)) || (double_buffer && !(renderer.buffer_modes & kCGLDoubleBufferBit))) {
            continue;
        }
        if (double_buffer) {
            attribs[n++] = kCGLPFADoubleBuffer;
        }
        memset(&request, 0, sizeof(request));
        request.accelerated = accelerated;
        request.double_buffer = double_buffer;

        /* Don't bother with in-between aux buffers values: either 0 or max. */
        n_stack[++n_stack_idx] = n;
        for (aux = 0; aux <= renderer.max_aux_buffers; aux += renderer.max_aux_buffers) {
            unsigned int color_mode;

            n = n_stack[n_stack_idx];

            attribs[n++] = kCGLPFAAuxBuffers;
            attribs[n++] = (CGLPixelFormatAttribute)aux;
            request.aux_buffers = aux;

            n_stack[++n_stack_idx] = n;
            for (color_mode = 0; color_mode < ARRAY_SIZE(color_modes); color_mode++) {
                unsigned int depth_mode;

                n = n_stack[n_stack_idx];

                if (!(renderer.color_modes & color_modes[color_mode].mode)) {
                    continue;
                }
                attribs[n++] = kCGLPFAColorSize;
                attribs[n++] = (CGLPixelFormatAttribute)color_modes[color_mode].color_bits;
                attribs[n++] = kCGLPFAAlphaSize;
                attribs[n++] = (CGLPixelFormatAttribute)color_modes[color_mode].alpha_bits;
                if (color_modes[color_mode].is_float) {
                    attribs[n++] = kCGLPFAColorFloat;
                }
                request.color_mode = color_mode;

                n_stack[++n_stack_idx] = n;
                for (depth_mode = 0; depth_mode < ARRAY_SIZE(depth_stencil_modes); depth_mode++) {
                    unsigned int stencil_mode;

                    n = n_stack[n_stack_idx];

                    if (!(renderer.depth_modes & depth_stencil_modes[depth_mode].mode)) {
                        continue;
                    }
                    
                    attribs[n++] = kCGLPFADepthSize;
                    attribs[n++] = (CGLPixelFormatAttribute)depth_stencil_modes[depth_mode].bits;
                    request.depth_bits = depth_stencil_modes[depth_mode].bits;

                    n_stack[++n_stack_idx] = n;
                    for (stencil_mode = 0; stencil_mode < ARRAY_SIZE(depth_stencil_modes); stencil_mode++) {
                        unsigned int stereo;

                        n = n_stack[n_stack_idx];

                        if (!(renderer.stencil_modes & depth_stencil_modes[stencil_mode].mode)) {
                            continue;
                        }
                        if (accelerated && depth_stencil_modes[depth_mode].bits != 24 && depth_stencil_modes[depth_mode].bits != 32 && stencil_mode > 0) {
                            continue;
                        }
                        
                        attribs[n++] = kCGLPFAStencilSize;
                        attribs[n++] = (CGLPixelFormatAttribute)depth_stencil_modes[stencil_mode].bits;
                        request.stencil_bits = depth_stencil_modes[stencil_mode].bits;

                        /* FIXME: Could trim search space a bit here depending on GPU.
                                  For ATI Radeon HD 4850, kCGLRGBA16161616Bit implies stereo-capable. */
                        n_stack[++n_stack_idx] = n;
                        for (stereo = 0; stereo <= 1; stereo++) {
                            int accum_mode;

                            n = n_stack[n_stack_idx];

                            if ((!stereo && !(renderer.buffer_modes & kCGLMonoscopicBit)) || (stereo && !(renderer.buffer_modes & kCGLStereoscopicBit))) {
                                continue;
                            }
                            if (stereo) {
                                attribs[n++] = kCGLPFAStereo;
                            }
                            request.stereo = stereo;

                            /* Starts at -1 for a 0 accum size */
                            n_stack[++n_stack_idx] = n;
                            for (accum_mode = -1; accum_mode < (int) ARRAY_SIZE(color_modes); accum_mode++) {
                                unsigned int target_pass;

                                n = n_stack[n_stack_idx];

                                if (accum_mode >= 0) {
                                    if (!(renderer.accum_modes & color_modes[accum_mode].mode)) {
                                        continue;
                                    }
                                    attribs[n++] = kCGLPFAAccumSize;
                                    attribs[n++] = (CGLPixelFormatAttribute)color_modes[accum_mode].color_bits;
                                    request.accum_mode = accum_mode + 1;
                                } else {
                                    request.accum_mode = 0;
                                }
                                /* Targets to request are:
                                        accelerated: window OR window + pbuffer
                                        software: window + pbuffer */
                                n_stack[++n_stack_idx] = n;
                                for (target_pass = 0; target_pass <= accelerated; target_pass++) {
                                    unsigned int samples, max_samples;

                                    n = n_stack[n_stack_idx];

                                    attribs[n++] = kCGLPFAWindow;
                                    request.window = 1;

                                    if (!accelerated || target_pass > 0) {
                                        attribs[n++] = kCGLPFAPBuffer;
                                        request.pbuffer = 1;
                                    } else {
                                        request.pbuffer = 0;
                                    }
                                    /* FIXME: Could trim search space a bit here depending on GPU.
                                              For Nvidia GeForce 8800 GT, limited to 4 samples for color_bits >= 128.
                                              For ATI Radeon HD 4850, can't multi-sample for color_bits >= 64 or pbuffer. */
                                    n_stack[++n_stack_idx] = n;
                                    max_samples = renderer.max_sample_buffers ? std::max(1, renderer.max_samples) : 1;
                                    for (samples = 1; samples <= max_samples; samples *= 2) {
                                        unsigned int backing_store, min_backing_store, max_backing_store;

                                        n = n_stack[n_stack_idx];

                                        if (samples > 1) {
                                            attribs[n++] = kCGLPFASampleBuffers;
                                            attribs[n++] = (CGLPixelFormatAttribute)renderer.max_sample_buffers;
                                            attribs[n++] = kCGLPFASamples;
                                            attribs[n++] = (CGLPixelFormatAttribute)samples;
                                            request.sample_buffers = renderer.max_sample_buffers;
                                            request.samples = samples;
                                        } else {
                                            request.sample_buffers = request.samples = 0;
                                        }
                                        if (renderer.backing_store && double_buffer) {
                                            /* The software renderer seems to always preserve the backing store, whether
                                               we ask for it or not.  So don't bother not asking for it. */
                                            min_backing_store = accelerated ? 0 : 1;
                                            max_backing_store = 1;
                                        } else {
                                            min_backing_store = max_backing_store = 0;
                                        }
                                        n_stack[++n_stack_idx] = n;
                                        for (backing_store = min_backing_store; backing_store <= max_backing_store; backing_store++) {
                                            CGLPixelFormatObj pix;
                                            GLint virtualScreens;
                                            CGLError err;

                                            n = n_stack[n_stack_idx];

                                            if (backing_store) {
                                                attribs[n++] = kCGLPFABackingStore;
                                            }
                                            request.backing_store = backing_store;

                                            attribs[n] = (CGLPixelFormatAttribute)0;

                                            err = CGLChoosePixelFormat(attribs, &pix, &virtualScreens);
                                            if (err == kCGLNoError && pix) {
                                                pixel_format pf;
                                                GLint value, color_size, alpha_size, color_float;
                                                UInt64 pf_code;
                                                CFNumberRef code_object;
                                                bool dupe;

                                                memset(&pf, 0, sizeof(pf));

                                                if (CGLDescribePixelFormat(pix, 0, kCGLPFAAccelerated, &value) == kCGLNoError) {
                                                    pf.accelerated = value;
                                                }
                                                if (CGLDescribePixelFormat(pix, 0, kCGLPFAAuxBuffers, &value) == kCGLNoError) {
                                                    pf.aux_buffers = value;
                                                }
                                                if (CGLDescribePixelFormat(pix, 0, kCGLPFADepthSize, &value) == kCGLNoError) {
                                                    pf.depth_bits = value;
                                                }
                                                if (CGLDescribePixelFormat(pix, 0, kCGLPFADoubleBuffer, &value) == kCGLNoError) {
                                                    pf.double_buffer = value;
                                                }
                                                if (pf.double_buffer && CGLDescribePixelFormat(pix, 0, kCGLPFABackingStore, &value) == kCGLNoError) {
                                                    pf.backing_store = value;
                                                }
                                                if (CGLDescribePixelFormat(pix, 0, kCGLPFAPBuffer, &value) == kCGLNoError) {
                                                    pf.pbuffer = value;
                                                }
                                                if (CGLDescribePixelFormat(pix, 0, kCGLPFASampleBuffers, &value) == kCGLNoError) {
                                                    pf.sample_buffers = value;
                                                }
                                                if (pf.sample_buffers && CGLDescribePixelFormat(pix, 0, kCGLPFASamples, &value) == kCGLNoError) {
                                                    pf.samples = value;
                                                }
                                                if (CGLDescribePixelFormat(pix, 0, kCGLPFAStencilSize, &value) == kCGLNoError) {
                                                    pf.stencil_bits = value;
                                                }
                                                if (CGLDescribePixelFormat(pix, 0, kCGLPFAStereo, &value) == kCGLNoError) {
                                                    pf.stereo = value;
                                                }
                                                if (CGLDescribePixelFormat(pix, 0, kCGLPFAWindow, &value) == kCGLNoError) {
                                                    pf.window = value;
                                                }
                                                if (CGLDescribePixelFormat(pix, 0, kCGLPFAColorSize, &color_size) != kCGLNoError) {
                                                    color_size = 0;
                                                }
                                                if (CGLDescribePixelFormat(pix, 0, kCGLPFAAlphaSize, &alpha_size) != kCGLNoError) {
                                                    alpha_size = 0;
                                                }
                                                if (CGLDescribePixelFormat(pix, 0, kCGLPFAColorFloat, &color_float) != kCGLNoError) {
                                                    color_float = 0;
                                                }
                                                pf.color_mode = best_color_mode(renderer.color_modes, color_size, alpha_size, color_float);

                                                if (CGLDescribePixelFormat(pix, 0, kCGLPFAAccumSize, &value) == kCGLNoError && value) {
                                                    pf.accum_mode = best_accum_mode(renderer.accum_modes, value) + 1;
                                                }

                                                CGLReleasePixelFormat(pix);

                                                pf_code = code_for_pixel_format(&pf);

                                                code_object = CFNumberCreate(NULL, kCFNumberSInt64Type, &pf_code);
                                                if ((dupe = CFSetContainsValue(pixel_format_set, code_object))) {
                                                    dupe_pixel_formats++;
                                                } else {
                                                    CFSetAddValue(pixel_format_set, code_object);
                                                    CFArrayAppendValue(pixel_format_array, code_object);
                                                    new_pixel_formats++;
                                                }
                                                CFRelease(code_object);
                                            }
                                            else {
                                                failed_pixel_formats++;
                                            }

                                            tried_pixel_formats++;
                                        }

                                        n_stack_idx--;
                                    }

                                    n_stack_idx--;
                                }

                                n_stack_idx--;
                            }

                            n_stack_idx--;
                        }

                        n_stack_idx--;
                    }

                    n_stack_idx--;
                }

                n_stack_idx--;
            }

            n_stack_idx--;
        }

        n_stack_idx--;
    }

    n_stack_idx--;

    klog("Number of pixel format attribute combinations: %u\n", tried_pixel_formats);
    klog(" Number which failed to choose a pixel format: %u\n", failed_pixel_formats);
    klog("  Number which chose redundant pixel formats: %u\n", dupe_pixel_formats);
    klog("Number of new pixel formats for this renderer: %u\n", new_pixel_formats);
}


/* The docs for WGL_ARB_pixel_format say:
    Indices are assigned to pixel formats in the following order:
    1. Accelerated pixel formats that are displayable
    2. Accelerated pixel formats that are displayable and which have
       extended attributes
    3. Generic pixel formats
    4. Accelerated pixel formats that are non displayable
 */
static int pixel_format_category(pixel_format pf)
{
    /* non-displayable */
    if (!pf.window)
        return 4;

    /* non-accelerated a.k.a. software a.k.a. generic */
    if (!pf.accelerated)
        return 3;

    /* extended attributes that can't be represented in PIXELFORMATDESCRIPTOR */
    if (color_modes[pf.color_mode].is_float)
        return 2;

    /* accelerated, displayable, no extended attributes */
    return 1;
}


static CFComparisonResult pixel_format_comparator(const void *val1, const void *val2, void *context)
{
    CFNumberRef number1 = (CFNumberRef)val1;
    CFNumberRef number2 = (CFNumberRef)val2;
    UInt64 code1, code2;
    pixel_format pf1, pf2;
    int category1, category2;

    CFNumberGetValue(number1, kCFNumberLongLongType, &code1);
    CFNumberGetValue(number2, kCFNumberLongLongType, &code2);
    pf1 = pixel_format_for_code(code1);
    pf2 = pixel_format_for_code(code2);
    category1 = pixel_format_category(pf1);
    category2 = pixel_format_category(pf2);

    if (category1 < category2) {
        return kCFCompareLessThan;
    }
    if (category1 > category2) {
        return kCFCompareGreaterThan;
    }
    /* Within a category, sort the "best" formats toward the front since that's
       what wglChoosePixelFormatARB() has to do.  The ordering implemented here
       matches at least one Windows 7 machine's behavior.
     */
    /* Accelerated before unaccelerated. */
    if (pf1.accelerated && !pf2.accelerated) {
        return kCFCompareLessThan;
    }
    if (!pf1.accelerated && pf2.accelerated) {
        return kCFCompareGreaterThan;
    }
    /* Explicit color mode ordering. */
    if (color_modes[pf1.color_mode].color_ordering < color_modes[pf2.color_mode].color_ordering) {
        return kCFCompareLessThan;
    }
    if (color_modes[pf1.color_mode].color_ordering > color_modes[pf2.color_mode].color_ordering) {
        return kCFCompareGreaterThan;
    }

    /* Non-pbuffer-capable before pbuffer-capable. */
    if (!pf1.pbuffer && pf2.pbuffer) {
        return kCFCompareLessThan;
    }
    if (pf1.pbuffer && !pf2.pbuffer) {
        return kCFCompareGreaterThan;
    }
    /* Fewer samples before more samples. */
    if (pf1.samples < pf2.samples) {
        return kCFCompareLessThan;
    }
    if (pf1.samples > pf2.samples) {
        return kCFCompareGreaterThan;
    }
    /* Monoscopic before stereoscopic.  (This is a guess.) */
    if (!pf1.stereo && pf2.stereo) {
        return kCFCompareLessThan;
    }
    if (pf1.stereo && !pf2.stereo) {
        return kCFCompareGreaterThan;
    }

    /* Single buffered before double buffered. */
    if (!pf1.double_buffer && pf2.double_buffer) {
        return kCFCompareLessThan;
    }
    if (pf1.double_buffer && !pf2.double_buffer) {
        return kCFCompareGreaterThan;
    }
    /* Possibly-optimized double buffering before backing-store-preserving
       double buffering. */
    if (!pf1.backing_store && pf2.backing_store) {
        return kCFCompareLessThan;
    }
    if (pf1.backing_store && !pf2.backing_store) {
        return kCFCompareGreaterThan;
    }
    /* Bigger depth buffer before smaller depth buffer. */
    if (pf1.depth_bits > pf2.depth_bits) {
        return kCFCompareLessThan;
    }
    if (pf1.depth_bits < pf2.depth_bits) {
        return kCFCompareGreaterThan;
    }
    /* Smaller stencil buffer before bigger stencil buffer. */
    if (pf1.stencil_bits < pf2.stencil_bits) {
        return kCFCompareLessThan;
    }
    if (pf1.stencil_bits > pf2.stencil_bits) {
        return kCFCompareGreaterThan;
    }
    /* Smaller alpha bits before larger alpha bits. */
    if (color_modes[pf1.color_mode].alpha_bits < color_modes[pf2.color_mode].alpha_bits) {
        return kCFCompareLessThan;
    }
    if (color_modes[pf1.color_mode].alpha_bits > color_modes[pf2.color_mode].alpha_bits) {
        return kCFCompareGreaterThan;
    }
    /* Smaller accum buffer before larger accum buffer.  (This is a guess.) */
    if (pf1.accum_mode) {
        if (pf2.accum_mode) {
            if (color_modes[pf1.accum_mode - 1].color_bits - color_modes[pf1.accum_mode - 1].alpha_bits < color_modes[pf2.accum_mode - 1].color_bits - color_modes[pf2.accum_mode - 1].alpha_bits) {
                return kCFCompareLessThan;
            }
            if (color_modes[pf1.accum_mode - 1].color_bits - color_modes[pf1.accum_mode - 1].alpha_bits > color_modes[pf2.accum_mode - 1].color_bits - color_modes[pf2.accum_mode - 1].alpha_bits) {
                return kCFCompareGreaterThan;
            }
            if (color_modes[pf1.accum_mode - 1].bits_per_pixel < color_modes[pf2.accum_mode - 1].bits_per_pixel) {
                return kCFCompareLessThan;
            }
            if (color_modes[pf1.accum_mode - 1].bits_per_pixel > color_modes[pf2.accum_mode - 1].bits_per_pixel) {
                return kCFCompareGreaterThan;
            }
            if (color_modes[pf1.accum_mode - 1].alpha_bits < color_modes[pf2.accum_mode - 1].alpha_bits) {
                return kCFCompareLessThan;
            }
            if (color_modes[pf1.accum_mode - 1].alpha_bits > color_modes[pf2.accum_mode - 1].alpha_bits) {
                return kCFCompareGreaterThan;
            }
        } else {
            return kCFCompareGreaterThan;
        }
    } else if (pf2.accum_mode) {
        return kCFCompareLessThan;
    }

    /* Fewer auxiliary buffers before more auxiliary buffers.  (This is a guess.) */
    if (pf1.aux_buffers < pf2.aux_buffers) {
        return kCFCompareLessThan;
    }
    if (pf1.aux_buffers > pf2.aux_buffers) {
        return kCFCompareGreaterThan;
    }

    /* If we get here, arbitrarily sort based on code. */
    if (code1 < code2) {
        return kCFCompareLessThan;
    }
    if (code1 > code2) {
        return kCFCompareGreaterThan;
    }
    return kCFCompareEqualTo;
}


static bool init_pixel_formats(void)
{
    bool ret = false;
    CGLRendererInfoObj renderer_info;
    GLint rendererCount;
    CGLError err;
    CFMutableSetRef pixel_format_set;
    CFMutableArrayRef pixel_format_array;
    int i;
    CFRange range;

    err = CGLQueryRendererInfo(CGDisplayIDToOpenGLDisplayMask(CGMainDisplayID()), &renderer_info, &rendererCount);
    if (err) {
        kwarn("CGLQueryRendererInfo failed (%d) %s", err, CGLErrorString(err));
        return false;
    }

    pixel_format_set = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
    if (!pixel_format_set) {
        kwarn("CFSetCreateMutable failed");
        CGLDestroyRendererInfo(renderer_info);
        return false;
    }

    pixel_format_array = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    if (!pixel_format_array) {
        kwarn("CFArrayCreateMutable failed");
        CFRelease(pixel_format_set);
        CGLDestroyRendererInfo(renderer_info);
        return false;
    }

    for (i = 0; i < rendererCount; i++) {
        renderer_properties renderer;

        get_renderer_properties(renderer_info, i, &renderer);
        enum_renderer_pixel_formats(renderer, pixel_format_array, pixel_format_set);
    }

    CFRelease(pixel_format_set);
    CGLDestroyRendererInfo(renderer_info);

    range = CFRangeMake(0, CFArrayGetCount(pixel_format_array));
    if (range.length) {
        pixel_formats = new pixel_format[range.length];
        if (pixel_formats) {
            CFArraySortValues(pixel_format_array, range, pixel_format_comparator, NULL);
            for (i = 0; i < range.length; i++) {
                CFNumberRef number = (CFNumberRef)CFArrayGetValueAtIndex(pixel_format_array, i);
                UInt64 code;

                CFNumberGetValue(number, kCFNumberLongLongType, &code);
                pixel_formats[i] = pixel_format_for_code(code);
                if (pixel_formats[i].window)
                    nb_displayable_formats++;
            }

            nb_formats = (int)range.length;
            ret = true;
        } else {
            kwarn("failed to allocate pixel format list");
        }
    } else {
        kwarn("got no pixel formats");
    }
    CFRelease(pixel_format_array);
    return ret;
}


static inline bool is_valid_pixel_format(int format) {
    return format > 0 && format <= nb_formats;
}


static inline bool is_displayable_pixel_format(int format) {
    return format > 0 && format <= nb_displayable_formats;
}


static const pixel_format *get_pixel_format(int format, bool allow_nondisplayable) {
    /* Check if the pixel format is valid. Note that it is legal to pass an invalid
     * format in case of probing the number of pixel formats.
     */
    if (is_valid_pixel_format(format) && (is_displayable_pixel_format(format) || allow_nondisplayable)) {
        return &pixel_formats[format - 1];
    }
    return NULL;
}

static bool initialized = false;

int getPixelFormats(PixelFormat* pfs, int maxPfs) {
    if (!initialized) {
        init_pixel_formats();
        initialized = true;
    }
    const pixel_format *pf;
    const struct color_mode *mode;
    int count = nb_displayable_formats;
    int result = 0;
    
    if (count > maxPfs) {
        count = maxPfs;
    }
    for (int i=0;i<nb_formats && result < count;i++) {

        if (!(pf = get_pixel_format(i+1, FALSE))) {
            continue;
        }
            

        memset(&pfs[i], 0, sizeof(PixelFormat));
        pfs[i].nSize            = 40;
        pfs[i].nVersion         = 1;

        pfs[i].dwFlags          = K_PFD_SUPPORT_OPENGL;
        if (pf->window)         pfs[i].dwFlags |= K_PFD_DRAW_TO_WINDOW;
        if (!pf->accelerated)   pfs[i].dwFlags |= K_PFD_GENERIC_FORMAT;
        if (pf->double_buffer)  pfs[i].dwFlags |= K_PFD_DOUBLEBUFFER;
        if (pf->stereo)         pfs[i].dwFlags |= K_PFD_STEREO;
        if (pf->backing_store)  pfs[i].dwFlags |= K_PFD_SWAP_COPY;

        pfs[i].iPixelType       = K_PFD_TYPE_RGBA;

        mode = &color_modes[pf->color_mode];
        /* If the mode doesn't have alpha, return bits per pixel instead of color bits.
           On Windows, color bits sometimes exceeds r+g+b (e.g. it's 32 for an
           R8G8B8A0 pixel format).  If an app depends on that and expects that
           cColorBits >= 32 for such a pixel format, we need to accommodate that. */
        if (mode->alpha_bits) {
            pfs[i].cColorBits   = mode->color_bits;
        } else {
            pfs[i].cColorBits   = mode->bits_per_pixel;
        }
        pfs[i].cRedBits         = mode->red_bits;
        pfs[i].cRedShift        = mode->red_shift;
        pfs[i].cGreenBits       = mode->green_bits;
        pfs[i].cGreenShift      = mode->green_shift;
        pfs[i].cBlueBits        = mode->blue_bits;
        pfs[i].cBlueShift       = mode->blue_shift;
        pfs[i].cAlphaBits       = mode->alpha_bits;
        pfs[i].cAlphaShift      = mode->alpha_shift;

        if (pf->accum_mode) {
            mode = &color_modes[pf->accum_mode - 1];
            pfs[i].cAccumBits       = mode->color_bits;
            pfs[i].cAccumRedBits    = mode->red_bits;
            pfs[i].cAccumGreenBits  = mode->green_bits;
            pfs[i].cAccumBlueBits   = mode->blue_bits;
            pfs[i].cAccumAlphaBits  = mode->alpha_bits;
        }

        pfs[i].cDepthBits       = pf->depth_bits;
        pfs[i].cStencilBits     = pf->stencil_bits;
        pfs[i].cAuxBuffers      = pf->aux_buffers;
        pfs[i].iLayerType       = K_PFD_MAIN_PLANE;
    }
    return result;
}
