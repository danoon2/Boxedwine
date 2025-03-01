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

#ifndef __NETWORK_UTILS_H__
#define __NETWORK_UTILS_H__

class NetworkProxy {
public:
	BString host;
	U32 port;
	BString username;
	BString password;
};

bool downloadFile(BString url, BString filePath, std::function<void(U64 bytesCompleted)> f, NetworkProxy* proxy, BString& errorMsg, bool* cancel=nullptr);

#endif