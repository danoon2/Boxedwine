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

#ifndef __BASE_VIEW_H__
#define __BASE_VIEW_H__

class BaseViewTab {
public:
	BaseViewTab(BString id, BString name, const std::shared_ptr<ImGuiLayout>& model, std::function<void(bool buttonPressed, BaseViewTab& tab)> drawTab, std::function<void()> drawTabIcon, std::function<void()> onRightClick) : name(name), drawTab(drawTab), model(model), drawTabIcon(drawTabIcon), onRightClick(onRightClick), id(id) {}
	BString name;
	std::function<void(bool buttonPressed, BaseViewTab& tab)> drawTab;
	std::shared_ptr<ImGuiLayout> model;
	std::function<void()> drawTabIcon;
	std::function<void()> onRightClick;
	BString id;
};

class BaseView {
public:
	BaseView(BString viewName);
        virtual ~BaseView() {}

	virtual bool saveChanges()=0;
	void run(const ImVec2& size);

protected:
	float toolTipWidth;
	float extraVerticalSpacing;

	void addTab(BString id, BString name, const std::shared_ptr<ImGuiLayout>& model, std::function<void(bool buttonPressed, BaseViewTab& tab)> drawTab, std::function<void()> drawTabIcon=nullptr, std::function<void()> onRightClick = nullptr);
	void runErrorMsg(bool open);
	int getTabCount() {return (int)tabs.size();}
	void setTabName(int index, BString name) { tabs[index].name = name; }
	void drawToolTip(BString help);

	std::shared_ptr<LayoutComboboxControl> createWindowsVersionCombobox(const std::shared_ptr<LayoutSection>& section);
	std::shared_ptr<LayoutComboboxControl> createFileSystemVersionCombobox(const std::shared_ptr<LayoutSection>& section);

	BString errorMsg;
	BString errorMsgString;
	int tabIndex;
private:
	void addTab(const BaseViewTab& tab, int index);
		
	BString viewName;
	std::vector<BaseViewTab> tabs;
	bool errorMsgOpen;
	bool tabChanged;
};

#endif
