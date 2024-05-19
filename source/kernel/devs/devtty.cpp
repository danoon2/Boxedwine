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

#include "../../io/fsvirtualopennode.h"

#include UNISTD
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define VT_AUTO 0
#define VT_PROCESS 1
#define VT_ACKACQ 2   

#define K_RAW 0x00
#define K_XLATE 0x01
#define K_MEDIUMRAW 0x02
#define K_UNICODE 0x03
#define KDSKBMODE 0x04

static U32 activeTTY;

class DevTTY : public FsVirtualOpenNode {
public:
    DevTTY(const std::shared_ptr<FsNode>& node, U32 flags) : FsVirtualOpenNode(node, flags), 
        c_iflag(0),
        c_oflag(0),
        c_cflag(0),
        c_lflag(0),
        c_line(0),
        mode(VT_AUTO), 
        kbMode(K_UNICODE),
        waitv(0),
        relsig(0), 
        acqsig(0),
        graphics(false) {memset(c_cc, 0, sizeof(c_cc));}

    // From FsOpenNode
    U32 ioctl(KThread* thread, U32 request) override;
    U32 readNative(U8* buffer, U32 len)  override {return 0;}
    U32 writeNative(U8* buffer, U32 len) override;

private:
    void readTermios(KMemory* memory, U32 address);
    void writeTermios(KMemory* memory, U32 address);

    U32 c_iflag;               /* input mode flags */
    U32 c_oflag;               /* output mode flags */
    U32 c_cflag;               /* control mode flags */
    U32 c_lflag;               /* local mode flags */
    U32 c_line;                    /* line discipline */
    U32 c_cc[19];                /* control characters */

    U32 mode;
    U32 kbMode;

    U32 waitv;
    U32 relsig;
    U32 acqsig;
    bool graphics;
};

FsOpenNode* openDevTTY(const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
    return new DevTTY(node, flags);
}


void DevTTY::readTermios(KMemory* memory, U32 address) {
    this->c_iflag = memory->readd(address);
    this->c_oflag = memory->readd(address+4);
    this->c_cflag = memory->readd(address+8);
    this->c_lflag = memory->readd(address+12);
    this->c_line = memory->readb(address+16);
    for (int i=0;i<19;i++) {
        this->c_cc[i] = memory->readb(address+17+i);
    }
}

void DevTTY::writeTermios(KMemory* memory, U32 address) {
    memory->writed(address, this->c_iflag);
    memory->writed(address+4, this->c_oflag);
    memory->writed(address+8, this->c_cflag);
    memory->writed(address+12, this->c_lflag);
    memory->writeb(address+16, this->c_line);
    for (int i=0;i<19;i++) {
        memory->writeb(address+17+i, this->c_cc[i]);
    }
}

U32 DevTTY::writeNative(U8* buffer, U32 len) {
    BString s = BString::copy((char*)buffer, len);
    // winemenubuilder was removed because it is not necessary and this will speed up start time
    if (s.contains("winemenubuilder")) {
        return len;
    }
    // for now I don't want users seeing this and assuming its a problem
    if (s.contains("WS_getaddrinfo Failed to resolve your host name IP")) {
        return len;
    }
    if (KSystem::logFile.isOpen()) {
        KSystem::logFile.write((char*)buffer, len);
    }
    if (KSystem::watchTTY) {
        KSystem::watchTTY(s);
    }
    if (KSystem::ttyPrepend) {
        thread_local static bool newLine = true;
        if (newLine) {
            static_cast<void>(::write(1, "TTY:", 4));
            BString name = KThread::currentThread()->process->name;
            static_cast<void>(::write(1, name.c_str(), (U32)name.length()));
            static_cast<void>(::write(1, ":", 1));
            newLine = false;
        }
        if (buffer[len - 1] == '\n') {
            newLine = true;
        }
    }
    return (U32)::write(1, buffer, len);    
}


U32 DevTTY::ioctl(KThread* thread, U32 request) {
    CPU* cpu = thread->cpu;
    KMemory* memory = thread->memory;

    switch (request) {
        case 0x4B32: // KBSETLED
            break;
        case 0x4B3A: // KDSETMODE
            this->graphics = IOCTL_ARG1==1;
            break;
        case 0x4B44: // KDGKBMODE
            memory->writed(IOCTL_ARG1, this->kbMode);
            break;
        case 0x4B45: // KDSKBMODE
            this->kbMode = IOCTL_ARG1;
            break;
        case 0x4B46: { // KDGKBENT
            U32 kbentry = IOCTL_ARG1;
            U32 table = memory->readb(kbentry);
            U32 index = memory->readb(kbentry + 1);
            //U32 value = readw(kbentry+2);
            switch (table) {
                case 0: // K_NORMTAB
                    memory->writew(kbentry + 2, index);
                    break;
                case 1: // K_SHIFTTAB
                    memory->writew(kbentry + 2, toupper((char)index));
                    break;
                case 2: // K_ALTTAB
                    memory->writew(kbentry + 2, index);
                    break;
                case 3: // K_ALTSHIFTTAB
                    memory->writew(kbentry + 2, index);
                    break;
                default:
                    memory->writew(kbentry + 2, index);
                    break;
            }
            break;
        }
        case 0x4B51: // KDSKBMUTE
            return -1;
        case 0x5401: // TCGETS
            this->writeTermios(memory, IOCTL_ARG1);
            break;
        case 0x5402: // TCSETS
            this->readTermios(memory, IOCTL_ARG1);
            break;
        case 0x5403: // TCSETSW
            this->readTermios(memory, IOCTL_ARG1);
            break;
        case 0x5404: // TCSETSF
            this->readTermios(memory, IOCTL_ARG1);
            break;
        case 0x5600: // VT_OPENQRY
            memory->writed(IOCTL_ARG1, 2);
            break;
        case 0x5601: { // VT_GETMODE
            U32 address = IOCTL_ARG1;
            memory->writeb(address, this->mode);
            memory->writeb(address + 1, this->waitv); // waitv
            memory->writew(address + 2, this->relsig); // relsig
            memory->writew(address + 4, this->acqsig); // acqsig
            memory->writew(address + 6, 0); // frsig
            break;
        }
        case 0x5602: { // VT_SETMODE
            U32 address = IOCTL_ARG1;
            this->mode = memory->readb(address); // VT_AUTO
            this->waitv = memory->readb(address + 1); // waitv
            this->relsig = memory->readw(address + 2); // relsig
            this->acqsig = memory->readw(address + 4); // acqsig
            break;
        }
        case 0x5603: { // VT_GETSTATE
            U32 address = IOCTL_ARG1;
            memory->writew(address, 0); // v_active
            memory->writew(address, 0); // v_signal
            memory->writew(address, 1); // v_state
            break;
        }
        case 0x5605: { // VT_RELDISP
            break;
        }
        case 0x5606: // VT_ACTIVATE
            activeTTY = IOCTL_ARG1;
            break;
        case 0x5607: { // VT_WAITACTIVE
            U32 id = IOCTL_ARG1;
            if (id!=activeTTY)
                kpanic("VT_WAITACTIVE not fully implemented");
            break;
        }
        case 0x5608: { // VT_GETMODE
            break;
        }
        default:
            return -1;
    }
    return 0;
}
