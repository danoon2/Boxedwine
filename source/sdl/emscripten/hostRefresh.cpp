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

#include "boxedwine.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>

extern "C" {

// Called by the browser host folder refresh after it changed files under the
// native directory mounted at mountPath. relativeDirs holds one directory per
// line, as a native path relative to the mount ("." for the mount itself).
// Only directories whose children were already loaded are updated, and only
// entries that changed are added or removed: unchanged nodes, including open
// files, keep their identity. In multi-threaded builds this runs on the browser
// thread while guest threads use the same nodes, so all changes go through the
// FsNode locks.
EMSCRIPTEN_KEEPALIVE
void boxedwineHostFolderSyncDirectories(const char* mountPath, const char* relativeDirs) {
    if (!mountPath || !mountPath[0] || !relativeDirs) {
        return;
    }
    std::shared_ptr<FsNode> mountNode = Fs::getNodeFromLocalPath(B(""), BString::copy(mountPath), true);
    if (!mountNode) {
        return;
    }
    std::vector<BString> dirs;
    BString::copy(relativeDirs).split('\n', dirs);
    for (auto& dir : dirs) {
        std::shared_ptr<FsNode> node = mountNode;
        std::vector<BString> parts;
        dir.split('/', parts);
        for (auto& part : parts) {
            if (!node) {
                break;
            }
            if (part.length() == 0 || part == ".") {
                continue;
            }
            node = node->getLoadedChildByNativePath(node->nativePath.stringByApppendingPath(part));
        }
        if (node && node->isDirectory()) {
            node->syncLoadedChildrenWithFileSystem();
        }
    }
}

}

#endif
