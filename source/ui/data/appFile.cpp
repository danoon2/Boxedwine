#include "boxedwine.h"
#include "../boxedwineui.h"

AppFile::AppFile(const std::string& name, const std::string& installType, const std::string& iconPath, const std::string& filePath, U32 size, const std::string& exe, const std::string& exeOptions, const std::string& help, const std::string& optionsName, const std::string& installOptions, const std::string& installExe, const std::vector<std::string>& args) : name(name), optionsName(optionsName), installType(installType), filePath(filePath), iconPath(iconPath), size(size), exe(exe), args(args), installExe(installExe), help(help), iconTexture(NULL) {
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

void AppFile::runOptions(BoxedContainer* container, BoxedApp* app, const std::vector<std::string>& options, std::list< std::function<bool() > >& runner, std::list<AppFile*>& downloads) {
    // :TODO: find dependencies, if there are some then delay the other changes
    bool hasContainerOption = false;
    std::string containerDir = container->getDir();

    for (auto& option : options) {
        BoxedWinVersion* ver = BoxedwineData::getWinVersionFromName(option);
        if (ver) {
            container->setWindowsVersion(*ver);
            hasContainerOption = true;
        } else if (option=="GDI") {
            container->setGDI(true);
            container->setRenderer("gdi");
            hasContainerOption = true;
        } else if (stringStartsWith(option, "glext=")) {
            if (app) {
                app->glExt = option.substr(6);
            }
        } else if (option=="16bpp") {
            if (app) {
                app->bpp = 16;
            }
        } else if (stringStartsWith(option, "cpuAffinity=")) {
            if (app) {
                std::string s = option.substr(12);
                app->cpuAffinity = atoi(s.c_str());
            }
        } else if (stringStartsWith(option, "wine=")) {
            WineVersion* wineVer = GlobalSettings::getAvailableWineFromName(option.substr(5));
            if (wineVer) {
                container->setWineVersion(wineVer->name);
                hasContainerOption = true;
            }
        } else if (stringStartsWith(option, "resolution=")) {
            if (app) {
                app->resolution = option.substr(11);
            }
        } else {
            AppFile* component = GlobalSettings::getComponentByOptionName(option);
            if (component) {
                if (!Fs::doesNativePathExist(component->localFilePath)) {
                    downloads.push_back(component);
                }
                component->install(false, container, runner, downloads);
            }
        }
    }
    if (hasContainerOption) {
        container->saveContainer();
    }
}

static std::list< std::function<bool() > > globalRunner;

void runApps(std::list< std::function<bool() > > runner) {
    GlobalSettings::startUpArgs = StartUpArgs();
    while (runner.size() > 0) {
        std::function<bool()> next = runner.front();        
        runner.pop_front();
        globalRunner = runner;
        bool result = next();
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
    std::list<AppFile*> downloads;
    globalRunner.clear();
    install(chooseShortCut, container, globalRunner, downloads);
    if (downloads.size()) {    
        downloadNext(globalRunner, downloads);
    } else {
        runApps(globalRunner);
    }
}

void AppFile::install(bool chooseShortCut, BoxedContainer* container, std::list< std::function<bool() > >& runner, std::list<AppFile*>& downloads) {
    if (!container) {
        std::string containerFilePath = GlobalSettings::createUniqueContainerPath(this->name);
        container = BoxedContainer::createContainer(containerFilePath, this->name, GlobalSettings::getWineVersions()[0].name);
        BoxedwineData::addContainer(container);
        container->saveContainer();
    }
    runOptions(container, NULL, installOptions, runner, downloads);

    std::string containerDir = container->getDir();
    std::string cmd = this->exe;
    std::string appName = this->name;
    std::vector<std::string> exeOptions = this->exeOptions;
    std::vector<std::string> args = this->args;
    std::string appPath = localFilePath;
    std::string mountPath;

    if (stringHasEnding(appPath, "zip", true)) {
        if (this->installType == "Installer") {
            mountPath = appPath;
            appPath = this->installExe;
        } else {
            std::string root = GlobalSettings::getRootFolder(container);
            std::string fileName = Fs::getFileNameFromNativePath(appPath);
            fileName = fileName.substr(0, fileName.length() - 4);
            std::string path = root + Fs::nativePathSeperator + "home" + Fs::nativePathSeperator + "username" + Fs::nativePathSeperator + ".wine" + Fs::nativePathSeperator + "drive_c" + Fs::nativePathSeperator + fileName;
            if (!Fs::doesNativePathExist(path)) {
                if (!Fs::makeNativeDirs(path)) {
                    std::string errorMsg = getTranslationWithFormat(INSTALLVIEW_ERROR_FAILED_TO_CREATE_TEMP_DIR, true, path, strerror(errno));
                    runOnMainUI([errorMsg]() {
                        new OkDlg(GENERIC_DLG_ERROR_TITLE, errorMsg, nullptr, 500, 300);
                        return false;
                        });
                    return;
                }
            }
            std::string destDir = path;
            std::function<bool() > unzip = [containerDir, appPath, appName, destDir]() {
                runOnMainUI([appPath, destDir, appName]() {
                    new UnzipDlg(UNZIP_DLG_TITLE, appName, appPath, destDir, [](bool success) {
                        if (success) {
                            runApps(globalRunner);
                        }
                        });
                    gotoView(VIEW_APPS);
                    return false;
                    });
                return false; // don't continue to the next part of the install until we are done unzipping
            };
            runner.push_back(unzip);
            appPath = destDir + Fs::nativePathSeperator + this->installExe;
        }
    }

    if (this->installType == "Installer") {
        std::function<bool() > runInstall = [containerDir, appPath, appName, mountPath]() {
            BoxedContainer* container = BoxedwineData::getContainerByDir(containerDir);
            if (container) {
                GlobalSettings::startUpArgs = StartUpArgs(); // reset parameters
                GlobalSettings::startUpArgs.setScale(GlobalSettings::getDefaultScale());
                GlobalSettings::startUpArgs.setVsync(GlobalSettings::getDefaultVsync());
                GlobalSettings::startUpArgs.setResolution(GlobalSettings::getDefaultResolution());
                container->launch();
                if (mountPath.length()) {
                    GlobalSettings::startUpArgs.mountInfo.push_back(MountInfo("/mnt/demo", mountPath, false));
                    GlobalSettings::startUpArgs.setWorkingDir("/mnt/demo");
                    GlobalSettings::startUpArgs.addArg("/bin/wine");
                    GlobalSettings::startUpArgs.addArg("/mnt/demo/" + appPath);
                } else {
                    GlobalSettings::startUpArgs.addArg(appPath);
                }
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
    }

    if (cmd.length() || exeOptions.size() || chooseShortCut) {
        std::function<bool() > runPostInstall = [args, appName, cmd, containerDir, chooseShortCut, exeOptions]() {
            runOnMainUI([args, appName, cmd, containerDir, chooseShortCut, exeOptions]() {
                BoxedContainer* container = BoxedwineData::getContainerByDir(containerDir);
                if (container) {
                    std::vector<BoxedApp> items;
                    container->getNewApps(items);
                    std::list< std::function<bool() > > r; // for now, post install cannot run anything
                    std::list<AppFile*> d;                    
                    for (auto& app : items) {
                        if (app.getCmd() == cmd) {
                            app.setName(appName);
                            app.setArgs(args);
                            runOptions(container, &app, exeOptions, r, d);
                            app.saveApp();
                            app.getContainer()->reload();
                            GlobalSettings::reloadApps();
                            return false;;
                        }
                    }
                    runOptions(container, NULL, exeOptions, r, d);
                    if (chooseShortCut) {
                        new AppChooserDlg(items, [container](BoxedApp app) {
                            gotoView(VIEW_CONTAINERS, container->getDir(), app.getIniFilePath());
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
