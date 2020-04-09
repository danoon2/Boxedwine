#include "boxedwine.h"
#include "boxedTranslation.h"
#include <stdarg.h>
const std::string& translateString(const std::string& s) {
    return s;
}

std::string getTranslationWithFormat(int msg, bool useDefaultIfMissing, const std::string& string1) {
    return getTranslationWithFormat(msg, useDefaultIfMissing, std::vector<std::string>(1, string1));
}

std::string getTranslationWithFormat(int msg, bool useDefaultIfMissing, const std::string& string1, const std::string& string2) {
    std::vector<std::string> values;
    values.push_back(string1);
    values.push_back(string2);
    return getTranslationWithFormat(msg, useDefaultIfMissing, values);
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
        return "Failed to create a directory for the new container:\n\nerror msg: {0}";
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
    case INSTALLDLG_ERROR_FILESYSTEM_COPY_DIRECTORY:
        return "The file system reported an error while trying to copy the directory:\n\n{0}";
    case INSTALLDLG_ERROR_FILESYSTEM_FAIL_TO_CREATE_DIRS:
        return "The file system reported an error while trying to create the directory:\n\n{0}";
    case INSTALLDLG_ERROR_FAILED_TO_MOUNT:
        return "Failed to mount directory at drive t:.  If this is not a new container, perhaps you can try the same thing with a new container.";
    case SETTINGS_DLG_TITLE:
        return "Options";
    case OPTIONSVIEW_SAVE_FOLDER_LABEL:
        return "Save Folder Location:";
    case OPTIONSVIEW_SAVE_FOLDER_HELP:
        return "This is the location on your computer where the games and apps will be installed.  If you already installed some games or apps, if you change this you will no longer see them.";
    case OPTIONSVIEW_THEME_LABEL:
        return "Color Theme:";
    case OPTIONSVIEW_THEME_HELP:
        return "If you prefer Boxedwine to use different colors for the windows, you can try changing this.";
    case OPTIONSVIEW_THEME_DARK:
        return "Dark";
    case OPTIONSVIEW_THEME_CLASSIC:
        return "Classic";
    case OPTIONSVIEW_THEME_LIGHT:
        return "Light";
    case OPTIONSVIEW_ERROR_DATA_DIR_NOT_FOUND:
        return "The Save Folder Location does not exist.";
    case OPTIONSVIEW_TITLE_GENERAL:
        return "General";
    case OPTIONSVIEW_TITLE_DISPLAY:
        return "Display";
    case OPTIONSVIEW_TITLE_WINE_VERSISONS:
        return "Wine Versions";
    case OPTIONSVIEW_WINE_VERSION_UPTODATE:
        return "Up to date, size: ";
    case OPTIONSVIEW_WINE_VERSION_UPDATE_AVAILABLE:
        return "Update available, download size: ";
    case OPTIONSVIEW_WINE_VERSION_NOT_INSTALLED:
        return "Not installed, download size: ";
    case OPTIONSVIEW_WINE_VERSION_DELETE:
        return "Delete";
    case OPTIONSVIEW_WINE_VERSION_INSTALL:
        return "Install";
    case OPTIONSVIEW_WINE_VERSION_UPDATE:
        return "Update";
    case OPTIONSVIEW_WINE_VERSION_DELETE_CONFIRM_TITLE:
        return "Confirm";
    case OPTIONSVIEW_WINE_VERSION_DELETE_CONFIRM_LABEL:
        return "Are you sure you want to delete {0}?";
    case APPCHOOSER_DLG_TITLE:
        return "Create Shortcut";
    case APPCHOOSER_DLG_CHOOSE_APP_LABEL:
        return "Please select a file to use for the shortcut.";
    case APPCHOOSER_DLG_NO_APPS_LABEL:
        return "Could not find any new apps.";
    case CONTAINER_OPTIONS_DLG_TITLE:
        return "Container Options";
    case CONTAINER_OPTIONS_DLG_WINE_VERSION_LABEL:
        return "Wine Version:";
    case CONTAINER_OPTIONS_DLG_ADD_APP_LABEL:
        return "Create New App Shortcut:";
    case CONTAINER_OPTIONS_DLG_ADD_APP_HELP:
        return "You can choose from a list of applications in this container to create a shortcut to.";
    case CONTAINER_OPTIONS_DLG_ADD_APP_BUTTON_LABEL:
        return "Choose App";
    case COMMON_WINE_VERSION_LABEL:
        return "Wine Version:";
    case COMMON_WINE_VERSION_HELP:
        return "You should just use the default Wine version, which is the most recent version, unless you know your application needs another version.";
    case GENERIC_BROWSE_BUTTON:
        return "Browse";
    case GENERIC_DLG_OK:
        return "Ok";
    case GENERIC_DLG_YES:
        return "Yes";
    case GENERIC_DLG_NO:
        return "No";
    case GENERIC_DLG_CANCEL:
        return "Cancel";
    case GENERIC_DLG_ERROR_TITLE:
        return "Error";
    case MAIN_BUTTON_INSTALL:
        return "Install";
    case MAIN_BUTTON_CONTAINERS:
        return "Containers";
    case MAIN_BUTTON_SETTINGS:
        return "Options";
    case MAIN_BUTTON_APPS:
        return "Apps";
    case INSTALLDLG_OPEN_SETUP_FILE_TITLE:
        return "Open File";
    case GENERIC_OPEN_FOLDER_TITLE:
        return "Select Folder";
    case WAITDLG_LAUNCH_APP_TITLE:
        return "Please Wait";
    case WAITDLG_LAUNCH_APP_LABEL:
        return "Launching {0} ...";
    case WAITDLG_GET_FILE_LIST_TITLE:
        return "Please Wait";
    case WAITDLG_GET_FILE_LIST_LABEL:
        return "Fetching list of Wine versions";
    case DOWNLOADDLG_TITLE:
        return "Please Wait";
    case DOWNLOADDLG_LABEL:
        return "Downloading {0} ...";
    case APP_OPTIONS_DLG_TITLE:
        return "Application Options";
    case APP_OPTIONS_DLG_NAME_LABEL:
        return "Name:";
    case APP_OPTIONS_DLG_NAME_HELP:
        return "This is the name you will see in the Apps list view";
    case APP_OPTIONS_DLG_RESOLUTION_LABEL:
        return "Emulated Screen Resolution:";
    case APP_OPTIONS_DLG_RESOLUTION_HELP:
        return "This is the desktop/monitor resolution that will be used for the initial window and reported back to the application.  Normally the application will set its own resolution if it goes full screen.  This would only need to be adjusted if you want more space for applications that do not go full screen.";
    case APP_OPTIONS_DLG_BPP_LABEL:
        return "Emulated Screen Bits per Pixel:";
    case APP_OPTIONS_DLG_BPP_HELP:
        return "Some applications might check the screen BPP before starting.  If those applications require a specific BPP, you can set it here.";        
    case APP_OPTIONS_DLG_FULL_SCREEN_LABEL:
        return "Fullscreen:";
    case APP_OPTIONS_DLG_FULL_SCREEN_HELP:
        return "Will launch the emulated desktop with the same resolution as your computer's desktop/monitor";
    case APP_OPTIONS_DLG_SCALE_LABEL:
        return "Emulated Screen Scaling:";
    case APP_OPTIONS_DLG_SCALE_HELP:
        return "This simple scaling option will blow up or shrink the emualted screen.  This is useful for applications that want to run in low resolution, like 640x480, and you want it to show larger on your monitor.";
    case APP_OPTIONS_DLG_SCALE_QUALITY_LABEL:
        return "Scale Quality:";
    case APP_OPTIONS_DLG_SCALE_QUALITY_HELP:
        return "The scale quality options that SDL supplies.  Probably no need to ever change the default.";
    case APP_OPTIONS_DLG_GL_EXT_LABEL:
        return "Allowed OpenGL Extensions:";
    case APP_OPTIONS_DLG_GL_EXT_HELP:
        return "If empty then all extension will be allowed.\n\nSome applications, like Quake 2 and Unreal, can crash if the extension list is too large, like what is seen with modern computers.  Hit the set button to fill in a small list of common extensions or enter your own if necessary";
    case APP_OPTIONS_DLG_GL_EXT_SET_BUTTON_LABEL:
        return "Set";
    case APP_OPTIONS_DLG_DEFAULT_RESOLUTION_LABEL:
        return "Default";
    case APP_OPTIONS_DLG_NAME_REQUIRED:
        return "The Name is required and cannot be empty.";
    case ERROR_NO_WINE:
        return "There are no versions of Wine installed.  Would you like to install the default version of Wine now?";
    case ERROR_MISSING_WINE:
        return "{0} is missing.  Would you like to use {1} instead?";
    case ERROR_NO_FILE_LIST:
        return "Was unable to download the list of Wine versions.  Boxedwine will not work without a Wine file system.  Make sure your internet is working and you can try again by re-launching Boxedwine.  You can also manually download a Wine file system from https://sourceforge.net/projects/boxedwine/files/FileSystems/ and place it in the same directory as the Boxedwine application";
    default:
        if (useDefaultIfMissing) {
            return "Unknown msg";
        }
        return NULL;
    }
}