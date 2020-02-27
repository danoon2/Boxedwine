#include "boxedwine.h"
#include "../boxedwineui.h"

std::vector<BoxedContainer*> BoxedwineData::containers;

void BoxedwineData::init(int argc, const char **argv) {
    GlobalSettings::init(argc, argv);
}

void BoxedwineData::startApp() {
    GlobalSettings::startUpArgs.apply();
}

void BoxedwineData::loadContainers() {
     for (auto &container : BoxedwineData::containers) {
        delete container;
    }
    BoxedwineData::containers.clear();
    Fs::iterateAllNativeFiles(GlobalSettings::getContainerFolder(), false, true, [] (const std::string& filepath, bool isDir)->U32 {
        if (isDir) {
            BoxedContainer* container = new BoxedContainer();
            if (container->load(filepath)) {
                BoxedwineData::containers.push_back(container);
            } else {
                delete container;
            }
        }
        return 0;
    });
}

void BoxedwineData::loadUI() {
    loadContainers();
}