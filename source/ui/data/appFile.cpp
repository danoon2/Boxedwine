#include "boxedwine.h"
#include "../boxedwineui.h"

AppFile::AppFile(BString name, BString installType, BString iconPath, BString filePath, U32 size, BString exe, BString exeOptions, BString help, BString optionsName, BString installOptions, BString installExe, const std::vector<BString>& args) : name(name), optionsName(optionsName), installType(installType), filePath(filePath), iconPath(iconPath), size(size), exe(exe), args(args), installExe(installExe), help(help), iconTexture(NULL) {
    if (iconPath.length()) {
        int pos = iconPath.lastIndexOf('/');
        if (pos != -1) {
            localIconPath = GlobalSettings::getDemoFolder() ^ iconPath.substr(pos + 1);
        }
    }
    if (filePath.length()) {
        int pos = filePath.lastIndexOf('/');
        if (pos != -1) {
            localFilePath = GlobalSettings::getDemoFolder() ^ filePath.substr(pos + 1);
        }
    }

    exeOptions.split(',', this->exeOptions);
    for (auto& s : this->exeOptions) {
        s = s.trim();
    }
    installOptions.split(',', this->installOptions);
    for (auto& s : this->installOptions) {
        s = s.trim();
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

void AppFile::runOptions(BoxedContainer* container, BoxedApp* app, const std::vector<BString>& options, std::list< std::function<bool() > >& runner, std::list<AppFile*>& downloads) {
    // :TODO: find dependencies, if there are some then delay the other changes
    bool hasContainerOption = false;
    BString containerDir = container->getDir();

    for (auto& option : options) {
        BoxedWinVersion* ver = BoxedwineData::getWinVersionFromName(option);
        if (ver) {
            container->setWindowsVersion(*ver);
            hasContainerOption = true;
        } else if (option=="GDI") {
            container->setGDI(true);
            container->setRenderer(B("gdi"));
            hasContainerOption = true;
        } else if (option.startsWith("glext=")) {
            if (app) {
                app->glExt = option.substr(6);
            }
        } else if (option=="16bpp") {
            if (app) {
                app->bpp = 16;
            }
        } else if (option.startsWith("cpuAffinity=")) {
            if (app) {
                BString s = option.substr(12);
                app->cpuAffinity = atoi(s.c_str());
            }
        } else if (option.startsWith("wine=")) {
            std::shared_ptr<FileSystemZip> wineVer = GlobalSettings::getInstalledFileSystemFromName(option.substr(5), true);
            if (wineVer) {
                container->setFileSystem(wineVer);
                hasContainerOption = true;
            } else {
                std::shared_ptr<FileSystemZip> availableWineVer = GlobalSettings::getAvailableFileSystemFromName(option.substr(5), true);
                if (availableWineVer) {
                    GlobalSettings::downloadFileSystem(availableWineVer, [container, availableWineVer](bool success) {
                        container->setFileSystem(availableWineVer);
                        container->saveContainer();
                        }
                    );
                }
            }
        } else if (option.startsWith("resolution=")) {
            if (app) {
                app->resolution = option.substr(11);
            }
        } else {
            AppFile* component = GlobalSettings::getComponentByOptionName(option);
            if (component) {
                if (!Fs::doesNativePathExist(component->localFilePath)) {
                    downloads.push_back(component);
                }
                // :TODO: delay this if a wine version is specified and needs downloading
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
        BString containerFilePath = GlobalSettings::createUniqueContainerPath(this->name);
        container = BoxedContainer::createContainer(containerFilePath, this->name, GlobalSettings::getFileSystemVersions().front());
        BoxedwineData::addContainer(container);
        container->saveContainer();
    }
    runOptions(container, nullptr, installOptions, runner, downloads);

    BString containerDir = container->getDir();
    BString cmd = this->exe;
    BString appName = this->name;
    std::vector<BString> exeOptions = this->exeOptions;
    std::vector<BString> args = this->args;
    BString appPath = localFilePath;
    BString mountPath;

    if (appPath.endsWith("zip", true)) {
        if (this->installType == "Installer") {
            mountPath = appPath;
            appPath = this->installExe;
        } else {
            BString root = GlobalSettings::getRootFolder(container);
            BString fileName = Fs::getFileNameFromNativePath(appPath);
            fileName = fileName.substr(0, fileName.length() - 4);
            BString path = root ^ "home" ^ "username" ^ ".wine" ^ "drive_c" ^ fileName;
            if (!Fs::doesNativePathExist(path)) {
                if (!Fs::makeNativeDirs(path)) {
                    BString errorMsg = getTranslationWithFormat(Msg::INSTALLVIEW_ERROR_FAILED_TO_CREATE_TEMP_DIR, true, path, BString::copy(strerror(errno)));
                    runOnMainUI([errorMsg]() {
                        new OkDlg(Msg::GENERIC_DLG_ERROR_TITLE, errorMsg, nullptr, 500, 300);
                        return false;
                        });
                    return;
                }
            }
            BString destDir = path;
            std::function<bool() > unzip = [containerDir, appPath, appName, destDir]() {
                runOnMainUI([appPath, destDir, appName]() {
                    new UnzipDlg(Msg::UNZIP_DLG_TITLE, appName, appPath, destDir, [](bool success) {
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
            appPath = destDir ^ this->installExe;
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
                    GlobalSettings::startUpArgs.mountInfo.push_back(MountInfo(B("/mnt/demo"), mountPath, false));
                    GlobalSettings::startUpArgs.setWorkingDir(B("/mnt/demo"));
                    GlobalSettings::startUpArgs.addArg(B("/bin/wine"));
                    GlobalSettings::startUpArgs.addArg("/mnt/demo/" + appPath);
                } else {
                    GlobalSettings::startUpArgs.addArg(appPath);
                }
                GlobalSettings::startUpArgs.readyToLaunch = true;

                runOnMainUI([appName]() {
                    new WaitDlg(Msg::WAITDLG_LAUNCH_APP_TITLE, getTranslationWithFormat(Msg::WAITDLG_LAUNCH_APP_LABEL, true, appName));
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
                    runOptions(container, nullptr, exeOptions, r, d);
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
