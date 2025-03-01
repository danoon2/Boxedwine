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

// the core audio specific code was taken from
// https://github.com/wine-mirror/wine/tree/master/dlls/winecoreaudio.drv
// License from that file

/*
 * Copyright 2011 Andrew Eikum for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "boxedwine.h"
#include "knativeaudio.h"

#ifdef BOXEDWINE_CORE_AUDIO
#include <os/lock.h>
#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioFormat.h>
#include <AudioToolbox/AudioConverter.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreMIDI/CoreMIDI.h>
#include <AudioToolbox/AudioToolbox.h>

#ifdef S_OK
#undef S_OK
#endif
#define S_OK 0

#ifdef E_FAIL
#undef E_FAIL
#endif
#define E_FAIL 0x80004005

class KNativeAudioCoreAudio : public KNativeAudio {
public:
    virtual ~KNativeAudioCoreAudio() {}
    bool load() override;
    
    U32 midiOutOpen(U32 wDevID) override;
    U32 midiOutClose(U32 wDevID) override;
    U32 midiOutData(U32 wDevID, U8* buffer, U32 len) override;
    U32 midiOutLongData(U32 wDevID, U8* buffer, U32 len) override;
    U32 midiOutGetNumDevs() override;
    U32 midiOutGetVolume(KThread* thread, U32 wDevID, U32 lpdwVolume) override;
    U32 midiOutSetVolume(U32 wDevID, U32 dwVolume) override;
    U32 midiOutReset(U32 wDevID) override;
    BString midiOutGetName(U32 wDevId) override;
    bool midiOutIsOpen(U32 wDevID) override;
};

static bool CoreAudio_MIDIInit();
static bool initialized;
static bool isLoaded;
static U32 MIDIOut_NumDevs = 0;

bool KNativeAudioCoreAudio::load() {
    if (!initialized) {
        isLoaded = CoreAudio_MIDIInit();
        initialized = true;
    }
    return isLoaded;
}

#include "coremidi.h"
static MIDIClientRef wineMIDIClient = NULL;

#define MAXPNAMELEN 32
typedef struct midioutcaps_tag {
  U16    wMid;
  U16    wPid;
  U32    vDriverVersion;
  char   szPname[MAXPNAMELEN];
  U16    wTechnology;
  U16    wVoices;
  U16    wNotes;
  U16    wChannelMask;
  U32    dwSupport;
} MIDIOUTCAPS;

typedef struct tagMIDIDestination {
    /* graph and synth are only used for MIDI Synth */
    AUGraph graph;
    AudioUnit synth;

    MIDIEndpointRef dest;

    MIDIOUTCAPS caps;
    U16 wFlags;
} MIDIDestination;

static MIDIPortRef MIDIOutPort = NULL;

#define CALLBACK_TYPEMASK 0x00070000

#define HIWORD(x) (((x) >> 16) & 0xffff)

#define MAX_MIDI_SYNTHS 1
#define MIDICAPS_VOLUME 1

#define MOD_MIDIPORT 1
#define MOD_SYNTH 2

#define MMSYSERR_NOERROR 0
#define MMSYSERR_ERROR 1
#define MMSYSERR_BADDEVICEID 2
#define MMSYSERR_ALLOCATED 4
#define MMSYSERR_NOTSUPPORTED 8
#define MMSYSERR_INVALFLAG 10
#define MMSYSERR_INVALPARAM 11

MIDIDestination *destinations;

int SynthUnit_CreateDefaultSynthUnit(AUGraph *graph, AudioUnit *synth);
int SynthUnit_Initialize(AudioUnit synth, AUGraph graph);
int SynthUnit_Close(AUGraph graph);
int AudioUnit_GetVolume(AudioUnit au, float *left, float *right);
int AudioUnit_SetVolume(AudioUnit au, float left, float right);

static bool CoreAudio_MIDIInit()
{
    int i;

    ItemCount numDest = MIDIGetNumberOfDestinations();
    CFStringRef name = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("wineMIDIClient.%d"), getpid());

    wineMIDIClient = CoreMIDI_CreateClient( name );
    if (!wineMIDIClient)
    {
        CFRelease(name);
        klog("can't create wineMIDIClient\n");
        return false;
    }
    CFRelease(name);

    MIDIOut_NumDevs = MAX_MIDI_SYNTHS;
    MIDIOut_NumDevs += numDest;

    destinations = (MIDIDestination*)calloc(MIDIOut_NumDevs, sizeof(MIDIDestination));

    if (numDest > 0)
    {
        name = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("WineOutputPort.%u"), getpid());
        MIDIOutputPortCreate(wineMIDIClient, name, &MIDIOutPort);
        CFRelease(name);
    }

    /* initialise MIDI synths */
    for (i = 0; i < MAX_MIDI_SYNTHS; i++)
    {
        snprintf(destinations[i].caps.szPname, sizeof(destinations[i].caps.szPname), "CoreAudio MIDI Synth %d", i);

        destinations[i].caps.wTechnology = MOD_SYNTH;
        destinations[i].caps.wChannelMask = 0xFFFF;

        destinations[i].caps.wMid = 0x00FF;     /* Manufac ID */
        destinations[i].caps.wPid = 0x0001;     /* Product ID */
        destinations[i].caps.vDriverVersion = 0x0001;
        destinations[i].caps.dwSupport = MIDICAPS_VOLUME;
        destinations[i].caps.wVoices = 16;
        destinations[i].caps.wNotes = 16;
    }
    /* initialise available destinations */
    for (i = MAX_MIDI_SYNTHS; i < numDest + MAX_MIDI_SYNTHS; i++)
    {
        destinations[i].dest = MIDIGetDestination(i - MAX_MIDI_SYNTHS);

        CoreMIDI_GetObjectName(destinations[i].dest, destinations[i].caps.szPname, sizeof(destinations[i].caps.szPname));

        destinations[i].caps.wTechnology = MOD_MIDIPORT;
        destinations[i].caps.wChannelMask = 0xFFFF;

        destinations[i].caps.wMid = 0x00FF;     /* Manufac ID */
        destinations[i].caps.wPid = 0x0001;
        destinations[i].caps.vDriverVersion = 0x0001;
        destinations[i].caps.dwSupport = 0;
        destinations[i].caps.wVoices = 0;
        destinations[i].caps.wNotes = 0;
    }
    return true;
}
/*
static void CoreAudio_MIDIRelease() {
    if (wineMIDIClient) MIDIClientDispose(wineMIDIClient); // MIDIClientDispose will close all ports
    free(destinations);
}
*/
U32 KNativeAudioCoreAudio::midiOutOpen(U32 wDevID) {
	MIDIDestination *dest;

    if (wDevID >= MIDIOut_NumDevs) {
        klog_fmt("KNativeAudioCoreAudio::midiOutOpen bad device ID : %d", wDevID);
        return MMSYSERR_BADDEVICEID;
    }

    dest = &destinations[wDevID];

    if (dest->caps.wTechnology == MOD_SYNTH)
    {
        if (!SynthUnit_CreateDefaultSynthUnit(&dest->graph, &dest->synth))
        {
            klog_fmt("KNativeAudioCoreAudio::midiOutOpen SynthUnit_CreateDefaultSynthUnit dest=%p failed", dest);
            return MMSYSERR_ERROR;
        }

        if (!SynthUnit_Initialize(dest->synth, dest->graph))
        {
            klog_fmt("KNativeAudioCoreAudio::midiOutOpen SynthUnit_Initialise dest=%p failed", dest);
            return MMSYSERR_ERROR;
        }
    }
    
    // MIDI_NotifyClient(wDevID, MOM_OPEN, 0L, 0L);
    return MMSYSERR_NOERROR;
}

U32 KNativeAudioCoreAudio::midiOutClose(U32 wDevID) {
    U32 ret = MMSYSERR_NOERROR;

    if (wDevID >= MIDIOut_NumDevs) {
        klog_fmt("KNativeAudioCoreAudio::midiOutClose bad device ID : %d", wDevID);
        return MMSYSERR_BADDEVICEID;
    }

    if (destinations[wDevID].caps.wTechnology == MOD_SYNTH) {
        SynthUnit_Close(destinations[wDevID].graph);
    }
    destinations[wDevID].graph = 0;
    destinations[wDevID].synth = 0;

    return ret;
}

U32 KNativeAudioCoreAudio::midiOutData(U32 wDevID, U8* buffer, U32 len) {
    U32 evt = buffer[0];
    UInt8 chn = (evt & 0x0F);

    if (wDevID >= MIDIOut_NumDevs) {
        klog_fmt("KNativeAudioCoreAudio::midiOutData bad device ID : %d", wDevID);
        return MMSYSERR_BADDEVICEID;
    }

    if (destinations[wDevID].caps.wTechnology == MOD_SYNTH)
    {
        U32 d1  = 0;
        U32 d2  = 0;
        OSStatus err = noErr;

        if (len >= 1) {
            d1 = buffer[1];
        }
        if (len >= 2) {
            d2 = buffer[2];
        }
        err = MusicDeviceMIDIEvent(destinations[wDevID].synth, (evt & 0xF0) | chn, d1, d2, 0);
        if (err != noErr)
        {
            klog_fmt("KNativeAudioCoreAudio::midiOutData MusicDeviceMIDIEvent(%p, %04x, %04x, %04x, %d) return %d", destinations[wDevID].synth, (evt & 0xF0) | chn, d1, d2, 0, err);
            return MMSYSERR_ERROR;
        }
    }
    else
    {
        MIDIOut_Send(MIDIOutPort, destinations[wDevID].dest, buffer, len);
    }

    return MMSYSERR_NOERROR;
}

U32 KNativeAudioCoreAudio::midiOutLongData(U32 wDevID, U8* buffer, U32 len) {
    OSStatus err = noErr;
    
    if (wDevID >= MIDIOut_NumDevs) {
        klog_fmt("KNativeAudioCoreAudio::midiOutLongData bad device ID : %d\n", wDevID);
        return MMSYSERR_BADDEVICEID;
    }
    
    if (buffer[0] != 0xF0 || buffer[len - 1] != 0xF7) {
        klog("KNativeAudioCoreAudio::midiOutLongDataThe allegedly system exclusive buffer is not correct\n\tPlease report with MIDI file");
    }

    if (destinations[wDevID].caps.wTechnology == MOD_SYNTH) /* FIXME */
    {
        err = MusicDeviceSysEx(destinations[wDevID].synth, (const UInt8 *) buffer, len);
        if (err != noErr)
        {
            klog_fmt("KNativeAudioCoreAudio::midiOutLongData MusicDeviceSysEx(%p, %p, %d) return %d", destinations[wDevID].synth, buffer, len, err);
            return MMSYSERR_ERROR;
        }
    }
    else if (destinations[wDevID].caps.wTechnology == MOD_MIDIPORT) {
        MIDIOut_Send(MIDIOutPort, destinations[wDevID].dest, buffer, len);
    }

    return MMSYSERR_NOERROR;
}

U32 KNativeAudioCoreAudio::midiOutGetNumDevs() {
    return MIDIOut_NumDevs;
}

U32 KNativeAudioCoreAudio::midiOutGetVolume(KThread* thread, U32 wDevID, U32 lpdwVolume) {
    if (wDevID >= MIDIOut_NumDevs) {
        klog_fmt("KNativeAudioCoreAudio::midiOutGetVolume bad device ID : %d", wDevID);
        return MMSYSERR_BADDEVICEID;
    }
    if (lpdwVolume == 0) {
        klog("KNativeAudioCoreAudio::midiOutGetVolume Invalid Parameter");
        return MMSYSERR_INVALPARAM;
    }

    if (destinations[wDevID].caps.wTechnology == MOD_SYNTH)
    {
        float left;
        float right;
        AudioUnit_GetVolume(destinations[wDevID].synth, &left, &right);

        thread->memory->writed(lpdwVolume, (U32) (left * 0xFFFF) + ((U32) (right * 0xFFFF) << 16));

        return MMSYSERR_NOERROR;
    }

    return MMSYSERR_NOTSUPPORTED;
}

U32 KNativeAudioCoreAudio::midiOutSetVolume(U32 wDevID, U32 dwVolume) {
    if (wDevID >= MIDIOut_NumDevs) {
        klog_fmt("KNativeAudioCoreAudio::midiOutSetVolume bad device ID : %d", wDevID);
        return MMSYSERR_BADDEVICEID;
    }
    if (destinations[wDevID].caps.wTechnology == MOD_SYNTH)
    {
        float left;
        float right;

        left  = (dwVolume & 0xFFFF) / 65535.0f;
        right = ((dwVolume >> 16) & 0xFFFF) / 65535.0f;
        AudioUnit_SetVolume(destinations[wDevID].synth, left, right);

        return MMSYSERR_NOERROR;
    }

    return MMSYSERR_NOTSUPPORTED;
}

BString KNativeAudioCoreAudio::midiOutGetName(U32 wDevID) {
    if (wDevID >= MIDIOut_NumDevs) {
        klog_fmt("KNativeAudioCoreAudio::midiOutGetName bad device ID : %d", wDevID);
        return BString::empty;
    }
    return BString::copy(destinations[wDevID].caps.szPname);
}

bool KNativeAudioCoreAudio::midiOutIsOpen(U32 wDevID) {
    if (wDevID >= MIDIOut_NumDevs) {
        klog_fmt("KNativeAudioCoreAudio::midiOutIsOpen bad device ID : %d", wDevID);
        return false;
    }
    if (destinations[wDevID].caps.wTechnology == MOD_SYNTH) {
        return destinations[wDevID].graph != 0;
    }
    return true;
}

U32 KNativeAudioCoreAudio::midiOutReset(U32 wDevID) {
    return 0;
}

void initCoreAudio() {
    KNativeAudio::availableAudio.push_back(std::make_shared<KNativeAudioCoreAudio>());
}

#endif
