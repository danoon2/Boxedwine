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

#ifndef __KNATIVE_AUDIO_SDL_H__
#define __KNATIVE_AUDIO_SDL_H__

class KNativeAudioSDL : public KNativeAudio {
public:
	virtual ~KNativeAudioSDL() {}

	bool load() override;
    U32 midiOutOpen(U32 wDevID) override;
    U32 midiOutClose(U32 wDevID) override;
    U32 midiOutData(U32 wDevID, U8* buffer, U32 len) override;
    U32 midiOutLongData(U32 wDevID, U8* buffer, U32 len) override;
    U32 midiOutGetNumDevs() override;
    U32 midiOutGetVolume(KThread* thread, U32 wDevID, U32 lpdwVolume) override;
    U32 midiOutSetVolume(U32 wDevID, U32 dwVolume) override;
    U32 midiOutReset(U32 wDevID) override;
    BString midiOutGetName(U32 wDevID) override;
    bool midiOutIsOpen(U32 wDevID) override;
};

#endif
