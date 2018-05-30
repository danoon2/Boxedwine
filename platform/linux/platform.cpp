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

#include <sys/time.h>
#include <dirent.h>
#include <string.h>

#include "log.h"
#include "pixelformat.h"
#include "fsapi.h"

unsigned long long int getSystemTimeAsMicroSeconds() {
	struct timeval  tv;
	gettimeofday(&tv, NULL);
	return ((unsigned long long int)tv.tv_sec) * 1000000l + (tv.tv_usec);
}

//#ifdef __EMSCRIPTEN__
#ifdef __THIS_HANGS__
// error TypeError: asm.js type error: missing definition of function _testSetjmp
#include <emscripten.h>
void startMicroCounter()
{    
}

unsigned long long int getMicroCounter()
{
    return (unsigned long long int)(emscripten_get_now()*1000000.0);
}
#else
long long int CounterStart;
void startMicroCounter()
{
    CounterStart = getSystemTimeAsMicroSeconds();
}

unsigned long long int getMicroCounter()
{
    return getSystemTimeAsMicroSeconds()-CounterStart;
}
#endif

int listNodes(struct FsNode* dir, struct FsNode** nodes, int maxCount) {
	DIR *dp = NULL;
	struct dirent *dptr = NULL;
	U32 result = 0;

	dp = opendir(dir->reserved1);
	if (dp) {
		nodes[result++]=getNodeFromLocalPath(dir->path, ".", FALSE);
                nodes[result++]=getNodeFromLocalPath(dir->path, "..", FALSE);
        	while(NULL != (dptr = readdir(dp))) {
			if (strcmp(dptr->d_name, ".") && strcmp(dptr->d_name, ".."))  {
				nodes[result] = getNodeFromLocalPath(dir->path, dptr->d_name, TRUE);
				result++;
				if (result==maxCount) {
					kwarn("hit the maximum number of files that can be returned in a director for %s", dir->reserved1);
					break;
				}
			}
        	}
        	closedir(dp);
    	}
	return result;
}

int getPixelFormats(PixelFormat* pfs, int maxPfs) {
    pfs[1].nSize = 40;
    pfs[1].nVersion = 1;
    pfs[1].dwFlags = K_PFD_SUPPORT_OPENGL|K_PFD_DRAW_TO_WINDOW|K_PFD_DOUBLEBUFFER;
    pfs[1].iPixelType = K_PFD_TYPE_RGBA;                  
    pfs[1].cRedBits = 8;
    pfs[1].cGreenBits = 8;
    pfs[1].cBlueBits = 8;
    pfs[1].cAlphaBits = 0;
    pfs[1].cAccumRedBits = 16;
    pfs[1].cAccumGreenBits = 16;
    pfs[1].cAccumBlueBits = 16;
    pfs[1].cAccumAlphaBits = 16;
    pfs[1].cAccumBits = 64;
    pfs[1].cDepthBits = 24;
    pfs[1].cStencilBits = 8;
    pfs[1].cColorBits = 32;
    memcpy(&pfs[2], &pfs[1], sizeof(PixelFormat));
    pfs[1].dwFlags|=K_PFD_GENERIC_FORMAT;
    return 2;
}
