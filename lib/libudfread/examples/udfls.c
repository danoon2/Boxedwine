/*
 * This file is part of libudfread
 * Copyright (C) 2014-2015 VLC authors and VideoLAN
 *
 * Authors: Petri Hintukainen <phintuka@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#include "udfread.h"

static int _lsdir_at(UDFDIR *dir, const char *path, int depth)
{
    struct udfread_dirent dirent;

    while (udfread_readdir(dir, &dirent)) {
        if (!strcmp(dirent.d_name, ".") || !strcmp(dirent.d_name, "..")) continue;

        if (dirent.d_type == UDF_DT_DIR) {
            UDFDIR *child;
            char *next_dir;

            printf("\t\t %s%s\n", path, dirent.d_name);

            next_dir = (char*)malloc(strlen(path) + strlen(dirent.d_name) + 2);
            if (!next_dir) {
                fprintf(stderr, "out of memory\n");
                continue;
            }
            sprintf(next_dir, "%s%s/",  path, dirent.d_name);

            child = udfread_opendir_at(dir, dirent.d_name);
            if (!child) {
                free(next_dir);
                fprintf(stderr, "error opening directory %s\n", dirent.d_name);
                continue;
            }

            _lsdir_at(child, next_dir, depth + 1);
            udfread_closedir(child);

            free(next_dir);
        } else {
            UDFFILE *fp;

            fp = udfread_file_openat(dir, dirent.d_name);
            if (!fp) {
                fprintf(stderr, "error opening file '%s%s'\n", path, dirent.d_name);
                continue;
            }
            printf("%16" PRId64 " %s%s\n",  udfread_file_size(fp), path, dirent.d_name);
            udfread_file_close(fp);
        }
    }

    return 0;
}

int main(int argc, const char *argv[])
{
    udfread *udf;
    UDFDIR *root;

    if (argc < 2) {
        fprintf(stderr, "usage: udfls <path>\n"
                "    path   path to UDF filesystem image (raw device or image file)\n");
        return -1;
    }

    udf = udfread_init();
    if (!udf) {
        fprintf(stderr, "udfread_init() failed\n");
        return -1;
    }
    if (udfread_open(udf, argv[1]) < 0) {
        fprintf(stderr, "udfread_open(%s) failed\n", argv[1]);
        udfread_close(udf);
        return -1;
    }

    printf("Volume ID: %s\n", udfread_get_volume_id(udf));

    root  = udfread_opendir(udf, "/");
    if (!root) {
        fprintf(stderr, "error opening root directory\n");
    } else {
        _lsdir_at(root, "/", 0);
        udfread_closedir(root);
    }

    udfread_close(udf);

    return 1;
}
