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

#include "kscheduler.h"
#include "../../io/fsvirtualopennode.h"
#include "oss.h"
#ifdef __EMSCRIPTEN__
#include <algorithm>
#endif
#include <math.h>
#include <string.h>
#include "kdspaudio.h"
#include "kdspaudio_math.h"

#ifdef __EMSCRIPTEN__
static U32 dspMaxOutputFreq = 11025;
static const U32 DSP_DEFAULT_FRAGMENT_SIZE = 1024;
static const U32 DSP_DEFAULT_FRAGMENT_COUNT = 8;
#endif

class DevDsp : public FsVirtualOpenNode {
public:
    DevDsp(const std::shared_ptr<FsNode>& node, U32 flags) : FsVirtualOpenNode(node, flags) {                
        this->audio = KDspAudio::createDspAudio();
        this->freq = 11025;
        this->channels = 1;
        this->format = AFMT_U8;
#ifdef __EMSCRIPTEN__
        this->audio->setFragmentSize(DSP_DEFAULT_FRAGMENT_SIZE);
#endif
    } 
    virtual ~DevDsp() {this->audio->closeAudio();}

    // From FsOpenNode
    bool setLength(S64 length) override;
    U32 ioctl(KThread* thread, U32 request) override;
    U32 readNative(U8* buffer, U32 len) override;
    U32 writeNative(U8* buffer, U32 len) override;
    void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) override;
    bool isWriteReady() override;

    std::shared_ptr<KDspAudio> audio;
    U32 freq;
    U32 channels;
    U32 format;
#ifdef __EMSCRIPTEN__
    U32 fragmentCount = DSP_DEFAULT_FRAGMENT_COUNT;
    U32 bytesWritten = 0;
    U32 lastOutputBlocks = 0;

private:
    U32 getEffectiveBufferCapacity();
    U32 getUsedBufferSize();
    U32 getAvailableBufferSize();
#endif
};


void dspShutdown() {
    KDspAudio::shutdown();
}

#ifdef __EMSCRIPTEN__
void dspSetMaxOutputFreq(U32 freq) {
    if (freq == 11025 || freq == 22050) {
        dspMaxOutputFreq = freq;
    } else {
        kwarn_fmt("Unsupported Emscripten audio frequency %d, using %d", freq, dspMaxOutputFreq);
    }
}
#endif

bool DevDsp::setLength(S64 len) {
    return false;
}

U32 DevDsp::readNative(U8* buffer, U32 len){
    return 0;
}

U32 DevDsp::writeNative(U8* buffer, U32 len) {    
    if (!this->audio->isOpen()) {
        this->audio->openAudio(this->format, this->freq, this->channels);
    }
#ifdef __EMSCRIPTEN__
    U32 result = this->audio->writeAudio(buffer, len);
    if ((S32)result > 0) {
        this->bytesWritten += result;
    }
    return result;
#else
    return this->audio->writeAudio(buffer, len);
#endif
}

#ifdef __EMSCRIPTEN__
U32 DevDsp::getEffectiveBufferCapacity() {
    U32 capacity = this->audio->getBufferCapacity();
    U32 fragmentCapacity = this->audio->getFragmentSize() * this->fragmentCount;
    return std::min(capacity, fragmentCapacity ? fragmentCapacity : capacity);
}

U32 DevDsp::getUsedBufferSize() {
    return std::min(this->audio->getBufferSize(), this->getEffectiveBufferCapacity());
}

U32 DevDsp::getAvailableBufferSize() {
    return this->getEffectiveBufferCapacity() - this->getUsedBufferSize();
}
#endif

bool DevDsp::isWriteReady() {
#ifdef __EMSCRIPTEN__
    return this->getAvailableBufferSize() >= this->audio->getFragmentSize();
#else
    return this->audio->isWriteReady();
#endif
}

U32 DevDsp::ioctl(KThread* thread, U32 request) {
    U32 len = (request >> 16) & 0x3FFF;
    CPU* cpu = thread->cpu;
    KMemory* memory = thread->memory;

    //BOOL read = request & 0x40000000;
    bool write = (request & 0x80000000)!=0;

    switch (request & 0xFFFF) {
    case 0x5000: // SNDCTL_DSP_RESET
        this->audio->closeAudio();
        this->freq = 8000;
        this->channels = 1;
        this->format = AFMT_U8;
#ifdef __EMSCRIPTEN__
        this->audio->setFragmentSize(DSP_DEFAULT_FRAGMENT_SIZE);
        this->fragmentCount = DSP_DEFAULT_FRAGMENT_COUNT;
        this->bytesWritten = 0;
        this->lastOutputBlocks = 0;
#endif
        return 0;
    case 0x5002: { // SNDCTL_DSP_SPEED 
        if (len!=4) {
            kpanic("SNDCTL_DSP_SPEED was expecting a len of 4");
        }
#ifdef __EMSCRIPTEN__
        U32 oldFreq = this->freq;
        this->freq = std::min(memory->readd(IOCTL_ARG1), dspMaxOutputFreq);
        if (oldFreq != this->freq) {
#else
		this->freq = memory->readd(IOCTL_ARG1);
        if (freq != this->freq) {
#endif
            this->audio->closeAudio();
        }
		if (write)
            memory->writed(IOCTL_ARG1, this->freq);
        return 0;
    }
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
            kpanic_fmt("SNDCTL_DSP_STEREO wasn't expecting a value of %d", fmt);
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
        case AFMT_FLOAT:
            this->format = AFMT_FLOAT;
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
#ifdef __EMSCRIPTEN__
    {
        U32 value = memory->readd(IOCTL_ARG1);
        U32 shift = value & 0xFFFF;
        U32 count = value >> 16;
        if (shift > 15) {
            shift = 15;
        }
        this->audio->setFragmentSize(1 << shift);
        this->fragmentCount = std::clamp(count, (U32)2, (U32)64);
        return 0;
    }
#else
		// this->data->dspFragSize = 1 << (readd(IOCTL_ARG1) & 0xFFFF);
        klog("DevDsp::ioctl was not expecting SNDCTL_DSP_SETFRAGMENT");
        return 0;
#endif
    case 0x500B: // SNDCTL_DSP_GETFMTS
#ifdef __EMSCRIPTEN__
        memory->writed(IOCTL_ARG1, AFMT_U8 | AFMT_S16_LE | AFMT_S16_BE | AFMT_S8 | AFMT_U16_LE | AFMT_U16_BE);
#else
        memory->writed(IOCTL_ARG1, AFMT_U8 | AFMT_S16_LE | AFMT_S16_BE | AFMT_S8 | AFMT_U16_LE | AFMT_U16_BE | AFMT_FLOAT);
#endif
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
#ifdef __EMSCRIPTEN__
        U32 capacity = this->getEffectiveBufferCapacity();
        U32 used = this->getUsedBufferSize();
        U32 available = KDspAudioMath::getOutputSpaceAvailable(capacity, used, true);
        memory->writed(IOCTL_ARG1, available / this->audio->getFragmentSize()); // fragments
        memory->writed(IOCTL_ARG1 + 4, capacity / this->audio->getFragmentSize());
        memory->writed(IOCTL_ARG1 + 8, this->audio->getFragmentSize());
        memory->writed(IOCTL_ARG1 + 12, available);
#else
        U32 capacity = this->audio->getBufferCapacity();
        U32 available = KDspAudioMath::getOutputSpaceAvailable(capacity, 0, false);
        memory->writed(IOCTL_ARG1, available / this->audio->getFragmentSize()); // fragments
        memory->writed(IOCTL_ARG1 + 4, capacity / this->audio->getFragmentSize());
        memory->writed(IOCTL_ARG1 + 8, this->audio->getFragmentSize());
        memory->writed(IOCTL_ARG1 + 12, available);
#endif
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
#ifdef __EMSCRIPTEN__
    {
        U32 fragmentSize = this->audio->getFragmentSize();
        U32 currentBlocks = fragmentSize ? this->bytesWritten / fragmentSize : 0;
        U32 blocks = currentBlocks - this->lastOutputBlocks;
        this->lastOutputBlocks = currentBlocks;
        U32 capacity = this->getEffectiveBufferCapacity();
        memory->writed(IOCTL_ARG1, this->bytesWritten); // Total # of bytes written
        memory->writed(IOCTL_ARG1 + 4, blocks); // # of fragment transitions since last time
        memory->writed(IOCTL_ARG1 + 8, capacity ? this->bytesWritten % capacity : 0); // Current DMA pointer value
        return 0;
    }
#else
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
#endif
    case 0x5016: // SNDCTL_DSP_SETDUPLEX
        return -K_EINVAL;
    case 0x5017: // SNDCTL_DSP_GETODELAY
#ifdef __EMSCRIPTEN__
        memory->writed(IOCTL_ARG1, this->getUsedBufferSize());
        return 0;
#else
        /*
        if (write) {
			writed(IOCTL_ARG1, (U32)audioBuffer.size());
            return 0;
        }
        */
        klog("DevDsp::ioctl was not expecting SNDCTL_DSP_GETODELAY");
        return 0;
#endif
    case 0x580C: // SNDCTL_ENGINEINFO
        if (write) {
            U32 p = IOCTL_ARG1;
            p+=4; // int dev; /* Audio device number */
            memory->strcpy(p, "BoxedWine audio"); p+=64; // oss_devname_t name;
            memory->writed(p, 0); p+=4; // int busy; /* 0, OPEN_READ, OPEN_WRITE or OPEN_READWRITE */
            memory->writed(p, -1); p+=4; // int pid;
            memory->writed(p, PCM_CAP_OUTPUT); p+=4; // int caps;			/* PCM_CAP_INPUT, PCM_CAP_OUTPUT */
            memory->writed(p, 0); p+=4; // int iformats
#ifdef __EMSCRIPTEN__
            memory->writed(p, AFMT_U8 | AFMT_S16_LE | AFMT_S16_BE | AFMT_S8 | AFMT_U16_BE); p+=4; // int oformats;
#else
            memory->writed(p, AFMT_U8 | AFMT_S16_LE | AFMT_S16_BE | AFMT_S8 | AFMT_U16_BE | AFMT_FLOAT); p+=4; // int oformats;
#endif
            memory->writed(p, 0); p+=4; // int magic;			/* Reserved for internal use */
            memory->strcpy(p, ""); p+=64; // oss_cmd_t cmd;		/* Command using the device (if known) */
            memory->writed(p, 0); p+=4; // int card_number;
            memory->writed(p, 0); p+=4; // int port_number;
            memory->writed(p, 0); p+=4; // int mixer_dev;
            memory->writed(p, 0); p+=4; // int legacy_device;		/* Obsolete field. Replaced by devnode */
            memory->writed(p, 1); p+=4; // int enabled;			/* 1=enabled, 0=device not ready at this moment */
            memory->writed(p, 0); p+=4; // int flags;			/* For internal use only - no practical meaning */
            memory->writed(p, 11025); p += 4; // int min_rate
#ifdef __EMSCRIPTEN__
            memory->writed(p, dspMaxOutputFreq); p+=4; // max_rate;	/* Sample rate limits */
#else
            memory->writed(p, 48000); p+=4; // max_rate;	/* Sample rate limits */
#endif
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
            memory->strcpy(p, "/dev/dsp"); p+=16; // oss_devnode_t devnode;	/* Device special file name (absolute path) */
            memory->writed(p, 0); p+=4; // int next_play_engine;		/* Read the documentation for more info */
            memory->writed(p, 0); // int next_rec_engine;		/* Read the documentation for more info */
            return 0;
        }        
    }
    return -K_ENODEV;
}

void DevDsp::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    this->audio->waitForEvents(parentCondition, events);
}

FsOpenNode* openDevDsp(const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
    return new DevDsp(node, flags);
}
