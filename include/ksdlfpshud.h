/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
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

#ifndef __KSDLFPSHUD_H__
#define __KSDLFPSHUD_H__

#include "boxedwine.h"
#include <SDL.h>
#include <stdio.h>
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

static inline const U8* kSDLFpsHudGlyph(char c) {
    static const U8 zero[5] = {7, 5, 5, 5, 7};
    static const U8 one[5] = {2, 6, 2, 2, 7};
    static const U8 two[5] = {7, 1, 7, 4, 7};
    static const U8 three[5] = {7, 1, 7, 1, 7};
    static const U8 four[5] = {5, 5, 7, 1, 1};
    static const U8 five[5] = {7, 4, 7, 1, 7};
    static const U8 six[5] = {7, 4, 7, 5, 7};
    static const U8 seven[5] = {7, 1, 1, 1, 1};
    static const U8 eight[5] = {7, 5, 7, 5, 7};
    static const U8 nine[5] = {7, 5, 7, 1, 7};
    static const U8 a[5] = {7, 5, 7, 5, 5};
    static const U8 e[5] = {7, 4, 6, 4, 7};
    static const U8 f[5] = {7, 4, 6, 4, 4};
    static const U8 m[5] = {5, 7, 7, 5, 5};
    static const U8 p[5] = {6, 5, 6, 4, 4};
    static const U8 s[5] = {7, 4, 7, 1, 7};
    static const U8 percent[5] = {5, 1, 2, 4, 5};

    switch (c) {
    case '0': return zero;
    case '1': return one;
    case '2': return two;
    case '3': return three;
    case '4': return four;
    case '5': return five;
    case '6': return six;
    case '7': return seven;
    case '8': return eight;
    case '9': return nine;
    case 'A': return a;
    case 'E': return e;
    case 'F': return f;
    case 'M': return m;
    case 'P': return p;
    case 'S': return s;
    case '%': return percent;
    default: return nullptr;
    }
}

static inline U64& kSDLFpsHudLastRenderTime() {
    static U64 value;
    return value;
}

static inline U32& kSDLFpsHudSameCandidateFrames() {
    static U32 value;
    return value;
}

static inline U32& kSDLFpsHudUnchangedFrames() {
    static U32 value;
    return value;
}

static inline U32& kSDLFpsHudDisplayedSamePercent() {
    static U32 value;
    return value;
}

static inline U64& kSDLFpsHudAverageStartTime() {
    static U64 value;
    return value;
}

static inline U32& kSDLFpsHudAveragePresentedFrames() {
    static U32 value;
    return value;
}

static inline U32& kSDLFpsHudAverageCandidateFrames() {
    static U32 value;
    return value;
}

static inline U32& kSDLFpsHudAverageCounter() {
    static U32 value;
    return value;
}

static inline void kSDLFpsHudRecordSameCandidate(bool unchanged) {
    kSDLFpsHudSameCandidateFrames()++;
    if (unchanged) {
        kSDLFpsHudUnchangedFrames()++;
    }
}

static inline bool kSDLFpsHudNeedsRefresh() {
    U64 lastRenderTime = kSDLFpsHudLastRenderTime();
    return KSystem::showFPS && lastRenderTime && KSystem::getMicroCounter() - lastRenderTime >= 1000000;
}

static inline void kSDLFpsHudRecordFrameForConsole(bool presented) {
#ifdef __EMSCRIPTEN__
    if (!KSystem::showFPS) {
        return;
    }

    U64 now = KSystem::getMicroCounter();
    if (!kSDLFpsHudAverageStartTime()) {
        kSDLFpsHudAverageStartTime() = now;
    }

    kSDLFpsHudAverageCandidateFrames()++;
    if (presented) {
        kSDLFpsHudAveragePresentedFrames()++;
    }

    U64 elapsed = now - kSDLFpsHudAverageStartTime();
    if (elapsed >= 20000000) {
        U32 counter = ++kSDLFpsHudAverageCounter();
        U32 presentedFrames = kSDLFpsHudAveragePresentedFrames();
        U32 candidateFrames = kSDLFpsHudAverageCandidateFrames();
        double averageFPS = (double)presentedFrames * 1000000.0 / (double)elapsed;
        EM_ASM({
            console.log("[BoxedWine] X11 average FPS sample=" + $0 + " fps=" + $1.toFixed(2) + " presentedFrames=" + $2 + " candidateFrames=" + $3);
        }, counter, averageFPS, presentedFrames, candidateFrames);

        kSDLFpsHudAverageStartTime() = now;
        kSDLFpsHudAveragePresentedFrames() = 0;
        kSDLFpsHudAverageCandidateFrames() = 0;
    }
#endif
}

static inline int kSDLFpsHudTextWidth(const char* text, int scale) {
    int width = 0;
    for (const char* p = text; *p; ++p) {
        width += (*p == ' ') ? 2 * scale : 4 * scale;
    }
    return width ? width - scale : 0;
}

static inline void kSDLFpsHudDrawGlyph(SDL_Renderer* renderer, const U8* glyph, int x, int y, int scale) {
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 3; ++col) {
            if (glyph[row] & (1 << (2 - col))) {
                SDL_Rect rect = {x + col * scale, y + row * scale, scale, scale};
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

static inline void kSDLFpsHudDrawText(SDL_Renderer* renderer, const char* text, int x, int y, int scale) {
    int cursor = x;
    for (const char* p = text; *p; ++p) {
        if (*p == ' ') {
            cursor += 2 * scale;
            continue;
        }
        const U8* glyph = kSDLFpsHudGlyph(*p);
        if (glyph) {
            kSDLFpsHudDrawGlyph(renderer, glyph, cursor, y, scale);
        }
        cursor += 4 * scale;
    }
}

static inline void kSDLFpsHudRender(SDL_Renderer* renderer, bool showSamePercent = false) {
    if (!KSystem::showFPS || !renderer) {
        return;
    }

    static U64 lastUpdateTime;
    static U32 presentedFrames;
    static U32 displayedFPS;

    U64 now = KSystem::getMicroCounter();
    if (!lastUpdateTime) {
        lastUpdateTime = now;
    }
    presentedFrames++;
    U64 elapsed = now - lastUpdateTime;
    if (elapsed >= 1000000) {
        displayedFPS = (U32)((presentedFrames * 1000000ULL + elapsed / 2) / elapsed);
        presentedFrames = 0;
        lastUpdateTime = now;

        U32 sameCandidates = kSDLFpsHudSameCandidateFrames();
        if (sameCandidates) {
            kSDLFpsHudDisplayedSamePercent() = kSDLFpsHudUnchangedFrames() * 100 / sameCandidates;
        } else {
            kSDLFpsHudDisplayedSamePercent() = 0;
        }
        kSDLFpsHudSameCandidateFrames() = 0;
        kSDLFpsHudUnchangedFrames() = 0;
    }
    kSDLFpsHudLastRenderTime() = now;

    char fpsText[16];
    snprintf(fpsText, sizeof(fpsText), "FPS %u", displayedFPS);

    char sameText[16];
    snprintf(sameText, sizeof(sameText), "SAME %u%%", kSDLFpsHudDisplayedSamePercent());

    int outputWidth = 0;
    int outputHeight = 0;
    SDL_GetRendererOutputSize(renderer, &outputWidth, &outputHeight);
    if (outputWidth <= 0 || outputHeight <= 0) {
        return;
    }

    int scale = outputWidth < 400 ? 2 : 3;
    int padding = scale * 2;
    int fpsTextWidth = kSDLFpsHudTextWidth(fpsText, scale);
    int sameTextWidth = showSamePercent ? kSDLFpsHudTextWidth(sameText, scale) : 0;
    int textWidth = sameTextWidth > fpsTextWidth ? sameTextWidth : fpsTextWidth;
    int textHeight = showSamePercent ? 11 * scale : 5 * scale;
    int x = outputWidth - textWidth - padding * 2;
    int y = padding;

    Uint8 oldR = 0;
    Uint8 oldG = 0;
    Uint8 oldB = 0;
    Uint8 oldA = 0;
    SDL_BlendMode oldBlendMode = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawColor(renderer, &oldR, &oldG, &oldB, &oldA);
    SDL_GetRenderDrawBlendMode(renderer, &oldBlendMode);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect bg = {x - padding, y - padding, textWidth + padding * 2, textHeight + padding * 2};
    SDL_RenderFillRect(renderer, &bg);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    kSDLFpsHudDrawText(renderer, fpsText, x, y, scale);
    if (showSamePercent) {
        kSDLFpsHudDrawText(renderer, sameText, x, y + 6 * scale, scale);
    }

    SDL_SetRenderDrawBlendMode(renderer, oldBlendMode);
    SDL_SetRenderDrawColor(renderer, oldR, oldG, oldB, oldA);
}

#endif
