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
#include "../../../lib/imgui/addon/imguitinyfiledialogs.h"

OptionsView::OptionsView(BString tab) : BaseView(B("OptionsView")) {
    if (tab.length()) {
        if (tab == "Wine") {
            this->tabIndex = 2;
        } if (tab == "Display") {
            this->tabIndex = 1;
        }

    }
    this->wineTitle = c_getTranslation(Msg::OPTIONSVIEW_TITLE_FILESYSTEM);

    createGeneralTab();
    createThemeTab();
    
    BString name;
    if (GlobalSettings::hasIconsFont()) {
        name += OPTION_WINE_ICON;
        name += " ";
    }
    name += this->wineTitle;
    addTab(name, name, NULL, [this](bool buttonPressed, BaseViewTab& tab) {
        this->runWineOptions();
        });    

    loadFileSystemVersions();
}

void OptionsView::createThemeTab() {
    std::shared_ptr<ImGuiLayout> model = std::make_shared<ImGuiLayout>();
    std::shared_ptr<LayoutSection> section = model->addSection(Msg::OPTIONSVIEW_TITLE_DISPLAY);
    
    std::vector<ComboboxItem> themes;
    themes.push_back(ComboboxItem(getTranslation(Msg::OPTIONSVIEW_THEME_DARK), B("Dark")));
    themes.push_back(ComboboxItem(getTranslation(Msg::OPTIONSVIEW_THEME_LIGHT), B("Light")));
    themes.push_back(ComboboxItem(getTranslation(Msg::OPTIONSVIEW_THEME_CLASSIC), B("Classic")));

    themeControl = section->addComboboxRow(Msg::OPTIONSVIEW_THEME_LABEL, Msg::OPTIONSVIEW_THEME_HELP, themes);
    themeControl->setWidth((int)GlobalSettings::scaleFloatUIAndFont(150));

    themeControl->setSelectionStringValue(GlobalSettings::getTheme());
    themeControl->onChange = [this]() {
        GlobalSettings::setTheme(this->themeControl->getSelectionStringValue());
        GlobalSettings::saveConfig();
    };

    std::vector<ComboboxItem> fontScales;
    fontScales.push_back(ComboboxItem(B("50%"), 50));
    fontScales.push_back(ComboboxItem(B("75%"), 75));
    fontScales.push_back(ComboboxItem(B("100%"), 100));
    fontScales.push_back(ComboboxItem(B("125%"), 125));
    fontScales.push_back(ComboboxItem(B("150%"), 150));
    fontScales.push_back(ComboboxItem(B("200%"), 200));
    std::shared_ptr<LayoutComboboxControl> fontScale = section->addComboboxRow(Msg::OPTIONSVIEW_DEFAULT_FONT_SCALE_LABEL, Msg::OPTIONSVIEW_DEFAULT_FONT_SCALE_HELP, fontScales);
    fontScale->setWidth((int)GlobalSettings::scaleFloatUIAndFont(150));
    fontScale->setSelectionIntValue((int)(GlobalSettings::fontScale * 100+.5f));
    fontScale->onChange = [fontScale]() {
        runAfterFrame([fontScale]() {
            GlobalSettings::setFontScale(fontScale->getSelectionIntValue() / 100.0f);
            GlobalSettings::saveConfig();
            GlobalSettings::restartUI = true;
            GlobalSettings::startUpArgs.runOnRestartUI = []() {
                gotoView(VIEW_OPTIONS, B("Display"));
            };
            return false;
        });
    };

    BString name;
    if (GlobalSettings::hasIconsFont()) {
        name += OPTION_DISPLAY_ICON;
        name += " ";
    }
    name += getTranslation(Msg::OPTIONSVIEW_TITLE_DISPLAY);
    addTab(name, name, model, [](bool buttonPressed, BaseViewTab& tab) {
        });
}

void OptionsView::createGeneralTab() {
    std::shared_ptr<ImGuiLayout> model = std::make_shared<ImGuiLayout>();
    std::shared_ptr<LayoutSection> section = model->addSection(Msg::OPTIONSVIEW_TITLE_GENERAL);

    saveLocationControl = section->addTextInputRow(Msg::OPTIONSVIEW_SAVE_FOLDER_LABEL, Msg::OPTIONSVIEW_SAVE_FOLDER_HELP, GlobalSettings::getDataFolder());
    saveLocationControl->setBrowseDirButton();
    // :TODO: track when we loose focus and save

    std::vector<ComboboxItem> resolutions;
    for (auto& res : GlobalSettings::getAvailableResolutions()) {
        resolutions.push_back(ComboboxItem(res));
    }
    resolutionControl = section->addComboboxRow(Msg::OPTIONSVIEW_DEFAULT_RESOLUTION_LABEL, Msg::OPTIONSVIEW_DEFAULT_RESOLUTION_HELP, resolutions);
    resolutionControl->setWidth((int)GlobalSettings::scaleFloatUIAndFont(150));
    resolutionControl->setSelectionByLabel(GlobalSettings::getDefaultResolution());
    resolutionControl->onChange = [this]() {
        GlobalSettings::defaultResolution = GlobalSettings::availableResolutions[this->resolutionControl->getSelection()];
        GlobalSettings::saveConfig();
    };

    std::vector<ComboboxItem> vsync;
    vsync.push_back(ComboboxItem(B("Disabled"), VSYNC_DISABLED));
    vsync.push_back(ComboboxItem(B("Enabled"), VSYNC_ENABLED));
    vsync.push_back(ComboboxItem(B("Adaptive"), VSYNC_ADAPTIVE));
    vsyncControl = section->addComboboxRow(Msg::OPTIONS_VIEW_VSYNC_LABEL, Msg::OPTIONS_VIEW_VSYNC_HELP, vsync, GlobalSettings::defaultVsync);
    vsyncControl->setWidth((int)GlobalSettings::scaleFloatUIAndFont(150));
    vsyncControl->onChange = [this]() {
        GlobalSettings::defaultVsync = this->vsyncControl->getSelectionIntValue();
        GlobalSettings::saveConfig();
    };

    std::vector<ComboboxItem> scales;
    scales.push_back(ComboboxItem(B("1/2x"), 50));
    scales.push_back(ComboboxItem(B("1x"), 100));
    scales.push_back(ComboboxItem(B("2x"), 200));
    scales.push_back(ComboboxItem(B("3x"), 300));
    scaleControl = section->addComboboxRow(Msg::OPTIONSVIEW_DEFAULT_SCALE_LABEL, Msg::OPTIONSVIEW_DEFAULT_SCALE_HELP, scales, GlobalSettings::getDefaultScale()/100);
    scaleControl->setWidth((int)GlobalSettings::scaleFloatUIAndFont(150));
    scaleControl->onChange = [this]() {
        GlobalSettings::defaultScale = this->scaleControl->getSelectionIntValue();
        GlobalSettings::saveConfig();
    };

#if defined(BOXEDWINE_OPENGL_OSMESA)
    std::vector<ComboboxItem> glOptions;
    glOptions.push_back(ComboboxItem(B("Native"), OPENGL_TYPE_NATIVE));
    glOptions.push_back(ComboboxItem(B("Software - Mesa LLVM Pipe"), OPENGL_TYPE_LLVM_PIPE));
    openGlControl = section->addComboboxRow(Msg::OPTIONSVIEW_DEFAULT_OPENGL_LABEL, Msg::OPTIONSVIEW_DEFAULT_OPENGL_HELP, glOptions, GlobalSettings::defaultOpenGL);
    openGlControl->setWidth((int)GlobalSettings::scaleFloatUIAndFont(250));
    openGlControl->setSelectionIntValue(GlobalSettings::defaultOpenGL);
    openGlControl->onChange = [this]() {
        GlobalSettings::defaultOpenGL = this->openGlControl->getSelectionIntValue();
        GlobalSettings::saveConfig();
    };
#endif
#if defined(BOXEDWINE_OPENGL_SDL)
    if (KSystem::isWindows()) {
        std::vector<ComboboxItem> glOptions;
        glOptions.push_back(ComboboxItem(B("Native"), OPENGL_TYPE_NATIVE));
        glOptions.push_back(ComboboxItem(B("Software - Mesa LLVM Pipe"), OPENGL_TYPE_LLVM_PIPE));
        // https://github.com/mmozeiko/build-mesa/releases has a working version for x86 and llvm
        // https://github.com/pal1000/mesa-dist-win has a bad calling convention set for x86, but x64 works well
        if (KSystem::getArchitecture() == "x64") { // keep in sync with containersView.cpp
            glOptions.push_back(ComboboxItem(B("OpenGL on D3D12"), OPENGL_TYPE_ON_D3D12));
            glOptions.push_back(ComboboxItem(B("OpenGL on Vulkan - Zink"), OPENGL_TYPE_ON_VULKAN));
        }
        openGlControl = section->addComboboxRow(Msg::OPTIONSVIEW_DEFAULT_OPENGL_LABEL, Msg::OPTIONSVIEW_DEFAULT_OPENGL_HELP, glOptions, GlobalSettings::defaultOpenGL);
        openGlControl->setWidth((int)GlobalSettings::scaleFloatUIAndFont(250));
        openGlControl->setSelectionIntValue(GlobalSettings::defaultOpenGL);
        openGlControl->onChange = [this]() {
            if (this->openGlControl->getSelectionIntValue() != OPENGL_TYPE_NATIVE && !GlobalSettings::isAlternativeOpenGlDownloaded()) {
                GlobalSettings::downloadOpenGL([this](bool sucess) {
                    if (sucess) {
                        GlobalSettings::defaultOpenGL = this->openGlControl->getSelectionIntValue();
                        GlobalSettings::saveConfig();
                    } else {
                        openGlControl->setSelectionIntValue(OPENGL_TYPE_NATIVE);
                    }
                });
            } else {
                GlobalSettings::defaultOpenGL = this->openGlControl->getSelectionIntValue();
                GlobalSettings::saveConfig();
            }
        };
    }
#endif

#ifdef BOXEDWINE_RECORDER
    automationControl = section->addCheckbox(Msg::OPTIONSVIEW_ENABLE_AUTOMATION_LABEL, Msg::OPTIONSVIEW_ENABLE_AUTOMATION_HELP, GlobalSettings::enabledAutomation);
    automationControl->onChange = [this]() {
        GlobalSettings::enabledAutomation = this->automationControl->isChecked();
        GlobalSettings::saveConfig();
    };
#endif

    cacheFilesControl = section->addCheckbox(Msg::OPTIONSVIEW_ENABLE_COPY_TOUCHED_FILES_LABEL, Msg::OPTIONSVIEW_ENABLE_COPY_TOUCHED_FILES_HELP, GlobalSettings::enabledCachedReadFiles);
    cacheFilesControl->onChange = [this]() {
        GlobalSettings::enabledCachedReadFiles = this->cacheFilesControl->isChecked();
        GlobalSettings::saveConfig();
        };

    std::shared_ptr<LayoutSection> bottomSection = model->addSection();
    bottomSection->addSeparator();
    BString deleteLabel;
    if (GlobalSettings::hasIconsFont()) {
        deleteLabel += TRASH_ICON;
        deleteLabel += " ";
    }
    deleteLabel += getTranslation(Msg::OPTIONS_VIEW_DELETE_ALL_BUTTON_LABEL);
    std::shared_ptr<LayoutButtonControl> deleteAllContainersButton = bottomSection->addButton(Msg::NONE, Msg::OPTIONS_VIEW_DELETE_ALL_BUTTON_HELP, deleteLabel);
    deleteAllContainersButton->onChange = [this]() {
        runOnMainUI([this]() {
            new YesNoDlg(Msg::GENERIC_DLG_CONFIRM_TITLE, getTranslation(Msg::OPTIONS_VIEW_DELETE_ALL_CONFIRM), [this](bool yes) {
                if (yes) {
                    runOnMainUI([]() {
                        Fs::deleteNativeDirAndAllFilesInDir(GlobalSettings::getDataFolder());
                        GlobalSettings::restartUI = true;
                        GlobalSettings::reinit = true;                      
                        return false;
                        });
                }
                });
            return false;
            });
    };

    BString name;
    if (GlobalSettings::hasIconsFont()) {
        name += OPTIONS_GENERAL;
        name += " ";
    }
    name += getTranslation(Msg::OPTIONSVIEW_TITLE_GENERAL);
    addTab(name, name, model, [](bool buttonPressed, BaseViewTab& tab) {

        });
}

void OptionsView::loadFileSystemVersions() {
    this->wineButtonTotalColumnWidth = 0;
    this->wineButtonFirstColumnWidth = 0;
    this->fileSystemVersions.clear();
    this->fileSystemVersionsSorted.clear();

    for (auto& fileSystem : GlobalSettings::availableFileSystemVersions) {
        OptionsViewWineVersion v;
        v.availableVersion = fileSystem;
        v.name = fileSystem->name;

        U64 size = fileSystem->size;
        std::shared_ptr<FileSystemZip> dep = fileSystem->getMissingDependency();
        if (dep) {
            size += dep->size;
        }
        v.size = BString::valueOf(size);
        this->fileSystemVersions[v.name] = v;
    }

    for (auto& fileSystem : GlobalSettings::fileSystemVersions) {
        if (fileSystem->size == 0) {
            fileSystem->size = (U32)(Fs::getNativeFileSize(fileSystem->filePath) / 1024 / 1024);
        }
        if (this->fileSystemVersions.count(fileSystem->name)) {
            OptionsViewWineVersion& v = this->fileSystemVersions[fileSystem->name];
            v.currentVersion = fileSystem;
        } else {
            OptionsViewWineVersion v;
            v.availableVersion = fileSystem;
            v.name = fileSystem->name;
            
            U64 size = fileSystem->size;
            std::shared_ptr<FileSystemZip> dep = fileSystem->getMissingDependency();
            if (dep) {
                size += dep->size;
            }
            v.size = BString::valueOf(size);
            this->fileSystemVersions[v.name] = v;
        }
    }
    float leftColumn = 0;
    float rightColumn = 0;

    ImGui::PushFont(GlobalSettings::mediumFont);
    for (auto& wine : this->fileSystemVersions) {
        float width = 0.0f;
        if (wine.second.availableVersion && !wine.second.currentVersion) {
            width += ImGui::CalcTextSize(c_getTranslation(Msg::OPTIONSVIEW_WINE_VERSION_INSTALL)).x;            
            width += ImGui::GetStyle().FramePadding.x * 2;
            if (width > leftColumn) {
                leftColumn = width;
            }
        } else if (wine.second.availableVersion && wine.second.currentVersion && wine.second.currentVersion->fsVersion != wine.second.availableVersion->fsVersion) {
            width += ImGui::CalcTextSize(c_getTranslation(Msg::OPTIONSVIEW_WINE_VERSION_UPDATE)).x;
            width += ImGui::GetStyle().FramePadding.x * 2;
            if (width > leftColumn) {
                leftColumn = width;
            }
        }
        if (wine.second.currentVersion) {
            width = ImGui::CalcTextSize(c_getTranslation(Msg::OPTIONSVIEW_WINE_VERSION_DELETE)).x;
            width += ImGui::GetStyle().FramePadding.x * 2;
            if (width > rightColumn) {
                rightColumn = width;
            }
        } 
        if (width > 0) {
            width += ImGui::GetStyle().ItemSpacing.x;
        }
        if (width > this->wineButtonTotalColumnWidth) {
            this->wineButtonTotalColumnWidth = width;
        }
        fileSystemVersionsSorted.push_back(wine.second);
    }
    std::sort(fileSystemVersionsSorted.rbegin(), fileSystemVersionsSorted.rend(), [](auto& l, auto& r) {
        return l < r;
        });
    if (leftColumn > 0 && rightColumn > 0) {
        this->wineButtonFirstColumnWidth = leftColumn + ImGui::GetStyle().ItemSpacing.x;
        this->wineButtonTotalColumnWidth = this->wineButtonFirstColumnWidth + rightColumn + ImGui::GetStyle().ItemSpacing.x;
    } else if (leftColumn > 0) {
        this->wineButtonTotalColumnWidth = leftColumn + ImGui::GetStyle().ItemSpacing.x;
    } else if (rightColumn > 0) {
        this->wineButtonTotalColumnWidth = rightColumn + ImGui::GetStyle().ItemSpacing.x;
    }
    ImGui::PopFont();
}

void OptionsView::runWineOptions() {
    ImGui::PushFont(GlobalSettings::largeFont);
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    SAFE_IMGUI_TEXT(this->wineTitle);
    ImGui::PopFont();

    ImVec2 size = ImGui::GetWindowContentRegionMax();
    size.y -= ImGui::GetCursorPosY() + GlobalSettings::scaleFloatUI(8.0f);

    ImGui::PushFont(GlobalSettings::mediumFont);
    //ImGui::BeginChildFrame(401, size);
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    for (auto& wine : this->fileSystemVersionsSorted) {
        ImGui::Dummy(ImVec2(this->extraVerticalSpacing, 0.0f));        
        ImGui::SameLine();
        ImVec2 pos = ImGui::GetCursorPos();
        pos.y += this->extraVerticalSpacing;
        ImGui::SetCursorPos(pos);
        if (wine.availableVersion && !wine.currentVersion) {
            BString buttonLabel = getTranslation(Msg::OPTIONSVIEW_WINE_VERSION_INSTALL);
            buttonLabel += "##";
            buttonLabel += wine.name;
            if (ImGui::Button(buttonLabel.c_str())) {
                download(wine.availableVersion);
            }
            ImGui::SameLine();
        } else if (wine.availableVersion && wine.currentVersion && wine.currentVersion->fsVersion!=wine.availableVersion->fsVersion){
            BString buttonLabel = getTranslation(Msg::OPTIONSVIEW_WINE_VERSION_UPDATE);
            buttonLabel += "##";
            buttonLabel += wine.name;
            if (ImGui::Button(buttonLabel.c_str())) {
                download(wine.availableVersion);
            }
            ImGui::SameLine();
        }
        if (wine.currentVersion) {
            ImGui::SameLine(pos.x + this->wineButtonFirstColumnWidth);
            bool buttonPressed = false;
            BString buttonLabel = getTranslation(Msg::OPTIONSVIEW_WINE_VERSION_DELETE);
            buttonLabel += "##";
            buttonLabel += wine.name;
            if (ImGui::Button(buttonLabel.c_str())) {
                buttonPressed = true;
            }
            bool yes = false;
            BString label = getTranslationWithFormat(Msg::OPTIONSVIEW_WINE_VERSION_DELETE_CONFIRM_LABEL, true, wine.name);
            if (!showYesNoMessageBox("confirm"+wine.name, buttonPressed, c_getTranslation(Msg::GENERIC_DLG_CONFIRM_TITLE), label.c_str(), &yes)) {
                if (yes) {
                    Fs::deleteNativeFile(wine.currentVersion->filePath);
                    // run later, we don't want to change versions while we are iterating them
                    runOnMainUI([this]()->bool {
                        GlobalSettings::reloadWineVersions();
                        this->loadFileSystemVersions();
                        return false;
                        });
                }
            }
            ImGui::SameLine();
        }     
        ImGui::SameLine(pos.x+this->wineButtonTotalColumnWidth);
        BString name = wine.name;
        BString name2;
        OptionsViewWineVersion& version = wine;

        if (version.currentVersion) {
            if (version.availableVersion && version.availableVersion->fsVersion==version.currentVersion->fsVersion) {
                name2 += "   ";
                name2 += getTranslation(Msg::OPTIONSVIEW_WINE_VERSION_UPTODATE);
                name2 += version.size;
                name2 += " MB";
            } else {
                name2 += "   ";
                name2 += getTranslation(Msg::OPTIONSVIEW_WINE_VERSION_UPDATE_AVAILABLE);
                name2 += version.size;
                name2 += " MB";
            }
        } else if (version.availableVersion) {
            name2 += "   ";
            name2 += getTranslation(Msg::OPTIONSVIEW_WINE_VERSION_NOT_INSTALLED);
            name2 += version.size;
            name2 += " MB";
        }
        SAFE_IMGUI_TEXT(name.c_str());
        ImGui::SameLine();
        ImGui::PushFont(GlobalSettings::defaultFont);
        pos = ImGui::GetCursorPos();
        if (GlobalSettings::mediumFont) {
            pos.y += GlobalSettings::mediumFont->FontSize / 2 - GlobalSettings::defaultFont->FontSize / 2;
        }
        ImGui::SetCursorPos(pos);
        SAFE_IMGUI_TEXT(name2.c_str());
        ImGui::PopFont();
        pos = ImGui::GetCursorPos();
        pos.y += this->extraVerticalSpacing;
        ImGui::SetCursorPos(pos);
    }
    ImGui::PopFont();
    //ImGui::EndChildFrame();
}

void OptionsView::download(const std::shared_ptr<FileSystemZip>& version) {
    GlobalSettings::downloadFileSystem(version, [this](bool success) {
        if (success) {
            runOnMainUI([this]()->bool {        
                GlobalSettings::reloadWineVersions();
                this->loadFileSystemVersions();
                return false;
                });
        }
        });
}

bool OptionsView::saveChanges() {
    this->errorMsg = B("");
    if (!Fs::doesNativePathExist(this->saveLocationControl->getText())) {
        this->errorMsg = getTranslation(Msg::OPTIONSVIEW_ERROR_DATA_DIR_NOT_FOUND);
    }
    if (this->errorMsg.isEmpty()) {
        GlobalSettings::setDataFolder(this->saveLocationControl->getText());
        GlobalSettings::saveConfig();
    }

    if (!this->errorMsg.isEmpty()) {
        runErrorMsg(true);
    }
    return this->errorMsg.isEmpty();
}
