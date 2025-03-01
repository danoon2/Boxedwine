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

HelpView::HelpView(BString tab) : BaseView(B("HelpView")) {
    if (tab.length()) {
        if (tab == "Wine") {
            this->tabIndex = 2;
        } if (tab == "Display") {
            this->tabIndex = 1;
        }
    }
    createAboutTab();
    createHelpInstallTab();
    createHelpTroubleshootingTab();
}

bool HelpView::saveChanges() {
    return true;
}

void HelpView::createAboutTab() {
    std::shared_ptr<ImGuiLayout> model = std::make_shared<ImGuiLayout>();
    std::shared_ptr<LayoutSection> section = model->addSection(Msg::HELPVIEW_TITLE_ABOUT);

    section->addText(Msg::NONE, Msg::NONE, getTranslation(Msg::HELPVIEW_ABOUT_LABEL));

    BString name;
    if (GlobalSettings::hasIconsFont()) {
        name += ABOUT_ICON;
        name += " ";
    }
    name += getTranslation(Msg::HELPVIEW_TITLE_ABOUT);
    addTab(name, name, model, [this](bool buttonPressed, BaseViewTab& tab) {

        });
}

void HelpView::createHelpInstallTab() {
    std::shared_ptr<ImGuiLayout> model = std::make_shared<ImGuiLayout>();
    std::shared_ptr<LayoutSection> section = model->addSection(Msg::HELPVIEW_TITLE_HELP_INSTALL);

    section->addText(Msg::NONE, Msg::NONE, getTranslation(Msg::HELPVIEW_HELP_INSTALL_LABEL));

    BString name;
    if (GlobalSettings::hasIconsFont()) {
        name += QUESTION_ICON;
        name += " ";
    }
    name += getTranslation(Msg::HELPVIEW_TITLE_HELP_INSTALL);
    addTab(name, name, model, [this](bool buttonPressed, BaseViewTab& tab) {

        });
}

void HelpView::createHelpTroubleshootingTab() {
    std::shared_ptr<ImGuiLayout> model = std::make_shared<ImGuiLayout>();
    std::shared_ptr<LayoutSection> section = model->addSection(Msg::HELPVIEW_TITLE_HELP_TROUBLESHOOTING);

    section->addText(Msg::NONE, Msg::NONE, getTranslation(Msg::HELPVIEW_HELP_TROUBLESHOOTING_LABEL));

    BString name;
    if (GlobalSettings::hasIconsFont()) {
        name += QUESTION_ICON;
        name += " ";
    }
    name += getTranslation(Msg::HELPVIEW_TITLE_HELP_TROUBLESHOOTING);
    addTab(name, name, model, [this](bool buttonPressed, BaseViewTab& tab) {

        });
}
