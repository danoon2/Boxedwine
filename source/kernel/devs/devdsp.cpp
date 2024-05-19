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

#include "kscheduler.h"
#include "../../io/fsvirtualopennode.h"
#include "oss.h"
#include <math.h>
#include <string.h>
#include "kdspaudio.h"

class DevDsp : public FsVirtualOpenNode {
public:
    DevDsp(const std::shared_ptr<FsNode>& node, U32 flags) : FsVirtualOpenNode(node, flags) {                
        this->audio = KDspAudio::createDspAudio();
        this->freq = 11025;
        this->channels = 1;
        this->format = AFMT_U8;
    } 
    virtual ~DevDsp() {this->audio->closeAudio();}

    // From FsOpenNode
    bool setLength(S64 length) override;
    U32 ioctl(KThread* thread, U32 request) override;
    U32 readNative(U8* buffer, U32 len) override;
    U32 writeNative(U8* buffer, U32 len) override;
    void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) override;

    std::shared_ptr<KDspAudio> audio;
    U32 freq;
    U32 channels;
    U32 format;
};


void dspShutdown() {
    KDspAudio::shutdown();
}

bool DevDsp::setLength(S64 len) {
    return false;
}

U32 DevDsp::readNative(U8* buffer, U32 len){
    return 0;
}

U32 DevDsp::writeNative(U8* buffer, U32 len) {    
    if (KSystem::soundEnabled) {
        if (!this->audio->isOpen()) {
            this->audio->openAudio(this->format, this->freq, this->channels);
        }
        this->audio->writeAudio(buffer, len);
    }
    return len;
}

U32 DevDsp::ioctl(KThread* thread, U32 request) {
    U32 len = (request >> 16) & 0x3FFF;
    CPU* cpu = thread->cpu;
    KMemory* memory = thread->memory;

    //BOOL read = request & 0x40000000;
    bool write = (request & 0x80000000)!=0;

    switch (request & 0xFFFF) {
    case 0x5000: // SNDCTL_DSP_RESET
        return 0;
    case 0x5002:  // SNDCTL_DSP_SPEED 
        if (len!=4) {
            kpanic("SNDCTL_DSP_SPEED was expecting a len of 4");
        }
		this->freq = memory->readd(IOCTL_ARG1);
		if (write)
            memory->writed(IOCTL_ARG1, this->freq);
        return 0;
    case 0x5003: { // SNDCTL_DSP_STEREO
        if (len!=4) {
            kpanic("SNDCTL_DSP_STEREO was expecting a len of 4");
        }
        U32 fmt = memory->readd(IOCTL_ARG1);
        if (fmt != (U32)(this->channels - 1)) {
            this->audio->closeAudio();
        }
        if (fmt == 0) {
            this->channels = 1;
        } else if (fmt == 1) {
            this->channels = 2;
        } else {
            kpanic("SNDCTL_DSP_STEREO wasn't expecting a value of %d", fmt);
        }
        if (write)
            memory->writed(IOCTL_ARG1, this->channels - 1);
        return 0;
    }
    case 0x5005: { // SNDCTL_DSP_SETFMT 
        if (len!=4) {
            kpanic("SNDCTL_DSP_SETFMT was expecting a len of 4");
        }
        U32 fmt = memory->readd(IOCTL_ARG1);
		if (fmt != AFMT_QUERY && fmt != this->format) {
            this->audio->closeAudio();
        }
        switch (fmt) {
        case AFMT_QUERY:
            break;
        case AFMT_MU_LAW:
        case AFMT_A_LAW:
        case AFMT_IMA_ADPCM:
        case AFMT_U8:
			this->format = AFMT_U8;
            break;
        case AFMT_S16_LE:
			this->format = AFMT_S16_LE;
            break;
        case AFMT_S16_BE:
			this->format = AFMT_S16_BE;
            break;
        case AFMT_S8:
			this->format = AFMT_S8;
            break;
        case AFMT_U16_LE:
			this->format = AFMT_U16_LE;
            break;
        case AFMT_U16_BE:
			this->format = AFMT_U16_BE;
            break;
        case AFMT_MPEG:
			this->format = AFMT_U8;
            break;
        }
        if (write)
            memory->writed(IOCTL_ARG1, this->format);
		else if (this->format != fmt) {
            kpanic("SNDCTL_DSP_SETFMT dspFmt!=fmt and can't write result");
        }
        return 0;
        }
    case 0x5006: {// SOUND_PCM_WRITE_CHANNELS
        U32 channels = memory->readd(IOCTL_ARG1);
		if (channels != this->channels) {
            this->audio->closeAudio();
        }
        if (channels==1) {
            this->channels = 1;
        } else if (channels == 2) {
            this->channels = 2;
        } else {
            this->channels = 2;
        }
        if (write)
            memory->writed(IOCTL_ARG1, this->channels);
        return 0;
        }
    case 0x500A: // SNDCTL_DSP_SETFRAGMENT
		// this->data->dspFragSize = 1 << (readd(IOCTL_ARG1) & 0xFFFF);
        klog("DevDsp::ioctl was not expecting SNDCTL_DSP_SETFRAGMENT");
        return 0;
    case 0x500B: // SNDCTL_DSP_GETFMTS
        memory->writed(IOCTL_ARG1, AFMT_U8 | AFMT_S16_LE | AFMT_S16_BE | AFMT_S8 | AFMT_U16_BE);
        return 0;

		//typedef struct audio_buf_info {
		//	int fragments;     /* # of available fragments (partially usend ones not counted) */
		//	int fragstotal;    /* Total # of fragments allocated */
		//	int fragsize;      /* Size of a fragment in bytes */
		//
		//	int bytes;         /* Available space in bytes (includes partially used fragments) */
		//	/* Note! 'bytes' could be more than fragments*fragsize */
		//} audio_buf_info;

    case 0x500C: // SNDCTL_DSP_GETOSPACE
    {
        S32 osLen = ((S32)this->audio->getBufferSize()-(S32)this->audio->getFragmentSize());
        if (osLen<0)
            osLen = 0;
        memory->writed(IOCTL_ARG1, ((this->audio->getBufferCapacity() - this->audio->getBufferSize()) / this->audio->getFragmentSize())); // fragments
        memory->writed(IOCTL_ARG1 + 4, this->audio->getBufferCapacity() / this->audio->getFragmentSize());
        memory->writed(IOCTL_ARG1 + 8, this->audio->getFragmentSize());
        memory->writed(IOCTL_ARG1 + 12, this->audio->getBufferCapacity() - osLen);
        return 0;
    }
    case 0x500F: // SNDCTL_DSP_GETCAPS
        memory->writed(IOCTL_ARG1, DSP_CAP_TRIGGER);
        return 0;
    case 0x5010: // SNDCTL_DSP_SETTRIGGER
        /*
        if (readd(IOCTL_ARG1) & PCM_ENABLE_OUTPUT) {
            if (sdlSoundEnabled) {
                SDL_PauseAudio(0);
            }
			this->data->pauseAtLen = 0xFFFFFFFF;
        } else {            
			this->data->pauseAtLen = (U32)audioBuffer.size();
			if (this->data->pauseAtLen == 0) {
                if (sdlSoundEnabled) {
                    SDL_PauseAudio(0);
                }
            }
        }
        */
        klog("DevDsp::ioctl was not expecting SNDCTL_DSP_SETTRIGGER");
        return 0;
    case 0x5012: // SNDCTL_DSP_GETOPTR
        /*
        writed(IOCTL_ARG1, 0); // Total # of bytes processed
        writed(IOCTL_ARG1 + 4, 0); // # of fragment transitions since last time
        if (data->pauseEnabled()) {
			writed(IOCTL_ARG1 + 8, this->data->pauseAtLen); // Current DMA pointer value
			if (this->data->pauseAtLen == 0) {
                if (sdlSoundEnabled) {
                    SDL_PauseAudio(0);
                }
            }
        } else {
			writed(IOCTL_ARG1 + 8, (U32)audioBuffer.size()); // Current DMA pointer value
        }
        */
        klog("DevDsp::ioctl was not expecting SNDCTL_DSP_GETOPTR");
        return 0;
    case 0x5016: // SNDCTL_DSP_SETDUPLEX
        return -K_EINVAL;
    case 0x5017: // SNDCTL_DSP_GETODELAY 
        /*
        if (write) {
			writed(IOCTL_ARG1, (U32)audioBuffer.size());
            return 0;
        }
        */
        klog("DevDsp::ioctl was not expecting SNDCTL_DSP_GETODELAY");
        return 0;
    case 0x580C: // SNDCTL_ENGINEINFO
        if (write) {
            U32 p = IOCTL_ARG1;
            p+=4; // int dev; /* Audio device number */
            memory->strcpy(p, "BoxedWine audio"); p+=64; // oss_devname_t name;
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
            memory->writed(p, 11025); p += 4; // int min_rate
            memory->writed(p, 44100); p+=4; // max_rate;	/* Sample rate limits */
            memory->writed(p, 1); p+=4; // int min_channels
            memory->writed(p, 2); p+=4; // max_channels;	/* Number of channels supported */
            memory->writed(p, 0); p+=4; // int binding;			/* DSP_BIND_FRONT, etc. 0 means undefined */
            memory->writed(p, 0); p+=4; // int rate_source;
            memory->strcpy(p, ""); p+=32; // oss_handle_t handle;
            memory->writed(p, 0); p+=4; // unsigned int nrates
            for (int i=0;i<20;i++) {
                memory->writed(p, 0); p+=4; // rates[20];	/* Please read the manual before using these */
            }
            memory->strcpy(p, ""); p+=64; // oss_longname_t song_name;	/* Song name (if given) */
            memory->strcpy(p, ""); p+=16; // oss_label_t label;		/* Device label (if given) */
            memory->writed(p, -1); p+=4; // int latency;			/* In usecs, -1=unknown */
            memory->strcpy(p, "dsp"); p+=16; // oss_devnode_t devnode;	/* Device special file name (absolute path) */
            memory->writed(p, 0); p+=4; // int next_play_engine;		/* Read the documentation for more info */
            memory->writed(p, 0); // int next_rec_engine;		/* Read the documentation for more info */
            return 0;
        }        
    }
    return -K_ENODEV;
}

void DevDsp::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    if (events & K_POLLOUT) {
        // BOXEDWINE_CONDITION_ADD_CHILD_CONDITION(parentCondition, this->data->bufferCond, nullptr);
        klog("DevDsp::waitForEvents POLLOUT not implemented");
    }
}

FsOpenNode* openDevDsp(const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
    return new DevDsp(node, flags);
}
