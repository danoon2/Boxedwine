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

#include "oss.h"
#include "../../io/fsvirtualopennode.h"

class DevMixer : public FsVirtualOpenNode {
public:
    DevMixer(const BoxedPtr<FsNode>& node, U32 flags) : FsVirtualOpenNode(node, flags) {}
    virtual U32 ioctl(U32 request);
    virtual U32 readNative(U8* buffer, U32 len) {return 0;}
    virtual U32 writeNative(U8* buffer, U32 len) {return 0;}
};

FsOpenNode* openDevMixer(const BoxedPtr<FsNode>& node, U32 flags, U32 data) {
    return new DevMixer(node, flags);
}

U32 DevMixer::ioctl(U32 request) {
    U32 len = (request >> 16) & 0x3FFF;
    KThread* thread = KThread::currentThread();
    CPU* cpu = thread->cpu;
    //bool read = (request & 0x40000000) != 0;
    bool write = (request & 0x80000000) != 0;
    U32 i;

    switch (request & 0xFFFF) {
    case 0x5801: // SNDCTL_SYSINFO
        if (write) {
            U32 p = IOCTL_ARG1;
            for (i=0;i<len/4;i++)
                writed(p+i*4, 2000+i);
            writeNativeString(p, "OSS/Linux"); p+=32; // char product[32];		/* For example OSS/Free, OSS/Linux or OSS/Solaris */
            writeNativeString(p, "4.0.0a"); p+=32; // char version[32];		/* For example 4.0a */
            writed(p, 0x040000); p+=4; // int versionnum;		/* See OSS_GETVERSION */

            for (i=0;i<128;i++) {
                writeb(p, i+100); p+=1; // char options[128];		/* Reserved */
            }

            writed(p, 1); p+=4; // offset 196 int numaudios;		/* # of audio/dsp devices */
            for (i=0;i<8;i++) {
                writed(p, 200+i); p+=4; // int openedaudio[8];		/* Bit mask telling which audio devices are busy */
            }

            writed(p, 1); p+=4; // int numsynths;		/* # of availavle synth devices */
            writed(p, 1); p+=4; // int nummidis;			/* # of available MIDI ports */
            writed(p, 1); p+=4; // int numtimers;		/* # of available timer devices */
            writed(p, 1); p+=4; // offset 244 int nummixers;		/* # of mixer devices */

            for (i=0;i<8;i++) {
                writed(p, 0); p+=4; // int openedmidi[8];		/* Bit mask telling which midi devices are busy */
            }
            writed(p, 1); p+=4; // offset 280 int numcards;			/* Number of sound cards in the system */
            writed(p, 1); p+=4; // offset 284 int numaudioengines;		/* Number of audio engines in the system */
            writeNativeString(p, "GPL"); p+=16; // char license[16];		/* For example "GPL" or "CDDL" */
            return 0;
        }
        break;
    case 0x5807: // SNDCTL_AUDIOINFO	
        if (write) {
            U32 p = IOCTL_ARG1;
            p+=4; // int dev; /* Audio device number */
            writeNativeString(p, "BoxedWine mixer"); p+=64; // oss_devname_t name;
            writed(p, 0); p+=4; // int busy; /* 0, OPEN_READ, OPEN_WRITE or OPEN_READWRITE */
            writed(p, -1); p+=4; // int pid;
            writed(p, PCM_CAP_OUTPUT); p+=4; // int caps;			/* PCM_CAP_INPUT, PCM_CAP_OUTPUT */
            writed(p, 0); p+=4; // int iformats
            writed(p, AFMT_U8 | AFMT_S16_LE | AFMT_S16_BE | AFMT_S8 | AFMT_U16_BE); p+=4; // int oformats;
            writed(p, 0); p+=4; // int magic;			/* Reserved for internal use */
            writeNativeString(p, ""); p+=64; // oss_cmd_t cmd;		/* Command using the device (if known) */
            writed(p, 0); p+=4; // int card_number;
            writed(p, 0); p+=4; // int port_number;
            writed(p, 0); p+=4; // int mixer_dev;
            writed(p, 0); p+=4; // int legacy_device;		/* Obsolete field. Replaced by devnode */
            writed(p, 1); p+=4; // int enabled;			/* 1=enabled, 0=device not ready at this moment */
            writed(p, 0); p+=4; // int flags;			/* For internal use only - no practical meaning */
            writed(p, 0); p+=4; // int min_rate
            writed(p, 0); p+=4; // max_rate;	/* Sample rate limits */
            writed(p, 0); p+=4; // int min_channels
            writed(p, 0); p+=4; // max_channels;	/* Number of channels supported */
            writed(p, 0); p+=4; // int binding;			/* DSP_BIND_FRONT, etc. 0 means undefined */
            writed(p, 0); p+=4; // int rate_source;
            writeNativeString(p, ""); p+=64; // oss_handle_t handle;
            writed(p, 0); p+=4; // unsigned int nrates
            for (i=0;i<20;i++) {
                writed(p, 0); p+=4; // rates[20];	/* Please read the manual before using these */
            }
            writeNativeString(p, ""); p+=32; // oss_longname_t song_name;	/* Song name (if given) */
            writeNativeString(p, ""); p+=16; // oss_label_t label;		/* Device label (if given) */
            writed(p, -1); p+=4; // int latency;			/* In usecs, -1=unknown */
            writeNativeString(p, "/dev/dsp"); p+=16; // oss_devnode_t devnode;	/* Device special file name (absolute path) */
            writed(p, 0); p+=4; // int next_play_engine;		/* Read the documentation for more info */
            writed(p, 0); p+=4; // int next_rec_engine;		/* Read the documentation for more info */
            return 0;
        }        
    }
    return -K_ENODEV;
}
