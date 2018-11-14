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
#include <SDL.h>
#include <string.h>

#define DSP_BUFFER_SIZE 1024*256
ringbuffer<U8> audioBuffer(DSP_BUFFER_SIZE);
U8 audioSilence;
static bool closeWhenDone = false;
static bool isAudioOpen = false;

class DevDspData {
public:
    DevDspData() {
        memset(&this->want, 0, sizeof(this->want));
        memset(&this->got, 0, sizeof(this->got));

        this->pos = 0;
        this->isDspOpen = false;
        this->sameFormat = false;
        this->dspFmt = AFMT_U8;
	    this->want.format = AUDIO_U8;
	    this->want.channels = 1;
	    this->dspFragSize = 5512;
	    this->want.freq = 11025;
        this->pauseAtLen = 0xFFFFFFFF;
        this->cvtBufLen = 0;
        this->cvtBuf = NULL;
        this->cvtBufPos = 0;
    }

    ~DevDspData() {
        if (this->cvtBuf) {
            SDL_free(this->cvtBuf);
        }
    }

    bool pauseEnabled() {
        return this->pauseAtLen!=0xFFFFFFFF;
    }

    S64 pos;

    bool isDspOpen;
	U32 dspFmt;
    SDL_AudioSpec want;
    SDL_AudioSpec got;
    bool sameFormat;
	U32 dspFragSize;
	KList<KThread*> dspWaitingToWriteThread;
	S32 pauseAtLen;
    SDL_AudioCVT cvt;
    int cvtBufLen;
    int cvtBufPos;
    unsigned char* cvtBuf;
};

class DevDsp : public FsVirtualOpenNode {
public:
    DevDsp(const BoxedPtr<FsNode>& node, U32 flags) : FsVirtualOpenNode(node, flags) {                
        this->data = new DevDspData();
    }

    ~DevDsp() {
        this->closeAudio();
        delete this->data;
    }

    void closeAudio() {
        if (this->data->isDspOpen) {
            bool needClose = true;
            SDL_LockAudio();
            if (audioBuffer.getOccupied() || (this->data->cvtBufPos!=0 && this->data->cvtBufPos<this->data->cvt.len_cvt)) {                
                closeWhenDone = true;
                needClose = false;
            }
            SDL_UnlockAudio();
            if (needClose) {
                SDL_CloseAudio();
                delete this->data;                
            }
            this->data = new DevDspData();
        }
    }

    U32 bytesPerSample() {
        if (this->data->want.format == AUDIO_S16LSB || this->data->want.format == AUDIO_S16MSB || this->data->want.format == AUDIO_U16LSB || this->data->want.format == AUDIO_U16MSB)
            return 2;
#ifdef SDL2
        else if (this->data->want.format == AUDIO_F32LSB)
             return 4;
#endif
         else
            return 1;
    }

    void openAudio();    

    virtual bool setLength(S64 length);
    virtual U32 ioctl(U32 request);
    virtual U32 readNative(U8* buffer, U32 len);
    virtual U32 writeNative(U8* buffer, U32 len);
    virtual void waitForEvents(U32 events);    

    DevDspData* data;
};


void audioCallback(void *userdata, U8* stream, S32 len) {
    S32 available = (S32)audioBuffer.getOccupied();
    DevDspData* data = (DevDspData*)userdata;
    S32 originalAvailable = available;
    S32 originalLen = len;

    if (available==0 && closeWhenDone && (data->cvtBufPos==0 || data->cvtBufPos>=data->cvt.len_cvt)) {
        closeWhenDone = false;
        delete data;
        SDL_CloseAudio(); // might not return since it can kill audio thread   
        return;
    }
    if (data->pauseEnabled() && available > data->pauseAtLen) {
		available = data->pauseAtLen;
    }
    if (!data->sameFormat) {
        if (data->cvtBufPos<data->cvt.len_cvt) {
            S32 todo = data->cvt.len_cvt - data->cvtBufPos;
            if (todo>len)
                todo = len;
            memcpy(stream, data->cvt.buf+data->cvtBufPos, todo);
            data->cvtBufPos+=todo;
            stream+=todo;
            len-=todo;
        }
        if (len) {   
            data->cvt.len = available;
            if (data->cvtBufLen && data->cvtBufLen < data->cvt.len * data->cvt.len_mult) {
                data->cvtBufLen = 0;
                SDL_free(data->cvtBuf);
                data->cvtBuf = NULL;
            }
            if (!data->cvtBufLen) {
                data->cvtBufLen = data->cvt.len * data->cvt.len_mult;
                data->cvtBuf = (Uint8 *)SDL_malloc(data->cvt.len * data->cvt.len_mult);
            }
            data->cvt.buf = data->cvtBuf;    
            audioBuffer.read(data->cvt.buf, available);
            SDL_ConvertAudio(&data->cvt);
            S32 todo = data->cvt.len_cvt;
            if (todo>len)
                todo = len;
            memcpy(stream, data->cvt.buf, todo);
            stream+=todo;
            len-=todo;
            data->cvtBufPos=todo;
        }
    } else {
        if (available>len)
            available = len;
        if (available) {
            audioBuffer.read(stream, available);
            len-=available;
            stream+=available;
        }
    }
    if (len) {
        memset(stream, audioSilence, len);
    }
   	
    if (data->pauseEnabled())
		data->pauseAtLen -= available;

    if (data->dspWaitingToWriteThread.size()) {
        data->dspWaitingToWriteThread.for_each([] (KListNode<KThread*>* node) {
		    wakeThread(node->data);
        });
    }
}

void DevDsp::openAudio() {
    //want.samples = 4096;    
    this->data->want.callback = audioCallback;
	this->data->want.userdata = this->data;

    if (closeWhenDone) {
        SDL_CloseAudio();
        closeWhenDone = false;
    }
    if (SDL_OpenAudio(&this->data->want, &this->data->got) < 0) {
        printf("Failed to open audio: %s\n", SDL_GetError());
    }
    if (this->data->want.freq != this->data->got.freq || this->data->want.channels != this->data->got.channels || this->data->want.format != this->data->got.format) {
        this->data->sameFormat = false;
        SDL_BuildAudioCVT(&this->data->cvt, this->data->want.format, this->data->want.channels, this->data->want.freq, this->data->got.format, this->data->got.channels, this->data->got.freq);
    } else {
        this->data->sameFormat = true;
    }
	this->data->isDspOpen = true;
    
    SDL_PauseAudio(0);
	this->data->pauseAtLen = 0xFFFFFFFF;
	this->data->dspFragSize = this->data->got.size;

	printf("openAudio: freq=%d(got %d) format=%d(%x/got %x) channels=%d(got %d)\n", this->data->want.freq, this->data->got.freq, this->data->dspFmt, this->data->want.format, this->data->got.format, this->data->want.channels, this->data->got.channels);
}

bool DevDsp::setLength(S64 len) {
    return false;
}

U32 DevDsp::readNative(U8* buffer, U32 len){
    return 0;
}

U32 DevDsp::writeNative(U8* buffer, U32 len) {    
    U32 result;

    if (!this->data->isDspOpen)
        this->openAudio();
    SDL_LockAudio();
    if (len>audioBuffer.getFree()) {
        KThread* thread = KThread::currentThread();
		this->data->dspWaitingToWriteThread.addToBack(thread->getWaitNofiyNode());
        result = -K_WAIT;
        printf("DevDsp::write wait\n");
    } else {
        audioBuffer.write(buffer, len);
        result = len;
    }
    SDL_UnlockAudio();	
    return result;
}

U32 DevDsp::ioctl(U32 request) {
    U32 len = (request >> 16) & 0x3FFF;
    KThread* thread = KThread::currentThread();
    CPU* cpu = thread->cpu;
    //BOOL read = request & 0x40000000;
    bool write = (request & 0x80000000)!=0;
    int i;

    switch (request & 0xFFFF) {
    case 0x5000: // SNDCTL_DSP_RESET
        return 0;
    case 0x5002:  // SNDCTL_DSP_SPEED 
        if (len!=4) {
            kpanic("SNDCTL_DSP_SPEED was expecting a len of 4");
        }
		this->data->want.freq = readd(IOCTL_ARG1);
		if (write)
            writed(IOCTL_ARG1, this->data->want.freq);
        return 0;
    case 0x5003: { // SNDCTL_DSP_STEREO
        U32 fmt;

        if (len!=4) {
            kpanic("SNDCTL_DSP_STEREO was expecting a len of 4");
        }
        fmt = readd(IOCTL_ARG1);
		if (fmt != this->data->want.channels - 1) {
            this->closeAudio();
        }
        if (fmt == 0) {
            this->data->want.channels = 1;
        } else if (fmt == 1) {
            this->data->want.channels = 2;
        } else {
            kpanic("SNDCTL_DSP_STEREO wasn't expecting a value of %d", fmt);
        }
        if (write)
            writed(IOCTL_ARG1, this->data->want.channels - 1);
        return 0;
    }
    case 0x5005: { // SNDCTL_DSP_SETFMT 
        U32 fmt;

        if (len!=4) {
            kpanic("SNDCTL_DSP_SETFMT was expecting a len of 4");
        }
        fmt = readd(IOCTL_ARG1);
		if (fmt != AFMT_QUERY && fmt != this->data->dspFmt) {
            this->closeAudio();
        }
        switch (fmt) {
        case AFMT_QUERY:
            break;
        case AFMT_MU_LAW:
        case AFMT_A_LAW:
        case AFMT_IMA_ADPCM:
			this->data->dspFmt = AFMT_U8;
			this->data->want.format = AUDIO_U8;
            break;
        case AFMT_U8:
			this->data->dspFmt = AFMT_U8;
			this->data->want.format = AUDIO_U8;
            break;
        case AFMT_S16_LE:
			this->data->dspFmt = AFMT_S16_LE;
			this->data->want.format = AUDIO_S16LSB;
            break;
        case AFMT_S16_BE:
			this->data->dspFmt = AFMT_S16_BE;
            this->data->want.format = AUDIO_S16MSB;
            break;
        case AFMT_S8:
			this->data->dspFmt = AFMT_S8;
            this->data->want.format = AUDIO_S8;
            break;
        case AFMT_U16_LE:
			this->data->dspFmt = AFMT_U16_LE;
            this->data->want.format = AUDIO_U16LSB;
            break;
        case AFMT_U16_BE:
			this->data->dspFmt = AFMT_U16_BE;
            this->data->want.format = AUDIO_U16MSB;
            break;
        case AFMT_MPEG:
			this->data->dspFmt = AFMT_U8;
            this->data->want.format = AUDIO_U8;
            break;
        }
        if (write)
			writed(IOCTL_ARG1, this->data->dspFmt);
		else if (this->data->dspFmt != fmt) {
            kpanic("SNDCTL_DSP_SETFMT dspFmt!=fmt and can't write result");
        }
        return 0;
        }
    case 0x5006: {// SOUND_PCM_WRITE_CHANNELS
        U32 channels = readd(IOCTL_ARG1);
		if (channels != this->data->want.channels) {
            this->closeAudio();
        }
        if (channels==1) {
            this->data->want.channels = 1;
        } else if (channels == 2) {
            this->data->want.channels = 2;
        } else {
            this->data->want.channels = 2;
        }
        if (write)
            writed(IOCTL_ARG1, this->data->want.channels);
        return 0;
        }
    case 0x500A: // SNDCTL_DSP_SETFRAGMENT
		this->data->dspFragSize = 1 << (readd(IOCTL_ARG1) & 0xFFFF);
        return 0;
    case 0x500B: // SNDCTL_DSP_GETFMTS
        writed(IOCTL_ARG1, AFMT_U8 | AFMT_S16_LE | AFMT_S16_BE | AFMT_S8 | AFMT_U16_BE);
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
        S32 osLen = ((S32)audioBuffer.getOccupied()-(S32)this->data->dspFragSize);
        if (osLen<0)
            osLen = 0;
		writed(IOCTL_ARG1, ((DSP_BUFFER_SIZE - (U32)audioBuffer.getOccupied()) / this->data->dspFragSize)); // fragments
		writed(IOCTL_ARG1 + 4, DSP_BUFFER_SIZE / this->data->dspFragSize);
		writed(IOCTL_ARG1 + 8, this->data->dspFragSize);
		writed(IOCTL_ARG1 + 12, DSP_BUFFER_SIZE - osLen);
        return 0;
    }
    case 0x500F: // SNDCTL_DSP_GETCAPS
        writed(IOCTL_ARG1, DSP_CAP_TRIGGER);
        return 0;
    case 0x5010: // SNDCTL_DSP_SETTRIGGER
        if (readd(IOCTL_ARG1) & PCM_ENABLE_OUTPUT) {
            SDL_PauseAudio(0);
			this->data->pauseAtLen = 0xFFFFFFFF;
        } else {            
			this->data->pauseAtLen = (U32)audioBuffer.getOccupied();
			if (this->data->pauseAtLen == 0) {
                SDL_PauseAudio(0);
            }
        }
        return 0;
    case 0x5012: // SNDCTL_DSP_GETOPTR
        writed(IOCTL_ARG1, 0); // Total # of bytes processed
        writed(IOCTL_ARG1 + 4, 0); // # of fragment transitions since last time
        if (data->pauseEnabled()) {
			writed(IOCTL_ARG1 + 8, this->data->pauseAtLen); // Current DMA pointer value
			if (this->data->pauseAtLen == 0) {
                SDL_PauseAudio(0);
            }
        } else {
			writed(IOCTL_ARG1 + 8, (U32)audioBuffer.getOccupied()); // Current DMA pointer value
        }
        return 0;
    case 0x5016: // SNDCTL_DSP_SETDUPLEX
        return -K_EINVAL;
    case 0x5017: // SNDCTL_DSP_GETODELAY 
        if (write) {
			writed(IOCTL_ARG1, (U32)audioBuffer.getOccupied());
            return 0;
        }
    case 0x580C: // SNDCTL_ENGINEINFO
        if (write) {
            U32 p = IOCTL_ARG1;
            p+=4; // int dev; /* Audio device number */
            writeNativeString(p, "BoxedWine audio"); p+=64; // oss_devname_t name;
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
			writed(p, 11025); p += 4; // int min_rate
            writed(p, 44100); p+=4; // max_rate;	/* Sample rate limits */
            writed(p, 1); p+=4; // int min_channels
            writed(p, 2); p+=4; // max_channels;	/* Number of channels supported */
            writed(p, 0); p+=4; // int binding;			/* DSP_BIND_FRONT, etc. 0 means undefined */
            writed(p, 0); p+=4; // int rate_source;
            writeNativeString(p, ""); p+=32; // oss_handle_t handle;
            writed(p, 0); p+=4; // unsigned int nrates
            for (i=0;i<20;i++) {
                writed(p, 0); p+=4; // rates[20];	/* Please read the manual before using these */
            }
            writeNativeString(p, ""); p+=64; // oss_longname_t song_name;	/* Song name (if given) */
            writeNativeString(p, ""); p+=16; // oss_label_t label;		/* Device label (if given) */
            writed(p, -1); p+=4; // int latency;			/* In usecs, -1=unknown */
            writeNativeString(p, "dsp"); p+=16; // oss_devnode_t devnode;	/* Device special file name (absolute path) */
            writed(p, 0); p+=4; // int next_play_engine;		/* Read the documentation for more info */
            writed(p, 0); p+=4; // int next_rec_engine;		/* Read the documentation for more info */
            return 0;
        }        
    }
    return -K_ENODEV;
}

void DevDsp::waitForEvents(U32 events) {
    if (events & K_POLLOUT) {
        KThread* thread = KThread::currentThread();
		this->data->dspWaitingToWriteThread.addToBack(thread->getWaitNofiyNode());
    }
}

FsOpenNode* openDevDsp(const BoxedPtr<FsNode>& node, U32 flags) {
    return new DevDsp(node, flags);
}
