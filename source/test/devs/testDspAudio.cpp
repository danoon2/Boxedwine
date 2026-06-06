/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"

#ifdef __TEST

#include "testDspAudio.h"
#include "kdspaudio_math.h"
#include "../cpu/testCPU.h"

namespace {
	constexpr U32 DSP_TEST_BUFFER_SIZE = 1024 * 32;

	void expectEqual(U32 actual, U32 expected, const char* label) {
		if (actual != expected) {
			testFail("%s expected %u, got %u", label, expected, actual);
		}
	}
}

void testDspAudioWriteMath() {
	expectEqual(
		KDspAudioMath::getWriteCapacity(11025, 4096, DSP_TEST_BUFFER_SIZE),
		4096,
		"default 11025Hz mono U8 capacity uses at least one fragment");

	expectEqual(
		KDspAudioMath::getWriteCapacity(48000 * 2 * 2, 4096, DSP_TEST_BUFFER_SIZE),
		24000,
		"48KHz stereo 16-bit capacity uses 125ms of audio");

	expectEqual(
		KDspAudioMath::getWriteCapacity(192000 * 2 * 4, 4096, DSP_TEST_BUFFER_SIZE),
		DSP_TEST_BUFFER_SIZE,
		"high-rate capacity clamps to DSP buffer size");

	expectEqual(
		KDspAudioMath::getAvailableWriteBytes(4096, 1024),
		3072,
		"available bytes subtract queued bytes");

	expectEqual(
		KDspAudioMath::getAvailableWriteBytes(4096, 4096),
		0,
		"full queue has no available bytes");

	expectEqual(
		KDspAudioMath::getAvailableWriteBytes(4096, 5000),
		0,
		"overfull queue has no available bytes");

	expectEqual(
		KDspAudioMath::getWritableBytes(5000, 4096, 1025, 4),
		3068,
		"writable bytes are capped by available space and block aligned");

	expectEqual(
		KDspAudioMath::getWritableBytes(1024, 4096, 0, 4),
		1024,
		"writable bytes preserve smaller aligned requests");

	expectEqual(
		KDspAudioMath::getWritableBytes(3, 4096, 0, 4),
		0,
		"sub-block writes align down to zero");

	expectEqual(
		KDspAudioMath::getQueuedAfterElapsed(4096, 11025, 100),
		2994,
		"queued no-sound bytes drain as time passes");

	expectEqual(
		KDspAudioMath::getQueuedAfterElapsed(4096, 11025, 1000),
		0,
		"queued no-sound bytes clamp to zero after enough time passes");
}

#endif
