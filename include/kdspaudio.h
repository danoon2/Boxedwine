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

#ifndef __KDSPAUDIO_H__
#define __KDSPAUDIO_H__

class KDspAudio;
typedef std::shared_ptr<KDspAudio> KDspAudioPtr;
typedef std::weak_ptr<KDspAudio> KDspAudioWeakPtr;

class KDspAudio {
public:
	static KDspAudioPtr createDspAudio();
	static void shutdown();

	virtual ~KDspAudio();

	virtual void openAudio(U32 format, U32 freq, U32 channels) = 0;
	virtual void soundEnabled() = 0;
	virtual bool isOpen() = 0;
	virtual void closeAudio() = 0;
	virtual U32 writeAudio(U8* data, U32 len) = 0;
	virtual U32 getFragmentSize() = 0;
	virtual U32 getBufferSize() = 0;
	virtual U32 getBufferCapacity() = 0;

	U32 id = 0;

	static BOXEDWINE_MUTEX mutex;
	static BHashTable<U32, KDspAudioWeakPtr> openAudios;

	static void iterateOpenAudio(std::function<void(KDspAudioPtr&)> callback);
};

#endif