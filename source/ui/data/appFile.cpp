#include "boxedwine.h"
#include "../boxedwineui.h"

AppFile::AppFile(const std::string& name, const std::string& installType, const std::string& iconPath, const std::string& filePath, U32 size, const std::string& exe, const std::string& exeOptions, const std::string& help) : name(name), installType(installType), filePath(filePath), iconPath(iconPath), size(size), exe(exe), exeOptions(exeOptions), help(help), iconTexture(NULL) {
    if (iconPath.length()) {
        size_t pos = iconPath.rfind("/");
        if (pos != std::string::npos) {
            localIconPath = GlobalSettings::getDemoFolder() + Fs::nativePathSeperator + iconPath.substr(pos + 1);
        }
    }
    if (filePath.length()) {
        size_t pos = filePath.rfind("/");
        if (pos != std::string::npos) {
            localFilePath = GlobalSettings::getDemoFolder() + Fs::nativePathSeperator + filePath.substr(pos + 1);
        }
    }
}

void AppFile::buildIconTexture() {
    if (Fs::doesNativePathExist(localIconPath)) {
        int w = 0, h = 0;
        unsigned char* data = LoadImageFromFile(localIconPath.c_str(), &w, &h);
        if (data) {
            this->iconTexture = std::make_shared<BoxedTexture>([data, w, h]() {
                return MakeRGBATexture(data, w, h);
                });
        }
            
    }
}

void AppFile::install() {

    GlobalSettings::startUpArgs = StartUpArgs(); // reset parameters
    GlobalSettings::startUpArgs.setScale(GlobalSettings::getDefaultScale());
    GlobalSettings::startUpArgs.setResolution(GlobalSettings::getDefaultResolution());
    std::string containerFilePath = GlobalSettings::createUniqueContainerPath(this->name);
    BoxedContainer* container = BoxedContainer::createContainer(containerFilePath, this->name, GlobalSettings::getWineVersions()[0].name);
    BoxedwineData::addContainer(container);
    container->launch();
    GlobalSettings::startUpArgs.addArg(localFilePath);
    GlobalSettings::startUpArgs.readyToLaunch = true;
    container->saveContainer();

    runOnMainUI([this]() {
        new WaitDlg(WAITDLG_LAUNCH_APP_TITLE, getTranslationWithFormat(WAITDLG_LAUNCH_APP_LABEL, true, name.c_str()));
        return false;
        });

    std::string containerDir = container->getDir();
    std::string cmd = this->exe;
    std::string exeOptions = this->exeOptions;
    std::string appName = this->name;
    GlobalSettings::startUpArgs.runOnRestartUI = [appName, cmd, exeOptions, containerDir]() {
        runOnMainUI([appName, cmd, exeOptions, containerDir]() {
            BoxedContainer* container = BoxedwineData::getContainerByDir(containerDir);
            if (container) {
                std::vector<BoxedApp> items;
                container->getNewApps(items);
                for (auto& app : items) {
                    if (app.getCmd() == cmd) {
                        if (exeOptions.length()) {
                            std::vector<std::string> parts;
                            stringSplit(parts, exeOptions, ',');
                            for (auto& s : parts) {
                                stringTrim(s);
                                if (s == "GDI") {
                                    container->setGDI(true);
                                }
                            }
                            container->saveContainer();
                        }
                        app.setName(appName);
                        app.saveApp();
                        app.getContainer()->reload();
                        GlobalSettings::reloadApps();
                        return false;;
                    }
                }
                new AppChooserDlg(items, [container](BoxedApp* app) {
                    gotoView(VIEW_CONTAINERS, container->getDir(), app->getIniFilePath());
                    });
            }
            return false;
            });
    };
}
