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

#include "udfread.h"

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

static void _cat(UDFFILE *fp)
{
    uint8_t buf[2048];
    int64_t got = 0;
    ssize_t result;

    while ((result = udfread_file_read(fp, buf, sizeof(buf))) > 0) {
        fwrite(buf, 1, (size_t)result, stdout);
        got += result;
    }

    if (result < 0) {
        fprintf(stderr, "udfread_file_read(offset=%"PRId64") failed\n", udfread_file_tell(fp));
    }
    fprintf(stderr, "wrote %"PRId64" bytes of %"PRId64"\n", got, udfread_file_size(fp));
}

int main(int argc, const char *argv[])
{
    udfread *udf;
    UDFFILE *fp;

    if (argc < 3) {
        fprintf(stderr, "usage: udfcat <image> <file>\n"
                "    image   path to UDF filesystem image (raw device or image file)\n"
                "    file    path to file inside UDF filesystem\n");
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

    fp = udfread_file_open(udf, argv[2]);
    if (!fp) {
        fprintf(stderr, "udfread_file_open(%s) failed\n", argv[2]);
        udfread_close(udf);
        return -1;
    }

    _cat(fp);

    udfread_file_close(fp);
    udfread_close(udf);
}

