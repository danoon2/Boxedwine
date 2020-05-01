#include "boxedwine.h"
#include "../boxedwineui.h"

AppFile::AppFile(const std::string& name, const std::string& installType, const std::string& iconPath, const std::string& filePath, U32 size, const std::string& exe, const std::string& exeOptions, const std::string& help, const std::string& optionsName, const std::string& installOptions) : name(name), optionsName(optionsName), installType(installType), filePath(filePath), iconPath(iconPath), size(size), exe(exe), help(help), iconTexture(NULL) {
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

    stringSplit(this->exeOptions, exeOptions, ',');
    for (auto& s : this->exeOptions) {
        stringTrim(s);
    }
    stringSplit(this->installOptions, installOptions, ',');
    for (auto& s : this->installOptions) {
        stringTrim(s);
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

void AppFile::runOptions(BoxedContainer* container, const std::vector<std::string>& options, std::list< std::function<bool() > >& runner, std::list<AppFile*>& downloads) {
    // :TODO: find dependencies, if there are some then delay the other changes
    std::string winVersion;
    bool gdi = false;
    bool hasContainerOption = false;
    std::string containerDir = container->getDir();

    for (auto& option : options) {
        BoxedWinVersion* ver = BoxedwineData::getWinVersionFromName(option);
        if (ver) {
            winVersion = option;
            hasContainerOption = true;
        } else if (vectorContainsIgnoreCase(options, "GDI")) {
            gdi = true;
            hasContainerOption = true;
        } else {
            AppFile* app = GlobalSettings::getComponentByOptionName(option);
            if (app) {
                if (!Fs::doesNativePathExist(app->localFilePath)) {
                    downloads.push_back(app);
                }
                app->install(false, container, runner, downloads);
            }
        }
    }
    if (hasContainerOption) {
        runner.push_back([winVersion, gdi, containerDir]() {
            BoxedContainer* container = BoxedwineData::getContainerByDir(containerDir);
            if (container) {
                if (gdi) {
                    container->setGDI(true);
                }
                if (winVersion.length()) {
                    BoxedWinVersion* ver = BoxedwineData::getWinVersionFromName(winVersion);
                    if (ver) {
                        container->setWindowsVersion(*ver);
                    }
                }
                container->saveContainer();
                return true;
            }            
            return false;
            });
    }
}

void runApps(std::list< std::function<bool() > > runner) {
    GlobalSettings::startUpArgs = StartUpArgs();
    while (runner.size() > 0) {
        bool result = runner.front()();
        runner.pop_front();
        if (!result) {
            return;
        }
        if (runner.size()) {            
            if (GlobalSettings::startUpArgs.runOnRestartUI) {
                kpanic("runApps runOnRestartUI should not be set");
            }
            if (GlobalSettings::startUpArgs.readyToLaunch) {
                GlobalSettings::startUpArgs.runOnRestartUI = [runner]() {
                    runApps(runner);
                };
                break;
            }
        }
    }
}

void downloadNext(std::list< std::function<bool() > > runner, std::list<AppFile*> downloads) {
    while (downloads.size()) {
        AppFile* app = downloads.front();
        downloads.pop_front();
        if (!Fs::doesNativePathExist(app->localFilePath)) {
            GlobalSettings::downloadFile(app->filePath, app->localFilePath, app->name, app->size, [runner, downloads](bool sucess) {
                downloadNext(runner, downloads);
                });
            return;
        }
    }
    runApps(runner);
}

void AppFile::install(bool chooseShortCut, BoxedContainer* container) {
    std::list< std::function<bool() > > runner;
    std::list<AppFile*> downloads;
    install(chooseShortCut, container, runner, downloads);
    if (downloads.size()) {    
        downloadNext(runner, downloads);
    } else {
        runApps(runner);
    }
}

void AppFile::install(bool chooseShortCut, BoxedContainer* container, std::list< std::function<bool() > >& runner, std::list<AppFile*>& downloads) {
    if (!container) {
        std::string containerFilePath = GlobalSettings::createUniqueContainerPath(this->name);
        container = BoxedContainer::createContainer(containerFilePath, this->name, GlobalSettings::getWineVersions()[0].name);
        BoxedwineData::addContainer(container);
        container->saveContainer();
    }
    runOptions(container, installOptions, runner, downloads);

    std::string containerDir = container->getDir();
    std::string cmd = this->exe;
    std::string appName = this->name;
    std::vector<std::string> exeOptions = this->exeOptions;
    std::string appPath = localFilePath;

    std::function<bool() > runInstall = [containerDir, appPath, appName]() {
        BoxedContainer* container = BoxedwineData::getContainerByDir(containerDir);
        if (container) {
            GlobalSettings::startUpArgs = StartUpArgs(); // reset parameters
            GlobalSettings::startUpArgs.setScale(GlobalSettings::getDefaultScale());
            GlobalSettings::startUpArgs.setResolution(GlobalSettings::getDefaultResolution());
            container->launch();
            GlobalSettings::startUpArgs.addArg(appPath);
            GlobalSettings::startUpArgs.readyToLaunch = true;

            runOnMainUI([appName]() {
                new WaitDlg(WAITDLG_LAUNCH_APP_TITLE, getTranslationWithFormat(WAITDLG_LAUNCH_APP_LABEL, true, appName.c_str()));
                return false;
                });
            return true;
        }
        return false;
    };
    runner.push_back(runInstall);

    if (cmd.length() || exeOptions.size() || chooseShortCut) {
        std::function<bool() > runPostInstall = [appName, cmd, containerDir, chooseShortCut, exeOptions]() {
            runOnMainUI([appName, cmd, containerDir, chooseShortCut, exeOptions]() {
                BoxedContainer* container = BoxedwineData::getContainerByDir(containerDir);
                if (container) {
                    std::vector<BoxedApp> items;
                    container->getNewApps(items);
                    std::list< std::function<bool() > > r; // for now, post install cannot run anything
                    std::list<AppFile*> d;
                    runOptions(container, exeOptions, r, d);
                    for (auto& app : items) {
                        if (app.getCmd() == cmd) {
                            app.setName(appName);
                            app.saveApp();
                            app.getContainer()->reload();
                            GlobalSettings::reloadApps();
                            return false;;
                        }
                    }
                    if (chooseShortCut) {
                        new AppChooserDlg(items, [container](BoxedApp* app) {
                            gotoView(VIEW_CONTAINERS, container->getDir(), app->getIniFilePath());
                            });
                    }
                }
                return false;
                });
            return true;
        };
        runner.push_back(runPostInstall);
    }    
}
