#ifndef __BOXEDWINEUI_H__
#define __BOXEDWINEUI_H__

#include <thread>

#undef ID
#include "GL/glew.h"
#include "imgui.h"
#include "platformhelper.h"
#include "utils/boxedTranslation.h"
#include "utils/uihelper.h"
#include "utils/readIcons.h"

#include "data/boxedwineData.h"
#include "data/globalSettings.h"
#include "data/configFile.h"
#include "data/boxedApp.h"
#include "data/boxedContainer.h"

#include "controls/spinnerControl.h"
#include "controls/uiSettings.h"
#include "controls/baseDlg.h"
#include "controls/appbar.h"
#include "controls/installDlg.h"
#include "controls/listViewItem.h"
#include "controls/listView.h"
#include "controls/containersView.h"
#include "controls/containerOptionsDlg.h"
#include "controls/settingsDlg.h"
#include "controls/appChooserDlg.h"
#include "controls/waitDlg.h"

#include "mainui.h"

#define SAFE_IMGUI_TEXT(x) ImGui::Text("%s", x)
#define SAFE_IMGUI_TEXT_DISABLED(x) ImGui::TextDisabled("%s", x)

#endif