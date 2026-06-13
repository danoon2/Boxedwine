#include "javaJarInfo.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
#include <vector>

#undef OF
#define STRICTUNZIP
extern "C"
{
#include "../../../lib/zlib/contrib/minizip/unzip.h"
}
#undef OF

static const int MAX_JAR_FILEPATH_LEN = 4096;

static std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return (char)std::tolower(c);
        });
    return value;
}

static bool endsWith(const std::string& value, const char* suffix) {
    std::string suffixString(suffix);
    return value.size() >= suffixString.size() && value.compare(value.size() - suffixString.size(), suffixString.size(), suffixString) == 0;
}

static bool isMultiReleaseVersionedClass(const std::string& lowerName) {
    return lowerName.rfind("meta-inf/versions/", 0) == 0;
}

static std::string trimManifestValue(std::string value) {
    size_t start = 0;
    while (start < value.size() && std::isspace((unsigned char)value[start])) {
        start++;
    }
    size_t end = value.size();
    while (end > start && std::isspace((unsigned char)value[end - 1])) {
        end--;
    }
    return value.substr(start, end - start);
}

static bool manifestHasMainClass(const std::string& manifest) {
    std::vector<std::string> lines;
    size_t lineStart = 0;
    while (lineStart <= manifest.size()) {
        size_t lineEnd = manifest.find_first_of("\r\n", lineStart);
        if (lineEnd == std::string::npos) {
            lineEnd = manifest.size();
        }
        lines.push_back(manifest.substr(lineStart, lineEnd - lineStart));
        if (lineEnd == manifest.size()) {
            break;
        }
        lineStart = lineEnd + 1;
        if (lineStart < manifest.size() && manifest[lineEnd] == '\r' && manifest[lineStart] == '\n') {
            lineStart++;
        }
    }

    for (size_t i = 0; i < lines.size(); i++) {
        std::string line = lines[i];
        if (toLower(line).rfind("main-class:", 0) != 0) {
            continue;
        }
        std::string value = line.substr(11);
        while (i + 1 < lines.size() && !lines[i + 1].empty() && lines[i + 1][0] == ' ') {
            value += lines[i + 1].substr(1);
            i++;
        }
        return !trimManifestValue(value).empty();
    }
    return false;
}

int javaFeatureVersionFromClassMajor(int major) {
    if (major < 45) {
        return 0;
    }
    if (major == 45) {
        return 1;
    }
    return major - 44;
}

bool isJavaJarLaunchable(const char* jarPath) {
    if (!jarPath || !jarPath[0]) {
        return false;
    }

    unzFile z = unzOpen(jarPath);
    if (!z) {
        return false;
    }

    unz_global_info globalInfo = {};
    if (unzGetGlobalInfo(z, &globalInfo) != UNZ_OK) {
        unzClose(z);
        return false;
    }

    bool result = false;
    for (uint32_t i = 0; i < globalInfo.number_entry; ++i) {
        unz_file_info fileInfo = {};
        char fileName[MAX_JAR_FILEPATH_LEN] = {};

        if (unzGetCurrentFileInfo(z, &fileInfo, fileName, MAX_JAR_FILEPATH_LEN, nullptr, 0, nullptr, 0) != UNZ_OK) {
            break;
        }

        if (toLower(fileName) == "meta-inf/manifest.mf" && unzOpenCurrentFile(z) == UNZ_OK) {
            std::string manifest;
            char buffer[1024];
            int bytesRead = unzReadCurrentFile(z, buffer, sizeof(buffer));
            while (bytesRead > 0) {
                manifest.append(buffer, bytesRead);
                bytesRead = unzReadCurrentFile(z, buffer, sizeof(buffer));
            }
            unzCloseCurrentFile(z);
            result = manifestHasMainClass(manifest);
            break;
        }

        if (i + 1 < globalInfo.number_entry) {
            unzGoToNextFile(z);
        }
    }

    unzClose(z);
    return result;
}

int getRequiredJavaVersionFromJar(const char* jarPath) {
    if (!jarPath || !jarPath[0]) {
        return 0;
    }

    unzFile z = unzOpen(jarPath);
    if (!z) {
        return 0;
    }

    unz_global_info globalInfo = {};
    if (unzGetGlobalInfo(z, &globalInfo) != UNZ_OK) {
        unzClose(z);
        return 0;
    }

    int requiredVersion = 0;
    for (uint32_t i = 0; i < globalInfo.number_entry; ++i) {
        unz_file_info fileInfo = {};
        char fileName[MAX_JAR_FILEPATH_LEN] = {};

        if (unzGetCurrentFileInfo(z, &fileInfo, fileName, MAX_JAR_FILEPATH_LEN, nullptr, 0, nullptr, 0) != UNZ_OK) {
            unzClose(z);
            return requiredVersion;
        }

        std::string lowerName = toLower(fileName);
        if (endsWith(lowerName, ".class") && !isMultiReleaseVersionedClass(lowerName)) {
            unsigned char header[8] = {};
            if (unzOpenCurrentFile(z) == UNZ_OK) {
                int bytesRead = unzReadCurrentFile(z, header, sizeof(header));
                unzCloseCurrentFile(z);
                if (bytesRead == sizeof(header) &&
                    header[0] == 0xca && header[1] == 0xfe && header[2] == 0xba && header[3] == 0xbe) {
                    int major = (header[6] << 8) | header[7];
                    requiredVersion = std::max(requiredVersion, javaFeatureVersionFromClassMajor(major));
                }
            }
        }

        if (i + 1 < globalInfo.number_entry) {
            unzGoToNextFile(z);
        }
    }

    unzClose(z);
    return requiredVersion;
}
