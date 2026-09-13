// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef CBOXEDWINEZIP_H
#define CBOXEDWINEZIP_H
#include <stdint.h>
#include <stddef.h>

typedef struct BWZip BWZip;
typedef struct {
    uint64_t size, compressed_size;
    uint32_t flags, method, attributes, disk;
} BWZipEntry;

BWZip *bwzip_open(const char *path, uint64_t *count);
void bwzip_close(BWZip *zip);
int bwzip_info(BWZip *zip, BWZipEntry *info, char *name, size_t capacity);
int bwzip_begin(BWZip *zip);
int bwzip_read(BWZip *zip, void *buffer, unsigned int capacity);
int bwzip_end(BWZip *zip); // Includes the entry CRC check.
int bwzip_next(BWZip *zip);
#endif
