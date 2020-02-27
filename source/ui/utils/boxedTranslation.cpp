#include "boxedwine.h"
#include "boxedTranslation.h"

const std::string& translateString(const std::string& s) {
    return s;
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
    case INSTALLDLG_WINE_VERSION_HELP:
        return "You should just use the default Wine version, which is the most recent version, unless you know your application needs another version.";
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
    case INSTALLDLG_CONTAINER_WINE_VERSION_LABEL:
        return "Wine Version:";
    case INSTALLDLG_CONTAINER_RUN_WINE_CONFIG_LABEL:
        return "Run Wine Config:";
    case INSTALLDLG_SETUP_FILE_LOCATION_LABEL:
        return "Setup File Location:";
    case INSTALLDLG_DIRECTORY_LABEL:
        return "Directory:";
    case INSTALLDLG_TITLE:
        return "Install Application";
    case GENERIC_BROWSE_BUTTON:
        return "Browse";
    case GENERIC_DLG_OK:
        return "Ok";
    case GENERIC_DLG_CANCEL:
        return "Cancel";
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
    case INSTALLDLG_OPEN_FOLDER_TITLE:
        return "Select Folder";
    default:
        if (useDefaultIfMissing) {
            return "Unknown msg";
        }
        return NULL;
    }
}