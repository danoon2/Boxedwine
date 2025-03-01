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

#ifndef __APP_CHOOSER_DLG_H__
#define __APP_CHOOSER_DLG_H__

class AppChooserDlg : public BaseDlg {
public:
    AppChooserDlg(std::vector<BoxedApp>& items, std::vector<BoxedApp>& wineApps, std::function<void(BoxedApp)> onSelected, bool saveApp = true, BaseDlg* parent=nullptr);
    AppChooserDlg(std::vector<BoxedApp>& items, std::function<void(BoxedApp)> onSelected, bool saveApp = true, BaseDlg* parent = nullptr, Msg titleId = Msg::APPCHOOSER_DLG_TITLE);

    void setLabelId(Msg id) {this->labelId = id;}
protected:
    // from BaseDlg
    void run() override;
    void onOk(bool buttonClicked) override;

private:
    void drawItems(std::vector<BoxedApp>& apps, int startingIndex);

    std::vector<BoxedApp> items;
    std::vector<BoxedApp> wineApps;
    std::function<void(BoxedApp app)> onSelected;
    Msg labelId;
    bool saveApp;
};

#endif