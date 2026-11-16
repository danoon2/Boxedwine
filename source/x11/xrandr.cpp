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

#include "boxedwine.h"
#include "x11.h"
#include "knativesystem.h"

U32 XrrConfigCurrentRate() {
    return KNativeSystem::getScreen()->screenRate();
}

bool XrrGetSize(KThread* thread, const DisplayDataPtr& displayData, U32 sizeIndex, U32& cx, U32& cy) {
    KMemory* memory = thread->memory;
    XrrData* data = displayData->xrrData;

    if (!data || !data->sizesAddress) {
        XrrGetSizes(thread, displayData, 0, 0);
        data = displayData->xrrData;
    }
    if (sizeIndex >= data->sizesCount) {
        return false;
    }

    cx = memory->readd(data->sizesAddress + sizeIndex * 16);
    cy = memory->readd(data->sizesAddress + sizeIndex * 16 + 4);
    return true;
}

U32 XrrRates(KThread* thread, const DisplayDataPtr& displayData, U32 screen, U32 sizeIndex, U32 rateCountAddress) {
    KMemory* memory = thread->memory;
    XrrData* data = displayData->xrrData;

    if (!data) {
        XrrGetSizes(thread, displayData, screen, 0);
        data = displayData->xrrData;
    }
    if (!data->ratesAddress) {
        // Guest modes resize a virtual desktop, not the host monitor. Always
        // offer 60 Hz: older games such as The Sims 2 request it explicitly
        // and reject every adapter when only a high host refresh is listed.
        U32 hostRate = XrrConfigCurrentRate();
        data->ratesCount = hostRate && hostRate != 60 ? 2 : 1;
        data->ratesAddress = thread->process->alloc(thread, data->ratesCount * 2);
        memory->writew(data->ratesAddress, 60);
        if (data->ratesCount == 2) {
            memory->writew(data->ratesAddress + 2, hostRate);
        }
    }
    memory->writed(rateCountAddress, data->ratesCount);
    return data->ratesAddress;
}

U32 XrrConfigCurrentConfiguration(KThread* thread, const DisplayDataPtr& displayData, U32 rotationAddress) {
    KMemory* memory = thread->memory;
    XrrData* data = displayData->xrrData;

    if (!data || !data->sizesAddress) {
        XrrGetSizes(thread, displayData, 0, 0);
        data = displayData->xrrData;
    }
    KNativeScreenPtr screen = KNativeSystem::getScreen();
    U32 cx = screen->screenWidth();
    U32 cy = screen->screenHeight();

    memory->writed(rotationAddress, 0);
    for (U32 i = 0; i < data->sizesCount; i++) {
        if (memory->readd(data->sizesAddress + i * 16) == cx && memory->readd(data->sizesAddress + i * 16 + 4) == cy) {
            return i;
        }
    }
    return 0;
}

U32 XrrGetSizes(KThread* thread, const DisplayDataPtr& data, U32 screen, U32 countAddress) {
    KMemory* memory = thread->memory;

    if (!data->xrrData) {
        data->xrrData = new XrrData();
    }
    if (data->xrrData->sizesAddress) {
        if (countAddress) {
            memory->writed(countAddress, data->xrrData->sizesCount);
        }
        return data->xrrData->sizesAddress;
    }
    std::vector<std::pair<U32, U32>> resolutions;
    auto addResolution = [&resolutions](U32 width, U32 height) {
        if (!width || !height) {
            return;
        }
        for (const auto& size : resolutions) {
            if (size.first == width && size.second == height) {
                return;
            }
        }
        resolutions.emplace_back(width, height);
    };
#ifndef __EMSCRIPTEN__
    U32 desktopCx = 0;
    U32 desktopCy = 0;
    KNativeSystem::getScreenDimensions(&desktopCx, &desktopCy);
    if (desktopCx > 1600) {
        addResolution(desktopCx, desktopCy);
    }
    if (desktopCx && desktopCx > 1600 && desktopCy > 1200) {
        addResolution(1600, 1200);
    }
    if (desktopCx && desktopCx > 1280 && desktopCy > 1024) {
        addResolution(1280, 1024);
    }
#endif
    addResolution(1024, 768);
    addResolution(800, 600);
    addResolution(640, 480);
    // Keep the initial virtual desktop selectable even when it is a custom size.
    // The list is cached for this display, so later mode changes do not replace it.
    KNativeScreenPtr nativeScreen = KNativeSystem::getScreen();
    addResolution(nativeScreen->screenWidth(), nativeScreen->screenHeight());
    U32 count = (U32)resolutions.size();
    U32 sizes = thread->process->alloc(thread, sizeof(XRRScreenSize) * count);
    
    data->xrrData->sizesAddress = sizes;
    data->xrrData->sizesCount = count;
    for (const auto& size : resolutions) {
        memory->writed(sizes, size.first); sizes += 4;
        memory->writed(sizes, size.second); sizes += 4;
        memory->writed(sizes, 0); sizes += 4;
        memory->writed(sizes, 0); sizes += 4;
    }
    if (countAddress) {
        memory->writed(countAddress, data->xrrData->sizesCount);
    }
    return data->xrrData->sizesAddress;
}
