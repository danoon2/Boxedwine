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
    DevMixer(const std::shared_ptr<FsNode>& node, U32 flags) : FsVirtualOpenNode(node, flags) {}

    // From FsOpenNode
    U32 ioctl(KThread* thread, U32 request) override;
    U32 readNative(U8* buffer, U32 len) override {return 0;}
    U32 writeNative(U8* buffer, U32 len) override {return 0;}
};

FsOpenNode* openDevMixer(const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
    return new DevMixer(node, flags);
}

U32 DevMixer::ioctl(KThread* thread, U32 request) {
    U32 len = (request >> 16) & 0x3FFF;
    CPU* cpu = thread->cpu;
    KMemory* memory = thread->memory;
    //bool read = (request & 0x40000000) != 0;
    bool write = (request & 0x80000000) != 0;

    switch (request & 0xFFFF) {
    case 0x5801: // SNDCTL_SYSINFO
        if (write) {
            U32 p = IOCTL_ARG1;
            for (U32 i=0;i<len/4;i++)
                memory->writed(p+i*4, 2000+i);
            memory->strcpy(p, "OSS/Linux"); p+=32; // char product[32];		/* For example OSS/Free, OSS/Linux or OSS/Solaris */
            memory->strcpy(p, "4.0.0a"); p+=32; // char version[32];		/* For example 4.0a */
            memory->writed(p, 0x040000); p+=4; // int versionnum;		/* See OSS_GETVERSION */

            for (U32 i=0;i<128;i++) {
                memory->writeb(p, i+100); p+=1; // char options[128];		/* Reserved */
            }

            memory->writed(p, 1); p+=4; // offset 196 int numaudios;		/* # of audio/dsp devices */
            for (U32 i=0;i<8;i++) {
                memory->writed(p, 200+i); p+=4; // int openedaudio[8];		/* Bit mask telling which audio devices are busy */
            }

            memory->writed(p, 1); p+=4; // int numsynths;		/* # of availavle synth devices */
            memory->writed(p, 1); p+=4; // int nummidis;			/* # of available MIDI ports */
            memory->writed(p, 1); p+=4; // int numtimers;		/* # of available timer devices */
            memory->writed(p, 1); p+=4; // offset 244 int nummixers;		/* # of mixer devices */

            for (U32 i=0;i<8;i++) {
                memory->writed(p, 0); p+=4; // int openedmidi[8];		/* Bit mask telling which midi devices are busy */
            }
            memory->writed(p, 1); p+=4; // offset 280 int numcards;			/* Number of sound cards in the system */
            memory->writed(p, 1); p+=4; // offset 284 int numaudioengines;		/* Number of audio engines in the system */
            memory->strcpy(p, "GPL"); // char license[16];		/* For example "GPL" or "CDDL" */
            return 0;
        }
        break;
    case 0x5807: // SNDCTL_AUDIOINFO	
        if (write) {
            U32 p = IOCTL_ARG1;
            p+=4; // int dev; /* Audio device number */
            memory->strcpy(p, "BoxedWine mixer"); p+=64; // oss_devname_t name;
            memory->writed(p, 0); p+=4; // int busy; /* 0, OPEN_READ, OPEN_WRITE or OPEN_READWRITE */
            memory->writed(p, -1); p+=4; // int pid;
            memory->writed(p, PCM_CAP_OUTPUT); p+=4; // int caps;			/* PCM_CAP_INPUT, PCM_CAP_OUTPUT */
            memory->writed(p, 0); p+=4; // int iformats
            memory->writed(p, AFMT_U8 | AFMT_S16_LE | AFMT_S16_BE | AFMT_S8 | AFMT_U16_BE); p+=4; // int oformats;
            memory->writed(p, 0); p+=4; // int magic;			/* Reserved for internal use */
            memory->strcpy(p, ""); p+=64; // oss_cmd_t cmd;		/* Command using the device (if known) */
            memory->writed(p, 0); p+=4; // int card_number;
            memory->writed(p, 0); p+=4; // int port_number;
            memory->writed(p, 0); p+=4; // int mixer_dev;
            memory->writed(p, 0); p+=4; // int legacy_device;		/* Obsolete field. Replaced by devnode */
            memory->writed(p, 1); p+=4; // int enabled;			/* 1=enabled, 0=device not ready at this moment */
            memory->writed(p, 0); p+=4; // int flags;			/* For internal use only - no practical meaning */
            memory->writed(p, 0); p+=4; // int min_rate
            memory->writed(p, 0); p+=4; // max_rate;	/* Sample rate limits */
            memory->writed(p, 0); p+=4; // int min_channels
            memory->writed(p, 0); p+=4; // max_channels;	/* Number of channels supported */
            memory->writed(p, 0); p+=4; // int binding;			/* DSP_BIND_FRONT, etc. 0 means undefined */
            memory->writed(p, 0); p+=4; // int rate_source;
            memory->strcpy(p, ""); p+=64; // oss_handle_t handle;
            memory->writed(p, 0); p+=4; // unsigned int nrates
            for (U32 i=0;i<20;i++) {
                memory->writed(p, 0); p+=4; // rates[20];	/* Please read the manual before using these */
            }
            memory->strcpy(p, ""); p+=32; // oss_longname_t song_name;	/* Song name (if given) */
            memory->strcpy(p, ""); p+=16; // oss_label_t label;		/* Device label (if given) */
            memory->writed(p, -1); p+=4; // int latency;			/* In usecs, -1=unknown */
            memory->strcpy(p, "/dev/dsp"); p+=16; // oss_devnode_t devnode;	/* Device special file name (absolute path) */
            memory->writed(p, 0); p+=4; // int next_play_engine;		/* Read the documentation for more info */
            memory->writed(p, 0); // int next_rec_engine;		/* Read the documentation for more info */
            return 0;
        }        
    }
    return -K_ENODEV;
}
