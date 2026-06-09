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
#include <thread>

AppChooserDlg::AppChooserDlg(std::vector<BoxedApp>& items, std::vector<BoxedApp>& wineApps, std::function<void(BoxedApp)> onSelected, bool saveApp, BaseDlg* parent) : BaseDlg(Msg::APPCHOOSER_DLG_TITLE, 600, 400, nullptr, parent), items(items), wineApps(wineApps), onSelected(onSelected), labelId(Msg::APPCHOOSER_DLG_CHOOSE_APP_LABEL), saveApp(saveApp) {
}

AppChooserDlg::AppChooserDlg(std::vector<BoxedApp>& items, std::function<void(BoxedApp)> onSelected, bool saveApp, BaseDlg* parent, Msg titleId) : BaseDlg(titleId, 600, 400, nullptr, parent), items(items), onSelected(onSelected), labelId(Msg::APPCHOOSER_DLG_CHOOSE_APP_LABEL), saveApp(saveApp) {
}

void AppChooserDlg::drawItems(std::vector<BoxedApp>& apps, int startingIndex) {
    for (int i = 0; i < (int)apps.size(); i++) {
        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
        ImVec2 pos = ImGui::GetCursorPos();
        ImGui::PushID(i+startingIndex);
        if (ImGui::Selectable("", false, 0, ImVec2(ImGui::GetColumnWidth(), ImGui::GetTextLineHeight() + ImGui::GetStyle().ItemSpacing.y))) {
            std::function<void(BoxedApp)> onSelected = this->onSelected;
            selectApp(apps[i], onSelected, this->saveApp);
            this->done();
        }
        ImGui::PopID();
        ImGui::SetCursorPos(pos);
        const BoxedAppIcon* icon = apps[i].getIconTexture(16);
        if (icon) {
            ImGui::Image(icon->texture->getTexture(), ImVec2(GlobalSettings::scaleFloatUI(16.0f), GlobalSettings::scaleFloatUI(16.0f)));
            ImGui::SameLine();
        } else {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + GlobalSettings::scaleFloatUI(16.0f) + ImGui::GetStyle().ItemSpacing.x);
        }
        SAFE_IMGUI_TEXT(apps[i].getName().c_str());
        ImGui::NextColumn();
    }
}

bool AppChooserDlg::saveAndSelectApp(BoxedApp app, std::function<void(BoxedApp app)> onSelected) {
    if (!app.saveApp()) {
        BString label = B("Failed to save shortcut ");
        label += app.getName();
        label += B(".");
        new OkDlg(Msg::GENERIC_DLG_ERROR_TITLE, label, nullptr, 500, 200);
        return false;
    }
    app.getContainer()->reload();
    GlobalSettings::reloadApps();

    runOnMainUI([app, onSelected]() {
        if (onSelected) {
            onSelected(app);
        }
        return false;
        });
    return true;
}

void AppChooserDlg::selectApp(BoxedApp app, std::function<void(BoxedApp app)> onSelected, bool saveApp) {
    if (!saveApp) {
        runOnMainUI([app, onSelected]() {
            if (onSelected) {
                onSelected(app);
            }
            return false;
            });
        return;
    }

    BString requiredComponent = app.getRequiredComponentOptionsName();
    BoxedContainer* container = app.getContainer();

    if (container && requiredComponent.length() && !container->isComponentInstalled(requiredComponent)) {
        AppFilePtr component = GlobalSettings::getComponentByOptionName(requiredComponent);
        if (component) {
            std::function<bool()> saveShortcut = [app, onSelected]() {
                return saveAndSelectApp(app, onSelected);
                };
            std::function<void()> installComponent = [component, container]() {
                component->install(false, container);
                };

            if (Fs::doesNativePathExist(component->localFilePath)) {
                if (!saveShortcut()) {
                    return;
                }
                BString label = component->name;
                label += B(" is required to run ");
                label += app.getName();
                label += B(". Install it in this container now?");
                runOnMainUI([label, installComponent]() {
                    new YesNoDlg(Msg::GENERIC_DLG_CONFIRM_TITLE, label, [installComponent](bool yes) {
                        if (yes) {
                            installComponent();
                        }
                        });
                    return false;
                    });
            } else {
                if (!saveShortcut()) {
                    return;
                }
                BString label = component->name;
                label += B(" is required to run ");
                label += app.getName();
                label += B(". Download and install ");
                label += BString::valueOf(component->size);
                label += B(" MB now?");
                runOnMainUI([label, component, installComponent]() {
                    new YesNoDlg(Msg::GENERIC_DLG_CONFIRM_TITLE, label, [component, installComponent](bool yes) {
                        if (yes) {
                            GlobalSettings::downloadFile(component->filePath, component->localFilePath, component->name, component->size, [installComponent](bool success) {
                                if (success) {
                                    installComponent();
                                }
                                });
                        }
                        });
                    return false;
                    });
            }
            return;
        }
    }

    saveAndSelectApp(app, onSelected);
}

void AppChooserDlg::run() {
    ImVec2 pos = ImGui::GetCursorPos();
    pos.y += this->extraVerticalSpacing*2;
    pos.x += getOuterFramePadding();
    ImGui::SetCursorPos(pos);
    if (this->items.size()==0) {
        SAFE_IMGUI_TEXT(c_getTranslation(Msg::APPCHOOSER_DLG_NO_APPS_LABEL));
    } else {
        SAFE_IMGUI_TEXT(c_getTranslation(this->labelId));
    }
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    pos = ImGui::GetCursorPos();
    pos.x += getOuterFramePadding();
    ImGui::SetCursorPos(pos);
    ImGui::BeginChildFrame(1, ImVec2(-1 - getOuterFramePadding(), this->height - ImGui::GetCursorPosY() - getReservedHeightForButtons()), 0);
    
    ImGui::Columns(3);
    drawItems(this->items, 0);
    ImGui::Columns(1);
    if (this->wineApps.size()) {
        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing * 4));
        SAFE_IMGUI_TEXT(c_getTranslation(Msg::APPCHOOSER_DLG_WINE_APPS_LABEL));
        ImGui::Separator();
        ImGui::Columns(3);
        drawItems(this->wineApps, (int)this->items.size());
        ImGui::Columns(1);
    }
    ImGui::EndChildFrame();
    
    
    this->addCancelButton();
}

void AppChooserDlg::onOk(bool buttonClicked) {
    if (buttonClicked) {
        this->done();            
    }
}
