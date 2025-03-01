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

#ifndef __BASE_DLG_H__
#define __BASE_DLG_H__

class BaseDlg {
public:
    BaseDlg(Msg title, int width, int height, ImFont* font = nullptr, BaseDlg* parent = nullptr);
    virtual ~BaseDlg() {}

    static void runDialogs();
    static void stopAllDialogs();
protected:
    virtual void run() = 0;
    virtual void onOk(bool buttonClicked);

    virtual void done();
    void toolTip(const char* desc);
    void addOkAndCancelButtons();
    void addCancelButton();
    float getReservedHeightForButtons();
    float getOuterFramePadding();

    int width;
    int height;
    float toolTipWidth;
    float extraVerticalSpacing;
private:
    static std::list<BaseDlg*> activeDialogs;

    float getButtonRowY();
    void runIfVisible();

    const char* title;
    bool isDone;
    BaseDlg* child;
    ImFont* font;
};
#endif