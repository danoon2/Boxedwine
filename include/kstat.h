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

#ifndef __KSTAT_H__
#define __KSTAT_H__

#define K_S_IFMT         0xF000
#define K_S_IFSOCK       0xC000
#define K__S_IFLNK		 0xA000
#define K__S_IFDIR       0x4000
#define K__S_IFCHR       0x2000
#define K__S_IFIFO       0x1000
#define K__S_IFREG       0x8000
#define K__S_IREAD       0x0100
#define K__S_IWRITE      0x0080
#define K__S_IEXEC       0x0040

#define K__S_IRGRP       0x0020
#define K__S_IWGRP		 0x0010
#define K__S_IXGRP		 0x0008

#define K__S_IROTH		 0x0004
#define K__S_IWOTH		 0x0002
#define K__S_IXOTH		 0x0001

#define	K__S_ISUID	 0x800		/* set user id on execution */
#define	K__S_ISGID	 0x400		/* set group id on execution */
#define	K__S_ISVTX	 0x200

#define	K__S_IRWXU  0x1c0
#define	K__S_IRWXG	 0x38
#define	K__S_IRWXO	 0x7
#endif