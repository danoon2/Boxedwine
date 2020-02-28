#ifndef __BOXEDWINEUI_H__
#define __BOXEDWINEUI_H__

#undef ID
#include "GL/glew.h"
#include "imgui.h"
#include "utils/boxedTranslation.h"
#include "utils/uihelper.h"

#include "data/boxedwineData.h"
#include "data/globalSettings.h"
#include "data/configFile.h"
#include "data/boxedApp.h"
#include "data/boxedContainer.h"

#include "controls/appbar.h"
#include "controls/installDlg.h"
#include "controls/listViewItem.h"
#include "controls/listView.h"
#include "controls/containersView.h"

#define SAFE_IMGUI_TEXT(x) ImGui::Text("%s", x)
#define SAFE_IMGUI_TEXT_DISABLED(x) ImGui::TextDisabled("%s", x)
#endif