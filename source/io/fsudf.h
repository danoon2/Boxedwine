/* Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef __FSUDF_H__
#define __FSUDF_H__

#include "platformBoxedwine.h"
#include "../../lib/libudfread/src/blockinput.h"

class FsIso;
class DiscImage;
struct udfread;
struct udfread_dir;

// Read-only UDF on the same sector source and lifetime as an ISO/CUE disc.
class FsUdf {
public:
    explicit FsUdf(FsIso& disc);
    ~FsUdf();
    static bool probe(DiscImage& image, U32 start);
    bool mount(const BString& path, const std::shared_ptr<FsNode>& root, U32 sessionStart);

private:
    struct Input : udfread_block_input {
        FsIso* disc = nullptr;
        U32 remainingReads = 0;
    } input;
    udfread* reader = nullptr;
    FsIso& disc;
    U32 entries = 0;

    bool scanDirectory(udfread_dir* directory, const BString& path,
        const BString& udfPath, const std::shared_ptr<FsNode>& parent, U32 depth);
};

#endif
