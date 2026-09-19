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

#ifndef __LOG_H__
#define __LOG_H__

void internal_log(BString msg, FILE* f);
void internal_kpanic(BString msg);

void kpanic(const char* msg);

#ifdef __clang__
// Clang supports printf checking on template parameter packs as an extension.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgcc-compat"
#endif

template <class... Args>
#ifdef __clang__
__attribute__((format(printf, 1, 2)))
#endif
#ifdef BOXEDWINE_MSVC
__declspec(noreturn)
#endif
void kpanic_fmt(const char* format, Args&&... args) {
	auto size = std::snprintf(nullptr, 0, format, std::forward<Args>(args)...);
	BString msg(size + 1, '\0');
	std::snprintf(msg.str(), size + 1, format, std::forward<Args>(args)...);
	msg += "\n";
	internal_kpanic(msg);
}

void kwarn(const char* msg);

template <class... Args>
#ifdef __clang__
__attribute__((format(printf, 1, 2)))
#endif
void kwarn_fmt(const char* format, Args&&... args) {
	auto size = std::snprintf(nullptr, 0, format, std::forward<Args>(args)...);
	BString msg(size + 1, '\0');
	std::snprintf(msg.str(), size + 1, format, std::forward<Args>(args)...);
	msg += "\n";
	internal_log(msg, stdout);
}

void klog(const char* msg);
void klog_nonewline(const char* msg);

template <class... Args>
#ifdef __clang__
__attribute__((format(printf, 1, 2)))
#endif
void klog_fmt(const char* format, Args&&... args) {
	auto size = std::snprintf(nullptr, 0, format, std::forward<Args>(args)...);
	BString msg(size + 1, '\0');
	std::snprintf(msg.str(), size + 1, format, std::forward<Args>(args)...);
	msg += "\n";
	internal_log(msg, stdout);
}

template <class... Args>
#ifdef __clang__
__attribute__((format(printf, 1, 2)))
#endif
void klog_nonewline_fmt(const char* format, Args&&... args) {
	auto size = std::snprintf(nullptr, 0, format, std::forward<Args>(args)...);
	BString msg(size + 1, '\0');
	std::snprintf(msg.str(), size + 1, format, std::forward<Args>(args)...);
	internal_log(msg, stdout);
}

template <class... Args>
#ifdef __clang__
__attribute__((format(printf, 1, 2)))
#endif
void kdebug(const char* format, Args&&... args) {
#ifdef _DEBUG
	auto size = std::snprintf(nullptr, 0, format, std::forward<Args>(args)...);
	BString msg(size + 1, '\0');
	std::snprintf(msg.str(), size + 1, format, std::forward<Args>(args)...);
	msg += "\n";
	internal_log(msg, stderr);
#endif
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif
