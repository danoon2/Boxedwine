/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __KDSPAUDIO_MATH_H__
#define __KDSPAUDIO_MATH_H__

namespace KDspAudioMath {
	inline U32 getWriteCapacity(U32 bytesPerSecond, U32 fragmentSize, U32 bufferSize) {
		U32 capacity = bytesPerSecond / 8;
		if (capacity < fragmentSize) {
			capacity = fragmentSize;
		}
		if (capacity > bufferSize) {
			capacity = bufferSize;
		}
		return capacity;
	}

	inline U32 getAvailableWriteBytes(U32 capacity, U32 queued) {
		return queued >= capacity ? 0 : capacity - queued;
	}

	inline U32 getQueuedAfterElapsed(U32 queued, U32 bytesPerSecond, U32 elapsedMs) {
		U64 consumed = (U64)bytesPerSecond * elapsedMs / 1000;
		return consumed >= queued ? 0 : queued - (U32)consumed;
	}

	inline U32 alignWriteBytes(U32 bytes, U32 blockSize) {
		if (!blockSize) {
			return 0;
		}
		return bytes & ~(blockSize - 1);
	}

	inline U32 getWritableBytes(U32 requested, U32 capacity, U32 queued, U32 blockSize) {
		U32 available = getAvailableWriteBytes(capacity, queued);
		U32 writable = requested < available ? requested : available;
		return alignWriteBytes(writable, blockSize);
	}
}

#endif
