/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later.
 * Build: i686-w64-mingw32-gcc waveout_stream_probe.c -O2 -lwinmm -lm -o waveout-stream-probe.exe
 * Exercises PCM conversion, queue backpressure, draining, and reopen through Wine.
 * Run with sound enabled and disabled. Does not measure acoustic output latency.
 */
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *log_file;
#ifndef AUDIO_PROBE_BLOCK_MS
#define AUDIO_PROBE_BLOCK_MS 100
#endif
#ifndef AUDIO_PROBE_CYCLES
#define AUDIO_PROBE_CYCLES 5
#endif

static int play(int rate, int channels, int bits, int floating) {
    WAVEFORMATEX format;
    HWAVEOUT device = NULL;
    WAVEHDR headers[4];
    MMTIME position;
    int prepared = 0, ok = 0;
    const int frames = rate * AUDIO_PROBE_BLOCK_MS / 1000;
    const int bytes = frames * channels * bits / 8;
    char *pcm = (char *)malloc(bytes * 4);
    memset(&format, 0, sizeof(format));
    memset(headers, 0, sizeof(headers));
    format.wFormatTag = floating ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
    format.nChannels = channels;
    format.nSamplesPerSec = rate;
    format.wBitsPerSample = bits;
    format.nBlockAlign = channels * bits / 8;
    format.nAvgBytesPerSec = rate * format.nBlockAlign;
    MMRESULT result = waveOutOpen(&device, WAVE_MAPPER, &format, 0, 0, CALLBACK_NULL);
    if (result || !pcm) {
        fprintf(log_file, "AUDIO_ERROR open=%u rate=%d bits=%d\n", result, rate, bits);
        goto done;
    }
    for (int block = 0; block < 4; ++block) {
        headers[block].lpData = pcm + block * bytes;
        headers[block].dwBufferLength = bytes;
        for (int frame = 0; frame < frames; ++frame) {
            float sample = (float)(0.2 * sin(2 * 3.141592653589793 * 440 * (block * frames + frame) / rate));
            for (int channel = 0; channel < channels; ++channel) {
                int index = frame * channels + channel;
                if (floating) ((float *)headers[block].lpData)[index] = sample;
                else if (bits == 16) ((short *)headers[block].lpData)[index] = (short)(sample * 32767);
                else ((unsigned char *)headers[block].lpData)[index] = (unsigned char)(128 + sample * 127);
            }
        }
        result = waveOutPrepareHeader(device, &headers[block], sizeof(WAVEHDR));
        if (result) goto done;
        ++prepared;
    }
    for (int cycle = 0; cycle < AUDIO_PROBE_CYCLES; ++cycle) {
        for (int block = 0; block < 4; ++block) {
            result = waveOutWrite(device, &headers[block], sizeof(WAVEHDR));
            if (result) goto done;
        }
        DWORD start = GetTickCount();
        while (!(headers[3].dwFlags & WHDR_DONE)) {
            if (GetTickCount() - start > AUDIO_PROBE_BLOCK_MS * 4 + 10000) {
                fprintf(log_file, "AUDIO_ERROR timeout rate=%d bits=%d\n", rate, bits);
                goto done;
            }
            Sleep(5);
        }
    }
    memset(&position, 0, sizeof(position));
    position.wType = TIME_SAMPLES;
    result = waveOutGetPosition(device, &position, sizeof(position));
    if (result) goto done;
    fprintf(log_file, "AUDIO_PASS rate=%d channels=%d bits=%d float=%d positionType=%u position=%lu\n",
        rate, channels, bits, floating, position.wType, (unsigned long)position.u.sample);
    ok = 1;
done:
    if (device) {
        if (!ok) waveOutReset(device);
        for (int block = 0; block < prepared; ++block) waveOutUnprepareHeader(device, &headers[block], sizeof(WAVEHDR));
        result = waveOutClose(device);
        if (result) ok = 0;
    }
    if (!ok) fprintf(log_file, "AUDIO_ERROR mmresult=%u\n", result);
    fflush(log_file);
    free(pcm);
    Sleep(100);
    return ok;
}

int main(void) {
    log_file = fopen("c:/audio-output-probe.log", "w");
    if (!log_file) return 2;
    fprintf(log_file, "AUDIO_START\n");
    fflush(log_file);
    puts("AUDIO_START: click the canvas now to enable Boxedwine sound");
    fflush(stdout);
    Sleep(8000);
    int ok = play(11025, 1, 8, 0) && play(22050, 2, 16, 0) && play(11025, 2, 32, 1);
    fprintf(log_file, "AUDIO_RESULT %s\n", ok ? "PASS" : "FAIL");
    fclose(log_file);
    // Give the host's final output queue a chance to drain before Wine exits.
    Sleep(500);
    return ok ? 0 : 1;
}
