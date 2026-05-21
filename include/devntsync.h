/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __DEVNTSYNC_H__
#define __DEVNTSYNC_H__

FsOpenNode* openDevNTSync(const std::shared_ptr<FsNode>& node, U32 flags, U32 data);

#endif
