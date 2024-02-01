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
