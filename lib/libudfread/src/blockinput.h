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

#ifndef UDFREAD_BLOCKINPUT_H_
#define UDFREAD_BLOCKINPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @file udfread/blockinput.h
 * external API header
 */

#ifndef UDF_BLOCK_SIZE
#  define UDF_BLOCK_SIZE  2048
#endif

typedef struct udfread_block_input udfread_block_input;

struct udfread_block_input {
    /* Close input. Optional. */
    int      (*close) (udfread_block_input *);
    /* Read block(s) from input. Mandatory. */
    int      (*read)  (udfread_block_input *, uint32_t lba, void *buf, uint32_t nblocks, int flags);
    /* Input size in blocks. Optional. */
    uint32_t (*size)  (udfread_block_input *);
};


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* UDFREAD_BLOCKINPUT_H_ */
