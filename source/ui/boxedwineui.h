/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

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

#include "data/boxedTexture.h"
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
#include "controls/helpView.h"
#include "controls/appChooserDlg.h"
#include "controls/waitDlg.h"
#include "controls/downloadDlg.h"
#include "controls/unzipDlg.h"
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
