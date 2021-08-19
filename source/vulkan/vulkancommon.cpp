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
#include "vk_host.h"
#include "vkdef.h"

Int99Callback int9ACallback[VK_LAST_VALUE+1];
U32 int9ACallbackSize;

#define VKINSTANCEPROC(p) if (!p) {p = (PFN_##p)vkGetInstanceProcAddr(Instance, #p); \
    if (!p) \
        Error("Failed to get address of \""#p"\"");}

void vulkan_init() {
    int99CallbackSize = VK_LAST_VALUE+1;

#undef VKFUNC
#define VKFUNC(name) int9ACallback[name] = vk_##name;
#include "vkfuncs.h"      
}


void callVulkan(CPU* cpu, U32 index) {
#ifdef BOXEDWINE_VULKAN
    if (index < int9ACallbackSize) {
        if (int9ACallback[index]) {
            int9ACallback[index](cpu);
        } else {
            kpanic("Vulkan tried to call missing function: %d", index);
        }
    } else 
#endif
    {
        kpanic("Vulkan not compiled into Boxedwine: %d", index);
    }
}
