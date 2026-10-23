/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"
#include "discimage.h"
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>

#define FRAMES_PER_SECOND 75

DiscImage::~DiscImage() {
    for (FILE* file : files) {
        fclose(file);
    }
}

// A BIN's CUE sheet usually has the same name. It says where any audio tracks are,
// which the BIN itself does not.
static BString cueBeside(const BString& path) {
    if (!path.endsWith(".bin", true)) {
        return BString::empty;
    }
    BString base = path.substr(0, path.length() - 4);
    for (const char* extension : { ".cue", ".CUE" }) {
        BString cue = base + extension;
        FILE* file = fopen(cue.c_str(), "rb");
        if (file) {
            fclose(file);
            return cue;
        }
    }
    return BString::empty;
}

std::shared_ptr<DiscImage> DiscImage::open(const BString& path) {
    std::shared_ptr<DiscImage> image = std::make_shared<DiscImage>();
    BString cue = path.endsWith(".cue", true) ? path : cueBeside(path);
    bool opened;
    if (cue.length()) {
        if (cue != path) {
            klog_fmt("DiscImage: reading %s through %s", path.c_str(), cue.c_str());
        }
        opened = image->openCue(cue);
    } else {
        opened = image->openDataTrack(path);
    }
    if (!opened || image->tracks.empty()) {
        return nullptr;
    }
    return image;
}

const DiscTrack* DiscImage::findTrack(U32 number) const {
    for (const DiscTrack& track : tracks) {
        if (track.number == number) {
            return &track;
        }
    }
    return nullptr;
}

const DiscTrack* DiscImage::firstDataTrack() const {
    for (const DiscTrack& track : tracks) {
        if (!track.audio) {
            return &track;
        }
    }
    return nullptr;
}

static bool seekFile(FILE* file, U64 offset) {
#ifdef BOXEDWINE_MSVC
    return _fseeki64(file, (S64)offset, SEEK_SET) == 0;
#else
    return fseeko(file, (off_t)offset, SEEK_SET) == 0;
#endif
}

static U64 getFileSize(FILE* file) {
#ifdef BOXEDWINE_MSVC
    _fseeki64(file, 0, SEEK_END);
    return (U64)_ftelli64(file);
#else
    fseeko(file, 0, SEEK_END);
    return (U64)ftello(file);
#endif
}

namespace {

// how an image of one data track can store its sectors
struct DataLayout {
    U32 sectorSize;
    U32 dataOffset; // of the 2048 bytes of user data
    U32 mode;
};

const DataLayout dataLayouts[] = {
    { DISC_DATA_SECTOR_SIZE, 0, 1 }, // an ISO image
    { DISC_RAW_SECTOR_SIZE, 16, 1 }, // MODE1/2352: after the sync pattern and header
    { DISC_RAW_SECTOR_SIZE, 24, 2 }, // MODE2/2352: after the sync pattern, header and form 1 subheader
    { 2336, 8, 2 },                  // MODE2/2336: after the form 1 subheader
};

}

bool DiscImage::openDataTrack(const BString& path) {
    FILE* file = fopen(path.c_str(), "rb");
    if (!file) {
        klog_fmt("DiscImage: could not open %s", path.c_str());
        return false;
    }
    files.push_back(file);

    // The layout is the one that finds ISO 9660's primary volume descriptor in sector 16.
    // If none does, FsIso reports that the image is not ISO 9660.
    static const U8 primaryVolumeDescriptor[6] = { 1, 'C', 'D', '0', '0', '1' };
    DataLayout layout = dataLayouts[0];
    for (const DataLayout& candidate : dataLayouts) {
        U8 descriptor[sizeof(primaryVolumeDescriptor)];
        if (seekFile(file, 16ull * candidate.sectorSize + candidate.dataOffset)
                && fread(descriptor, 1, sizeof(descriptor), file) == sizeof(descriptor)
                && !memcmp(descriptor, primaryVolumeDescriptor, sizeof(descriptor))) {
            layout = candidate;
            break;
        }
    }
    if (layout.sectorSize != DISC_DATA_SECTOR_SIZE) {
        klog_fmt("DiscImage: %s is a raw image of %d byte mode %d sectors", path.c_str(), layout.sectorSize, layout.mode);
    }
    U32 sectors = (U32)(getFileSize(file) / layout.sectorSize);
    extents.push_back({ 0, sectors, 0, 0, layout.sectorSize, layout.dataOffset, false, false });

    DiscTrack track;
    track.number = 1;
    track.mode = layout.mode;
    track.control = DISC_CONTROL_DATA;
    tracks.push_back(track);
    leadOut = sectors;
    return true;
}

namespace {

struct CueTrack {
    DiscTrack toc;
    U32 sectorSize = DISC_RAW_SECTOR_SIZE;
    U32 dataOffset = 0;
    S32 index0 = -1; // frames from the start of the file
    S32 index1 = -1;
    U32 pregap = 0;  // PREGAP and POSTGAP sectors are in no file
    U32 postgap = 0;
};

struct CueFile {
    S32 file = -1;
    U64 dataStart = 0; // the sectors start after a WAVE file's header
    U64 dataSize = 0;
    bool bigEndian = false; // MOTOROLA
    std::vector<CueTrack> tracks;
};

std::string upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)toupper(c); });
    return s;
}

// splits a CUE sheet line into words, keeping a quoted file name together
std::vector<std::string> cueWords(const std::string& line) {
    std::vector<std::string> words;
    size_t pos = 0;
    while (pos < line.size()) {
        while (pos < line.size() && isspace((unsigned char)line[pos])) {
            pos++;
        }
        if (pos >= line.size()) {
            break;
        }
        if (line[pos] == '"') {
            size_t end = line.find('"', pos + 1);
            if (end == std::string::npos) {
                end = line.size();
            }
            words.push_back(line.substr(pos + 1, end - pos - 1));
            pos = end + 1;
        } else {
            size_t end = pos;
            while (end < line.size() && !isspace((unsigned char)line[end])) {
                end++;
            }
            words.push_back(line.substr(pos, end - pos));
            pos = end;
        }
    }
    return words;
}

// mm:ss:ff to frames (sectors)
bool parseMsf(const std::string& msf, U32& frames) {
    unsigned minutes, seconds, frame;
    if (sscanf(msf.c_str(), "%u:%u:%u", &minutes, &seconds, &frame) != 3 || seconds >= 60 || frame >= FRAMES_PER_SECOND) {
        return false;
    }
    frames = (minutes * 60 + seconds) * FRAMES_PER_SECOND + frame;
    return true;
}

bool setTrackType(CueTrack& track, const std::string& type) {
    std::string t = upper(type);
    track.toc.audio = false;
    track.toc.mode = 1;
    if (t == "AUDIO") {
        track.toc.audio = true;
        track.toc.mode = 0;
        track.sectorSize = DISC_RAW_SECTOR_SIZE;
    } else if (t == "MODE1/2048") {
        track.sectorSize = 2048;
    } else if (t == "MODE1/2352") {
        track.sectorSize = 2352;
        track.dataOffset = 16; // after the sync pattern and header
    } else if (t == "MODE2/2048") {
        track.toc.mode = 2;
        track.sectorSize = 2048;
    } else if (t == "MODE2/2336" || t == "CDI/2336") {
        track.toc.mode = 2;
        track.sectorSize = 2336;
        track.dataOffset = 8; // after the form 1 subheader
    } else if (t == "MODE2/2352" || t == "CDI/2352") {
        track.toc.mode = 2;
        track.sectorSize = 2352;
        track.dataOffset = 24; // after the sync pattern, header and form 1 subheader
    } else {
        return false;
    }
    if (!track.toc.audio) {
        track.toc.control |= DISC_CONTROL_DATA;
    }
    return true;
}

U32 readU32LE(const U8* p) {
    return (U32)p[0] | ((U32)p[1] << 8) | ((U32)p[2] << 16) | ((U32)p[3] << 24);
}

// finds the samples of a WAVE file, which must be CD audio: 16-bit stereo PCM at 44.1kHz
bool findWaveData(FILE* file, U64& start, U64& size) {
    U8 header[12];
    if (!seekFile(file, 0) || fread(header, 1, 12, file) != 12 || memcmp(header, "RIFF", 4) || memcmp(header + 8, "WAVE", 4)) {
        return false;
    }
    U64 pos = 12;
    bool cdAudio = false;
    U8 chunk[8];
    while (seekFile(file, pos) && fread(chunk, 1, 8, file) == 8) {
        U32 chunkSize = readU32LE(chunk + 4);
        if (!memcmp(chunk, "fmt ", 4)) {
            U8 format[16];
            if (chunkSize < 16 || fread(format, 1, 16, file) != 16) {
                return false;
            }
            U32 tag = format[0] | (format[1] << 8);
            U32 channels = format[2] | (format[3] << 8);
            U32 bits = format[14] | (format[15] << 8);
            cdAudio = tag == 1 && channels == 2 && readU32LE(format + 4) == 44100 && bits == 16;
        } else if (!memcmp(chunk, "data", 4)) {
            start = pos + 8;
            size = chunkSize;
            return cdAudio;
        }
        pos += 8 + (U64)chunkSize + (chunkSize & 1);
    }
    return false;
}

}

bool DiscImage::openCue(const BString& path) {
    FILE* cue = fopen(path.c_str(), "rb");
    if (!cue) {
        klog_fmt("DiscImage: could not open %s", path.c_str());
        return false;
    }
    std::string text;
    char buffer[4096];
    size_t len;
    while ((len = fread(buffer, 1, sizeof(buffer), cue)) > 0) {
        text.append(buffer, len);
    }
    fclose(cue);

    // the files a CUE sheet lists are relative to it
    std::string dir = path.c_str();
    size_t slash = dir.find_last_of("/\\");
    dir = (slash == std::string::npos) ? "" : dir.substr(0, slash + 1);

    std::vector<CueFile> cueFiles;
    CueTrack* track = nullptr;
    size_t pos = 0;
    while (pos < text.size()) {
        size_t end = text.find('\n', pos);
        if (end == std::string::npos) {
            end = text.size();
        }
        std::vector<std::string> words = cueWords(text.substr(pos, end - pos));
        pos = end + 1;
        if (words.empty()) {
            continue;
        }
        std::string command = upper(words[0]);
        if (command == "FILE" && words.size() >= 2) {
            std::string name = words[1];
            std::string type = words.size() >= 3 ? upper(words[2]) : "BINARY";
            if (name.empty()) {
                klog_fmt("DiscImage: %s has a FILE without a name", path.c_str());
                return false;
            }
            if (!(name[0] == '/' || name[0] == '\\' || (name.size() > 1 && name[1] == ':'))) {
                std::replace(name.begin(), name.end(), '\\', '/');
                name = dir + name;
            }
            FILE* file = fopen(name.c_str(), "rb");
            if (!file) {
                klog_fmt("DiscImage: could not open %s, which %s lists", name.c_str(), path.c_str());
                return false;
            }
            CueFile cueFile;
            if (type == "BINARY" || type == "MOTOROLA") {
                cueFile.dataSize = getFileSize(file);
                cueFile.bigEndian = type == "MOTOROLA";
            } else if (type == "WAVE") {
                if (!findWaveData(file, cueFile.dataStart, cueFile.dataSize)) {
                    klog_fmt("DiscImage: %s is not 16-bit stereo 44.1kHz PCM, as CD audio must be", name.c_str());
                    fclose(file);
                    return false;
                }
            } else {
                klog_fmt("DiscImage: %s files such as %s are not supported yet", type.c_str(), name.c_str());
                fclose(file);
                return false;
            }
            files.push_back(file);
            cueFile.file = (S32)files.size() - 1;
            cueFiles.push_back(cueFile);
            track = nullptr;
        } else if (command == "TRACK" && words.size() >= 3) {
            if (cueFiles.empty()) {
                klog_fmt("DiscImage: %s has a TRACK before any FILE", path.c_str());
                return false;
            }
            CueTrack newTrack;
            newTrack.toc.number = (U32)atoi(words[1].c_str());
            if (!setTrackType(newTrack, words[2])) {
                klog_fmt("DiscImage: %s tracks in %s are not supported", words[2].c_str(), path.c_str());
                return false;
            }
            cueFiles.back().tracks.push_back(newTrack);
            track = &cueFiles.back().tracks.back();
        } else if ((command == "INDEX" && words.size() >= 3) || ((command == "PREGAP" || command == "POSTGAP") && words.size() >= 2)) {
            U32 frames;
            if (!track || !parseMsf(words[command == "INDEX" ? 2 : 1], frames)) {
                klog_fmt("DiscImage: %s has a %s it cannot use", path.c_str(), command.c_str());
                return false;
            }
            if (command == "PREGAP") {
                track->pregap = frames;
            } else if (command == "POSTGAP") {
                track->postgap = frames;
            } else if (atoi(words[1].c_str()) == 0) {
                track->index0 = (S32)frames;
            } else if (atoi(words[1].c_str()) == 1) {
                track->index1 = (S32)frames;
            }
        } else if (command == "FLAGS" && track) {
            for (size_t i = 1; i < words.size(); i++) {
                std::string flag = upper(words[i]);
                if (flag == "DCP") {
                    track->toc.control |= DISC_CONTROL_COPY;
                } else if (flag == "4CH") {
                    track->toc.control |= DISC_CONTROL_4CHANNEL;
                } else if (flag == "PRE") {
                    track->toc.control |= DISC_CONTROL_PREEMPHASIS;
                }
            }
        }
        // REM, CATALOG, CDTEXTFILE, TITLE, PERFORMER, SONGWRITER and ISRC are not needed
    }

    // Lay the files out one after the other. A track's sectors run from its INDEX 00
    // (or INDEX 01 if it has no pregap in the file) to the next track's, or the end of
    // its file; PREGAP and POSTGAP add sectors that are in no file.
    U32 cursor = 0;
    for (CueFile& cueFile : cueFiles) {
        U64 byte = cueFile.dataStart;
        U64 end = cueFile.dataStart + cueFile.dataSize;
        for (size_t i = 0; i < cueFile.tracks.size(); i++) {
            CueTrack& t = cueFile.tracks[i];
            if (t.index1 < 0) {
                klog_fmt("DiscImage: track %d in %s has no INDEX 01", t.toc.number, path.c_str());
                return false;
            }
            U32 from = (i == 0) ? 0 : (U32)(t.index0 >= 0 ? t.index0 : t.index1);
            U32 count;
            if (i + 1 < cueFile.tracks.size()) {
                const CueTrack& next = cueFile.tracks[i + 1];
                S32 to = next.index0 >= 0 ? next.index0 : next.index1;
                if (to < (S32)from) {
                    klog_fmt("DiscImage: the INDEX times in %s go backwards at track %d", path.c_str(), next.toc.number);
                    return false;
                }
                count = (U32)to - from;
            } else {
                count = byte < end ? (U32)((end - byte) / t.sectorSize) : 0;
            }
            if ((U32)t.index1 < from || (U32)t.index1 - from > count) {
                klog_fmt("DiscImage: track %d in %s starts outside its file", t.toc.number, path.c_str());
                return false;
            }
            if (t.pregap) {
                extents.push_back({ cursor, t.pregap, -1, 0, t.sectorSize, 0, t.toc.audio, false });
                cursor += t.pregap;
            }
            extents.push_back({ cursor, count, cueFile.file, byte, t.sectorSize, t.dataOffset, t.toc.audio, cueFile.bigEndian });
            t.toc.start = cursor + ((U32)t.index1 - from);
            tracks.push_back(t.toc);
            cursor += count;
            byte += (U64)count * t.sectorSize;
            if (t.postgap) {
                extents.push_back({ cursor, t.postgap, -1, 0, t.sectorSize, 0, t.toc.audio, false });
                cursor += t.postgap;
            }
        }
    }
    leadOut = cursor;
    if (tracks.empty()) {
        klog_fmt("DiscImage: %s has no tracks", path.c_str());
        return false;
    }

    // Track 1 starts at sector 0 on a disc, so drop a pregap that is kept before it
    U32 skip = tracks[0].start;
    if (skip) {
        std::vector<Extent> kept;
        for (Extent extent : extents) {
            if (extent.start + extent.count <= skip) {
                continue;
            }
            if (extent.start < skip) {
                U32 cut = skip - extent.start;
                extent.count -= cut;
                extent.fileOffset += (U64)cut * extent.sectorSize;
                extent.start = skip;
            }
            extent.start -= skip;
            kept.push_back(extent);
        }
        extents = kept;
        for (DiscTrack& t : tracks) {
            t.start -= skip;
        }
        leadOut -= skip;
    }
    klog_fmt("DiscImage: %s has %d track(s) and %d sectors", path.c_str(), (U32)tracks.size(), leadOut);
    return true;
}

const DiscImage::Extent* DiscImage::findExtent(U64 sector) const {
    auto next = std::upper_bound(extents.begin(), extents.end(), sector, [](U64 s, const Extent& e) { return s < e.start; });
    if (next == extents.begin()) {
        return nullptr;
    }
    const Extent& extent = *(next - 1);
    return sector < (U64)extent.start + extent.count ? &extent : nullptr;
}

U32 DiscImage::readFile(S32 file, U64 offset, U8* buffer, U32 len) {
    if (file < 0 || !seekFile(files[file], offset)) {
        return 0;
    }
    return (U32)fread(buffer, 1, len, files[file]);
}

U32 DiscImage::readData(U64 byteOffset, U8* buffer, U32 len) {
    U32 done = 0;
    while (done < len) {
        U64 offset = byteOffset + done;
        U64 sector = offset / DISC_DATA_SECTOR_SIZE;
        U32 within = (U32)(offset % DISC_DATA_SECTOR_SIZE);
        const Extent* extent = findExtent(sector);
        if (!extent) {
            break;
        }
        U32 todo = len - done;
        if (extent->file >= 0 && !extent->audio && extent->sectorSize == DISC_DATA_SECTOR_SIZE) {
            // the sectors are stored back to back, so read the rest of the extent at once
            U64 available = ((U64)extent->start + extent->count - sector) * DISC_DATA_SECTOR_SIZE - within;
            if (todo > available) {
                todo = (U32)available;
            }
            U32 read = readFile(extent->file, extent->fileOffset + (sector - extent->start) * DISC_DATA_SECTOR_SIZE + within, buffer + done, todo);
            done += read;
            if (read < todo) {
                break;
            }
            continue;
        }
        todo = std::min(todo, DISC_DATA_SECTOR_SIZE - within);
        if (extent->file < 0 || extent->audio) {
            memset(buffer + done, 0, todo);
        } else {
            U64 at = extent->fileOffset + (sector - extent->start) * extent->sectorSize + extent->dataOffset + within;
            U32 read = readFile(extent->file, at, buffer + done, todo);
            if (read < todo) {
                done += read;
                break;
            }
        }
        done += todo;
    }
    return done;
}

bool DiscImage::readRaw(U32 sector, U32 count, U8* buffer) {
    if ((U64)sector + count > leadOut) {
        return false;
    }
    for (U32 i = 0; i < count; i++, buffer += DISC_RAW_SECTOR_SIZE) {
        const Extent* extent = findExtent((U64)sector + i);
        if (!extent || extent->file < 0 || extent->sectorSize != DISC_RAW_SECTOR_SIZE) {
            memset(buffer, 0, DISC_RAW_SECTOR_SIZE);
            continue;
        }
        U64 at = extent->fileOffset + ((U64)sector + i - extent->start) * DISC_RAW_SECTOR_SIZE;
        U32 read = readFile(extent->file, at, buffer, DISC_RAW_SECTOR_SIZE);
        memset(buffer + read, 0, DISC_RAW_SECTOR_SIZE - read);
        if (extent->bigEndian && extent->audio) {
            for (U32 b = 0; b < DISC_RAW_SECTOR_SIZE; b += 2) {
                std::swap(buffer[b], buffer[b + 1]);
            }
        }
    }
    return true;
}
