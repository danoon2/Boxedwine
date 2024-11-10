#ifndef __COREAUDIO_H__
#define __COREAUDIO_H__

#include <stdint.h>
#include <unistd.h>
#include <math.h>

// needed for Wine 6
#define HAVE_AUDIOUNIT_AUDIOCOMPONENT_H
#define HAVE_AUGRAPHADDNODE

#ifndef false
#define false 0
#endif

struct mach_timebase_info {
	uint32_t        numer;
	uint32_t        denom;
};

typedef struct mach_timebase_info* mach_timebase_info_t;
typedef struct mach_timebase_info  mach_timebase_info_data_t;

void mach_timebase_info(mach_timebase_info_t info);

uint64_t mach_absolute_time(void);

typedef double Float64;
typedef float Float32;
typedef uint32_t UInt32;
typedef int32_t OSStatus;
typedef uint64_t UInt64;
typedef int16_t SInt16;
typedef int32_t SInt32;
typedef int64_t SInt64;
typedef uint8_t UInt8;
typedef UInt8 Byte;
typedef uint16_t UInt16;

struct ComponentInstanceRecord {
	UInt32 deviceId;
	UInt32 nativeDeviceId;
	UInt32 threadId;
	SInt32 pipe[2];
};
typedef struct ComponentInstanceRecord ComponentInstanceRecord;
typedef ComponentInstanceRecord* AudioComponentInstance;
typedef AudioComponentInstance MusicDeviceComponent;
typedef void* AudioConverterRef;

typedef UInt32 AudioFormatID;
typedef UInt32 AudioFormatFlags;
typedef UInt32 AudioObjectID;
typedef AudioObjectID AudioDeviceID;
typedef AudioObjectID AudioStreamID;
typedef UInt32 AudioObjectPropertyScope;
typedef UInt32 AudioObjectPropertySelector;
typedef UInt32 AudioObjectPropertyElement;
typedef UInt32 AudioUnitRenderActionFlags;
typedef UInt32 SMPTETimeType;
typedef UInt32 SMPTETimeFlags;
typedef UInt32 AudioTimeStampFlags;
typedef void* AudioComponent;
typedef UInt32 FourCharCode;
typedef FourCharCode OSType;
typedef AudioComponentInstance AudioUnit;
typedef UInt32 AudioUnitPropertyID;
typedef UInt32 AudioUnitScope;
typedef UInt32 AudioUnitElement;
typedef UInt32 AudioChannelLayoutTag;
typedef UInt32 AudioChannelBitmap;
typedef UInt32 AudioChannelLabel;
typedef UInt32 AudioChannelFlags;
typedef void* AUGraph;
typedef UInt32 MIDIObjectRef;
typedef MIDIObjectRef MIDIEndpointRef;
typedef MIDIObjectRef MIDIClientRef;
typedef MIDIObjectRef MIDIPortRef;
typedef UInt64 MIDITimeStamp;
typedef SInt32 MIDINotificationMessageID;
typedef UInt32 ItemCount;
typedef SInt32 AUNode;
typedef UInt32 ByteCount;
typedef UInt32 AudioUnitParameterID;
typedef Float32 AudioUnitParameterValue;

#define noErr 0
#define kAudioObjectUnknown 0

#define kAudioFormatUnsupportedDataFormatError 0x77686174
#define kAudioFormatUnknownFormatError 0x21666D74

#define kAudioDeviceUnsupportedFormatError 0x21646174

#define kAudioHardwareBadDeviceError 0x21646576

#define kAudioHardwarePropertyDevices 0x64647623
#define kAudioHardwarePropertyDefaultOutputDevice 0x644F7574
#define kAudioHardwarePropertyDefaultInputDevice 0x64496E20
#define kAudioHardwarePropertyTranslateUIDToDevice 0x75696464

#define kAudioDevicePropertyScopeOutput 0x6F757470
#define kAudioDevicePropertyScopeInput 0x696E7074
#define kAudioDevicePropertyStreamConfiguration 0x736C6179
#define kAudioDevicePropertyDeviceUID 0x75696420
#define kAudioDevicePropertyStreams 0x73746D23
#define kAudioDevicePropertyLatency 0x6C746E63
#define kAudioDevicePropertyPreferredChannelLayout 0x73726E64
#define kAudioDevicePropertyNominalSampleRate 0x6E737274

#define kAudioStreamPropertyLatency kAudioDevicePropertyLatency

#define kAudioDevicePropertyVolumeScalar 0x766F6C6D
#define kAudioObjectPropertyScopeGlobal 0x676C6F62
#define kAudioObjectPropertyElementMaster 0

#define kAudioObjectPropertyName 0x666E616D
#define kAudioObjectSystemObject 1

#define kAudioUnitProperty_StreamFormat 8

#define kAudioUnitType_Output 0x61756F75

#define kAudioUnitSubType_HALOutput 0x6168616C

#define kAudioUnitManufacturer_Apple 0x6170706C

#define kAudioOutputUnitProperty_CurrentDevice 2000
#define kAudioOutputUnitProperty_EnableIO 2003
#define kAudioOutputUnitProperty_SetInputCallback 2005

#define kAudioUnitProperty_SetRenderCallback 23

#define kAudioUnitScope_Global 0
#define kAudioUnitScope_Input 1
#define kAudioUnitScope_Output 2

#define kAudioChannelLabel_Left 1
#define kAudioChannelLabel_Mono 42
#define kAudioChannelLabel_Center 3
#define kAudioChannelLabel_Right 2
#define kAudioChannelLabel_LeftSurround 5
#define kAudioChannelLabel_CenterSurround 9
#define kAudioChannelLabel_RightSurround 6
#define kAudioChannelLabel_LFEScreen 4
#define kAudioChannelLabel_LeftSurroundDirect 10
#define kAudioChannelLabel_RightSurroundDirect 11
#define kAudioChannelLabel_TopCenterSurround 12
#define kAudioChannelLabel_VerticalHeightLeft 13
#define kAudioChannelLabel_VerticalHeightCenter 14
#define kAudioChannelLabel_VerticalHeightRight 15
#define kAudioChannelLabel_TopBackLeft 16
#define kAudioChannelLabel_TopBackCenter 17
#define kAudioChannelLabel_TopBackRight 18
#define kAudioChannelLabel_LeftCenter 7
#define kAudioChannelLabel_RightCenter 8

#define kAudioChannelLayoutTag_UseChannelDescriptions 0

#define kAudioFormatLinearPCM 0x6C70636D
#define kAudioFormatULaw 0x756C6177
#define kAudioFormatALaw 0x616C6177
#define kAudioFormatFlagIsFloat 0x1
#define kAudioFormatFlagIsSignedInteger 0x4

#define kAudioUnitType_MusicDevice 0x61756D75
#define kAudioUnitType_Output 0x61756F75

#define kAudioUnitSubType_DLSSynth 0x646C7320
#define kAudioUnitSubType_DefaultOutput 0x64656620

#define kHALOutputParam_Volume 14

#define kAudioUnitParameterFlag_Output 0x4

struct AudioStreamBasicDescription
{
	Float64 mSampleRate;
	AudioFormatID mFormatID;
	AudioFormatFlags mFormatFlags;
	UInt32 mBytesPerPacket;
	UInt32 mFramesPerPacket;
	UInt32 mBytesPerFrame;
	UInt32 mChannelsPerFrame;
	UInt32 mBitsPerChannel;
	UInt32 mReserved;
};

typedef struct AudioStreamBasicDescription AudioStreamBasicDescription;

struct AudioObjectPropertyAddress
{
	AudioObjectPropertySelector mSelector;
	AudioObjectPropertyScope mScope;
	AudioObjectPropertyElement mElement;
};

typedef struct AudioObjectPropertyAddress AudioObjectPropertyAddress;

struct AudioBuffer
{
	UInt32 mNumberChannels;
	UInt32 mDataByteSize;
	void* mData;
};

typedef struct AudioBuffer AudioBuffer;

struct AudioBufferList {
	UInt32 mNumberBuffers;
	AudioBuffer mBuffers[1];
};

typedef struct AudioBufferList AudioBufferList;

struct SMPTETime
{
	SInt16 mSubframes;
	SInt16 mSubframeDivisor;
	UInt32 mCounter;
	SMPTETimeType mType;
	SMPTETimeFlags mFlags;
	SInt16 mHours;
	SInt16 mMinutes;
	SInt16 mSeconds;
	SInt16 mFrames;
};

typedef struct SMPTETime SMPTETime;

struct AudioTimeStamp
{
	Float64 mSampleTime;
	UInt64 mHostTime;
	Float64 mRateScalar;
	UInt64 mWordClockTime;
	SMPTETime mSMPTETime;
	AudioTimeStampFlags mFlags;
	UInt32 mReserved;
};

typedef struct AudioTimeStamp AudioTimeStamp;

struct AudioComponentDescription {
	OSType componentType;
	OSType componentSubType;
	OSType componentManufacturer;
	UInt32 componentFlags;
	UInt32 componentFlagsMask;
};

typedef struct AudioComponentDescription AudioComponentDescription;

struct AudioChannelDescription
{
	AudioChannelLabel mChannelLabel;
	AudioChannelFlags mChannelFlags;
	Float32 mCoordinates[3];
};

typedef struct AudioChannelDescription AudioChannelDescription;

struct AudioChannelLayout
{
	AudioChannelLayoutTag mChannelLayoutTag;
	AudioChannelBitmap mChannelBitmap;
	UInt32 mNumberChannelDescriptions;
	AudioChannelDescription mChannelDescriptions[1];
};

typedef struct AudioChannelLayout AudioChannelLayout;

struct  AudioStreamPacketDescription
{
	SInt64  mStartOffset;
	UInt32  mVariableFramesInPacket;
	UInt32  mDataByteSize;
};
typedef struct AudioStreamPacketDescription AudioStreamPacketDescription;

typedef OSStatus(*AURenderCallback)(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags, const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList* ioData);
struct AURenderCallbackStruct {
	AURenderCallback inputProc;
	void* inputProcRefCon;
};

typedef struct AURenderCallbackStruct AURenderCallbackStruct;

#define kCFStringEncodingASCII 0x0600
#define kCFStringEncodingUTF8 0x08000100

typedef int32_t CFIndex;
typedef unsigned char Boolean;
typedef UInt32 CFStringEncoding;
typedef uint16_t UniChar;

typedef struct {
	CFIndex location;
	CFIndex length;
} CFRange;

inline CFRange CFRangeMake(CFIndex loc, CFIndex len) {
	CFRange range;
	range.location = loc;
	range.length = len;
	return range;
}

typedef UInt32* CFAllocatorRef;
#define kCFAllocatorNull NULL
#define kCFAllocatorDefault (void*)1

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

struct CFStringData {
	int refCount;
	CFIndex len;
};

struct CFString {
	const char* str;
	struct CFStringData* data;
};

typedef struct CFString CFString;
typedef CFString CFStringRef;

CFIndex CFStringGetLength(CFStringRef theString);
Boolean CFStringGetCString(CFStringRef theString, char* buffer, CFIndex bufferSize, CFStringEncoding encoding);
void CFStringGetCharacters(CFStringRef theString, CFRange range, UniChar* buffer);
CFIndex CFStringGetBytes(CFStringRef theString, CFRange range, CFStringEncoding encoding, UInt8 lossByte, Boolean isExternalRepresentation, UInt8* buffer, CFIndex maxBufLen, CFIndex* usedBufLen);
CFStringRef CFStringCreateWithCStringNoCopy(CFAllocatorRef alloc, const char* cStr, CFStringEncoding encoding, CFAllocatorRef contentsDeallocator);
void CFRelease(CFStringRef cf);
// wine doesn't pass anything to formatOptions other than NULL
CFStringRef CFStringCreateWithFormat(CFAllocatorRef alloc, void* formatOptions, CFStringRef format, ...);

// technicall should not alloc memory, it should be allowed as a static initialize, but wine don't use that functionality, so treating it like a function is fine
inline CFStringRef CFSTR(const char* s) {
	CFString result;
	result.str = s;
	result.data = NULL;
	return result;
}

OSStatus AudioObjectGetPropertyDataSize(AudioObjectID inObjectID, const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32* outDataSize);
OSStatus AudioObjectGetPropertyData(AudioObjectID inObjectID, const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32* ioDataSize, void* outData);
OSStatus AudioUnitSetProperty(AudioUnit inUnit, AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, const void* inData, UInt32 inDataSize);
OSStatus AudioUnitGetProperty(AudioUnit inUnit, AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, void* outData, UInt32* ioDataSize);
OSStatus AudioUnitRender(AudioUnit inUnit, AudioUnitRenderActionFlags* ioActionFlags, const AudioTimeStamp* inTimeStamp, UInt32 inOutputBusNumber, UInt32 inNumberOfFrames, AudioBufferList* ioData);
OSStatus AudioObjectSetPropertyData(AudioObjectID inObjectID, const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData);
OSStatus AudioUnitInitialize(AudioUnit inUnit);

OSStatus AudioOutputUnitStart(AudioUnit ci);
OSStatus AudioOutputUnitStop(AudioUnit ci);

AudioComponent AudioComponentFindNext(AudioComponent inComponent, const AudioComponentDescription* inDesc);
OSStatus AudioComponentInstanceNew(AudioComponent inComponent, AudioComponentInstance* outInstance);
OSStatus AudioConverterDispose(AudioConverterRef inAudioConverter);
OSStatus AudioConverterNew(const AudioStreamBasicDescription* inSourceFormat, const AudioStreamBasicDescription* inDestinationFormat, AudioConverterRef* outAudioConverter);
typedef OSStatus(*AudioConverterComplexInputDataProc)(AudioConverterRef inAudioConverter, UInt32* ioNumberDataPackets, AudioBufferList* ioData, AudioStreamPacketDescription** outDataPacketDescription, void* inUserData);
OSStatus AudioConverterFillComplexBuffer(AudioConverterRef inAudioConverter, AudioConverterComplexInputDataProc inInputDataProc, void* inInputDataProcUserData, UInt32* ioOutputDataPacketSize, AudioBufferList* outOutputData, AudioStreamPacketDescription* outPacketDescription);
OSStatus AudioComponentInstanceDispose(AudioComponentInstance inInstance);
OSStatus AudioUnitGetParameter(AudioUnit inUnit, AudioUnitParameterID inID, AudioUnitScope inScope, AudioUnitElement inElement, AudioUnitParameterValue* outValue);
OSStatus AudioUnitSetParameter(AudioUnit inUnit, AudioUnitParameterID inID, AudioUnitScope inScope, AudioUnitElement inElement, AudioUnitParameterValue inValue, UInt32 inBufferOffsetInFrames);

struct ComponentDescription {
	OSType componentType;
	OSType componentSubType;
	OSType componentManufacturer;
	UInt32 componentFlags;
	UInt32 componentFlagsMask;
};

OSStatus AUGraphInitialize(AUGraph inGraph);
OSStatus AUGraphStart(AUGraph inGraph);
OSStatus AUGraphStop(AUGraph inGraph);
OSStatus DisposeAUGraph(AUGraph inGraph);
OSStatus NewAUGraph(AUGraph* outGraph);
OSStatus AUGraphAddNode(AUGraph inGraph, const AudioComponentDescription* inDescription, AUNode* outNode);
OSStatus AUGraphOpen(AUGraph inGraph);
OSStatus AUGraphConnectNodeInput(AUGraph inGraph, AUNode inSourceNode, UInt32 inSourceOutputNumber, AUNode inDestNode, UInt32 inDestInputNumber);
OSStatus AUGraphNodeInfo(AUGraph inGraph, AUNode inNode, AudioComponentDescription* outDescription, AudioUnit* outAudioUnit);
#include <pthread.h>

struct OSSpinLock {
	pthread_mutex_t mutex;
	int initialized;
};
typedef struct OSSpinLock OSSpinLock;

inline void OSSpinLockLock(OSSpinLock* p) {
	if (!p->initialized) {
		pthread_mutex_init(&p->mutex, NULL);
		p->initialized = 1;
	} 
	pthread_mutex_lock(&p->mutex);
}

inline void OSSpinLockUnlock(OSSpinLock* p) {
	pthread_mutex_unlock(&p->mutex);
}

extern const CFStringRef kMIDIPropertyName;

struct MIDIPacket
{
	MIDITimeStamp timeStamp;
	UInt16 length;
	Byte data[256];
};

typedef struct MIDIPacket MIDIPacket;

struct MIDIPacketList
{
	UInt32 numPackets;
	MIDIPacket packet[1];
};

typedef struct MIDIPacketList MIDIPacketList;

inline MIDIPacket* MIDIPacketNext(const MIDIPacket* pkt) {
	return (MIDIPacket*)&pkt->data[pkt->length];
}
MIDIPacket* MIDIPacketListInit(MIDIPacketList* pktlist);
MIDIPacket* MIDIPacketListAdd(MIDIPacketList* pktlist, ByteCount listSize, MIDIPacket* curPacket, MIDITimeStamp time, ByteCount nData, const Byte* data);
struct MIDINotification
{
	MIDINotificationMessageID messageID;
	UInt32 messageSize;
};

typedef struct MIDINotification MIDINotification;

typedef void (*MIDINotifyProc)(const MIDINotification* message, void* refCon);
OSStatus MIDIClientCreate(CFStringRef name, MIDINotifyProc notifyProc, void* notifyRefCon, MIDIClientRef* outClient);
OSStatus MIDIOutputPortCreate(MIDIClientRef client, CFStringRef portName, MIDIPortRef* outPort);
OSStatus MIDIObjectGetStringProperty(MIDIObjectRef obj, CFStringRef propertyID, CFStringRef* str);
OSStatus MIDIPortConnectSource(MIDIPortRef port, MIDIEndpointRef source, void* connRefCon);
MIDIEndpointRef MIDIGetDestination(ItemCount destIndex0);
ItemCount MIDIGetNumberOfDestinations(void);
ItemCount MIDIGetNumberOfSources(void);
typedef void (*MIDIReadProc)(const MIDIPacketList* pktlist, void* readProcRefCon, void* srcConnRefCon);
OSStatus MIDIInputPortCreate(MIDIClientRef client, CFStringRef portName, MIDIReadProc readProc, void* refCon, MIDIPortRef* outPort);
MIDIEndpointRef MIDIGetSource(ItemCount sourceIndex0);
OSStatus MIDIClientDispose(MIDIClientRef client);
OSStatus MIDISend(MIDIPortRef port, MIDIEndpointRef dest, const MIDIPacketList* pktlist);
OSStatus MusicDeviceMIDIEvent(MusicDeviceComponent inUnit, UInt32 inStatus, UInt32 inData1, UInt32 inData2, UInt32 inOffsetSampleFrame);
OSStatus MusicDeviceSysEx(MusicDeviceComponent inUnit, const UInt8* inData, UInt32 inLength);

#endif