#include "CoreAudio.h"
#include <pthread.h>

#define BOXED_AUDIO_DRV_GET_LATENCY 1
#define BOXED_AUDIO_DRV_GET_NOMINAL_SAMPLE_RATE 2
#define BOXED_AUDIO_DRV_SET_VOLUME 3
#define BOXED_AUDIO_DRV_START 4
#define BOXED_AUDIO_DRV_STOP 5
#define BOXED_AUDIO_DRV_GET_MILLIES 6
#define BOXED_AUDIO_DRV_SET_FORMAT 7
#define BOXED_AUDIO_DRV_SET_CALLBACK 8
#define BOXED_AUDIO_DRV_RELEASE 9
#define BOXED_AUDIO_DRV_OPEN 10
#define BOXED_AUDIO_DRV_INIT 11

#define CALL_0(index) __asm__("push %1\n\tint $0x98\n\taddl $4, %%esp": "=a" (result):"i"(index):); 
#define CALL_1(index, arg1) __asm__("push %2\n\tpush %1\n\tint $0x98\n\taddl $8, %%esp": "=a" (result):"i"(index), "g"((UInt32)arg1):); 
#define CALL_2(index, arg1,arg2) __asm__("push %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $12, %%esp": "=a" (result):"i"(index), "g"((UInt32)arg1), "g"((UInt32)arg2):);
#define CALL_3(index, arg1,arg2,arg3) __asm__("push %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $16, %%esp": "=a" (result):"i"(index), "g"((UInt32)arg1), "g"((UInt32)arg2), "g"((UInt32)arg3):);
#define CALL_4(index, arg1,arg2,arg3,arg4) __asm__("push %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $20, %%esp": "=a" (result):"i"(index), "g"((UInt32)arg1), "g"((UInt32)arg2), "g"((UInt32)arg3), "g"((UInt32)arg4):);
#define CALL_5(index, arg1,arg2,arg3,arg4,arg5) __asm__("push %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $24, %%esp": "=a" (result):"i"(index), "g"((UInt32)arg1), "g"((UInt32)arg2), "g"((UInt32)arg3), "g"((UInt32)arg4), "g"((UInt32)arg5):);
#define CALL_6(index, arg1,arg2,arg3,arg4,arg5,arg6) __asm__("push %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $28, %%esp": "=a" (result):"i"(index), "g"((UInt32)arg1), "g"((UInt32)arg2), "g"((UInt32)arg3), "g"((UInt32)arg4), "g"((UInt32)arg5), "g"((UInt32)arg6):);
#define CALL_7(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7) __asm__("push %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $32, %%esp": "=a" (result):"i"(index), "g"((UInt32)arg1), "g"((UInt32)arg2), "g"((UInt32)arg3), "g"((UInt32)arg4), "g"((UInt32)arg5), "g"((UInt32)arg6), "g"((UInt32)arg7):);
#define CALL_8(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) __asm__("push %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $36, %%esp": "=a" (result):"i"(index), "g"((UInt32)arg1), "g"((UInt32)arg2), "g"((UInt32)arg3), "g"((UInt32)arg4), "g"((UInt32)arg5), "g"((UInt32)arg6), "g"((UInt32)arg7), "g"((UInt32)arg8):);
#define CALL_9(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) __asm__("push %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $40, %%esp": "=a" (result):"i"(index), "g"((UInt32)arg1), "g"((UInt32)arg2), "g"((UInt32)arg3), "g"((UInt32)arg4), "g"((UInt32)arg5), "g"((UInt32)arg6), "g"((UInt32)arg7), "g"((UInt32)arg8), "g"((UInt32)arg9):);

#define CALL_NORETURN_0(index) __asm__("push %0\n\tint $0x98\n\taddl $4, %%esp"::"i"(index)); 
#define CALL_NORETURN_1(index, arg1) __asm__("push %1\n\tpush %0\n\tint $0x98\n\taddl $8, %%esp"::"i"(index), "g"((UInt32)arg1)); 
#define CALL_NORETURN_2(index, arg1, arg2) __asm__("push %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $12, %%esp"::"i"(index), "g"((UInt32)arg1), "g"((UInt32)arg2)); 
#define CALL_NORETURN_3(index, arg1, arg2, arg3) __asm__("push %3\n\rpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $16, %%esp"::"i"(index), "g"((UInt32)arg1), "g"((UInt32)arg2), "g"((UInt32)arg3)); 
#define CALL_NORETURN_4(index, arg1, arg2, arg3, arg4) __asm__("push %4\n\tpush %3\n\rpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $20, %%esp"::"i"(index), "g"((UInt32)arg1), "g"((UInt32)arg2), "g"((UInt32)arg3), "g"((UInt32)arg4)); 
#define CALL_NORETURN_5(index, arg1, arg2, arg3, arg4, arg5) __asm__("push %5\n\tpush %4\n\tpush %3\n\rpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $24, %%esp"::"i"(index), "g"((UInt32)arg1), "g"((UInt32)arg2), "g"((UInt32)arg3), "g"((UInt32)arg4), "g"((UInt32)arg5)); 
#define CALL_NORETURN_6(index, arg1,arg2,arg3,arg4,arg5,arg6) __asm__("push %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $28, %%esp"::"i"(index), "g"((UInt32)arg1), "g"((UInt32)arg2), "g"((UInt32)arg3), "g"((UInt32)arg4), "g"((UInt32)arg5), "g"((UInt32)arg6));
#define CALL_NORETURN_7(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7) __asm__("push %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $32, %%esp"::"i"(index), "g"((UInt32)arg1), "g"((UInt32)arg2), "g"((UInt32)arg3), "g"((UInt32)arg4), "g"((UInt32)arg5), "g"((UInt32)arg6), "g"((UInt32)arg7));
#define CALL_NORETURN_8(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) __asm__("push %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $36, %%esp"::"i"(index), "g"((UInt32)arg1), "g"((UInt32)arg2), "g"((UInt32)arg3), "g"((UInt32)arg4), "g"((UInt32)arg5), "g"((UInt32)arg6), "g"((UInt32)arg7), "g"((UInt32)arg8));
#define CALL_NORETURN_9(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) __asm__("push %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $40, %%esp"::"i"(index), "g"((UInt32)arg1), "g"((UInt32)arg2), "g"((UInt32)arg3), "g"((UInt32)arg4), "g"((UInt32)arg5), "g"((UInt32)arg6), "g"((UInt32)arg7), "g"((UInt32)arg8), "g"((UInt32)arg9));

UInt32 boxed_getLatency(AudioDeviceID devId) {
	UInt32 result;
	CALL_1(BOXED_AUDIO_DRV_GET_LATENCY, devId);
	return result;
}

UInt32 boxed_getNominalSampleRate(AudioDeviceID devId) {
	UInt32 result;
	CALL_1(BOXED_AUDIO_DRV_GET_NOMINAL_SAMPLE_RATE, devId);
	return result;
}

struct int2Float {
	union {
		UInt32 i;
		float f;
	};
};

void boxed_setVolume(AudioDeviceID devId, Float32 volume, UInt32 channel) {
	struct int2Float f;
	f.f = volume;
	CALL_NORETURN_3(BOXED_AUDIO_DRV_SET_VOLUME, devId, f.i, channel);
}

UInt32 boxed_start(AudioUnit devId) {
	UInt32 result;
	CALL_1(BOXED_AUDIO_DRV_START, devId->nativeDeviceId);
	return result;
}

UInt32 boxed_stop(AudioUnit devId) {
	UInt32 result;
	CALL_1(BOXED_AUDIO_DRV_STOP, devId->nativeDeviceId);
	return result;
}

UInt32 boxed_getMillies() {
	UInt32 result;
	CALL_0(BOXED_AUDIO_DRV_GET_MILLIES);
	return result;
}

UInt32 boxed_setFormat(AudioUnit devId, void* format) {
	UInt32 result;
	CALL_2(BOXED_AUDIO_DRV_SET_FORMAT, devId->nativeDeviceId, format);
	return result;
}

typedef void(*BoxedwineRenderCallback)(UInt32 frames, void* buffer);

void boxed_setCallback(AudioUnit devId, BoxedwineRenderCallback pfn, int fd) {
	CALL_NORETURN_3(BOXED_AUDIO_DRV_SET_CALLBACK, devId->nativeDeviceId, pfn, fd);
}

AURenderCallbackStruct boxed_renderCallback;

void boxed_callback(UInt32 frames, void* buffer) {
	AudioBufferList ioData;
	AURenderCallback inputProc = boxed_renderCallback.inputProc;
	void* inputProcRefCon = boxed_renderCallback.inputProcRefCon;

	ioData.mNumberBuffers = 1;
	ioData.mBuffers[0].mData = buffer;
	ioData.mBuffers[0].mNumberChannels = 0; // wine doesn't look at this
	ioData.mBuffers[0].mDataByteSize = 0; // wine doesn't look at this
	
	if (inputProc && inputProcRefCon) {
		inputProc(inputProcRefCon, NULL, NULL, 0, frames, &ioData);
	}
}

static unsigned char read8(int fd) {
	unsigned char b;
	while (read(fd, &b, 1) == 0) {
	}
	return b;
}

static UInt32 read32(int fd) {
	UInt32 result = read8(fd);
	result |= (read8(fd) << 8);
	result |= (read8(fd) << 16);
	result |= (read8(fd) << 24);
	return result;
}

static void* msg_thread(void* arg) {
	int fd = (int)arg;
	char b;

	while (read(fd, &b, 1) >= 0) {
		if (b == 1) {
			BoxedwineRenderCallback callback;
			UInt32 frames;
			void* buffer;

			callback = (BoxedwineRenderCallback)read32(fd);
			frames = read32(fd);
			buffer = (void*)read32(fd);
			callback(frames, buffer);
			write(fd, &b, 1); // signal that we are done
		} else if (b == 'x') {
			break;
		}
	}
	return NULL;
}

UInt32 boxed_open(UInt32 isRenderer) {
	UInt32 result;
	CALL_1(BOXED_AUDIO_DRV_OPEN, isRenderer);
	return result;
}

void boxed_release(AudioUnit unit) {
	CALL_NORETURN_1(BOXED_AUDIO_DRV_RELEASE, unit->nativeDeviceId);
}

UInt32 boxed_init(AudioUnit unit) {
	UInt32 result;
	CALL_1(BOXED_AUDIO_DRV_INIT, unit->nativeDeviceId);
	return result;
}

const CFStringRef kMIDIPropertyName;

#define BOXEDWINE_PLAYER_ID 1
#define BOXEDWINE_RECORDER_ID 2

void mach_timebase_info(mach_timebase_info_t info) {
	info->numer = 1;
	info->denom = 1;
}

uint64_t mach_absolute_time(void) {
	return ((uint64_t)boxed_getMillies()) * 1000000;
}

CFIndex CFStringGetLength(CFStringRef theString) {
	if (theString.data)
		return theString.data->len;
	return (CFIndex)strlen(theString.str);
}

Boolean CFStringGetCString(CFStringRef theString, char* buffer, CFIndex bufferSize, CFStringEncoding encoding) {
	memcpy(buffer, theString.str, (size_t)bufferSize);
	buffer[bufferSize - 1] = 0;
	return 1;
}

void CFStringGetCharacters(CFStringRef theString, CFRange range, UniChar* buffer) {
	int len = (int)CFStringGetLength(theString);
	int strIndex;
	int bufferIndex = 0;
	for (strIndex = range.location; strIndex < len && bufferIndex < range.length; strIndex++, bufferIndex++) {
		buffer[bufferIndex] = theString.str[strIndex];
	}
}

CFIndex CFStringGetBytes(CFStringRef theString, CFRange range, CFStringEncoding encoding, UInt8 lossByte, Boolean isExternalRepresentation, UInt8* buffer, CFIndex maxBufLen, CFIndex* usedBufLen) {
	int len = (int)CFStringGetLength(theString);
	int strIndex;
	int bufferIndex = 0;
	CFIndex copied = 0;

	for (strIndex = range.location; strIndex < len && bufferIndex < range.length && bufferIndex < maxBufLen; strIndex++, bufferIndex++) {
		if (buffer) {
			buffer[bufferIndex] = theString.str[strIndex];
		}
	}
	if (usedBufLen) {
		*usedBufLen = copied;
	}
	return copied;
}

CFStringRef CFStringCreateWithCStringNoCopy(CFAllocatorRef alloc, const char* cStr, CFStringEncoding encoding, CFAllocatorRef contentsDeallocator) {
	CFString result;
	result.str = cStr;
	result.data = NULL;
	return result;
}

void CFRelease(CFStringRef cf) {
	if (cf.data) {
		cf.data->refCount--;
		if (cf.data->refCount == 0) {
			free(cf.data);
			free((void*)cf.str);
			cf.data = NULL;
			cf.str = NULL;
		}
	}
}

// wine doesn't pass anything to formatOptions other than NULL
CFStringRef CFStringCreateWithFormat(CFAllocatorRef alloc, void* formatOptions, CFStringRef format, ...) {
	va_list argptr;
	va_start(argptr, format);

	{
		CFStringRef result;
		char buff[1024];
		char* s;

		vsnprintf(buff, sizeof(buff), format.str, argptr);
		result.data = (struct CFStringData*)malloc(sizeof(struct CFStringData));
		result.data->refCount = 1;
		result.data->len = strlen(buff);
		s = (char*)malloc(result.data->len + 1);
		memcpy(s, buff, result.data->len + 1);
		result.str = s;
		return result;
	}
}

OSStatus AudioObjectGetPropertyDataSize(AudioObjectID inObjectID, const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32* outDataSize) {
	if (inAddress->mSelector == kAudioDevicePropertyStreamConfiguration) {
		*outDataSize = sizeof(AudioBufferList);
	} else if (inAddress->mSelector == kAudioHardwarePropertyDevices) {
		*outDataSize = sizeof(AudioDeviceID)*2; // 2 audio device (playback and record)
	} else if (inAddress->mSelector == kAudioDevicePropertyPreferredChannelLayout) {
		*outDataSize = sizeof(AudioChannelLayout) + sizeof(AudioChannelDescription); // 2 channels
	} else if (inAddress->mSelector == kAudioDevicePropertyStreams) {
		*outDataSize = sizeof(AudioStreamID); // :TODO: what does it mean to have more that 1 input or 1 output stream?
	} else {
		printf("AudioObjectGetPropertyDataSize unknown mSelector %x\n", inAddress->mSelector);
		return 1;
	}
	return noErr;
}

OSStatus AudioObjectGetPropertyData(AudioObjectID inObjectID, const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32* ioDataSize, void* outData) {
	if (inAddress->mSelector == kAudioDevicePropertyStreamConfiguration) {
		AudioBufferList* p = (AudioBufferList*)outData;
		memset(p, 0, sizeof(AudioBufferList));
		p->mNumberBuffers = 1;
		p->mBuffers->mNumberChannels = 2;
	} else if (inAddress->mSelector == kAudioHardwarePropertyDevices) {
		AudioDeviceID* p = (AudioDeviceID*)outData;
		p[0] = BOXEDWINE_PLAYER_ID;
		p[1] = BOXEDWINE_RECORDER_ID;
	} else if (inAddress->mSelector == kAudioDevicePropertyPreferredChannelLayout) {
		AudioChannelLayout* p = (AudioChannelLayout*)outData;
		p->mChannelLayoutTag = kAudioChannelLayoutTag_UseChannelDescriptions;
		p->mNumberChannelDescriptions = 2;
		p->mChannelDescriptions[0].mChannelLabel = kAudioChannelLabel_Left;
		p->mChannelDescriptions[1].mChannelLabel = kAudioChannelLabel_Right;
	} else if (inAddress->mSelector == kAudioDevicePropertyNominalSampleRate) {
		Float64* p = (Float64*)outData;
		*p = boxed_getNominalSampleRate(inObjectID);
	} else if (inAddress->mSelector == kAudioStreamPropertyLatency) {
		UInt32* p = (UInt32*)outData;
		*p = boxed_getLatency(inObjectID);
	} else if (inAddress->mSelector == kAudioHardwarePropertyDefaultOutputDevice) {
		AudioDeviceID* p = (AudioDeviceID*)outData;
		*p = BOXEDWINE_PLAYER_ID;
	} else if (inAddress->mSelector == kAudioHardwarePropertyDefaultInputDevice) {
		AudioDeviceID* p = (AudioDeviceID*)outData;
		*p = BOXEDWINE_RECORDER_ID;
	} else if (inAddress->mSelector == kAudioObjectPropertyName) {
		CFStringRef* p = (CFStringRef*)outData;
		p->data = NULL;
		if (inAddress->mScope == kAudioDevicePropertyScopeOutput) {
			p->str = "Boxedwine Output";
		} else {
			p->str = "Boxedwine Input";
		}
	} else if (inAddress->mSelector == kAudioDevicePropertyDeviceUID) {
		CFStringRef* p = (CFStringRef*)outData;
		p->data = NULL;
		// randomly generated online
		if (inAddress->mScope == kAudioDevicePropertyScopeOutput) {
			p->str = "fa09b039-8214-409a-9e9d-37b36e3a485d";
		} else {
			p->str = "85389942-7b29-4eef-9d11-3a65c854e817";
		}
	} else if (inAddress->mSelector == kAudioHardwarePropertyTranslateUIDToDevice) {
		AudioDeviceID* p = (AudioDeviceID*)outData;
		CFStringRef* pUUID = (CFStringRef*)inQualifierData;
		if (strcmp(pUUID->str, "85389942-7b29-4eef-9d11-3a65c854e817") == 0) {
			*p = BOXEDWINE_RECORDER_ID;
		} else {
			*p = BOXEDWINE_PLAYER_ID;
		}
	} else if (inAddress->mSelector == kAudioDevicePropertyStreams) {
		AudioStreamID* p = (AudioStreamID*)outData;
		*p = inObjectID; // currently no difference between AudioDeviceID and AudioStreamID
	} else {
		printf("AudioObjectGetPropertyData unknown mSelector %x\n", inAddress->mSelector);
		return 1;
	}
	return noErr;
}

OSStatus AudioUnitSetProperty(AudioUnit inUnit, AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, const void* inData, UInt32 inDataSize) {
	if (inID == kAudioOutputUnitProperty_EnableIO) {
	} else if (inID == kAudioOutputUnitProperty_CurrentDevice) {
		inUnit->deviceId = *(AudioDeviceID*)inData;
		inUnit->nativeDeviceId = boxed_open(inUnit->deviceId == BOXEDWINE_PLAYER_ID);
		if (inUnit->nativeDeviceId == 0) {
			return 1;
		}
	} else if (inID == kAudioUnitProperty_StreamFormat) {
		return boxed_setFormat(inUnit, inData);
	} else if (inID == kAudioOutputUnitProperty_SetInputCallback) {
	} else if (inID == kAudioUnitProperty_SetRenderCallback) {
		boxed_renderCallback = *(AURenderCallbackStruct*)inData;
		if (!inUnit->threadId) {
			pipe(inUnit->pipe);
			pthread_create(&inUnit->threadId, NULL, msg_thread, (void*)inUnit->pipe[0]);
		}
		boxed_setCallback(inUnit, boxed_callback, inUnit->pipe[1]);
	} else {
		printf("AudioUnitSetProperty unknown inID %x\n", inID);
		return 1;
	}
	return noErr;
}

OSStatus AudioUnitGetProperty(AudioUnit inUnit, AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, void* outData, UInt32* ioDataSize) {
	if (inID == kAudioUnitProperty_StreamFormat) {
		// only used in dataflow == eCapture
	}
	printf("AudioUnitGetProperty not implemented\n");
	return 1;
}

OSStatus AudioUnitRender(AudioUnit inUnit, AudioUnitRenderActionFlags* ioActionFlags, const AudioTimeStamp* inTimeStamp, UInt32 inOutputBusNumber, UInt32 inNumberOfFrames, AudioBufferList* ioData) {
	// used in input callback
	printf("AudioUnitRender not implemented\n");
	return 1;
}

OSStatus AudioObjectSetPropertyData(AudioObjectID inObjectID, const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData) {
	if (inAddress->mSelector == kAudioDevicePropertyVolumeScalar) {
		Float32* p = (Float32*)inData;
		boxed_setVolume(inObjectID, *p, inAddress->mElement);
	} else {
		printf("AudioObjectSetPropertyData unknown mSelector %x\n", inAddress->mSelector);
		return 1;
	}
	return noErr;
}

OSStatus AudioUnitInitialize(AudioUnit inUnit) {
	return boxed_init(inUnit);
}

OSStatus AudioOutputUnitStart(AudioUnit ci) {
	return boxed_start(ci);
}

OSStatus AudioOutputUnitStop(AudioUnit ci) {
	return boxed_stop(ci);
}

AudioComponent AudioComponentFindNext(AudioComponent inComponent, const AudioComponentDescription* inDesc) {
	return (AudioComponent)1; // we don't care, just don't return NULL
}

OSStatus AudioComponentInstanceNew(AudioComponent inComponent, AudioComponentInstance* outInstance) {
	ComponentInstanceRecord* unit = (ComponentInstanceRecord*)malloc(sizeof(ComponentInstanceRecord));
	memset(unit, 0, sizeof(ComponentInstanceRecord));
	*outInstance = unit;	
	return noErr;
}

OSStatus AudioConverterDispose(AudioConverterRef inAudioConverter) {
	return noErr;
}

OSStatus AudioConverterNew(const AudioStreamBasicDescription* inSourceFormat, const AudioStreamBasicDescription* inDestinationFormat, AudioConverterRef* outAudioConverter) {
	// only used in dataflow == eCapture
	printf("AudioConverterNew not implemented\n");
	return 1;
}

OSStatus AudioConverterFillComplexBuffer(AudioConverterRef inAudioConverter, AudioConverterComplexInputDataProc inInputDataProc, void* inInputDataProcUserData, UInt32* ioOutputDataPacketSize, AudioBufferList* outOutputData, AudioStreamPacketDescription* outPacketDescription) {
	// only used in dataflow == eCapture
	printf("AudioConverterFillComplexBuffer not implemented\n");
	return 1;
}

OSStatus AudioComponentInstanceDispose(AudioComponentInstance inInstance) {
	boxed_release(inInstance);
	if (inInstance->threadId) {
		write(inInstance->pipe[1], "x", 1);
		pthread_join(inInstance->threadId, NULL);
		close(inInstance->pipe[0]);
		close(inInstance->pipe[1]);
	}
	free(inInstance);
	return noErr;
}

OSStatus AudioUnitGetParameter(AudioUnit inUnit, AudioUnitParameterID inID, AudioUnitScope inScope, AudioUnitElement inElement, AudioUnitParameterValue* outValue) {
	// used by midi
	return noErr;
}

OSStatus AudioUnitSetParameter(AudioUnit inUnit, AudioUnitParameterID inID, AudioUnitScope inScope, AudioUnitElement inElement, AudioUnitParameterValue inValue, UInt32 inBufferOffsetInFrames) {
	// used by midi
	return noErr;
}

OSStatus AUGraphInitialize(AUGraph inGraph) {
	// used by midi
	return noErr;
}

OSStatus AUGraphStart(AUGraph inGraph) {
	// used by midi
	return noErr;
}

OSStatus AUGraphStop(AUGraph inGraph) {
	// used by midi
	return noErr;
}

OSStatus DisposeAUGraph(AUGraph inGraph) {
	// used by midi
	return noErr;
}

OSStatus NewAUGraph(AUGraph* outGraph) {
	// used by midi
	return noErr;
}

OSStatus AUGraphAddNode(AUGraph inGraph, const AudioComponentDescription* inDescription, AUNode* outNode) {
	// used by midi
	return noErr;
}

OSStatus AUGraphOpen(AUGraph inGraph) {
	// used by midi
	return noErr;
}

OSStatus AUGraphConnectNodeInput(AUGraph inGraph, AUNode inSourceNode, UInt32 inSourceOutputNumber, AUNode inDestNode, UInt32 inDestInputNumber) {
	// used by midi
	return noErr;
}

OSStatus AUGraphNodeInfo(AUGraph inGraph, AUNode inNode, AudioComponentDescription* outDescription, AudioUnit* outAudioUnit) {
	// used by midi
	return noErr;
}

MIDIPacket* MIDIPacketListInit(MIDIPacketList* pktlist) {
	// used by midi
	return NULL;
}

MIDIPacket* MIDIPacketListAdd(MIDIPacketList* pktlist, ByteCount listSize, MIDIPacket* curPacket, MIDITimeStamp time, ByteCount nData, const Byte* data) {
	// used by midi
	return NULL;
}

OSStatus MIDIClientCreate(CFStringRef name, MIDINotifyProc notifyProc, void* notifyRefCon, MIDIClientRef* outClient) {
	// used by midi
	return noErr;
}

OSStatus MIDIOutputPortCreate(MIDIClientRef client, CFStringRef portName, MIDIPortRef* outPort) {
	// used by midi
	return noErr;
}

OSStatus MIDIObjectGetStringProperty(MIDIObjectRef obj, CFStringRef propertyID, CFStringRef* str) {
	// used by midi
	return noErr;
}

OSStatus MIDIPortConnectSource(MIDIPortRef port, MIDIEndpointRef source, void* connRefCon) {
	// used by midi
	return noErr;
}

MIDIEndpointRef MIDIGetDestination(ItemCount destIndex0) {
	// used by midi
	return 0;
}

ItemCount MIDIGetNumberOfDestinations(void) {
	// used by midi
	return 0;
}

ItemCount MIDIGetNumberOfSources(void) {
	// used by midi
	return 0;
}

OSStatus MIDIInputPortCreate(MIDIClientRef client, CFStringRef portName, MIDIReadProc readProc, void* refCon, MIDIPortRef* outPort) {
	// used by midi
	return noErr;
}

MIDIEndpointRef MIDIGetSource(ItemCount sourceIndex0) {
	// used by midi
	return 0;
}

OSStatus MIDIClientDispose(MIDIClientRef client) {
	// used by midi
	return noErr;
}

OSStatus MIDISend(MIDIPortRef port, MIDIEndpointRef dest, const MIDIPacketList* pktlist) {
	// used by midi
	return noErr;
}

OSStatus MusicDeviceMIDIEvent(MusicDeviceComponent inUnit, UInt32 inStatus, UInt32 inData1, UInt32 inData2, UInt32 inOffsetSampleFrame) {
	// used by midi
	return noErr;
}

OSStatus MusicDeviceSysEx(MusicDeviceComponent inUnit, const UInt8* inData, UInt32 inLength) {
	// used by midi
	return noErr;
}
