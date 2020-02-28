#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../../lib/imgui/addon/imguitinyfiledialogs.h"

void showInstallDlg() {    
    ImGui::OpenPopup(getTranslation(INSTALLDLG_TITLE));
}

#define INSTALL_TYPE_SETUP 0
#define INSTALL_TYPE_DIR 1
#define INSTALL_TYPE_MOUNT 2
#define INSTALL_TYPE_BLANK 3

#define INSTALLDLG_WIDTH 600
#define INSTALLDLG_HEIGHT 300

static void HelpMarker(const char* desc)
{
    SAFE_IMGUI_TEXT_DISABLED("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

class ComboboxData {
public:
    ComboboxData() : dataForCombobox(0), currentSelectedIndex(0) {}
    char* dataForCombobox;
    std::vector<std::string> data;
    int currentSelectedIndex;

    void dataChanged();
};

void ComboboxData::dataChanged() {
    int len = 1;
    
    if (dataForCombobox) {
        delete[] dataForCombobox;
    }
    for (auto& s : data) {
        len+=((int)s.length()+1);
    }
    dataForCombobox = new char[len];
    len = 0;
    for (auto& s : data) {
        strcpy(dataForCombobox+len, s.c_str());
        len+=(int)s.length()+1;
    }
    dataForCombobox[len]=0;
}

void onInstallOk(bool show);

static bool needToIntializeData = true;
static float extraVerticalSpacing = 5;
static float helpWidth = 28;
    
static char locationBuffer[1024];
static int lastInstallType;
static char containerName[256];        
static bool runWineConfig;
static ComboboxData installTypeComboboxData;
static ComboboxData containerComboboxData;
static ComboboxData wineVersionComboboxData;

void runInstallDlgIfVisible() {    
    if (ImGui::BeginPopupModal(getTranslation(INSTALLDLG_TITLE), NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse/* | ImGuiWindowFlags_NoMove*/))
    {
        ImGui::SetWindowSize(ImVec2(INSTALLDLG_WIDTH, INSTALLDLG_HEIGHT));
        const char* installLabelText = getTranslation(INSTALLDLG_INSTALL_TYPE_LABEL);
        const char* containerLabelText = getTranslation(INSTALLDLG_CONTAINER_LABEL);
        const char* locationLabelText = NULL;
        const char* browseButtonText = getTranslation(GENERIC_BROWSE_BUTTON);
        const char* installTypeHelp = getTranslation(INSTALLDLG_INSTALL_TYPE_HELP, false);
        const char* containerHelp = getTranslation(INSTALLDLG_CONTAINER_HELP, false);
        const char* containerNameHelp = getTranslation(INSTALLDLG_CONTAINER_NAME_HELP, false);
        const char* wineVersionHelp = getTranslation(INSTALLDLG_WINE_VERSION_HELP, false);
        const char* wineConfigHelp = getTranslation(INSTALLDLG_RUN_WINE_CONFIG_HELP, false);

        if (installTypeComboboxData.currentSelectedIndex==INSTALL_TYPE_SETUP) {
            locationLabelText = getTranslation(INSTALLDLG_SETUP_FILE_LOCATION_LABEL);
        } else if (installTypeComboboxData.currentSelectedIndex==INSTALL_TYPE_DIR || installTypeComboboxData.currentSelectedIndex==INSTALL_TYPE_MOUNT) {
            locationLabelText = getTranslation(INSTALLDLG_DIRECTORY_LABEL);
        }

        static ImVec2 leftColumnWidth;
        static ImVec2 rightColumnWidth;

        if (needToIntializeData) {
            leftColumnWidth = ImGui::CalcTextSize(installLabelText);
            rightColumnWidth = ImGui::CalcTextSize(browseButtonText);
            rightColumnWidth.x+=8; // more space for button;

            if (locationLabelText && locationLabelText[0]) {
                ImVec2 locationLabelWidth = ImGui::CalcTextSize(locationLabelText);
                if (locationLabelWidth.x>leftColumnWidth.x) {
                    leftColumnWidth = locationLabelWidth;
                }
            }

            ImVec2 containerLabelWidth = ImGui::CalcTextSize(containerLabelText);
            if (containerLabelWidth.x>leftColumnWidth.x) {
                leftColumnWidth = containerLabelWidth;
            }
            needToIntializeData = false;
            leftColumnWidth.x+=10;
            rightColumnWidth.x+=10;
            
            containerName[0]=0;
            locationBuffer[0]=0;

            containerComboboxData.data.clear();
            containerComboboxData.data.push_back("Create New Container (Recommended)");
            for (auto& container : BoxedwineData::getContainers()) {
                containerComboboxData.data.push_back(container->getName());
            }
            containerComboboxData.dataChanged();
            containerComboboxData.currentSelectedIndex=0;
            
            wineVersionComboboxData.data.clear();
            for (auto& ver : GlobalSettings::getWineVersions()) {
                wineVersionComboboxData.data.push_back(ver.name);
            }
            wineVersionComboboxData.dataChanged();
            wineVersionComboboxData.currentSelectedIndex=0;

            runWineConfig = false;
        } else if (lastInstallType!=installTypeComboboxData.currentSelectedIndex) {
            lastInstallType = installTypeComboboxData.currentSelectedIndex;
            locationBuffer[0] = 0;
        }
        

        ImGui::Dummy(ImVec2(0.0f, extraVerticalSpacing/2));
        SAFE_IMGUI_TEXT(installLabelText);
        ImGui::SameLine(leftColumnWidth.x);
        ImGui::PushItemWidth(-1-(installTypeHelp?helpWidth:0));
        ImGui::Combo("##InstallTypeCombo", &installTypeComboboxData.currentSelectedIndex, "Install using a setup program\0Install by copying a directory\0Install by mounting a directory\0Create a blank container\0\0");
        ImGui::PopItemWidth();        
        if (installTypeHelp) {
            ImGui::SameLine();
            HelpMarker(installTypeHelp);
        }
        ImGui::Dummy(ImVec2(0.0f, extraVerticalSpacing));

        if (locationLabelText && locationLabelText[0]) {
            const char* locationHelp = NULL;
            int locationHelpId = 0;

            if (installTypeComboboxData.currentSelectedIndex==INSTALL_TYPE_SETUP) {
                locationHelpId = INSTALLDLG_TYPE_SETUP_HELP;
            } else if (installTypeComboboxData.currentSelectedIndex==INSTALL_TYPE_DIR) {
                locationHelpId = INSTALLDLG_TYPE_DIR_HELP;
            } else if (installTypeComboboxData.currentSelectedIndex==INSTALL_TYPE_MOUNT) {
                locationHelpId = INSTALLDLG_TYPE_MOUNT_HELP;
            }
            if (locationHelpId) {
                locationHelp = getTranslation(locationHelpId);
                if (locationHelp && locationHelp[0]==0) {
                    locationHelp = NULL;
                }
            }
            SAFE_IMGUI_TEXT(locationLabelText);
            ImGui::SameLine(leftColumnWidth.x);
            ImGui::PushItemWidth(-rightColumnWidth.x-(locationHelp?helpWidth:0));
            ImGui::InputText("##LocationID", locationBuffer, sizeof(locationBuffer));
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(browseButtonText)) {
                if (installTypeComboboxData.currentSelectedIndex==INSTALL_TYPE_SETUP) {
                    const char* types[] = {"*.exe"};
                    const char* result = tfd::openFileDialog(getTranslation(INSTALLDLG_OPEN_SETUP_FILE_TITLE), locationBuffer, 1, types, NULL, 0);
                    if (result) {
                        strcpy(locationBuffer, result);
                    }
                } else {
                    const char* result = tfd::selectFolderDialog(getTranslation(INSTALLDLG_OPEN_FOLDER_TITLE), locationBuffer);
                    if (result) {
                        strcpy(locationBuffer, result);
                    }
                }
            }            
            if (locationHelp) {
                ImGui::SameLine();
                HelpMarker(getTranslation(locationHelpId));
            }
            ImGui::Dummy(ImVec2(0.0f, extraVerticalSpacing));
        }            
                
        SAFE_IMGUI_TEXT(containerLabelText);
        ImGui::SameLine(leftColumnWidth.x);
        ImGui::PushItemWidth(-1-(containerHelp?helpWidth:0));
        ImGui::Combo("##ContainerCombo", &containerComboboxData.currentSelectedIndex, containerComboboxData.dataForCombobox);
        ImGui::SameLine();
        ImGui::PopItemWidth();
        if (containerHelp) {
            ImGui::SameLine();
            HelpMarker(containerHelp);
        }
        if (containerComboboxData.currentSelectedIndex==0) { // create new container
            ImGui::Dummy(ImVec2(0.0f, extraVerticalSpacing));
            SAFE_IMGUI_TEXT("");
            ImGui::SameLine(leftColumnWidth.x);
            SAFE_IMGUI_TEXT(getTranslation(INSTALLDLG_CONTAINER_NAME_LABEL));
            ImGui::SameLine();
            ImGui::PushItemWidth(-1-(containerNameHelp?helpWidth:0));
            ImGui::InputText("##ContainerName", containerName, sizeof(containerName));
            if (containerNameHelp) {
                ImGui::SameLine();
                HelpMarker(containerNameHelp);
            }

            ImGui::Dummy(ImVec2(0.0f, extraVerticalSpacing));
            SAFE_IMGUI_TEXT("");
            ImGui::SameLine(leftColumnWidth.x);
            SAFE_IMGUI_TEXT(getTranslation(INSTALLDLG_CONTAINER_WINE_VERSION_LABEL));
            ImGui::SameLine();
            ImGui::PushItemWidth(-1-(wineVersionHelp?helpWidth:0));
            ImGui::Combo("##WineCombo", &wineVersionComboboxData.currentSelectedIndex, wineVersionComboboxData.dataForCombobox);
            ImGui::PopItemWidth();        
            if (wineVersionHelp) {
                ImGui::SameLine();
                HelpMarker(wineVersionHelp);
            }                        

            ImGui::Dummy(ImVec2(0.0f, extraVerticalSpacing));
            SAFE_IMGUI_TEXT("");
            ImGui::SameLine(leftColumnWidth.x);
            SAFE_IMGUI_TEXT(getTranslation(INSTALLDLG_CONTAINER_RUN_WINE_CONFIG_LABEL));
            ImGui::SameLine();
            ImGui::Checkbox("##WineConfigCheckbox", &runWineConfig);
            if (wineConfigHelp) {
                ImGui::SameLine();
                HelpMarker(wineConfigHelp);
            }   

        }
        float buttonArea = ImGui::CalcTextSize(getTranslation(GENERIC_DLG_OK)).x+ImGui::CalcTextSize(getTranslation(GENERIC_DLG_CANCEL)).x;
        ImGui::SetCursorPos(ImVec2(INSTALLDLG_WIDTH-buttonArea-35,INSTALLDLG_HEIGHT-32));

        onInstallOk(ImGui::Button(getTranslation(GENERIC_DLG_OK)));
        ImGui::SameLine();
        if (ImGui::Button(getTranslation(GENERIC_DLG_CANCEL))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    } else {
        needToIntializeData = true;
    }
}

void onInstallOk(bool buttonClicked) {
    static const char* errorMsg = NULL;
    static std::string errorMsgString;

    if (buttonClicked) {
        if (installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_SETUP) {
            if (strlen(locationBuffer)==0) {
                errorMsg = getTranslation(INSTALLDLG_ERROR_SETUP_FILE_MISSING);
            } else if (!Fs::doesNativePathExist(locationBuffer)) {
                errorMsg = getTranslation(INSTALLDLG_ERROR_SETUP_FILE_NOT_FOUND);
            }            
        } else if (installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_DIR || installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_MOUNT) {
            if (strlen(locationBuffer)==0) {
                errorMsg = getTranslation(INSTALLDLG_ERROR_DIR_MISSING);
            } else if (!Fs::doesNativePathExist(locationBuffer)) {
                errorMsg = getTranslation(INSTALLDLG_ERROR_DIR_NOT_FOUND);
            }            
        }

        BoxedContainer* container = NULL;

        if (containerComboboxData.currentSelectedIndex!=0) {
            container = BoxedwineData::getContainers()[containerComboboxData.currentSelectedIndex-1];
        } else {            
            if (containerName[0]==0) {
                errorMsg = getTranslation(INSTALLDLG_ERROR_CONTAINER_NAME_MISSING);
            } else {
                std::string containerFilePath = GlobalSettings::getContainerFolder() + Fs::nativePathSeperator + containerName;
                if (Fs::doesNativePathExist(containerFilePath)) {
                    if (!Fs::isNativeDirectoryEmpty(containerFilePath)) {
                        errorMsgString = getTranslationWithFormat(INSTALLDLG_ERROR_CONTAINER_ALREADY_EXISTS, true,  containerFilePath.c_str());
                        errorMsg = errorMsgString.c_str();
                    }            
                } else if (!Fs::makeNativeDirs(containerFilePath)) {
                    errorMsgString = getTranslationWithFormat(INSTALLDLG_ERROR_FAILED_TO_CREATE_CONTAINER_DIR, true, strerror(errno));
                    errorMsg = errorMsgString.c_str();
                }   
            }
        }
        if (!errorMsg) {            
            GlobalSettings::startUpArgs = StartUpArgs(); // reset parameters
            if (!container) {
                std::string containerFilePath = GlobalSettings::getContainerFolder() + Fs::nativePathSeperator + containerName;
                container = BoxedContainer::createContainer(containerFilePath, containerName, GlobalSettings::getWineVersions()[wineVersionComboboxData.currentSelectedIndex].name);
            }
            container->launch(); // fill out startUpArgs specific to a container
            BoxedwineData::addContainer(container);
            if (runWineConfig) {
                GlobalSettings::startUpArgs.setRunWineConfigFirst(true);                        
                GlobalSettings::startUpArgs.readyToLaunch = true;
            }   

            if (installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_SETUP) {
                GlobalSettings::startUpArgs.setIsInstallingApp(true);
                GlobalSettings::startUpArgs.addArg(locationBuffer);
                GlobalSettings::startUpArgs.readyToLaunch = true;
            }
        }

    }

    if (errorMsg) {        
        if (!showMessageBox(buttonClicked, getTranslation(GENERIC_DLG_ERROR_TITLE), errorMsg)) {
            errorMsg = NULL;
        }
    }
    if (buttonClicked && !errorMsg) {          
        ImGui::CloseCurrentPopup();
    }
}