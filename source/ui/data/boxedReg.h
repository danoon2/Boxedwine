#ifndef __BOXEDREG_H__
#define __BOXEDREG_H__

class BoxedReg {
public:
	BoxedReg(BoxedContainer* container, bool system);

	bool readKey(const char* path, const char* key, BString& value);
	void writeKey(const char* path, const char* key, const char* value, bool useQuotesAroundValue=true);
	void writeKeyDword(const char* path, const char* key, U32 value);
	void save();
private:
	std::vector<BString> lines;
	BString filePath;
};

#endif