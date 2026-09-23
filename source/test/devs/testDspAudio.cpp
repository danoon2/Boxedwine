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
	// Wine OSS remembers the maximum free space, then subtracts GETOSPACE's
	// current free bytes to infer queued PCM. Live growth from 330 to 512
	// would misreport 220 queued bytes as only 38, consuming 182 too early.
	const auto initial = KDspAudioMath::getWorkletBufferLayout(11025, 1);
	const U32 wineCachedCapacity = initial.capacityBytes;
	const auto playing = KDspAudioMath::getWorkletBufferLayout(11025, 1);
	expectEqual(wineCachedCapacity, 330, "worklet mono U8 capacity is 30 ms");
	expectEqual(playing.capacityBytes, wineCachedCapacity, "playback keeps the negotiated capacity");
	U32 freeBytes = KDspAudioMath::getOutputSpaceAvailable(playing.capacityBytes, 220, true);
	expectEqual(freeBytes, 110, "GETOSPACE reports actual remaining space");
	expectEqual(wineCachedCapacity - freeBytes, 220, "Wine still observes all 220 unplayed bytes");
	freeBytes = KDspAudioMath::getOutputSpaceAvailable(playing.capacityBytes, 110, true);
	expectEqual(wineCachedCapacity - freeBytes, 110, "Wine advances only after real consumption");

	// GETOPTR subtracts block numbers computed on separate queries. Keeping
	// its divisor at 128 avoids the old 86 -> 43 transition and unsigned wrap.
	expectEqual(initial.fragmentBytes, 128, "worklet fragment size is fixed for the PCM format");
	U32 previousBlocks = 11025 / initial.fragmentBytes;
	U32 currentBlocks = 11135 / playing.fragmentBytes;
	expectEqual(currentBlocks - previousBlocks, 0, "GETOPTR does not underflow between queries");
	previousBlocks = currentBlocks;
	currentBlocks = 11136 / playing.fragmentBytes;
	expectEqual(currentBlocks - previousBlocks, 1, "GETOPTR counts the next fragment boundary once");

	for (U32 frameSize : {1u, 2u, 4u, 8u}) {
		for (U32 rate : {11025u, 22050u}) {
			const auto layout = KDspAudioMath::getWorkletBufferLayout(rate, frameSize);
			expectEqual(layout.capacityBytes, (rate * 30 / 1000) * frameSize,
				"worklet capacity uses complete guest PCM frames");
			expectEqual(layout.fragmentBytes, (rate == 11025 ? 128 : 256) * frameSize,
				"worklet fragments use guest PCM frame size");
		}
	}

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
		KDspAudioMath::getAlignedDurationBytes(48000 * 2 * 4, 96, 8),
		36864,
		"96ms stereo float silence target is frame aligned");

	expectEqual(
		KDspAudioMath::getAlignedDurationBytes(48000 * 2 * 4, 48, 8),
		18432,
		"48ms stereo float silence target halves the queue duration");

	expectEqual(
		KDspAudioMath::getOutputSpaceAvailable(DSP_TEST_BUFFER_SIZE, 8192, false),
		DSP_TEST_BUFFER_SIZE,
		"native GETOSPACE compatibility ignores partially queued audio");

	expectEqual(
		KDspAudioMath::getOutputSpaceAvailable(DSP_TEST_BUFFER_SIZE, DSP_TEST_BUFFER_SIZE, false),
		DSP_TEST_BUFFER_SIZE,
		"native GETOSPACE compatibility reports a full queue as available");

	expectEqual(
		KDspAudioMath::getOutputSpaceAvailable(DSP_TEST_BUFFER_SIZE, 8192, true),
		24576,
		"queue-aware GETOSPACE subtracts queued audio");

	expectEqual(
		KDspAudioMath::getOutputSpaceAvailable(DSP_TEST_BUFFER_SIZE, DSP_TEST_BUFFER_SIZE + 1, true),
		0,
		"queue-aware GETOSPACE saturates an overfull queue at zero");

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
