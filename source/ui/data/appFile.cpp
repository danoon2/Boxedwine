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
#include "../boxedwineui.h"

AppFile::AppFile(BString name, BString installType, BString iconPath, BString filePath, U32 size, BString exe, BString exeOptions, BString help, BString optionsName, BString installOptions, BString installExe, const std::vector<BString>& args) : name(name), optionsName(optionsName), installType(installType), filePath(filePath), iconPath(iconPath), size(size), exe(exe), args(args), installExe(installExe), help(help), iconTexture(NULL) {
    if (iconPath.length()) {
        int pos = iconPath.lastIndexOf('/');
        if (pos != -1) {
            localIconPath = GlobalSettings::getDemoFolder().stringByApppendingPath(iconPath.substr(pos + 1));
        }
    }
    if (filePath.length()) {
        int pos = filePath.lastIndexOf('/');
        if (pos != -1) {
            localFilePath = GlobalSettings::getDemoFolder().stringByApppendingPath(filePath.substr(pos + 1));
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

void AppFile::runOptions(BoxedContainer* container, BoxedApp* app, const std::vector<BString>& options, std::list< std::function<bool() > >& runner, std::list<AppDownloadTask>& downloads) {
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
        } else if (option == "DDraw") {
            if (app) {
                app->ddrawOverride = true;
            }
        } else if (option == "DXVK=") {
            if (app) {
                app->ddrawOverride = option.substr(5).startsWith('t', true);
            }
        } else if (option == "DisableHideCursor") {
            if (app) {
                app->disableHideCursor = true;
            }
        } else if (option == "ForceRelativeMouse") {
            if (app) {
                app->forceRelativeMouse = true;
            }
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
            continue; // should have already been handled
        } else if (option.startsWith("resolution=")) {
            if (app) {
                app->resolution = option.substr(11);
            }
        } else if (option=="macNativeGL") {
#ifdef __MACH__
            if (app) {
                app->openGlType = OPENGL_TYPE_NATIVE;
            }
#endif
        } else {
            AppFilePtr component = GlobalSettings::getComponentByOptionName(option);
            if (component) {
                if (component->localFilePath.length() && !Fs::doesNativePathExist(component->localFilePath)) {
                    downloads.push_back(AppDownloadTask(component));
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

void downloadNext(std::list< std::function<bool() > > runner, std::list<AppDownloadTask> downloads) {
    while (downloads.size()) {
        AppDownloadTask task = downloads.front();
        downloads.pop_front();
        if (task.app) {
            if (!Fs::doesNativePathExist(task.app->localFilePath)) {
                GlobalSettings::downloadFile(task.app->filePath, task.app->localFilePath, task.app->name, task.app->size, [runner, downloads](bool sucess) {
                    downloadNext(runner, downloads);
                    });
                return;
            }
        } else if (task.fs) {
            if (!GlobalSettings::getInstalledFileSystemFromName(task.fs->name, true)) {
                GlobalSettings::downloadFileSystem(task.fs, [runner, downloads](bool success) {
                    downloadNext(runner, downloads);
                    });
                return;
            }
        }
    }
    runApps(runner);
}

void AppFile::install(bool chooseShortCut, BoxedContainer* container, bool alreadyCheckedWineOption) {
    std::list<AppDownloadTask> downloads;
    globalRunner.clear();
    install(chooseShortCut, container, globalRunner, downloads, alreadyCheckedWineOption);
    if (downloads.size()) {    
        downloadNext(globalRunner, downloads);
    } else {
        runApps(globalRunner);
    }
}

BString AppFile::getFileSystemInstallOption() {
    for (auto& option : installOptions) {
        if (option.startsWith("wine=")) {
            return option.substr(5);
        }
    }
    return BString::empty;
}

void AppFile::install(bool chooseShortCut, BoxedContainer* container, std::list< std::function<bool() > >& runner, std::list<AppDownloadTask>& downloads, bool alreadyCheckedWineOption) {
    if (!container) {        
        std::shared_ptr<FileSystemZip> fs;

        BString wineOption = getFileSystemInstallOption();
        if (!wineOption.isEmpty()) {
            fs = GlobalSettings::getInstalledFileSystemFromName(wineOption, true);
            if (!fs && !alreadyCheckedWineOption) {
                BString wineOption = getFileSystemInstallOption();
                if (!wineOption.isEmpty()) {
                    fs = GlobalSettings::getInstalledFileSystemFromName(wineOption, true);
                    if (!fs) {
                        std::shared_ptr<FileSystemZip> availableWineVer = GlobalSettings::getAvailableFileSystemFromName(wineOption, true);
                        if (availableWineVer) {
                            downloads.push_back(AppDownloadTask(availableWineVer));
                            std::function<bool() > restartInstall = [chooseShortCut, container, &runner, &downloads, this]() {
                                install(chooseShortCut, container, true);
                                return true;
                                };
                            runner.push_back(restartInstall);
                            return;
                        }
                    }
                }
            }
        }
        if (!fs) {
            fs = GlobalSettings::getFileSystemVersions().front();
        }
        BString containerFilePath = GlobalSettings::createUniqueContainerPath(this->name);
        container = BoxedContainer::createContainer(containerFilePath, this->name, fs);
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
            BString sep = BString::pathSeparator();
            BString path = root.stringByApppendingPath("home") + sep + "username" + sep + ".wine" + sep + "drive_c" + sep + fileName;
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
            appPath = destDir.stringByApppendingPath(this->installExe);
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
                GlobalSettings::setOpenGlTypeOnStartupArgs(GlobalSettings::getDefaultOpenGL());
                container->launch();

                if (GlobalSettings::isAutomationEnabled()) {
                    BString path = GlobalSettings::getAutomationFolder(container);
                    if (!Fs::doesNativePathExist(path)) {
                        Fs::makeNativeDirs(path);
                    }
                    GlobalSettings::startUpArgs.recordAutomation = path;
                }                
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
                    std::list< std::function<bool() > > runTasks; // for now, post install cannot run anything
                    std::list<AppDownloadTask> downloadTasks;
                    for (auto& app : items) {
                        if (app.getCmd() == cmd) {
                            app.setName(appName);
                            app.setArgs(args);
                            runOptions(container, &app, exeOptions, runTasks, downloadTasks);
                            app.saveApp();
                            app.getContainer()->reload();
                            GlobalSettings::reloadApps();
                            return false;;
                        }
                    }
                    runOptions(container, nullptr, exeOptions, runTasks, downloadTasks);
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
