#include "boxedwine.h"
#include "boxedTranslation.h"

BString translateString(BString s) {
    return s;
}

BString getTranslationWithFormat(Msg msg, bool useDefaultIfMissing, BString string1) {
    return getTranslationWithFormat(msg, useDefaultIfMissing, std::vector<BString>(1, string1));
}

BString getTranslationWithFormat(Msg msg, bool useDefaultIfMissing, BString string1, BString string2) {
    std::vector<BString> values;
    values.push_back(string1);
    values.push_back(string2);
    return getTranslationWithFormat(msg, useDefaultIfMissing, values);
}

BString getTranslationWithFormat(Msg msg, bool useDefaultIfMissing, const std::vector<BString>& replacements) {
    BString result = getTranslation(msg, useDefaultIfMissing);
    for (int i=0;i<(int)replacements.size();i++) {
        BString findText;
        findText += "{";
        findText += i;
        findText += "}";
        result.replace(findText, replacements[i]);
    }
    return result;
}

const char* c_getTranslation(Msg msg, bool useDefaultIfMissing) {
    switch (msg) {
    case Msg::INSTALLVIEW_INSTALL_TITLE:
        return "Install";
    case Msg::INSTALLVIEW_DEMO_TITLE:
        return "Demos";
    case Msg::INSTALLVIEW_CONTAINER_HELP:
        return "A Container is a directory on then file system where this application will installed into.  In general, is is best that each application have its own container, that way if you need to change anything in the container, like what version of Windows to emulate, then it will only affect this one application";
    case Msg::INSTALLVIEW_CONTAINER_NAME_HELP:
        return "This is the name of the container that you will see in the UI.";
    case Msg::INSTALLVIEW_TYPE_SETUP_HELP:
        return "This setup program will be run from here and it will be installed into a container";
    case Msg::INSTALLVIEW_TYPE_DIR_HELP:
        return "The contents of this directory will be copied from your system into a container";
    case Msg::INSTALLVIEW_TYPE_MOUNT_HELP:
        return "This folder on your system will be used by Boxedwine without copying it to a container.";    
    case Msg::INSTALLVIEW_RUN_WINE_CONFIG_HELP:
        return "Wine Config can be used to change the Windows version that is currently being emulated by Wine.  For example if your installer or application fails to run because it says it needs Windows 95, the you can use Wine Config to fix this.";
    case Msg::INSTALLVIEW_INSTALL_TYPE_HELP:
        return nullptr;
    case Msg::INSTALLVIEW_INSTALL_TYPE_LABEL:
        return "Install Type:";
    case Msg::INSTALLVIEW_CONTAINER_LABEL:
        return "Container:";
    case Msg::INSTALLVIEW_CONTAINER_NAME_LABEL:
        return "Name:";    
    case Msg::INSTALLVIEW_CONTAINER_RUN_WINE_CONFIG_LABEL:
        return "Run Wine Config:";
    case Msg::INSTALLVIEW_SETUP_FILE_LOCATION_LABEL:
        return "Setup File Location:";
    case Msg::INSTALLVIEW_DIRECTORY_LABEL:
        return "Directory:";
    case Msg::INSTALLVIEW_TITLE:
        return "Install Application";
    case Msg::INSTALLVIEW_ERROR_SETUP_FILE_MISSING:
        return "Setup file location is empty and is required.";
    case Msg::INSTALLVIEW_ERROR_SETUP_FILE_NOT_FOUND:
        return "The setup file was entered, but it does not exist.";
    case Msg::INSTALLVIEW_ERROR_SETUP_FILE_NOT_FILE:
        return "The setup file path is a directory, this should be the location of a setup file.";
    case Msg::INSTALLVIEW_ERROR_DIR_MISSING:
        return "Directory location is empty and is required.";
    case Msg::INSTALLVIEW_ERROR_DIR_NOT_FOUND:
        return "The directory was entered, but it does not exist.";
    case Msg::INSTALLVIEW_ERROR_CONTAINER_NAME_MISSING:
        return "You must enter a name for your new container.";
    case Msg::INSTALLVIEW_ERROR_FAILED_TO_CREATE_CONTAINER_DIR:
        return "Failed to create a directory for the new container:\n\nerror msg: {0}";
    case Msg::INSTALLVIEW_ERROR_FAILED_TO_CREATE_TEMP_DIR:
        return "Failed to create temp directory:\n\n{0}\n\nerror msg: {1}";
    case Msg::INSTALLVIEW_TYPE_SETUP:
        return "Install using a setup program";
    case Msg::INSTALLVIEW_TYPE_MOUNT:
        return "Install by mounting a directory";
    case Msg::INSTALLVIEW_TYPE_BLANK:
        return "Create a blank container";
    case Msg::INSTALLVIEW_TYPE_DIRECTORY:
        return "Install by copying a directory";
    case Msg::INSTALLVIEW_ERROR_FILESYSTEM_COPY_DIRECTORY:
        return "The file system reported an error while trying to copy the directory:\n\n{0}";
    case Msg::INSTALLVIEW_ERROR_FILESYSTEM_FAIL_TO_CREATE_DIRS:
        return "The file system reported an error while trying to create the directory:\n\n{0}";
    case Msg::INSTALLVIEW_ERROR_FAILED_TO_MOUNT:
        return "Failed to mount directory at drive t:.  If this is not a new container, perhaps you can try the same thing with a new container.";
    case Msg::INSTALLVIEW_INSTALL_BUTTON_LABEL:
        return "Install";
    case Msg::INSTALLVIEW_NEW_CONTAINER:
        return "Create New Container (Recommended)";
    case Msg::INSTALLVIEW_DEMO_DOWNLOAD_SIZE:
        return "Download size: ";
    case Msg::SETTINGS_DLG_TITLE:
        return "Options";
    case Msg::OPTIONSVIEW_SAVE_FOLDER_LABEL:
        return "Save Folder Location:";
    case Msg::OPTIONSVIEW_SAVE_FOLDER_HELP:
        return "This is the location on your computer where the games and apps will be installed.  If you already installed some games or apps, if you change this you will no longer see them.";
    case Msg::OPTIONSVIEW_THEME_LABEL:
        return "Color Theme:";
    case Msg::OPTIONSVIEW_THEME_HELP:
        return "If you prefer Boxedwine to use different colors for the windows, you can try changing this.";
    case Msg::OPTIONSVIEW_THEME_DARK:
        return "Dark";
    case Msg::OPTIONSVIEW_THEME_CLASSIC:
        return "Classic";
    case Msg::OPTIONSVIEW_THEME_LIGHT:
        return "Light";
    case Msg::OPTIONSVIEW_ERROR_DATA_DIR_NOT_FOUND:
        return "The Save Folder Location does not exist.";
    case Msg::OPTIONSVIEW_TITLE_GENERAL:
        return "General";
    case Msg::OPTIONSVIEW_TITLE_DISPLAY:
        return "Display";
    case Msg::OPTIONSVIEW_TITLE_FILESYSTEM:
        return "File Systems";
    case Msg::OPTIONSVIEW_WINE_VERSION_UPTODATE:
        return "Up to date, size: ";
    case Msg::OPTIONSVIEW_WINE_VERSION_UPDATE_AVAILABLE:
        return "Update available, download size: ";
    case Msg::OPTIONSVIEW_WINE_VERSION_NOT_INSTALLED:
        return "Not installed, download size: ";
    case Msg::OPTIONSVIEW_WINE_VERSION_DELETE:
        return "Delete";
    case Msg::OPTIONSVIEW_WINE_VERSION_INSTALL:
        return "Install";
    case Msg::OPTIONSVIEW_WINE_VERSION_UPDATE:
        return "Update";    
    case Msg::OPTIONSVIEW_WINE_VERSION_DELETE_CONFIRM_LABEL:
        return "Are you sure you want to delete {0}?";
    case Msg::OPTIONSVIEW_DEFAULT_RESOLUTION_LABEL:
        return "Default App Resolution:";
    case Msg::OPTIONSVIEW_DEFAULT_RESOLUTION_HELP:
        return "This is the default resolution that Boxedwine will use to create the main window.  This can be overriden in the Containers view under the shortcut specific options on a per app basis.";
    case Msg::OPTIONSVIEW_DEFAULT_SCALE_LABEL:
        return "Default App Scale:";
    case Msg::OPTIONSVIEW_DEFAULT_SCALE_HELP:
        return "This is the default scale that Boxedwine will use to create the main window.  This can be overriden in the Containers view under the shortcut specific options on a per app basis.\n\nThis simple scaling option will blow up or shrink the emualted screen.  This is useful for applications that want to run in low resolution, like 640x480, and you want it to show larger on your monitor.";
    case Msg::OPTIONSVIEW_DEFAULT_OPENGL_LABEL:
        return "Default OpenGL Backend:";
    case Msg::OPTIONSVIEW_DEFAULT_OPENGL_HELP:
        return "OpenGL Backends:\n\nNative - This will use the OpenGL that is installed on your computer for your graphics card.  This is the best option if it works for you and will probably be the fastest.\n\nThe other options available to choose are in the order of expected performance.  They might be help older games/apps that try to use a color depth no longer supported by modern OpenGL graphics cards.";
    case Msg::OPTIONSVIEW_ENABLE_AUTOMATION_LABEL:
        return "Enable Automation:";
    case Msg::OPTIONSVIEW_ENABLE_AUTOMATION_HELP:
        return "This option should not be enabled for most people.  This will add an automation option to the shortcuts that will allow someone to record some acation and play them back for the app.  This automation testing makes it easier to test apps quickly.  It works by comparing screen shots so it is not very useful for screens that change with animations.  F11 key will capture a full screen shot.  If you want a partial screen shot, then hit the F11 key and keep it down.  With the mouse hold down the left mouse button and raw a rectangle to capture just that area.";
    case Msg::OPTIONSVIEW_DEFAULT_FONT_SCALE_LABEL:
        return "Font Scale";
    case Msg::OPTIONSVIEW_DEFAULT_FONT_SCALE_HELP:
        return "This will change the size of the, which will make the UI look smaller or bigger.";
    case Msg::OPTIONS_VIEW_DELETE_ALL_BUTTON_LABEL:
        return "Delete all data";
    case Msg::OPTIONS_VIEW_DELETE_ALL_BUTTON_HELP:
        return "This will delete all data, including the containers folder, and cannot be undone.  You will loose all saved data inside the containers, including apps, and Boxedwine settings.";
    case Msg::OPTIONS_VIEW_DELETE_ALL_CONFIRM:
        return "Are you sure you want to permentantly delete all containers, apps, settings and any saved data you had in the containers?";
    case Msg::OPTIONS_VIEW_VSYNC_LABEL:
        return "Default App Vsync:";
    case Msg::OPTIONS_VIEW_VSYNC_HELP:
        return "Disabled: The frame rate can be faster than the monitor, but may introduce artifacts in the game.\n\nEnabled: Synchronizes the frame rate of the app/game with the monitor refresh rate for better stability.\n\nAdaptive: At high framerates, VSync is enabled to eliminate tearing. At low frame rates, it's disabled to minimise stuttering.  If the video card does not support this, then this option will act like Enabled.";
    case Msg::HELPVIEW_TITLE_ABOUT:
        return "About";
    case Msg::HELPVIEW_TITLE_HELP_INSTALL:
        return "Installing Apps";
    case Msg::HELPVIEW_TITLE_HELP_TROUBLESHOOTING:
        return "Troubleshooting";
    case Msg::HELPVIEW_ABOUT_LABEL:
        return "## What is Boxedwine?\n[Boxedwine](http://boxedwine.org) is a multi-platform app for running 16-bit and 32-bit Windows programs.  It does this by emulating the CPU and running [Wine](https://www.winehq.org/).  Wine is the program that emulates all of the Windows API calls.  The goal of Boxedwine is to be able to run everything that Wine can.  If you want to know if your app or game will run in Boxedwine or get hints how to run it, you should start by looking at the [Wine Application Database (AppDB)](https://appdb.winehq.org/)\n## Supported Platforms:\n* Windows 32-bit (Vista or later)\n* Windows 64-bit (Vista or later)\n* Mac OSX\n* Linux (32-bit and 64-bit)\n* Web Browsers using WASM\n* Raspberry Pi (32-bit and 64-bit)\n## Current Version: " BOXEDWINE_VERSION_DISPLAY "\n*Added support for Wine 6.\n*Added DirectDraw Auto Refresh Option for apps.  This is a hack to help some older games.";
    case Msg::HELPVIEW_HELP_INSTALL_LABEL:
        return "Apps/Games are installed into containers.  A container is a folder that contains all of the files necessary for Boxedwine and Wine to run and a place for them to store changes.  It acts as a virtual file system.  Each app can be installed into its own container.  To install an app or game, just drag the installer, for example, setup.exe, onto Boxedwine.  You can also drag a folder onto Boxedwine and it will be copied into a container.";
    case Msg::HELPVIEW_HELP_TROUBLESHOOTING_LABEL:
        return "The first place to look for help is the [Wine Application Database (AppDB)](https://appdb.winehq.org/).  If your app or game works on Wine, then there is a good chance it will work on Boxedwine.\n\n## Performance\nSince Boxedwine emulates the CPU, you may have performance issues.  The 64-bit Windows build uses a binary translator for the CPU emulation and is pretty fast, perhaps as much as 25% of the host computer.  The other platforms will be significantly slower.  You will probably not have much luck with demanding games after 1999.\n## Things to try if your app/game doesnt' work\n* If your app/game was made before the year 2000, try setting the Windows version for the container to be Windows 98.";
    case Msg::APPCHOOSER_DLG_TITLE:
        return "Create Shortcut";
    case Msg::APPCHOOSER_DLG_CHOOSE_APP_LABEL:
        return "Please select a file to use for the shortcut.";
    case Msg::APPCHOOSER_DLG_NO_APPS_LABEL:
        return "Could not find any new apps.";
    case Msg::APPCHOOSER_DLG_WINE_APPS_LABEL:
        return "Wine Apps:";
    case Msg::CONTAINER_VIEW_CONTAINER_NAME_LABEL:
        return "Container Name:";
    case Msg::CONTAINER_VIEW_ERROR_BLANK_NAME:
        return "Container Name can not be empty";
    case Msg::CONTAINER_VIEW_ERROR_MISSING_MOUNT_LOCATION:
        return "You specified a mount drive letter, but the mount folder location is empty.";
    case Msg::CONTAINER_VIEW_ERROR_MISSING_MOUNT_DRIVE:
        return "You specified a mount folder location, but the mount drive letter is empty.";
    case Msg::CONTAINER_VIEW_CONTAINER_LOCATION_LABEL:
        return "Storage Location:";
    case Msg::CONTAINER_VIEW_CONTAINER_LOCATION_SIZE_LABEL:
        return "Storage Size:";
    case Msg::CONTAINER_VIEW_DELETE_BUTTON_LABEL:
        return "Delete Container and its Apps";
    case Msg::CONTAINER_VIEW_DELETE_CONFIRMATION:
        return "Are you sure you want to delete the container {0}?";
    case Msg::CONTAINER_VIEW_DELETE_CONFIRMATION_WITH_APPS:
        return "Are you sure you want to delete the container {0} and the following associated apps: {1}?";
    case Msg::CONTAINER_VIEW_DELETE_BUTTON_HELP:
        return "Deleting a container will delete its data folder.  All apps associated with the container will also be deleted.  This can not be undone.";
    case Msg::CONTAINER_VIEW_MOUNT_DIR_LABEL:
        return "Mount Folder:";
    case Msg::CONTAINER_VIEW_MOUNT_DIR_HELP:
        return "Instead of copying data to the container folder, you can just mount a folder on your computer and use it in Wine as a drive";
    case Msg::CONTAINER_VIEW_SHORTCUT_LIST_LABEL:
        return "Shortcuts:";
    case Msg::CONTAINER_OPTIONS_DLG_TITLE:
        return "Container Options";
    case Msg::CONTAINER_OPTIONS_DLG_WINE_VERSION_LABEL:
        return "Wine Version:";
    case Msg::CONTAINER_OPTIONS_DLG_ADD_APP_LABEL:
        return "New Shortcut:";
    case Msg::CONTAINER_OPTIONS_DLG_ADD_APP_HELP:
        return "You can choose from a list of applications in this container to create a shortcut to.";
    case Msg::CONTAINER_OPTIONS_DLG_ADD_APP_BUTTON_LABEL:
        return "Choose App";
    case Msg::CONTAINER_OPTIONS_DOWNLOAD_WINETRICKS:
        return "Winetricks is a helper script to download and install various redistributable runtime libraries needed to run some programs in Wine.";
    case Msg::COMMON_FILESYSTEM_VERSION_LABEL:
        return "File System:";
    case Msg::COMMON_FILESYSTEM_VERSION_HELP:
        return "If you are not sure, then just use the default value which should be the most recent Wine version.  This value can not be changed after creation.";
    case Msg::UNZIP_DLG_TITLE:
        return "Unzipping ...";
    case Msg::GENERIC_BROWSE_BUTTON:
        return "Browse";
    case Msg::GENERIC_OPEN_BUTTON:
        return "Open";
    case Msg::GENERIC_DLG_OK:
        return "Ok";
    case Msg::GENERIC_DLG_YES:
        return "Yes";
    case Msg::GENERIC_DLG_NO:
        return "No";
    case Msg::GENERIC_DLG_CANCEL:
        return "Cancel";
    case Msg::GENERIC_DLG_ERROR_TITLE:
        return "Error";
    case Msg::GENERIC_DLG_CONFIRM_TITLE:
        return "Confirm";
    case Msg::GENERIC_DEFAULT:
        return "Default";
    case Msg::GENERIC_COMBOBOX_ALL:
        return "All";
    case Msg::GENERIC_DOWNLOAD:
        return "Download {0} MB";
    case Msg::MAIN_BUTTON_INSTALL:
        return "Install";
    case Msg::MAIN_BUTTON_CONTAINERS:
        return "Containers";
    case Msg::MAIN_BUTTON_SETTINGS:
        return "Options";
    case Msg::MAIN_BUTTON_HELP:
        return "Help";
    case Msg::MAIN_BUTTON_APPS:
        return "Apps";
    case Msg::INSTALLVIEW_OPEN_SETUP_FILE_TITLE:
        return "Open File";
    case Msg::GENERIC_OPEN_FOLDER_TITLE:
        return "Select Folder";
    case Msg::WAITDLG_LAUNCH_APP_TITLE:
        return "Please Wait";
    case Msg::WAITDLG_LAUNCH_APP_LABEL:
        return "Launching {0} ...";
    case Msg::WAITDLG_UNZIPPING_APP_LABEL:
        return "Unzipping ...";
    case Msg::WAITDLG_GET_FILE_LIST_TITLE:
        return "Please Wait";
    case Msg::WAITDLG_GET_FILE_LIST_LABEL:
        return "Fetching list of Wine versions";
    case Msg::DOWNLOADDLG_TITLE:
        return "Please Wait";
    case Msg::DOWNLOADDLG_LABEL:
        return "Downloading {0} ...";
    case Msg::DOWNLOADDLG_CANCELLING_LABEL:
        return "Trying to cancel download ...";
    case Msg::CONTAINER_VIEW_TITLE:
        return "Application Options";
    case Msg::CONTAINER_VIEW_NAME_LABEL:
        return "Name:";
    case Msg::CONTAINER_VIEW_NAME_HELP:
        return "This is the name you will see in the Apps list view";
    case Msg::CONTAINER_VIEW_RESOLUTION_LABEL:
        return "Resolution:";
    case Msg::CONTAINER_VIEW_RESOLUTION_HELP:
        return "This is the desktop/monitor resolution that will be used for the initial window and reported back to the application.  Normally the application will set its own resolution if it goes full screen.  This would only need to be adjusted if you want more space for applications that do not go full screen.";
    case Msg::CONTAINER_VIEW_BPP_LABEL:
        return "Bits per Pixel:";
    case Msg::CONTAINER_VIEW_BPP_HELP:
        return "Some applications might check the screen BPP before starting.  If those applications require a specific BPP, you can set it here.";        
    case Msg::CONTAINER_VIEW_FULL_SCREEN_LABEL:
        return "Fullscreen:";
    case Msg::CONTAINER_VIEW_FULL_SCREEN_HELP:
        return "Will launch the emulated desktop with the resolution selected and scale it to fit the screen.  This will not work for OpenGL so if your game uses OpenGL or Direct3D then this will have no effect.  If you are using a DirectDraw game you can click DD GDI Renderer above to force it not to use OpenGL.";
    case Msg::CONTAINER_VIEW_VSYNC_LABEL:
        return "Vsync:";
    case Msg::CONTAINER_VIEW_VSYNC_HELP:
        return "Not Set: Will use the value from the BoxedWine Options\\General screen.\n\nDisabled: The frame rate can be faster than the monitor, but may introduce artifacts in the game.\n\nEnabled: Synchronizes the frame rate of the app/game with the monitor refresh rate for better stability.\n\nAdaptive: At high framerates, VSync is enabled to eliminate tearing. At low frame rates, it's disabled to minimise stuttering.  If the video card does not support this, then this option will act like Enabled.";
    case Msg::CONTAINER_VIEW_DPI_AWARE_LABEL:
        return "DPI Aware:";
    case Msg::CONTAINER_VIEW_DPI_AWARE_HELP:
        return "DPI (dots per inch) Aware means the app will be responsible for itself when it comes to scaling to large resolutions.  If this is checked and the app can not handle it, then the app might appear to run in a window that is too small.";
    case Msg::CONTAINER_VIEW_SHOW_WINDOW_LABEL:
        return "Show Window Immediatly:";
    case Msg::CONTAINER_VIEW_SHOW_WINDOW_HELP:
        return "By default Boxedwine will hide new Windows until it looks like they will be used.  This is done to prevent a lot of Window flashing (create and destroy) when games test the system for what resolution and capabilities they will use.  Some simple OpenGL apps seem to have a problem with this feature of Boxedwine so this flag will disable it by showing the windows as soon as it is created.";
    case Msg::CONTAINER_VIEW_AUTO_REFRESH_LABEL:
        return "Direct Draw Auto Refresh:";
    case Msg::CONTAINER_VIEW_AUTO_REFRESH_HELP:
        return "Some Direct Draw games will lock the primary surface and draw directly on it.  If the game unlocks it, those changes will be flushed to the screen.  On older versions of Windows this unlock procedure was not required, but on newer Windows versions and in Wine it is.  Without the unlock the surface is never flushed and there will just be a back screen.  This hack will constantly flush the primary surface to the screen.";
    case Msg::CONTAINER_VIEW_AUTO_REFRESH_MISSING_HELP:
        return "This feature requires a newer version of the Wine file system";
    case Msg::CONTAINER_VIEW_SCALE_LABEL:
        return "Scaling:";
    case Msg::CONTAINER_VIEW_SCALE_HELP:
        return "This simple scaling option will blow up or shrink the emualted screen.  This is useful for applications that want to run in low resolution, like 640x480, and you want it to show larger on your monitor.\n\nScaling is disabled if full screen is selected because full screen will scale the emulated resolution to fill the screen.";
    case Msg::CONTAINER_VIEW_SCALE_QUALITY_LABEL:
        return "Scale Quality:";
    case Msg::CONTAINER_VIEW_SCALE_QUALITY_HELP:
        return "The scale quality options that SDL supplies.  Probably no need to ever change the default.";
    case Msg::CONTAINER_VIEW_GL_EXT_LABEL:
        return "OpenGL Extensions:";
    case Msg::CONTAINER_VIEW_GL_EXT_HELP:
        return "If empty then all extension will be allowed.\n\nSome applications, like Quake 2 and Unreal, can crash if the extension list is too large, like what is seen with modern computers.  Hit the set button to fill in a small list of common extensions or enter your own if necessary";
    case Msg::CONTAINER_VIEW_GL_EXT_SET_BUTTON_LABEL:
        return "Set";
    case Msg::CONTAINER_VIEW_DEFAULT_RESOLUTION_LABEL:
        return "Default";
    case Msg::CONTAINER_VIEW_NAME_REQUIRED:
        return "The shorcut name is required and cannot be empty.";
    case Msg::CONTAINER_VIEW_SHORTCUT_PATH_LABEL:
        return "Path:";
    case Msg::CONTAINER_VIEW_SHORTCUT_PATH_HELP:
        return "This is the path in the container that will be launched when starting this app.";
    case Msg::CONTAINER_VIEW_SHORTCUT_ARGUMENTS_LABEL:
        return "Arguments:";
    case Msg::CONTAINER_VIEW_SHORTCUT_ARGUMENTS_HELP:
        return "These arguments will be passed to the app.  Place each argument on its own line.";
    case Msg::CONTAINER_VIEW_DELETE_SHORTCUT:
        return "Delete Shortcut";
    case Msg::CONTAINER_VIEW_CREATE_AUTOMATION:
        return "Create Automation";
    case Msg::CONTAINER_VIEW_RUN_AUTOMATION:
        return "Run Automation";
    case Msg::CONTAINER_VIEW_SHORTCUT_LIST_HELP:
        return "Shortcuts are links to apps inside a container.  They will appear in the app list and clicking them in the app list will launch the app with the settings you set here.";
    case Msg::CONTAINER_VIEW_DELETE_SHORTCUT_CONFIRMATION:
        return "Are you sure you want to the {0} shortcut?";
    case Msg::CONTAINER_VIEW_GDI_RENDERER_LABEL:
        return "DD GDI Renderer:";
    case Msg::CONTAINER_VIEW_RENDERER_LABEL:
        return "Renderer:";
    case Msg::CONTAINER_VIEW_RENDERER_HELP:
        return "This requires Wine 6.0 or higher.  DirectDraw will use OpenGL by default.  But a few games, like Diable and Startcraft, may have graphical issues.  Sometimes changing the renderer to GDI can help.  Vulkan is considered experimental.";
    case Msg::CONTAINER_VIEW_PROGRAMS_LABEL:
        return "Run in Container:";
    case Msg::CONTAINER_VIEW_COMPONENTS_LABEL:
        return "Components:";
    case Msg::CONTAINER_VIEW_WINETRICKS_LABEL:
        return "Winetricks:";
    case Msg::CONTAINER_VIEW_WINETRICKS_FONTS_LABEL:
        return "Winetricks fonts:";
    case Msg::CONTAINER_VIEW_WINETRICKS_DLLS_LABEL:
        return "Winetricks Dlls:";
    case Msg::CONTAINER_VIEW_TINY_CORE_LABEL:
        return "Packages:";
    case Msg::CONTAINER_OPTIONS_DOWNLOAD_PACKAGE:
        return "This container uses Tiny Core Linux.  You maybe install a package for it.  If you find that the file system is missing a package that Wine needs, let me know by creating a github issue.";
    case Msg::CONTAINER_VIEW_REGEDIT_BUTTON_LABEL:
        return "Regedit";
    case Msg::CONTAINER_VIEW_WINECFG_BUTTON_LABEL:
        return "WineCfg";
    case Msg::CONTAINER_VIEW_GDI_RENDERER_HELP:
        return "DirectDraw will use OpenGL by default.  But a few games, like Diable and Startcraft, may have graphical issues.  Sometimes changing the renderer to GDI can help.";
    case Msg::CONTAINER_VIEW_MOUSE_WARP_LABEL:
        return "Mouse Warp Override:";
    case Msg::CONTAINER_VIEW_MOUSE_WARP_HELP:
        return "Enable:  (default) warp pointer when mouse exclusively acquired\nDisable : never warp the mouse pointer\nForce : always warp the pointer";
    case Msg::CONTAINER_VIEW_WINDOWS_VERION_LABEL:
        return "Windows Version:";
    case Msg::CONTAINER_VIEW_WINDOWS_VERION_HELP:
        return "This is the Windows version that will be reported to the running app.  You probably never need to change this unless the app asks for a specific version of Windows.";
    case Msg::CONTAINER_VIEW_CPU_AFFINITY_LABEL:
        return "CPU Affinity:";
    case Msg::CONTAINER_VIEW_CPU_AFFINITY_HELP:
        return "This is the number of host CPUs the app can use.  This should almost always be set to All.  But sometimes there will be an older game, around the late 90s, that requires this to be set to 1";
    case Msg::CONTAINER_VIEW_SELECT_WINE_APP_LABEL:
        return "Select which Wine app you would like to run in this container.";
    case Msg::CONTAINER_VIEW_SELECT_WINE_APP_DLG_TITLE:
        return "Run Wine App";
    case Msg::CONTAINER_VIEW_RUNE_APP_BUTTON_LABEL:
        return "Run App";
    case Msg::CONTAINER_VIEW_POLL_RATE_LABEL:
        return "Poll Rate:";
    case Msg::CONTAINER_VIEW_POLL_RATE_HELP:
        return "Poll Rate is how often a mouse or keyboard event will be given to Wine.  If the event happens before allowed, the event will be paused or in the case Msg::Of a mouse move event, dropped.  Some games will experience lockups/hangs when moving the mouse quickly with a high DPI mouse.  Lowering this number can help with that.  A value of 0 means there will be no delay.";
    case Msg::CONTAINER_VIEW_SKIP_FRAMES_LABEL:
        return "Skip Frames:";
    case Msg::CONTAINER_VIEW_SKIP_FRAMES_HELP:
            return "Some games draw directly to the screen/window instead of using DirectX or OpenGL.  There is no reason to update to the screen more than the refresh rate of the monitor, so if a game updates the screen/window by drawing small updates each time, this could result in things like 900 updates per seconds, which would make Boxedwine update the monitor at 900 FPS.  This is a waste of CPU and could slow down the emulation.\n\nIf your app/game is running slow and it doesn't use DirectX or OpenGL, you could try changing this value to 30 or 60.";
    case Msg::ERROR_NO_WINE:
        return "There are no versions of Wine installed.  Would you like to install the default version of Wine now?";
    case Msg::ERROR_NO_WINE_HIGH_DPI:
        return "There are no versions of Wine installed.  Would you like to install the default version of Wine now?  Installing a version of Wine will also fix the small fonts you are seeing.";
    case Msg::ERROR_NO_FILE_LIST:
        return "Was unable to download the list of Wine versions.  Boxedwine will not work without a Wine file system.  Make sure your internet is working and you can try again by re-launching Boxedwine.  You can also manually download a Wine file system from https://sourceforge.net/projects/boxedwine/files/FileSystems/ and place it in the same directory as the Boxedwine application";
    case Msg::ERROR_MISSING_FILE_SYSTEM:
        return "The file system is missing.  Would you like to download it now?";
    case Msg::WINE_UPGRADE_AVAILABLE_TITLE:
        return "Upgrade Wine?";
    case Msg::WINE_UPGRADE_AVAILABLE_LABEL:
        return "Would you like to upgrade Wine?  Upgrades are available for: {0}";
    default:
        if (useDefaultIfMissing) {
            return "Unknown msg";
        }
        return nullptr;
    }
}

BString getTranslation(Msg msg, bool useDefaultIfMissing) {
    return B(c_getTranslation(msg, useDefaultIfMissing));
}