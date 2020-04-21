#ifndef __BOXEDWINEUI_H__
#define __BOXEDWINEUI_H__

#include <thread>

#undef ID
#include <GL/glew.h>
#include <imgui.h>
#include "platformhelper.h"
#include "utils/boxedTranslation.h"
#include "utils/uihelper.h"
#include "utils/readIcons.h"

#include "data/boxedwineData.h"
#include "data/globalSettings.h"
#include "data/configFile.h"
#include "data/boxedApp.h"
#include "data/boxedContainer.h"
#include "data/boxedReg.h"

#include "controls/ImGuiLayout.h"
#include "controls/baseView.h"
#include "controls/spinnerControl.h"
#include "controls/uiSettings.h"
#include "controls/baseDlg.h"
#include "controls/appbar.h"
#include "controls/listViewItem.h"
#include "controls/listView.h"
#include "controls/containersView.h"
#include "controls/optionsView.h"
#include "controls/appChooserDlg.h"
#include "controls/waitDlg.h"
#include "controls/downloadDlg.h"
#include "controls/yesNoDlg.h"
#include "controls/okDlg.h"
#include "controls/installView.h"
#include "platformhelper.h"

#include "mainui.h"

#define SAFE_IMGUI_TEXT(x) ImGui::Text("%s", x)
#define SAFE_IMGUI_TEXT_DISABLED(x) ImGui::TextDisabled("%s", x)

#define EXTRA_VERTICAL_SPACING 5
#define TOOL_TIP_WIDTH 28

#endif
