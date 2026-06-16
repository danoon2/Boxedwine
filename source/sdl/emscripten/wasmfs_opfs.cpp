/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#if defined(__EMSCRIPTEN__) && defined(BOXEDWINE_WASMFS_OPFS)

#include <errno.h>
#include <sys/stat.h>

#include <emscripten/wasmfs.h>

static void ensureDirectory(const char* path) {
    if (mkdir(path, 0777) != 0 && errno != EEXIST) {
        // WasmFS startup happens before BoxedWine logging is initialized.
        // Let the later open/mount operation report the concrete failure.
    }
}

bool boxedwineSetupWasmFsOpfs() {
    backend_t opfs = wasmfs_create_opfs_backend();
    int result = wasmfs_create_directory("/root", 0777, opfs);
    if (result != 0 && result != -EEXIST) {
        return false;
    }

    // The OPFS backend maps a mount point to the OPFS root and currently
    // ignores mount options. Keep the D: drive as a normal subdirectory under
    // the persistent root so /root and D: do not alias the same OPFS directory.
    ensureDirectory("/root/.boxedwine-d-drive");
    return true;
}

#endif
