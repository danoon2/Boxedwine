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

#ifndef __APP_FILE_H__
#define __APP_FILE_H__

class BoxedTexture;
class BoxedContainer;
class BoxedApp;

class AppFile {
public:
    AppFile(BString name, BString installType, BString iconPath, BString filePath, U32 size, BString exe, BString exeOptions, BString help, BString optionsName, BString installOptions, BString installExe, const std::vector<BString>& args);
    BString name;
    BString optionsName;
    BString installType;
    BString filePath;
    BString iconPath;
    BString localIconPath;
    BString localFilePath;
    U32 size;
    BString exe;
    std::vector<BString> args;
    BString installExe;
    std::vector<BString> installOptions;
    std::vector<BString> exeOptions;
    BString help;

    std::shared_ptr<BoxedTexture> iconTexture;

    void buildIconTexture();
    void install(bool chooseShortCut=true, BoxedContainer* container=nullptr);    

    bool operator<(const AppFile& rhs) const { return name < rhs.name; }

private:
    static void runOptions(BoxedContainer* container, BoxedApp* app, const std::vector<BString>& options, std::list< std::function<bool() > >& runner, std::list<AppFile*>& downloads);
    void install(bool chooseShortCut, BoxedContainer* container, std::list< std::function<bool() > >& runner, std::list<AppFile*>& downloads);
};

#endif