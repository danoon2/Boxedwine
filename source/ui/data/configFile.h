#ifndef __CONFIG_FILE_H__
#define __CONFIG_FILE_H__

class ConfigFile {
public:
    ConfigFile(const std::string& fileName);

    std::string readString(const std::string& name, const std::string& defaultValue);
    bool readBool(const std::string& name, bool defaultValue);
    int readInt(const std::string& name, int defaultValue);

    void writeString(const std::string& name, const std::string& value);
    void writeBool(const std::string& name, bool value);
    void writeInt(const std::string& name, int value);

    bool saveChanges();

private:
    std::unordered_map<std::string,std::string> values;
    std::string fileName;
};
#endif