#include "boxedwine.h"
#include "boxedTranslation.h"
#include <stdarg.h>
const std::string& translateString(const std::string& s) {
    return s;
}

std::string getTranslationWithFormat(int msg, bool useDefaultIfMissing, const std::string& string1) {
    return getTranslationWithFormat(msg, useDefaultIfMissing, std::vector<std::string>(1, string1));
}

std::string getTranslationWithFormat(int msg, bool useDefaultIfMissing, const std::vector<std::string>& replacements) {
    std::string result = getTranslation(msg, useDefaultIfMissing);
    for (int i=0;i<(int)replacements.size();i++) {
        std::string findText = "{"+std::to_string(i)+"}";
        stringReplaceAll(result, findText, replacements[i]);
    }
    return result;
}

const char* getTranslation(int msg, bool useDefaultIfMissing) {
    switch (msg) {
    case INSTALLDLG_CONTAINER_HELP:
        return "A Container is a directory on then file system where this application will installed into.  In general, is is best that each application have its own container, that way if you need to change anything in the container, like what version of Windows to emulate, then it will only affect this one application";
    case INSTALLDLG_CONTAINER_NAME_HELP:
        return "This must be a unique container name, two containers can not have the same name.";
    case INSTALLDLG_TYPE_SETUP_HELP:
        return "This setup program will be run from here and it will be installed into a container";
    case INSTALLDLG_TYPE_DIR_HELP:
        return "The contents of this directory will be copied from your system into a container";
    case INSTALLDLG_TYPE_MOUNT_HELP:
        return "This folder on your system will be used by Boxedwine without copying it to a container.";    
    case INSTALLDLG_RUN_WINE_CONFIG_HELP:
        return "Wine Config can be used to change the Windows version that is currently being emulated by Wine.  For example if your installer or application fails to run because it says it needs Windows 95, the you can use Wine Config to fix this.";
    case INSTALLDLG_INSTALL_TYPE_HELP:
        return NULL;
    case INSTALLDLG_INSTALL_TYPE_LABEL:
        return "Install Type:";
    case INSTALLDLG_CONTAINER_LABEL:
        return "Container:";
    case INSTALLDLG_CONTAINER_NAME_LABEL:
        return "Name:";    
    case INSTALLDLG_CONTAINER_RUN_WINE_CONFIG_LABEL:
        return "Run Wine Config:";
    case INSTALLDLG_SETUP_FILE_LOCATION_LABEL:
        return "Setup File Location:";
    case INSTALLDLG_DIRECTORY_LABEL:
        return "Directory:";
    case INSTALLDLG_TITLE:
        return "Install Application";
    case INSTALLDLG_ERROR_SETUP_FILE_MISSING:
        return "Setup file location is empty and is required.";
    case INSTALLDLG_ERROR_SETUP_FILE_NOT_FOUND:
        return "The setup file was entered, but it does not exist.";
    case INSTALLDLG_ERROR_DIR_MISSING:
        return "Directory location is empty and is required.";
    case INSTALLDLG_ERROR_DIR_NOT_FOUND:
        return "The directory was entered, but it does not exist.";
    case INSTALLDLG_ERROR_CONTAINER_NAME_MISSING:
        return "You must enter a name for your new container.";
    case INSTALLDLG_ERROR_FAILED_TO_CREATE_CONTAINER_DIR:
        return "Failed to create a direcotry for the new container:\n\nerror msg: {0}";
    case INSTALLDLG_ERROR_CONTAINER_ALREADY_EXISTS:
        return "You chose to create a new container, but a folder with the name you entered already exists:\n\n{0}";
    case INSTALLDLG_TYPE_SETUP:
        return "Install using a setup program";
    case INSTALLDLG_TYPE_MOUNT:
        return "Install by mounting a directory";
    case INSTALLDLG_TYPE_BLANK:
        return "Create a blank container";
    case INSTALLDLG_TYPE_DIRECTORY:
        return "Install by copying a directory";
    case SETTINGS_DLG_TITLE:
        return "Settings";
    case SETUPDLG_SAVE_FOLDER_LABEL:
        return "Save Folder Location:";
    case SETUPDLG_SAVE_FOLDER_HELP:
        return "This is the location on your computer where the games and apps will be installed.  If you already installed some games or apps, if you change this you will no longer see them.";
    case SETUPDLG_THEME_LABEL:
        return "Color Theme:";
    case SETUPDLG_THEME_HELP:
        return "If you prefere Boxedwine to use different colors for the windows, you can try changing this.";
    case SETUPDLG_THEME_DARK:
        return "Dark";
    case SETUPDLG_THEME_CLASSIC:
        return "Classic";
    case SETUPDLG_THEME_LIGHT:
        return "Light";
    case SETUPDLG_ERROR_DATA_DIR_NOT_FOUND:
        return "The Save Folder Location does not exist.";
    case CONTAINER_OPTIONS_DLG_TITLE:
        return "Container Options";
    case CONTAINER_OPTIONS_DLG_WINE_VERSION_LABEL:
        return "Wine Version:";
    case COMMON_WINE_VERSION_LABEL:
        return "Wine Version:";
    case COMMON_WINE_VERSION_HELP:
        return "You should just use the default Wine version, which is the most recent version, unless you know your application needs another version.";
    case GENERIC_BROWSE_BUTTON:
        return "Browse";
    case GENERIC_DLG_OK:
        return "Ok";
    case GENERIC_DLG_CANCEL:
        return "Cancel";
    case GENERIC_DLG_ERROR_TITLE:
        return "Error";
    case MAIN_BUTTON_INSTALL:
        return "Install";
    case MAIN_BUTTON_CONTAINERS:
        return "Containers";
    case MAIN_BUTTON_SETTINGS:
        return "Settings";
    case MAIN_BUTTON_HELP:
        return "Help";
    case MAIN_BUTTON_APPS:
        return "Apps";
    case INSTALLDLG_OPEN_SETUP_FILE_TITLE:
        return "Open File";
    case GENERIC_OPEN_FOLDER_TITLE:
        return "Select Folder";
    default:
        if (useDefaultIfMissing) {
            return "Unknown msg";
        }
        return NULL;
    }
}