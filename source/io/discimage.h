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

#ifndef __DISCIMAGE_H__
#define __DISCIMAGE_H__

#include "platformBoxedwine.h"

#define DISC_DATA_SECTOR_SIZE 2048 // user data in a data sector
#define DISC_RAW_SECTOR_SIZE  2352 // a whole sector, also one frame of CD audio

// TOC control bits
#define DISC_CONTROL_PREEMPHASIS 0x1
#define DISC_CONTROL_COPY        0x2
#define DISC_CONTROL_DATA        0x4
#define DISC_CONTROL_4CHANNEL    0x8

// A track as the table of contents describes it
struct DiscTrack {
    U32 number = 0;
    bool audio = false;
    U32 mode = 1;    // 1 or 2 for data tracks
    U32 control = 0; // DISC_CONTROL_* bits
    U32 start = 0;   // first sector, the track's INDEX 01
};

// A disc image as a sequence of sectors: a plain ISO image, or a CUE sheet with the
// BIN (or WAVE) files it lists. Sector 0 is the start of track 1.
class DiscImage {
public:
    ~DiscImage();

    // .cue files are read as CUE sheets. A .bin with a CUE sheet of the same name beside
    // it is read through that sheet. Anything else is one data track: an ISO image, or a
    // raw image of 2352 or 2336 byte sectors, as a BIN without its CUE sheet is.
    static std::shared_ptr<DiscImage> open(const BString& path);

    // Reads the user data of data sectors, as a CD-ROM drive does: byteOffset counts
    // DISC_DATA_SECTOR_SIZE bytes per sector from sector 0. Audio sectors and gaps
    // read as zeros. Callers serialize reads.
    U32 readData(U64 byteOffset, U8* buffer, U32 len);
    // Reads whole DISC_RAW_SECTOR_SIZE sectors, as CD audio is read: 16-bit stereo
    // little-endian samples for audio tracks. Sectors the image does not store raw
    // (2048 or 2336 byte data sectors, gaps) read as zeros. Callers serialize reads.
    bool readRaw(U32 sector, U32 count, U8* buffer);

    const DiscTrack* findTrack(U32 number) const;
    const DiscTrack* firstDataTrack() const;

    std::vector<DiscTrack> tracks;
    U32 leadOut = 0; // the sector after the last track

private:
    // consecutive sectors that are stored the same way
    struct Extent {
        U32 start;
        U32 count;
        S32 file;       // index into files, -1 for a gap that is in no file
        U64 fileOffset; // of the extent's first sector
        U32 sectorSize; // bytes per sector in the file
        U32 dataOffset; // of the user data within a sector
        bool audio;
        bool bigEndian; // audio samples stored most significant byte first
    };

    std::vector<FILE*> files;
    std::vector<Extent> extents;

    bool openDataTrack(const BString& path);
    bool openCue(const BString& path);
    const Extent* findExtent(U64 sector) const;
    U32 readFile(S32 file, U64 offset, U8* buffer, U32 len);
};

#endif
