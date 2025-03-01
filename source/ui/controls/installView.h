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

#ifndef __INSTALL_VIEW_H__
#define __INSTALL_VIEW_H__

class InstallView : public BaseView {
public:
	InstallView(BString initialFileOrDirPath, BString tab=BString());

    // from BaseView
	bool saveChanges() override;

private:
    void createInstallTab(BString initialFileOrDirPath);
    void createDemoTab();
    void runApps(std::vector<AppFile>& apps);
    void setContainerName();
    void setWindowsVersionDefault();

    void onInstall();   

    std::shared_ptr<LayoutComboboxControl> installTypeControl;
    std::shared_ptr<LayoutTextInputControl> locationControl;
    std::shared_ptr<LayoutComboboxControl> containerControl;
    std::shared_ptr<LayoutSection> containerSection;
    std::shared_ptr<LayoutTextInputControl> containerNameControl;
    std::shared_ptr<LayoutComboboxControl> fileSystemVersionControl;
    std::shared_ptr<LayoutComboboxControl> windowsVersionControl;
};

#endif