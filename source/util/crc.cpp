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
#include "boxedwine.h"
#include <sys/stat.h>

unsigned int crc32b(unsigned char *message, int len) {
    unsigned int h = 0, g = 0;
    int i;

    for (i=0;i<len;i++) {
        h = (h << 4) + message[i];
        g = h & 0xf0000000;
        if (g)
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}

unsigned int crc32File(BString filePath) {
    PLATFORM_STAT_STRUCT buf;
    if (PLATFORM_STAT(filePath.c_str(), &buf) == 0 && buf.st_size) {
        BReadFile f(filePath);
        if (f.isOpen()) {
            unsigned char* buffer = new unsigned char[(int)buf.st_size];
            if ((U64)f.read(buffer, buf.st_size) == (U64)buf.st_size) {
                unsigned int result = crc32b(buffer, (int)buf.st_size);
                delete[] buffer;
                return result;
            }
            delete[] buffer;
        }
    }    
    return 0;
}
