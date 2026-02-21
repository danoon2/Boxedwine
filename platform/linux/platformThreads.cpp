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

#ifdef BOXEDWINE_HOST_EXCEPTIONS

#include <signal.h>

void platformHandler(int sig, siginfo_t* info, void* vcontext);

#ifdef __MACH__
#include <mach/task.h>
#include <mach/mach_init.h>
#include <mach/mach_port.h>
#endif

void platformInitExceptionHandling() {
    static bool initializedHandler = false;
    if (!initializedHandler) {
        struct sigaction sa;
        sa.sa_sigaction = platformHandler;
        sa.sa_flags = SA_SIGINFO;
        struct sigaction oldsa;
        sigaction(SIGBUS, &sa, &oldsa);
        sigaction(SIGSEGV, &sa, &oldsa);
        //sigaction(SIGILL, &sa, &oldsa);
        sigaction(SIGFPE, &sa, &oldsa);
        //for (int i = 0x91; i <= 0x96; i++) {
        //    sigaction(i, &sa, &oldsa);
        //}
        //sigaction(SIGTRAP, &sa, &oldsa);
        initializedHandler = true;
#ifdef __MACH__
        // proc hand -p true -s false SIGSEGV
        // proc hand -p true -s false SIGBUS
        // in the debug out put window, (lldb) enter the above 2 commands in order to run while debugging on Mac

        // set a break point on this line then enter the above commands.

        task_set_exception_ports(mach_task_self(), EXC_MASK_BAD_ACCESS | EXC_MASK_BAD_INSTRUCTION, MACH_PORT_NULL, EXCEPTION_DEFAULT, 0);
#endif
    }
}

#endif