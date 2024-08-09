#include "boxedwine.h"
#include "x11.h"
#include "knativesystem.h"
#include "knativewindow.h"

class XrrData {
public:
    U32 sizesAddress = 0; // XRRScreenSize*
    U32 sizesCount = 0;
    U32 ratesAddress = 0;
};

U32 XrrConfigCurrentRate() {
    return KNativeWindow::getNativeWindow()->screenRate();
}

U32 XrrRates(KThread* thread, U32 screen, U32 sizeIndex, U32 rateCountAddress) {
    KMemory* memory = thread->memory;
    XServer* server = XServer::getServer();
    XrrData* data = server->xrrData;

    memory->writed(rateCountAddress, 1);
    if (data->ratesAddress) {
        klog("rateCountAddress = %x rateAddress = %x", rateCountAddress, data->ratesAddress);
        return data->ratesAddress;
    }
    data->ratesAddress = thread->process->alloc(thread, 2);
    memory->writew(data->ratesAddress, XrrConfigCurrentRate());
    klog("rateCountAddress = %x rateAddress = %x", rateCountAddress, data->ratesAddress);
    return data->ratesAddress;
}

U32 XrrConfigCurrentConfiguration(KThread* thread, U32 screen, U32 rotationAddress) {
    KMemory* memory = thread->memory;
    XServer* server = XServer::getServer();
    XrrData* data = server->xrrData;

    if (!data || !data->sizesAddress) {
        XrrGetSizes(thread, 0, 0);
        data = server->xrrData;
    }
    U32 cx = KNativeWindow::getNativeWindow()->screenWidth();
    U32 cy = KNativeWindow::getNativeWindow()->screenHeight();

    memory->writed(rotationAddress, 0);
    for (U32 i = 0; i < data->sizesCount; i++) {
        if (memory->readd(data->sizesAddress + i * 16) == cx && memory->readd(data->sizesAddress + i * 16 + 4) == cy) {
            return i;
        }
    }
    return 0;
}

U32 XrrGetSizes(KThread* thread, U32 screen, U32 countAddress) {
    KMemory* memory = thread->memory;
    XServer* server = XServer::getServer();

    if (!server->xrrData) {
        server->xrrData = new XrrData();
    }
    if (server->xrrData->sizesAddress) {
        if (countAddress) {
            memory->writed(countAddress, server->xrrData->sizesCount);
        }
        return server->xrrData->sizesAddress;
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
    server->xrrData->sizesAddress = sizes;
    server->xrrData->sizesCount = count;
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
        memory->writed(countAddress, server->xrrData->sizesCount);
    }
    return server->xrrData->sizesAddress;
}