#include "javaJarInfo.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>

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

int javaFeatureVersionFromClassMajor(int major) {
    if (major < 45) {
        return 0;
    }
    if (major == 45) {
        return 1;
    }
    return major - 44;
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
