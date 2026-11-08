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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "udfread.h"

#ifdef HAVE_UDFREAD_VERSION_H
#include "udfread-version.h"
#endif

#include "blockinput.h"
#include "default_blockinput.h"
#include "ecma167.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define strtok_r strtok_s
#endif


/*
 * Logging
 */

#include <stdio.h>

static int enable_log   = 0;
static int enable_trace = 0;

#define udf_error(...)   do {                   fprintf(stderr, "udfread ERROR: " __VA_ARGS__); } while (0)
#define udf_log(...)     do { if (enable_log)   fprintf(stderr, "udfread LOG  : " __VA_ARGS__); } while (0)
#define udf_trace(...)   do { if (enable_trace) fprintf(stderr, "udfread TRACE: " __VA_ARGS__); } while (0)


/*
 * atomic operations
 */

#if defined (__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4) || (defined (__clang__) && (defined (__x86_64__) || defined (__i386__)))

#  define atomic_pointer_compare_and_exchange(atomic, oldval, newval) \
    __sync_bool_compare_and_swap((atomic), (oldval), (newval))

#elif defined(_WIN32)

#include <windows.h>

static int atomic_pointer_compare_and_exchange(void *atomic, void *oldval, void *newval)
{
    return InterlockedCompareExchangePointer((PVOID volatile *)atomic, newval, oldval) == oldval;
}

#elif defined(HAVE_PTHREAD_H)

#include <pthread.h>

static int atomic_pointer_compare_and_exchange(void *atomic, void *oldval, void *newval)
{
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    int result;
    pthread_mutex_lock(&lock);
    result = *(void**)atomic == oldval;
    if (result) {
        *(void**)atomic = newval;
    }
    pthread_mutex_unlock(&lock);
    return result;
}

#else
# error no atomic operation support
#endif


/*
 * utils
 */

static char *_str_dup(const char *s)
{
    size_t len = strlen(s);
    char *p = (char *)malloc(len + 1);
    if (p) {
        memcpy(p, s, len + 1);
    } else {
        udf_error("out of memory\n");
    }
    return p;
}

static void *_safe_realloc(void *p, size_t s)
{
    void *result = realloc(p, s);
    if (!result) {
        udf_error("out of memory\n");
        free(p);
    }
    return result;
}


/*
 * Decoding
 */

/*
 * outputs Modified UTF-8 (MUTF-8).
 * The null character (U+0000) uses the two-byte overlong encoding 11000000 10000000 (hexadecimal C0 80), instead of 00000000 (hexadecimal 00).
 *
 * - not strictly UTF-8 compilant, but works with C str*() functions and Java, while \0 bytes in middle of strings won't.
 */

#define utf16lo_to_mutf8(out, out_pos, out_size, ch)    \
  do {                                              \
    if (ch != 0 && ch < 0x80) {                     \
      out[out_pos++] = (uint8_t)ch;                 \
    } else {                                        \
      out_size++;                                   \
      out = (uint8_t *)_safe_realloc(out, out_size);\
      if (!out) return NULL;                        \
                                                    \
      out[out_pos++] = 0xc0 | (ch >> 6);            \
      out[out_pos++] = 0x80 | (ch & 0x3f);          \
    }                                               \
  } while (0)

#define utf16_to_mutf8(out, out_pos, out_size, ch)      \
  do {                                                  \
    if (ch < 0x7ff) {                                   \
      utf16lo_to_mutf8(out, out_pos, out_size, ch);     \
    } else {                                            \
      out_size += 2;                                    \
      out = (uint8_t *)_safe_realloc(out, out_size);    \
      if (!out) return NULL;                            \
                                                        \
      out[out_pos++] = 0xe0 | (ch >> 12);               \
      out[out_pos++] = 0x80 | ((ch >> 6) & 0x3f);       \
      out[out_pos++] = 0x80 | (ch & 0x3f);              \
                                                        \
    }                                                   \
  } while (0)

/* Strings, CS0 (UDF 2.1.1) */
static char *_cs0_to_mutf8(const uint8_t *cs0, size_t size)
{
    size_t   out_pos = 0;
    size_t   out_size = size;
    size_t   i;
    uint8_t *out;

    if (size < 1) {
        /* empty string */
        return calloc(1, 1);
    }

    out = (uint8_t *)malloc(size);
    if (!out) {
        udf_error("out of memory\n");
        return NULL;
    }

    switch (cs0[0]) {
    case 8:
        /*udf_trace("string in utf-8\n");*/
        for (i = 1; i < size; i++) {
            utf16lo_to_mutf8(out, out_pos, out_size, cs0[i]);
        }
        break;
    case 16:
        for (i = 1; i < size - 1; i+=2) {
            uint16_t ch = cs0[i + 1] | (cs0[i] << 8);
            utf16_to_mutf8(out, out_pos, out_size, ch);
        }
        break;
    default:
        udf_error("unregonized string encoding %u\n", cs0[0]);
        free(out);
        return NULL;
    }

    out[out_pos] = 0;
    return (char*)out;
}


/* Domain Identifiers, UDF 2.1.5.2 */

static const char lvd_domain_id[]  = "*OSTA UDF Compliant";
static const char meta_domain_id[] = "*UDF Metadata Partition";

static int _check_domain_identifier(const struct entity_id *eid, const char *value)
{
    return (!memcmp(value, eid->identifier, strlen(value))) ? 0 : -1;
}

/* Additional File Types (UDF 2.60, 2.3.5.2) */

enum udf_file_type {
    UDF_FT_METADATA        = 250,
    UDF_FT_METADATA_MIRROR = 251,
};


/*
 * Block access
 *
 * read block(s) from absolute lba
 */

static uint32_t _read_blocks(udfread_block_input *input,
                             uint32_t lba, void *buf, uint32_t nblocks,
                             int flags)
{
    int result;

    if (!input || (int)nblocks < 1) {
        return 0;
    }

    result = input->read(input, lba, buf, nblocks, flags);

    return result < 0 ? 0 : (uint32_t)result;
}

static int _read_descriptor_block(udfread_block_input *input, uint32_t lba, uint8_t *buf)
{
    if (_read_blocks(input, lba, buf, 1, 0) == 1) {
        return decode_descriptor_tag(buf);
    }

    return -1;
}


/*
 * Disc probing
 */

static int _probe_volume(udfread_block_input *input)
{
    /* Volume Recognition (ECMA 167 2/8, UDF 2.60 2.1.7) */

    static const char bea[]    = {'\0',  'B',  'E',  'A',  '0',  '1', '\1'};
    static const char nsr_02[] = {'\0',  'N',  'S',  'R',  '0',  '2', '\1'};
    static const char nsr_03[] = {'\0',  'N',  'S',  'R',  '0',  '3', '\1'};
    static const char tea[]    = {'\0',  'T',  'E',  'A',  '0',  '1', '\1'};
    static const char nul[]    = {'\0', '\0', '\0', '\0', '\0', '\0', '\0'};

    uint8_t  buf[UDF_BLOCK_SIZE];
    uint32_t lba;
    int      bea_seen = 0;

    for (lba = 16; lba < 256; lba++) {
        if (_read_blocks(input, lba, buf, 1, 0) == 1) {

            /* Terminating Extended Area Descriptor */
            if (!memcmp(buf, tea, sizeof(tea))) {
                udf_error("ECMA 167 Volume Recognition failed (no NSR descriptor)\n");
                return -1;
            }
            if (!memcmp(buf, nul, sizeof(nul))) {
                break;
            }
            if (!memcmp(buf, bea, sizeof(bea))) {
                udf_trace("ECMA 167 Volume, BEA01\n");
                bea_seen = 1;
            }

            if (bea_seen) {
                if (!memcmp(buf, nsr_02, sizeof(nsr_02))) {
                    udf_trace("ECMA 167 Volume, NSR02\n");
                    return 0;
                }
                if (!memcmp(buf, nsr_03, sizeof(nsr_03))) {
                    udf_trace("ECMA 167 Volume, NSR03\n");
                    return 0;
                }
            }
        }
    }

    udf_error("ECMA 167 Volume Recognition failed\n");
    return -1;
}

static int _read_avdp(udfread_block_input *input, struct anchor_volume_descriptor *avdp)
{
    uint8_t  buf[UDF_BLOCK_SIZE];
    int      tag_id;
    uint32_t lba = 256;

    /*
     * Find Anchor Volume Descriptor Pointer.
     * It is in block 256, last block or (last block - 256)
     * (UDF 2.60, 2.2.3)
     */

    /* try block 256 */
    tag_id = _read_descriptor_block(input, lba, buf);
    if (tag_id != ECMA_AnchorVolumeDescriptorPointer) {

        /* try last block */
        if (!input->size) {
            udf_error("Can't find Anchor Volume Descriptor Pointer\n");
            return -1;
        }

        lba = input->size(input) - 1;
        tag_id = _read_descriptor_block(input, lba, buf);
        if (tag_id != ECMA_AnchorVolumeDescriptorPointer) {

            /* try last block - 256 */
            lba -= 256;
            tag_id = _read_descriptor_block(input, lba, buf);
            if (tag_id != ECMA_AnchorVolumeDescriptorPointer) {
                udf_error("Can't find Anchor Volume Descriptor Pointer\n");
                return -1;
            }
        }
    }
    udf_log("Found Anchor Volume Descriptor Pointer from lba %u\n", lba);

    decode_avdp(buf, avdp);

    return 1;
}


/*
 * Volume structure
 */

/* Logical Partitions from Logical Volume Descriptor */
struct udf_partitions {
    uint32_t num_partition;
    struct {
        uint16_t number;
        uint32_t lba;
        uint32_t mirror_lba;
    } p[2];
};

struct volume_descriptor_set {
    struct partition_descriptor      pd;
    struct primary_volume_descriptor pvd;
    struct logical_volume_descriptor lvd;
};

static int _search_vds(udfread_block_input *input, int part_number,
                       const struct extent_ad *loc,
                       struct volume_descriptor_set *vds)

{
    struct volume_descriptor_pointer vdp;
    uint8_t  buf[UDF_BLOCK_SIZE];
    int      tag_id;
    uint32_t lba;
    uint32_t end_lba;
    int      have_part = 0, have_lvd = 0, have_pvd = 0;
    unsigned extents = 0;

    memset(vds, 0, sizeof(*vds));

next_extent:
    if (++extents > 64) return -1;
    udf_trace("reading Volume Descriptor Sequence at lba %u, len %u bytes\n", loc->lba, loc->length);

    if (!loc->length || loc->length / UDF_BLOCK_SIZE > 4096 ||
        (uint64_t)loc->lba + loc->length / UDF_BLOCK_SIZE > UINT32_MAX) return -1;
    end_lba = loc->lba + loc->length / UDF_BLOCK_SIZE;

    /* parse Volume Descriptor Sequence */
    for (lba = loc->lba; lba < end_lba; lba++) {

        tag_id = _read_descriptor_block(input, lba, buf);
        if (tag_id < 0) return -1;

        switch (tag_id) {

        case ECMA_VolumeDescriptorPointer:
            decode_vdp(buf, &vdp);
            loc = &vdp.next_extent;
            goto next_extent;

        case ECMA_PrimaryVolumeDescriptor:
            udf_log("Primary Volume Descriptor in lba %u\n", lba);
            decode_primary_volume(buf, &vds->pvd);
            have_pvd = 1;
            break;

        case ECMA_LogicalVolumeDescriptor:
            udf_log("Logical volume descriptor in lba %u\n", lba);
            decode_logical_volume(buf, &vds->lvd);
            have_lvd = 1;
            break;

        case ECMA_PartitionDescriptor:
          udf_log("Partition Descriptor in lba %u\n", lba);
          if (!have_part) {
              decode_partition(buf, &vds->pd);
              have_part = (part_number < 0 || part_number == vds->pd.number);
              udf_log("  partition %u at lba %u, %u blocks\n", vds->pd.number, vds->pd.start_block, vds->pd.num_blocks);
          }
          break;

        case ECMA_TerminatingDescriptor:
            udf_trace("Terminating Descriptor in lba %u\n", lba);
            return (have_part && have_lvd) ? 0 : -1;
        }

        if (have_part && have_lvd && have_pvd) {
            /* got everything interesting, skip rest blocks */
            return 0;
        }
    }

    return (have_part && have_lvd) ? 0 : -1;
}

static int _read_vds(udfread_block_input *input, int part_number,
                     struct volume_descriptor_set *vds)
{
    struct anchor_volume_descriptor avdp;

    /* Find Anchor Volume Descriptor */
    if (_read_avdp(input, &avdp) < 0) {
        return -1;
    }

    // XXX we could read part of descriptors from main area and rest from backup if both are partially corrupted ...

    /* try to read Main Volume Descriptor Sequence */
    if (!_search_vds(input, part_number, &avdp.mvds, vds)) {
        return 0;
    }

    /* try to read Backup Volume Descriptor */
    if (!_search_vds(input, part_number, &avdp.rvds, vds)) {
        return 0;
    }

    udf_error("failed reading Volume Descriptor Sequence\n");
    return -1;
}

static int _validate_logical_volume(const struct logical_volume_descriptor *lvd, struct long_ad *fsd_loc)
{
    if (lvd->block_size != UDF_BLOCK_SIZE) {
        udf_error("incompatible block size %u\n", lvd->block_size);
        return -1;
    }

    /* UDF 2.60 2.1.5.2 */
    if (_check_domain_identifier(&lvd->domain_id, lvd_domain_id) < 0) {
        udf_error("unknown Domain ID in Logical Volume Descriptor: %1.22s\n", lvd->domain_id.identifier);
        return -1;

    } else {

        /* UDF 2.60 2.1.5.3 */
        uint16_t rev = _get_u16(lvd->domain_id.identifier_suffix);
        udf_log("Found UDF %x.%02x Logical Volume\n", rev >> 8, rev & 0xff);

        /* UDF 2.60 2.2.4.4 */

        /* location of File Set Descriptors */
        decode_long_ad(lvd->contents_use, fsd_loc);

        udf_log("File Set Descriptor location: partition %u lba %u (len %u)\n",
                fsd_loc->partition, fsd_loc->lba, fsd_loc->length);
    }

    return 0;
}

static int _map_metadata_partition(udfread_block_input *input,
                                   struct udf_partitions *part,
                                   uint32_t lba, uint32_t mirror_lba,
                                   const struct partition_descriptor *pd)
{
    struct file_entry *fe;
    uint8_t       buf[UDF_BLOCK_SIZE];
    int           tag_id;
    unsigned int  i;

    /* resolve metadata partition location (it is virtual partition inside another partition) */
    udf_trace("Reading metadata file entry: lba %u, mirror lba %u\n", lba, mirror_lba);

    for (i = 0; i < 2; i++) {

        if (i == 0) {
            tag_id = _read_descriptor_block(input, pd->start_block + lba, buf);
        } else {
            tag_id = _read_descriptor_block(input, pd->start_block + mirror_lba, buf);
        }

        if (tag_id != ECMA_ExtendedFileEntry) {
            udf_error("read metadata file %u: unexpected tag %d\n", i, tag_id);
            continue;
        }

        fe = decode_ext_file_entry(buf, UDF_BLOCK_SIZE, pd->number);
        if (!fe) {
            udf_error("parsing metadata file entry %u failed\n", i);
            continue;
        }

        if (fe->content_inline) {
            udf_error("invalid metadata file (content inline)\n");
        } else if (!fe->u.ads.num_ad) {
            udf_error("invalid metadata file (no allocation descriptors)\n");
        } else if (fe->file_type == UDF_FT_METADATA) {
            part->p[1].lba = pd->start_block + fe->u.ads.ad[0].lba;
            udf_log("metadata file at lba %u\n", part->p[1].lba);
        } else if (fe->file_type == UDF_FT_METADATA_MIRROR) {
            part->p[1].mirror_lba = pd->start_block + fe->u.ads.ad[0].lba;
            udf_log("metadata mirror file at lba %u\n", part->p[1].mirror_lba);
        } else {
            udf_error("unknown metadata file type %u\n", fe->file_type);
        }

        free_file_entry(&fe);
    }

    if (!part->p[1].lba && part->p[1].mirror_lba) {
        /* failed reading primary location, must use mirror */
        part->p[1].lba        = part->p[1].mirror_lba;
        part->p[1].mirror_lba = 0;
    }

    return part->p[1].lba ? 0 : -1;
}

static int _parse_udf_partition_maps(udfread_block_input *input,
                                     struct udf_partitions *part,
                                     const struct volume_descriptor_set *vds)
{
    /* parse partition maps
     * There should be one type1 partition.
     * There may be separate metadata partition.
     * metadata partition is virtual partition that is mapped to metadata file.
     */

    const uint8_t *map = vds->lvd.partition_map_table;
    const uint8_t *end = map + vds->lvd.partition_map_lable_length;
    unsigned int   i;
    int            num_type1_partition = 0;

    udf_log("Partition map count: %u\n", vds->lvd.num_partition_maps);
    if (vds->lvd.partition_map_lable_length > sizeof(vds->lvd.partition_map_table)) {
        udf_error("partition map table too big !\n");
        end -= vds->lvd.partition_map_lable_length - sizeof(vds->lvd.partition_map_table);
    }

    for (i = 0; i < vds->lvd.num_partition_maps && map + 2 < end; i++) {

        /* Partition map, ECMA 167 3/10.7 */
        uint8_t  type = _get_u8(map + 0);
        uint8_t  len  = _get_u8(map + 1);
        uint16_t ref;

        if (len < 2) {
            udf_error("invalid partition map length %d\n", (int)len);
            break;
        }

        udf_trace("map %u: type %u\n", i, type);
        if (map + len > end) {
            udf_error("partition map table too short !\n");
            break;
        }

        if (type == 1) {

            /* ECMA 167 Type 1 partition map */

            if (len != 6) {
                udf_error("invalid type 1 partition map length %d\n", (int)len);
                break;
            }

            ref = _get_u16(map + 4);
            udf_log("partition map: %u: type 1 partition, ref %u\n", i, ref);

            if (num_type1_partition) {
                udf_error("more than one type1 partitions not supported\n");
            } else if (ref != vds->pd.number) {
                udf_error("Logical partition %u refers to another physical partition %u (expected %u)\n", i, ref, vds->pd.number);
            } else {
                part->num_partition   = 1;
                part->p[0].number     = i;
                part->p[0].lba        = vds->pd.start_block;
                part->p[0].mirror_lba = 0; /* no mirror for data partition */

                num_type1_partition++;
            }

        } else if (type == 2) {

            /* Type 2 partition map, UDF 2.60 2.2.18 */

            if (len != 64) {
                udf_error("invalid type 2 partition map length %d\n", (int)len);
                break;
            }

            struct entity_id type_id;
            decode_entity_id(map + 4, &type_id);
            if (!_check_domain_identifier(&type_id, meta_domain_id)) {

                /* Metadata Partition, UDF 2.60 2.2.10 */

                uint32_t lba, mirror_lba;

                ref        = _get_u16(map + 38);
                lba        = _get_u32(map + 40);
                mirror_lba = _get_u32(map + 44);
                if (ref != vds->pd.number) {
                    udf_error("metadata file partition %u != %u\n", ref, vds->pd.number);
                }

                if (!_map_metadata_partition(input, part, lba, mirror_lba, &vds->pd)) {
                    part->num_partition = 2;
                    part->p[1].number   = i;
                    udf_log("partition map: %u: metadata partition, ref %u. lba %u, mirror %u\n", i, ref, part->p[1].lba, part->p[1].mirror_lba);
                }

            } else {
                udf_log("%u: unsupported type 2 partition\n", i);
            }
        }
        map += len;
    }

    return num_type1_partition ? 0 : -1;
}


/*
 * Cached directory data
 */

struct udf_file_identifier {
    char           *filename;       /* MUTF-8 */
    struct long_ad  icb;            /* location of file entry */
    uint8_t         characteristic; /* CHAR_FLAG_* */
};

struct udf_dir {
    uint32_t                     num_entries;
    struct udf_file_identifier  *files;
    struct udf_dir             **subdirs;
};

static void _free_dir(struct udf_dir **pp)
{
    if (pp && *pp) {
        struct udf_dir *p = *pp;
        uint32_t i;

        if (p->subdirs) {
            for (i = 0; i < p->num_entries; i++) {
                _free_dir(&(p->subdirs[i]));
            }
            free(p->subdirs);
        }

        if (p->files) {
            for (i = 0; i < p->num_entries; i++) {
                free(p->files[i].filename);
            }
            free(p->files);
        }

        free(p);

        *pp = NULL;
    }
}


/*
 *
 */

struct udfread {

    udfread_block_input *input;

    /* Volume partitions */
    struct udf_partitions part;

    /* cached directory tree */
    struct udf_dir *root_dir;

    char *volume_identifier;
    char volume_set_identifier[128];

};

udfread *udfread_init(void)
{
    /* set up logging */
    if (getenv("UDFREAD_LOG")) {
        enable_log = 1;
    }
    if (getenv("UDFREAD_TRACE")) {
        enable_trace = 1;
        enable_log = 1;
    }

#ifdef HAVE_UDFREAD_VERSION_H
    udf_log("libudfread " UDFREAD_VERSION_STRING "\n");
#endif

    return (udfread *)calloc(1, sizeof(udfread));
}

/*
 * Metadata
 */

static int _partition_index(udfread *udf, uint16_t partition_number)
{
    if (partition_number == udf->part.p[0].number) {
        return 0;
    } else if (udf->part.num_partition > 1 && partition_number == udf->part.p[1].number) {
        return 1;
    }

    udf_error("unknown partition %u\n", partition_number);
    return -1;
}

/* read metadata blocks. If read fails, try from mirror (if available). */
static int _read_metadata_blocks(udfread *udf, uint8_t *buf,
                                const struct long_ad *loc)
{
    int      tag_id;
    uint32_t lba, i, got;
    int      part_idx;

    udf_trace("reading metadata from part %u lba %u\n", loc->partition, loc->lba);

    part_idx = _partition_index(udf, loc->partition);
    if (part_idx < 0) {
        return -1;
    }

    if (!loc->length || (uint64_t)udf->part.p[part_idx].lba + loc->lba +
        (loc->length - 1) / UDF_BLOCK_SIZE > UINT32_MAX) return -1;

    /* read first block. Parse and check tag. */

    lba    = udf->part.p[part_idx].lba + loc->lba;
    tag_id = _read_descriptor_block(udf->input, lba, buf);

    if (tag_id < 0) {

        /* try mirror */
        if (udf->part.p[part_idx].mirror_lba) {
            udf_log("read metadata from lba %u failed, trying mirror\n", lba);
            lba    = udf->part.p[part_idx].mirror_lba + loc->lba;
            tag_id = _read_descriptor_block(udf->input, lba, buf);
        }

        if (tag_id < 0) {
            udf_error("read metadata from lba %u failed\n", lba);
            return -1;
        }
    }

    /* read following blocks without tag parsing and checksum validation */

    for (i = 1; i <= (loc->length - 1) / UDF_BLOCK_SIZE; i++) {

        lba  = udf->part.p[part_idx].lba + loc->lba + i;
        buf += UDF_BLOCK_SIZE;

        got = _read_blocks(udf->input, lba, buf, 1, 0);
        if (got != 1) {
            if (udf->part.p[part_idx].mirror_lba) {
                udf_log("read metadata from lba %u failed, trying mirror\n", lba);
                lba = udf->part.p[part_idx].mirror_lba + loc->lba + i;
                got = _read_blocks(udf->input, lba, buf, 1, 0);
            }
            if (got != 1) {
                udf_error("read metadata from lba %u failed\n", lba);
                return -1;
            }
        }
    }

    return tag_id;
}

static uint8_t *_read_metadata(udfread *udf, const struct long_ad *icb, int *tag_id)
{
    uint32_t  num_blocks = (icb->length + UDF_BLOCK_SIZE - 1) / UDF_BLOCK_SIZE;
    uint8_t  *buf;

    /* Bound metadata allocations independently of the image's data size. */
    if (num_blocks < 1 || icb->length > 16 * 1024 * 1024) {
        return NULL;
    }

    buf = (uint8_t *)malloc(num_blocks * UDF_BLOCK_SIZE);
    if (!buf) {
        udf_error("out of memory\n");
        return NULL;
    }

    *tag_id = _read_metadata_blocks(udf, buf, icb);
    if (*tag_id < 0) {
        udf_log("reading icb blocks failed\n");
        free(buf);
        return NULL;
    }

    return buf;
}

static struct file_entry *_read_file_entry(udfread *udf,
                                           const struct long_ad *icb)
{
    struct file_entry *fe = NULL;
    uint8_t  *buf;
    int       tag_id;

    udf_trace("file entry size %u bytes\n", icb->length);

    buf = _read_metadata(udf, icb, &tag_id);
    if (!buf) {
        udf_error("reading file entry failed\n");
        return NULL;
    }

    switch (tag_id) {
        case ECMA_FileEntry:
            fe = decode_file_entry(buf, UDF_BLOCK_SIZE, icb->partition);
            break;
        case ECMA_ExtendedFileEntry:
            fe = decode_ext_file_entry(buf, UDF_BLOCK_SIZE, icb->partition);
            break;
        default:
            udf_error("_read_file_entry: unknown tag %d\n", tag_id);
            break;
    }

    free(buf);

    /* read possible additional allocation extents */
    if (fe && !fe->content_inline) {
        unsigned chains = 0;
        while (fe->u.ads.num_ad > 0 &&
               fe->u.ads.ad[fe->u.ads.num_ad - 1].extent_type == ECMA_AD_EXTENT_AD) {
            if (++chains > 1024) {
                free_file_entry(&fe);
                return NULL;
            }

            /* drop pointer to this extent from the end of AD list */
            fe->u.ads.num_ad--;

            icb = &fe->u.ads.ad[fe->u.ads.num_ad];
            udf_log("_read_file_entry: reading allocation extent @%u\n", icb->lba);

            buf = _read_metadata(udf, icb, &tag_id);
            if (!buf) {
                udf_error("_read_file_entry: reading allocation extent @%u failed\n", icb->lba);
                free_file_entry(&fe);
                return NULL;
            }

            if (tag_id != ECMA_AllocationExtentDescriptor) {
                free(buf);
                udf_error("_read_file_entry: unexpected tag %d (expected ECMA_AllocationExtentDescriptor)\n", tag_id);
                free_file_entry(&fe);
                return NULL;
            }

            if (decode_allocation_extent(&fe, buf, icb->length, icb->partition) < 0) {
                free(buf);
                udf_error("_read_file_entry: decode_allocation_extent() failed\n");
                free_file_entry(&fe);
                return NULL;
            }

            /* failure before this point will cause an error when reading the file past extent point.
               (extent ad is left in file ad list). */

            free(buf);
        }
    }

    return fe;
}

static int _parse_dir(const uint8_t *data, uint32_t length, struct udf_dir *dir)
{
    struct file_identifier fid;
    const uint8_t *p   = data;
    const uint8_t *end = data + length;
    int            tag_id;

    if (!length) return 0;
    if (length < 16) return -1;

    while (p < end - 16) {
        size_t used;

        if (dir->num_entries >= 100000) return -1;

        tag_id = decode_descriptor_tag(p);
        if (tag_id != ECMA_FileIdentifierDescriptor) {
            udf_error("unexpected tag %d in directory file\n", tag_id);
            return -1;
        }

        dir->files = (struct udf_file_identifier *)_safe_realloc(dir->files, sizeof(dir->files[0]) * (dir->num_entries + 1));
        if (!dir->files) {
            return -1;
        }

        used = decode_file_identifier(p, (size_t)(end - p), &fid);
        if (used == 0 || used > (size_t)(end - p)) return -1;
        p += used;

        if (fid.characteristic & (CHAR_FLAG_PARENT | CHAR_FLAG_DELETED)) {
            continue;
        }
        if (fid.filename_len < 1) {
            continue;
        }

        dir->files[dir->num_entries].characteristic = fid.characteristic;
        dir->files[dir->num_entries].icb = fid.icb;
        dir->files[dir->num_entries].filename = _cs0_to_mutf8(fid.filename, fid.filename_len);

        if (!dir->files[dir->num_entries].filename) {
            continue;
        }

        /* Skip empty file identifiers.
         * Not strictly compilant (?), \0 is allowed in
         * ECMA167 file identifier.
         */
        if (!dir->files[dir->num_entries].filename[0]) {
            udf_error("skipping empty file identifier\n");
            free(dir->files[dir->num_entries].filename);
            continue;
        }

        dir->num_entries++;
    }

    return p == end ? 0 : -1;
}

static struct udf_dir *_read_dir_file(udfread *udf, const struct long_ad *loc)
{
    struct udf_dir *dir = NULL;
    uint8_t        *data;
    int             tag_id;

    udf_trace("directory size %u bytes\n", loc->length);

    data = _read_metadata(udf, loc, &tag_id);
    if (!data) {
        udf_error("reading directory file failed\n");
        return NULL;
    }

    dir = (struct udf_dir *)calloc(1, sizeof(struct udf_dir));
    if (dir) {
        if (_parse_dir(data, loc->length, dir) < 0) {
            _free_dir(&dir);
        }
    }

    free(data);
    return dir;
}

static struct udf_dir *_read_dir(udfread *udf, const struct long_ad *icb)
{
    struct file_entry *fe;
    struct udf_dir    *dir = NULL;

    fe = _read_file_entry(udf, icb);
    if (!fe) {
        udf_error("error reading directory file entry\n");
        return NULL;
    }

    if (fe->file_type != ECMA_FT_DIR) {
        udf_error("directory file type is not directory\n");
        free_file_entry(&fe);
        return NULL;
    }

    if (fe->content_inline) {
        dir = (struct udf_dir *)calloc(1, sizeof(struct udf_dir));
        if (dir) {
            if (_parse_dir(&fe->u.data.content[0], fe->u.data.information_length, dir) < 0) {
                udf_error("failed parsing inline directory file\n");
                _free_dir(&dir);
            }
        }

    } else if (fe->u.ads.num_ad == 0) {
        udf_error("empty directory file");
    } else {
        if (fe->u.ads.num_ad > 1) {
            udf_error("unsupported fragmented directory file\n");
            free_file_entry(&fe);
            return NULL;
        }
        if (fe->length > fe->u.ads.ad[0].length) {
            free_file_entry(&fe);
            return NULL;
        }
        fe->u.ads.ad[0].length = (uint32_t)fe->length;
        dir = _read_dir_file(udf, &fe->u.ads.ad[0]);
    }

    free_file_entry(&fe);
    return dir;
}

static int _read_root_dir(udfread *udf, const struct long_ad *fsd_loc)
{
    struct file_set_descriptor fsd;
    uint8_t             buf[UDF_BLOCK_SIZE];
    int                 tag_id = -1;
    struct long_ad      loc = *fsd_loc;

    udf_trace("reading root directory fsd from part %u lba %u\n", fsd_loc->partition, fsd_loc->lba);

    /* search for File Set Descriptor from the area described by fsd_loc */

    loc.length = UDF_BLOCK_SIZE;
    if (!fsd_loc->length || fsd_loc->length > 16 * 1024 * 1024 ||
        (uint64_t)fsd_loc->lba + (fsd_loc->length - 1) / UDF_BLOCK_SIZE > UINT32_MAX) return -1;
    for (; loc.lba <= fsd_loc->lba + (fsd_loc->length - 1) / UDF_BLOCK_SIZE; loc.lba++) {

        tag_id = _read_metadata_blocks(udf, buf, &loc);
        if (tag_id == ECMA_FileSetDescriptor) {
            break;
        }
        if (tag_id == ECMA_TerminatingDescriptor) {
            break;
        }
        udf_error("unhandled tag %d in File Set Descriptor area\n", tag_id);
    }
    if (tag_id != ECMA_FileSetDescriptor) {
        udf_error("didn't find File Set Descriptor\n");
        return -1;
    }

    decode_file_set_descriptor(buf, &fsd);
    udf_log("root directory in part %u lba %u\n", fsd.root_icb.partition, fsd.root_icb.lba);

    /* read root directory from location given in File Set Descriptor */

    udf->root_dir = _read_dir(udf, &fsd.root_icb);
    if (!udf->root_dir) {
        udf_error("error reading root directory\n");
        return -1;
    }

    return 0;
}

static struct udf_dir *_read_subdir(udfread *udf, struct udf_dir *dir, uint32_t index)
{
    if (!(dir->files[index].characteristic & CHAR_FLAG_DIR)) {
        return NULL;
    }

    if (!dir->subdirs) {
        struct udf_dir **subdirs = (struct udf_dir **)calloc(sizeof(struct udf_dir *), dir->num_entries);
        if (!subdirs) {
            udf_error("out of memory\n");
            return NULL;
        }
        if (!atomic_pointer_compare_and_exchange(&dir->subdirs, NULL, subdirs)) {
            free(subdirs);
        }
    }

    if (!dir->subdirs[index]) {
        struct udf_dir *subdir = _read_dir(udf, &dir->files[index].icb);
        if (!subdir) {
            return NULL;
        }
        if (!atomic_pointer_compare_and_exchange(&dir->subdirs[index], NULL, subdir)) {
            _free_dir(&subdir);
        }
    }

    return dir->subdirs[index];
}

static int _scan_dir(const struct udf_dir *dir, const char *filename, uint32_t *index)
{
    uint32_t i;

    for (i = 0; i < dir->num_entries; i++) {
        if (!strcmp(filename, dir->files[i].filename)) {
            *index = i;
            return 0;
        }
    }

    udf_log("file %s not found\n", filename);
    return -1;
}

static int _find_file(udfread *udf, const char *path,
                      struct udf_dir **p_dir,
                      const struct udf_file_identifier **p_fid)
{
    const struct udf_file_identifier *fid = NULL;
    struct udf_dir *current_dir;
    char *tmp_path, *save_ptr, *token;

    current_dir = udf->root_dir;
    if (!current_dir) {
        return -1;
    }

    tmp_path = _str_dup(path);
    if (!tmp_path) {
        return -1;
    }

    token = strtok_r(tmp_path, "/\\", &save_ptr);
    if (token == NULL) {
        udf_trace("_find_file: requested root dir\n");
    }

    while (token) {
        uint32_t index;
        if (_scan_dir(current_dir, token, &index) < 0) {
            udf_log("_find_file: entry %s not found\n", token);
            goto error;
        }
        fid = &current_dir->files[index];

        token = strtok_r(NULL, "/\\", &save_ptr);

        if (fid->characteristic & CHAR_FLAG_DIR) {
            current_dir = _read_subdir(udf, current_dir, index);
            if (!current_dir) {
                goto error;
            }
        } else if (token) {
            udf_log("_find_file: entry %s not found (parent is file, not directory)\n", token);
            goto error;
        } else {
            // found a file, make sure we won't return directory data
            current_dir = NULL;
        }
    }

    if (p_fid) {
        if (!fid) {
            udf_log("no file identifier found for %s\n", path);
            goto error;
        }
        *p_fid = fid;
    }
    if (p_dir) {
        *p_dir = current_dir;
    }

    free(tmp_path);
    return 0;

error:
    free(tmp_path);
    return -1;
}


/*
 * Volume access API
 */

int udfread_open_input(udfread *udf, udfread_block_input *input/*, int partition*/)
{
    struct volume_descriptor_set vds;
    struct long_ad fsd_location;

    if (!udf || !input || !input->read) {
        return -1;
    }

    if (_probe_volume(input) < 0) {
        return -1;
    }

    /* read Volume Descriptor Sequence */
    if (_read_vds(input, 0, &vds) < 0) {
        return -1;
    }

    /* validate logical volume structure */
    if (_validate_logical_volume(&vds.lvd, &fsd_location) < 0) {
        return -1;
    }

    /* Volume Identifier. CS0, UDF 2.1.1 */
    udf->volume_identifier = _cs0_to_mutf8(vds.pvd.volume_identifier, vds.pvd.volume_identifier_length);
    if (udf->volume_identifier) {
        udf_log("Volume Identifier: %s\n", udf->volume_identifier);
    }

    memcpy(udf->volume_set_identifier, vds.pvd.volume_set_identifier, 128);

    /* map partitions */
    if (_parse_udf_partition_maps(input, &udf->part, &vds) < 0) {
        return -1;
    }

    /* Read root directory */
    udf->input = input;
    if (_read_root_dir(udf, &fsd_location) < 0) {
        udf->input = NULL;
        return -1;
    }

    return 0;
}

#ifndef UDFREAD_BLOCK_INPUT_ONLY
int udfread_open(udfread *udf, const char *path)
{
    udfread_block_input *input;
    int result;

    if (!path) {
        return -1;
    }

    input = block_input_new(path);
    if (!input) {
        return -1;
    }

    result = udfread_open_input(udf, input);
    if (result < 0) {
        if (input->close) {
            input->close(input);
        }
    }

    return result;
}
#endif

void udfread_close(udfread *udf)
{
    if (udf) {
        if (udf->input) {
            if (udf->input->close) {
                udf->input->close(udf->input);
            }
            udf->input = NULL;
        }

        _free_dir(&udf->root_dir);
        free(udf->volume_identifier);
        free(udf);
    }
}

const char *udfread_get_volume_id(udfread *udf)
{
    if (udf) {
        return udf->volume_identifier;
    }
    return NULL;
}

size_t udfread_get_volume_set_id (udfread *udf, void *buffer, size_t size)
{
    if (udf) {
        if (size > sizeof(udf->volume_set_identifier)) {
            size = sizeof(udf->volume_set_identifier);
        }
        memcpy(buffer, udf->volume_set_identifier, size);
        return sizeof(udf->volume_set_identifier);
    }
    return 0;
}

/*
 * Directory access API
 */

struct udfread_dir {
    udfread              *udf;
    struct udf_dir       *dir;
    uint32_t              current_file;
};

static UDFDIR *_new_udfdir(udfread *udf, struct udf_dir *dir)
{
    UDFDIR *result;

    if (!dir) {
        return NULL;
    }

    result = (UDFDIR *)calloc(1, sizeof(UDFDIR));
    if (result) {
        result->dir = dir;
        result->udf = udf;
    }

    return result;
}

UDFDIR *udfread_opendir(udfread *udf, const char *path)
{
    struct udf_dir *dir = NULL;

    if (!udf || !udf->input || !path) {
        return NULL;
    }

    if (_find_file(udf, path, &dir, NULL) < 0) {
        return NULL;
    }

    return _new_udfdir(udf, dir);
}

UDFDIR *udfread_opendir_at(UDFDIR *p, const char *name)
{
    struct udf_dir *dir = NULL;
    uint32_t index;

    if (!p || !name) {
        return NULL;
    }

    if (_scan_dir(p->dir, name, &index) < 0) {
        udf_log("udfread_opendir_at: entry %s not found\n", name);
        return NULL;
    }

    dir = _read_subdir(p->udf, p->dir, index);

    return _new_udfdir(p->udf, dir);
}

struct udfread_dirent *udfread_readdir(UDFDIR *p, struct udfread_dirent *entry)
{
    const struct udf_file_identifier *fi;

    if (!p || !entry || !p->dir) {
        return NULL;
    }

    if (p->current_file >= p->dir->num_entries) {
        return NULL;
    }

    fi = &p->dir->files[p->current_file];

    entry->d_name = fi->filename;

    if (fi->characteristic & CHAR_FLAG_PARENT) {
        entry->d_type = UDF_DT_DIR;
        entry->d_name = "..";
    } else if (fi->characteristic & CHAR_FLAG_DIR) {
        entry->d_type = UDF_DT_DIR;
    } else {
        entry->d_type = UDF_DT_REG;
    }

    p->current_file++;

    return entry;
}

void udfread_rewinddir(UDFDIR *p)
{
    if (p) {
        p->current_file = 0;
    }
}

void udfread_closedir(UDFDIR *p)
{
    free(p);
}


/*
 * File access API
 */

struct udfread_file {
    udfread           *udf;
    struct file_entry *fe;

    /* byte stream access */
    uint64_t    pos;
    uint8_t    *block;
    int         block_valid;

    void       *block_mem;
};

static UDFFILE *_file_open(udfread *udf, const char *path, const struct udf_file_identifier *fi)
{
    struct file_entry *fe;
    UDFFILE *result;

    if (fi->characteristic & CHAR_FLAG_DIR) {
        udf_log("error opening file %s (is directory)\n", path);
        return NULL;
    }

    fe = _read_file_entry(udf, &fi->icb);
    if (!fe) {
        udf_error("error reading file entry for %s\n", path);
        return NULL;
    }

    /* Boxedwine exposes only regular byte streams. Reject invalid lengths and
       extents while preparing the mount, before replacing the current disc. */
    if (fe->file_type != ECMA_FT_BYTESTREAM || fe->length > (uint64_t)UINT32_MAX * UDF_BLOCK_SIZE) {
        free_file_entry(&fe);
        return NULL;
    }
    if (!fe->content_inline) {
        uint64_t covered = 0;
        uint32_t i;
        for (i = 0; i < fe->u.ads.num_ad; ++i) {
            const struct long_ad *ad = &fe->u.ads.ad[i];
            int part = _partition_index(udf, ad->partition);
            uint64_t end = part < 0 ? UINT64_MAX : (uint64_t)udf->part.p[part].lba +
                ad->lba + ((uint64_t)ad->length + UDF_BLOCK_SIZE - 1) / UDF_BLOCK_SIZE;
            if (part < 0 || ad->extent_type == ECMA_AD_EXTENT_AD ||
                (i + 1 < fe->u.ads.num_ad && ad->length % UDF_BLOCK_SIZE) ||
                (ad->extent_type == ECMA_AD_EXTENT_NORMAL && ad->length &&
                 (end > UINT32_MAX || (udf->input->size && end > udf->input->size(udf->input))))) {
                free_file_entry(&fe);
                return NULL;
            }
            covered += ad->length;
        }
        if (covered < fe->length) {
            free_file_entry(&fe);
            return NULL;
        }
    }

    result = (UDFFILE *)calloc(1, sizeof(UDFFILE));
    if (!result) {
        free_file_entry(&fe);
        return NULL;
    }

    result->udf = udf;
    result->fe  = fe;

    return result;
}

UDFFILE *udfread_file_open(udfread *udf, const char *path)
{
    const struct udf_file_identifier *fi = NULL;

    if (!udf || !udf->input || !path) {
        return NULL;
    }

    if (_find_file(udf, path, NULL, &fi) < 0) {
        return NULL;
    }

    return _file_open(udf, path, fi);
}

UDFFILE *udfread_file_openat(UDFDIR *dir, const char *name)
{
    uint32_t index;

    if (!dir || !name) {
        return NULL;
    }

    if (_scan_dir(dir->dir, name, &index) < 0) {
        udf_log("udfread_file_openat: entry %s not found\n", name);
        return NULL;
    }

    return _file_open(dir->udf, name, &dir->dir->files[index]);
}

int64_t udfread_file_size(UDFFILE *p)
{
    if (p) {
        return (int64_t)p->fe->length;
    }
    return -1;
}

void udfread_file_close(UDFFILE *p)
{
    if (p) {
        free_file_entry(&p->fe);
        free(p->block_mem);
        free(p);
    }
}

/*
 * block access
 */

static uint32_t _file_lba(UDFFILE *p, uint32_t file_block, uint32_t *extent_length)
{
    const struct file_entry *fe;
    unsigned int i;
    uint32_t     ad_size;

    fe = p->fe;

    for (i = 0; i < fe->u.ads.num_ad; i++) {
        const struct long_ad *ad = &fe->u.ads.ad[0];
        ad_size = (ad[i].length + UDF_BLOCK_SIZE - 1) / UDF_BLOCK_SIZE;
        if (file_block < ad_size) {

            if (ad[i].extent_type != ECMA_AD_EXTENT_NORMAL) {
                if (ad[i].extent_type == ECMA_AD_EXTENT_AD) {
                    udf_error("unsupported allocation descriptor: extent type %u\n", ad[i].extent_type);
                }
                return 0;
            }

            if (!ad[i].lba) {
                /* empty file / no allocated space */
                return 0;
            }

            if (ad[i].partition != p->udf->part.p[0].number) {
                udf_error("file partition %u != %u\n", ad[i].partition, p->udf->part.p[0].number);
            }

            if (extent_length) {
                *extent_length = ad_size - file_block;
            }
            return p->udf->part.p[0].lba + ad[i].lba + file_block;
        }

        file_block -= ad_size;
    }

    return 0;
}

static int _file_lba_exists(UDFFILE *p)
{
    if (!p) {
        return 0;
    }

    if (p->fe->content_inline) {
        udf_error("can't map lba for inline file\n");
        return 0;
    }

    return 1;
}

uint32_t udfread_file_lba(UDFFILE *p, uint32_t file_block)
{
    if (!_file_lba_exists(p)) {
        return 0;
    }

    return _file_lba(p, file_block, NULL);
}

uint32_t udfread_read_blocks(UDFFILE *p, void *buf, uint32_t file_block, uint32_t num_blocks, int flags)
{
    uint32_t i;

    if (!num_blocks || !buf) {
        return 0;
    }

    if (!_file_lba_exists(p)) {
        return 0;
    }

    for (i = 0; i < num_blocks; ) {
        uint32_t extent_length = 0;
        uint32_t lba;
        uint8_t *block = (uint8_t *)buf + UDF_BLOCK_SIZE * i;

        lba = _file_lba(p, file_block + i, &extent_length);
        udf_trace("map block %u to lba %u\n", file_block + i, lba);

        if (!lba) {
            /* unallocated/unwritten block or EOF */
            uint32_t file_blocks = (uint32_t)((udfread_file_size(p) + UDF_BLOCK_SIZE - 1) / UDF_BLOCK_SIZE);
            if (file_block + i < file_blocks) {
                udf_trace("zero-fill unallocated / unwritten block %u\n", file_block + i);
                memset(block, 0, UDF_BLOCK_SIZE);
                i++;
                continue;
            }
            udf_error("block %u outside of file (size %u blocks)\n", file_block + i, file_blocks);
            break;
        }

        if (extent_length > num_blocks - i) {
            extent_length = num_blocks - i;
        }

        extent_length = _read_blocks(p->udf->input, lba, block, extent_length, flags);
        if (extent_length < 1) {
            break;
        }
        i += extent_length;
    }
    return i;
}

/*
 * byte stream
 */

static ssize_t _read(UDFFILE *p, void *buf, size_t bytes)
{
    /* start from middle of block ?
     * maximal file size, i.e. position, is 2^32 * block size
     */

    size_t pos_off = p->pos % UDF_BLOCK_SIZE;
    uint32_t file_block = (uint32_t)(p->pos / UDF_BLOCK_SIZE);
    if (pos_off) {
        size_t chunk_size = UDF_BLOCK_SIZE - pos_off;
        if (!p->block_valid) {
            if (udfread_read_blocks(p, p->block, file_block, 1, 0) != 1) {
                return -1;
            }
            p->block_valid = 1;
        }
        if (chunk_size > bytes) {
            chunk_size = bytes;
        }
        memcpy(buf, p->block + pos_off, chunk_size);
        p->pos += (uint64_t)chunk_size;
        return (ssize_t)chunk_size;
    }

    /* read full block(s) ? */
    if (bytes >= UDF_BLOCK_SIZE) {
        uint32_t num_blocks = (uint32_t)(bytes / UDF_BLOCK_SIZE > UINT32_MAX ? UINT32_MAX : bytes / UDF_BLOCK_SIZE);
        num_blocks = udfread_read_blocks(p, buf, file_block, num_blocks, 0);
        if (num_blocks < 1) {
            return -1;
        }
        p->pos += num_blocks * UDF_BLOCK_SIZE;
        return num_blocks * UDF_BLOCK_SIZE;
    }

    /* read beginning of a block */
    if (udfread_read_blocks(p, p->block, file_block, 1, 0) != 1) {
        return -1;
    }
    p->block_valid = 1;
    memcpy(buf, p->block, bytes);
    p->pos += bytes;
    return (ssize_t)bytes;
}

static ssize_t _read_inline(UDFFILE *p, void *buf, size_t bytes)
{
    uint64_t information_length = p->fe->u.data.information_length;
    size_t   pad_size = 0;

    if (p->pos + bytes > information_length) {
        udf_log("read hits padding in inline file\n");
        if (p->pos > information_length) {
            pad_size = bytes;
        } else {
            pad_size = (size_t)(p->pos + bytes - information_length);
        }
        memset((char*)buf + bytes - pad_size, 0, pad_size);
    }

    if (pad_size < bytes) {
        memcpy(buf, &p->fe->u.data.content[p->pos], bytes - pad_size);
    }

    p->pos = p->pos + bytes;
    return (ssize_t)bytes;
}

#define ALIGN(p, align) \
  (uint8_t *)( ((uintptr_t)(p) + ((align)-1)) & ~((uintptr_t)((align)-1)))

ssize_t udfread_file_read(UDFFILE *p, void *buf, size_t bytes)
{
    uint8_t *bufpt = (uint8_t *)buf;

    /* sanity checks */
    if (!p || !buf) {
        return -1;
    }
    if ((ssize_t)bytes < 0 || (int64_t)bytes < 0) {
        return -1;
    }

    if (p->pos >= p->fe->length) {
        return 0;
    }

    /* limit range to file size */
    if (p->pos + bytes > p->fe->length) {
        bytes = (size_t)(p->fe->length - p->pos);
    }

    /* small files may be stored inline in file entry */
    if (p->fe->content_inline) {
        return _read_inline(p, buf, bytes);
    }

    /* allocate temp storage for input block */
    if (!p->block) {
        p->block_mem = malloc(2 * UDF_BLOCK_SIZE);
        if (!p->block_mem) {
            return -1;
        }
        p->block = ALIGN(p->block_mem, UDF_BLOCK_SIZE);
    }

    /* read chunks */
    while (bytes > 0) {
        ssize_t r = _read(p, bufpt, bytes);
        if (r < 0) {
            if (bufpt != buf) {
                /* got some bytes */
                break;
            }
            /* got nothing */
            return -1;
        }
        bufpt += r;
        bytes -= (size_t)r;
    }

    return (intptr_t)bufpt - (intptr_t)buf;
}

int64_t udfread_file_tell(UDFFILE *p)
{
    if (p) {
        return (int64_t)p->pos;
    }
    return -1;
}

int64_t udfread_file_seek(UDFFILE *p, int64_t pos, int whence)
{
    if (!p) {
        return -1;
    }

    switch (whence) {
        case UDF_SEEK_CUR:
            pos = udfread_file_tell(p) + pos;
            break;
        case UDF_SEEK_END:
            pos = udfread_file_size(p) + pos;
            break;
        case UDF_SEEK_SET:
            break;
        default:
            return -1;
    }

    if (pos >= 0 && pos <= udfread_file_size(p)) {
        p->pos = (uint64_t)pos;
        p->block_valid = 0;
        return udfread_file_tell(p);
    }

    return -1;
}
