#include "boxedwine.h"
#include "../boxedwineui.h"

HelpView::HelpView(std::string tab) : BaseView("HelpView") {
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
    std::shared_ptr<LayoutSection> section = model->addSection(HELPVIEW_TITLE_ABOUT);

    section->addText(0, 0, getTranslation(HELPVIEW_ABOUT_LABEL));

    std::string name;
    if (GlobalSettings::hasIconsFont()) {
        name += ABOUT_ICON;
        name += " ";
    }
    name += getTranslation(HELPVIEW_TITLE_ABOUT);
    addTab(name, name, model, [this](bool buttonPressed, BaseViewTab& tab) {

        });
}

void HelpView::createHelpInstallTab() {
    std::shared_ptr<ImGuiLayout> model = std::make_shared<ImGuiLayout>();
    std::shared_ptr<LayoutSection> section = model->addSection(HELPVIEW_TITLE_HELP_INSTALL);

    section->addText(0, 0, getTranslation(HELPVIEW_HELP_INSTALL_LABEL));

    std::string name;
    if (GlobalSettings::hasIconsFont()) {
        name += QUESTION_ICON;
        name += " ";
    }
    name += getTranslation(HELPVIEW_TITLE_HELP_INSTALL);
    addTab(name, name, model, [this](bool buttonPressed, BaseViewTab& tab) {

        });
}

void HelpView::createHelpTroubleshootingTab() {
    std::shared_ptr<ImGuiLayout> model = std::make_shared<ImGuiLayout>();
    std::shared_ptr<LayoutSection> section = model->addSection(HELPVIEW_TITLE_HELP_TROUBLESHOOTING);

    section->addText(0, 0, getTranslation(HELPVIEW_HELP_TROUBLESHOOTING_LABEL));

    std::string name;
    if (GlobalSettings::hasIconsFont()) {
        name += QUESTION_ICON;
        name += " ";
    }
    name += getTranslation(HELPVIEW_TITLE_HELP_TROUBLESHOOTING);
    addTab(name, name, model, [this](bool buttonPressed, BaseViewTab& tab) {

        });
}
