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

#ifndef __OPTIONS_VIEW_H__
#define __OPTIONS_VIEW_H__

#include <map>

class OptionsViewWineVersion {
public:
	OptionsViewWineVersion() = default;
	std::shared_ptr<FileSystemZip> currentVersion;
	std::shared_ptr<FileSystemZip> availableVersion;
	BString name;
	BString size;
};

class OptionsView : public BaseView {
public:
	OptionsView(BString tab);

	// from BaseView
	bool saveChanges() override;

private:
	void createGeneralTab();
	void createThemeTab();

	void runWineOptions();	
	void download(const std::shared_ptr<FileSystemZip>& version);
	void loadFileSystemVersions();

	const char* wineTitle;
	float wineButtonTotalColumnWidth = 0.0f;
	float wineButtonFirstColumnWidth = 0.0f;
	std::map<BString, OptionsViewWineVersion, std::greater<BString>> fileSystemVersions;

	// General
	std::shared_ptr<LayoutTextInputControl> saveLocationControl;
	std::shared_ptr<LayoutComboboxControl> resolutionControl;
	std::shared_ptr<LayoutComboboxControl> vsyncControl;
	std::shared_ptr<LayoutComboboxControl> scaleControl;
	std::shared_ptr<LayoutComboboxControl> openGlControl;
	std::shared_ptr<LayoutCheckboxControl> automationControl;

	// Display
	std::shared_ptr<LayoutComboboxControl> themeControl;
};

#endif