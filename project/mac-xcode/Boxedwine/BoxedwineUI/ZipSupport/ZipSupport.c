// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
#include "include/CBoxedwineZIP.h"
#include "../../../../../lib/zlib/contrib/minizip/unzip.h"
#include <stdlib.h>
#include <string.h>

struct BWZip { unzFile file; int entry_open; };

BWZip *bwzip_open(const char *path, uint64_t *count) {
    unzFile file = unzOpen64(path);
    if (!file) return NULL;
    unz_global_info64 global = {0};
    if (unzGetGlobalInfo64(file, &global) != UNZ_OK) { unzClose(file); return NULL; }
    BWZip *zip = calloc(1, sizeof(BWZip));
    if (!zip) { unzClose(file); return NULL; }
    zip->file = file;
    *count = global.number_entry;
    return zip;
}
void bwzip_close(BWZip *zip) {
    if (!zip) return;
    if (zip->entry_open) unzCloseCurrentFile(zip->file);
    unzClose(zip->file);
    free(zip);
}
int bwzip_info(BWZip *zip, BWZipEntry *info, char *name, size_t capacity) {
    unz_file_info64 entry = {0};
    if (unzGetCurrentFileInfo64(zip->file, &entry, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK ||
        !entry.size_filename || entry.size_filename >= capacity) return -1;
    memset(name, 0, capacity);
    if (unzGetCurrentFileInfo64(zip->file, &entry, name, (uLong)capacity, NULL, 0, NULL, 0) != UNZ_OK ||
        memchr(name, 0, entry.size_filename)) return -1;
    info->size = entry.uncompressed_size;
    info->compressed_size = entry.compressed_size;
    info->flags = (uint32_t)entry.flag;
    info->method = (uint32_t)entry.compression_method;
    info->attributes = (uint32_t)entry.external_fa;
    info->disk = (uint32_t)entry.disk_num_start;
    return 0;
}
int bwzip_begin(BWZip *zip) {
    int result = unzOpenCurrentFile(zip->file);
    if (result == UNZ_OK) zip->entry_open = 1;
    return result;
}
int bwzip_read(BWZip *zip, void *buffer, unsigned int capacity) { return unzReadCurrentFile(zip->file, buffer, capacity); }
int bwzip_end(BWZip *zip) {
    int result = unzCloseCurrentFile(zip->file);
    zip->entry_open = 0;
    return result;
}
int bwzip_next(BWZip *zip) { return unzGoToNextFile(zip->file); }
