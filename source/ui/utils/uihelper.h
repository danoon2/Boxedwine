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

#ifndef __UIHELPER_H__
#define __UIHELPER_H__

class BoxedContainer;

bool showMessageBox(BString id, bool open, const char* title, const char* msg);
bool showYesNoMessageBox(BString id, bool open, const char* title, const char* msg, bool* yes);
BString getReadableSize(U64 bytes);
void alignNextTextRightInColumn(const char* text);
void askToDownloadDefaultWine();

#define VIEW_APPS 0
#define VIEW_INSTALL 1
#define VIEW_CONTAINERS 2
#define VIEW_OPTIONS 3
#define VIEW_HELP 4

void gotoView(int viewId, BString tab=BString(), BString param1=BString());

class UIDisableStyle {
public:
	UIDisableStyle(bool disabled=true);
	~UIDisableStyle();

private:
	bool disabled;
};

#endif