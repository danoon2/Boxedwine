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

    memory->writed(rateCountAddress, 1);
    if (data->ratesAddress) {
        return data->ratesAddress;
    }
    data->ratesAddress = thread->process->alloc(thread, 2);
    memory->writew(data->ratesAddress, XrrConfigCurrentRate());
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
    U32 desktopCx = 0;
    U32 desktopCy = 0;
    U32 count = 3;
    KNativeSystem::getScreenDimensions(&desktopCx, &desktopCy);

    if (desktopCx > 1600) {
        count++;
    }
    if (desktopCx && desktopCx > 1600 && desktopCy > 1200) {
        count++;
    }
    if (desktopCx && desktopCx > 1280 && desktopCy > 1024) {
        count++;
    }
    U32 sizes = thread->process->alloc(thread, sizeof(XRRScreenSize) * count);
    
    data->xrrData->sizesAddress = sizes;
    data->xrrData->sizesCount = count;
    if (desktopCx > 1600) {
        memory->writed(sizes, desktopCx); sizes += 4;
        memory->writed(sizes, desktopCy); sizes += 4;
        memory->writed(sizes, 0); sizes += 4;
        memory->writed(sizes, 0); sizes += 4;
    }
    if (desktopCx && desktopCx > 1600 && desktopCy > 1200) {
        memory->writed(sizes, 1600); sizes += 4;
        memory->writed(sizes, 1200); sizes += 4;
        memory->writed(sizes, 0); sizes += 4;
        memory->writed(sizes, 0); sizes += 4;
    }
    if (desktopCx && desktopCx > 1280 && desktopCy > 1024) {
        memory->writed(sizes, 1280); sizes += 4;
        memory->writed(sizes, 1024); sizes += 4;
        memory->writed(sizes, 0); sizes += 4;
        memory->writed(sizes, 0); sizes += 4;
    }
    memory->writed(sizes, 1024); sizes += 4;
    memory->writed(sizes, 768); sizes += 4;
    memory->writed(sizes, 0); sizes += 4;
    memory->writed(sizes, 0); sizes += 4;
    memory->writed(sizes, 800); sizes += 4;
    memory->writed(sizes, 600); sizes += 4;
    memory->writed(sizes, 0); sizes += 4;
    memory->writed(sizes, 0); sizes += 4;
    memory->writed(sizes, 640); sizes += 4;
    memory->writed(sizes, 480); sizes += 4;
    memory->writed(sizes, 0); sizes += 4;
    memory->writed(sizes, 0);
    if (countAddress) {
        memory->writed(countAddress, data->xrrData->sizesCount);
    }
    return data->xrrData->sizesAddress;
}
