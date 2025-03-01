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

#ifndef __KNATIVEAUDIO_H__
#define __KNATIVEAUDIO_H__

class KNativeAudio {
public:
	static std::shared_ptr<KNativeAudio> createNativeAudio();
    static std::vector< std::shared_ptr<KNativeAudio> > availableAudio;
    static void init();
	static void shutdown();

	KNativeAudio() {}
	virtual ~KNativeAudio() {}
	virtual bool load() = 0;
    
	virtual U32 midiOutOpen(U32 wDevID) = 0;
	virtual U32 midiOutClose(U32 wDevID) = 0;
	virtual U32 midiOutData(U32 wDevID, U8* buffer, U32 len) = 0;
	virtual U32 midiOutLongData(U32 wDevID, U8* buffer, U32 len) = 0;
	virtual U32 midiOutGetNumDevs() = 0;
	virtual U32 midiOutGetVolume(KThread* thread, U32 wDevID, U32 lpdwVolume) = 0;
	virtual U32 midiOutSetVolume(U32 wDevID, U32 dwVolume) = 0;
	virtual U32 midiOutReset(U32 wDevID) = 0;
    virtual BString midiOutGetName(U32 wDevID) = 0;
    virtual bool midiOutIsOpen(U32 wDevID) = 0;
};

typedef std::shared_ptr<KNativeAudio> KNativeAudioPtr;
#endif
