/*
 *  Copyright (C) 2016  The BoxedWine Team
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

#ifndef __LOG_H__
#define __LOG_H__

void internal_log(BString msg, FILE* f);
void internal_kpanic(BString msg);

template <class... Args>
#ifdef BOXEDWINE_MSVC
__declspec(noreturn)
#endif
void kpanic(const char* format, Args&&... args) {
	auto size = std::snprintf(nullptr, 0, format, std::forward<Args>(args)...);
	BString msg(size + 1, '\0');
	std::snprintf(msg.str(), size + 1, format, std::forward<Args>(args)...);
	msg += "\n";
	internal_kpanic(msg);
}

template <class... Args>
void kwarn(const char* format, Args&&... args) {
	auto size = std::snprintf(nullptr, 0, format, std::forward<Args>(args)...);
	BString msg(size + 1, '\0');
	std::snprintf(msg.str(), size + 1, format, std::forward<Args>(args)...);
	msg += "\n";
	internal_log(msg, stdout);
}

template <class... Args>
void klog(const char* format, Args&&... args) {
	auto size = std::snprintf(nullptr, 0, format, std::forward<Args>(args)...);
	BString msg(size + 1, '\0');
	std::snprintf(msg.str(), size + 1, format, std::forward<Args>(args)...);
	msg += "\n";
	internal_log(msg, stdout);
}

template <class... Args>
void klog_nonewline(const char* format, Args&&... args) {
	auto size = std::snprintf(nullptr, 0, format, std::forward<Args>(args)...);
	BString msg(size + 1, '\0');
	std::snprintf(msg.str(), size + 1, format, std::forward<Args>(args)...);
	internal_log(msg, stdout);
}

template <class... Args>
void kdebug(const char* format, Args&&... args) {
#ifdef _DEBUG
	auto size = std::snprintf(nullptr, 0, format, std::forward<Args>(args)...);
	BString msg(size + 1, '\0');
	std::snprintf(msg.str(), size + 1, format, std::forward<Args>(args)...);
	msg += "\n";
	internal_log(msg, stderr);
#endif
}
#endif
