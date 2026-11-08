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

#ifndef UDFREAD_H_
#define UDFREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>    /* *int_t */
#include <sys/types.h> /* *size_t */
#ifdef _MSC_VER
typedef intptr_t ssize_t;
#endif

#ifdef UDFREAD_API_EXPORT
#include "attributes.h"
#elif !defined(UDF_PUBLIC)
#define UDF_PUBLIC
#endif

/**
 * @file udfread/udfread.h
 * external API header
 */

/*
 * NOTE:
 *
 * UDF filesystem file identifiers may contain nul bytes (0x00).
 *
 * In libudfread API file and directory names are encoded as Modified UTF-8 (MUTF-8).
 * The null character (U+0000) uses two-byte overlong encoding 11000000 10000000
 * (hexadecimal C0 80) instead of 00000000 (hexadecimal 00).
 */

/*
 * UDF volume access
 */

/* opaque handle for UDF volume */
typedef struct udfread udfread;

struct udfread_block_input;

/**
 *  Initialize UDF reader
 *
 * @return allocated udfread object, NULL if error
 */
UDF_PUBLIC udfread *udfread_init (void);

/**
 *  Open UDF image
 *
 * @param p  udfread object
 * @param input  UDF image access functions
 * @return 0 on success, < 0 on error
 */
UDF_PUBLIC int udfread_open_input (udfread *, struct udfread_block_input *input);

/** Boxedwine: open a session whose volume recognition sequence and first
 * anchor start at session_start + 16 and session_start + 256. Allocation
 * descriptors and the input callbacks continue to use absolute disc LBAs. */
UDF_PUBLIC int udfread_open_input_at (udfread *, struct udfread_block_input *input, uint32_t session_start);

/**
 *  Open UDF image
 *
 * @param p  udfread object
 * @param path  path to device or image file
 * @return 0 on success, < 0 on error
 */
UDF_PUBLIC int udfread_open (udfread *, const char *path);

/**
 *  Close UDF image
 *
 * @param p  udfread object
 */
UDF_PUBLIC void udfread_close (udfread *);

/**
 *  Get UDF Volume Identifier
 *
 * @param p  udfread object
 * @return Volume ID as null-terminated MUTF-8 string, NULL if error. Returned pointer is valid until udfread_close().
 */
UDF_PUBLIC const char *udfread_get_volume_id (udfread *);

/**
 *  Get UDF Volume Set Identifier
 *
 * @param p  udfread object
 * @param buffer buffer to receive volume set id
 * @param size buffer size
 * @return Volume set id size, 0 if error
 */
UDF_PUBLIC size_t udfread_get_volume_set_id (udfread *, void *buffer, size_t size);


/*
 * Directory access
 */

/* File types for d_type */
enum {
    UDF_DT_UNKNOWN = 0,
    UDF_DT_DIR,
    UDF_DT_REG,
};

/* Directory stream entry */
struct udfread_dirent {
    unsigned int  d_type;    /* UDF_DT_* */
    const char   *d_name;    /* MUTF-8 */
};

/* opaque handle for directory stream */
typedef struct udfread_dir UDFDIR;

/**
 *  Open directory stream
 *
 * @param p  udfread object
 * @param path  path to the directory (MUTF-8)
 * @return directory stream handle on the directory, or NULL if it could not be opened.
 */
UDF_PUBLIC UDFDIR *udfread_opendir (udfread *, const char *path);

/**
 *  Open directory stream
 *
 *  Directory name may contain special chars (/, \, ...).
 *
 * @param dir  parent directory stream handle
 * @param name  name of the directory to open from dir (MUTF-8)
 * @return directory stream handle on the directory, or NULL if it could not be opened.
 */
UDF_PUBLIC UDFDIR *udfread_opendir_at(UDFDIR *dir, const char *name);

/**
 *  Read directory stream
 *
 *  Read a directory entry from directory stream. Return a pointer to
 *  udfread_dirent struct describing the entry, or NULL for EOF or error.
 *
 * @param p  directory stream
 * @param entry  storege space for directory entry
 * @return next directory stream entry, or NULL if EOF or error.
 */
UDF_PUBLIC struct udfread_dirent *udfread_readdir (UDFDIR *, struct udfread_dirent *entry);

/**
 *  Rewind directory stream
 *
 *  Rewind directory stream to the beginning of the directory.
 *
 * @param p  directory stream
 */
UDF_PUBLIC void udfread_rewinddir (UDFDIR *);

/**
 *  Close directory stream
 *
 * @param p  directory stream
 */
UDF_PUBLIC void udfread_closedir (UDFDIR *);


/*
 * File access
 */

/**
 * The length of one Logical Block
 */

#ifndef UDF_BLOCK_SIZE
#  define UDF_BLOCK_SIZE  2048
#endif

/* opaque handle for open file */
typedef struct udfread_file UDFFILE;

/**
 *  Open a file
 *
 *  Allowed separator chars are \ and /.
 *  Path to the file is always absolute (relative to disc image root).
 *  Path may begin with single separator char.
 *  Path may not contain "." or ".." directory components.
 *
 * @param p  udfread object
 * @param path  path to the file (MUTF-8)
 * @return file object, or NULL if it could not be opened.
 */
UDF_PUBLIC UDFFILE *udfread_file_open (udfread *, const char *path);

/**
 *  Open a file from directory
 *
 *  File name may contain special chars (/, \, ...).
 *
 * @param dir  parent directory stream handle
 * @param name  name of the file (MUTF-8)
 * @return file object, or NULL if it could not be opened.
 */
UDF_PUBLIC UDFFILE *udfread_file_openat (UDFDIR *dir, const char *name);

/**
 *  Close file object
 *
 * @param p  file object
 */
UDF_PUBLIC void udfread_file_close (UDFFILE *);

/**
 *  Get file size
 *
 * @param p  file object
 * @return file size, -1 on error
 */
UDF_PUBLIC int64_t udfread_file_size (UDFFILE *);

/*
 * Block access
 */

/**
 *  Get file block address
 *
 *  Convert file block number to absolute block address.
 *
 * @param p  file object
 * @param file_block  file block number
 * @return absolute block address, 0 on error
 */
UDF_PUBLIC uint32_t udfread_file_lba (UDFFILE *, uint32_t file_block);

/**
 *  Read blocks from a file
 *
 * @param p  file object
 * @param buf  buffer for data
 * @param file_block  file block number
 * @param num_blocks  number of blocks to read
 * @return number of blocks read, 0 on error
 */
UDF_PUBLIC uint32_t udfread_read_blocks (UDFFILE *, void *buf, uint32_t file_block, uint32_t num_blocks, int flags);


/*
 * Byte streams
 */

enum {
    UDF_SEEK_SET = 0,
    UDF_SEEK_CUR = 1,
    UDF_SEEK_END = 2,
};

/**
 *  Read bytes from a file
 *
 *  Reads the given number of bytes from the file and increment the
 *  current read position by number of bytes read.
 *
 * @param p  file object
 * @param buf  buffer for data
 * @param bytes  number of bytes to read
 * @return number of bytes read, 0 on EOF, -1 on error
 */
UDF_PUBLIC ssize_t udfread_file_read (UDFFILE *, void *buf, size_t bytes);

/**
 *  Get current read position of a file
 *
 * @param p  file object
 * @return current read position of the file, -1 on error
 */
UDF_PUBLIC int64_t udfread_file_tell (UDFFILE *);

/**
 *  Set read position of a file
 *
 *  New read position is calculated from offset according to the directive whence as follows:
 *    UDF_SEEK_SET  The offset is set to offset bytes.
 *    UDF_SEEK_CUR  The offset is set to its current location plus offset bytes.
 *    UDF_SEEK_END  The offset is set to the size of the file plus offset bytes.
 *
 * @param p  file object
 * @param pos  byte offset
 * @param whence  directive
 * @return current read position of the file, -1 on error
 */
UDF_PUBLIC int64_t udfread_file_seek (UDFFILE *, int64_t pos, int whence);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* UDFREAD_H_ */
