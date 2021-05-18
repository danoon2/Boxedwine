#ifndef __KNATIVESYSTEM_H__
#define __KNATIVESYSTEM_H__

#define SCALE_DENOMINATOR 1000

class KNativeSystem {
public:
	static bool init(bool allowVideo, bool allowAudio);
	static void exit(const char* msg, U32 code);
	static void cleanup();
	static void preReturnToUI();

	static void postQuit();
	static U32 getTicks();
	static bool getScreenDimensions(U32* width, U32* height);
	static std::string getAppDirectory();
	static std::string getLocalDirectory(); // ends with path separator
	static bool clipboardHasText();
	static std::string clipboardGetText();
	static bool clipboardSetText(const std::string& text);
	static U32 getDpiScale(); // returns 1/1000th of the scale, so a result of 1000 would mean no scaling
};

#endif