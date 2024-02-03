#ifndef __CONFIG_FILE_H__
#define __CONFIG_FILE_H__

class ConfigFile {
public:
    ConfigFile(BString fileName);

    BString readString(BString name, BString defaultValue);
    bool readBool(BString name, bool defaultValue);
    int readInt(BString name, int defaultValue);

    void writeString(BString name, BString value);
    void writeBool(BString name, bool value);
    void writeInt(BString name, int value);

    bool saveChanges();

private:
    BHashTable<BString,BString> values;
    BString fileName;
};
#endif