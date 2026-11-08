# libudfread in Boxedwine

Vendored from VideoLAN libudfread **1.2.0**, tag commit
`c3cd5cbb097924557ea4d9da1ff76a74620c51a8`:
https://code.videolan.org/videolan/libudfread

The upstream LGPL-2.1-or-later notices and `COPYING` are retained. Boxedwine
compiles `src/udfread.c` and `src/ecma167.c` as C, with
`UDFREAD_BLOCK_INPUT_ONLY`. Image access uses Boxedwine's `DiscImage` callback;
the upstream default host-file opener and version helper are not linked.

Local changes:

- Provide `ssize_t` when compiling with MSVC.
- Guard the default host-file opener with `UDFREAD_BLOCK_INPUT_ONLY`.
- Use Windows pointer atomics without a lazily initialized global lock.
- Bound metadata allocations, directory entries and descriptor chains; reject
  malformed/truncated directory entries and failed allocation continuations.
- Ignore deleted directory entries. Parse the directory's information length;
  reject unsupported fragmented directories instead of publishing a partial tree.
- Validate regular-file sizes, partition references and allocation ranges before
  exposing them through the mounted filesystem. Sparse extents remain supported.
- Make narrowing conversions explicit after the corresponding size validation.

`source/io/fsudf.cpp` supplies the sector adapter and read-only filesystem nodes.
All library operations on a mounted disc use the disc's recursive read mutex.
The directory tree is validated before disc replacement, and open files retain
their original disc after eject. The adapter bounds directory depth, total
entries and metadata reads. File modification times currently default to zero;
the upstream public API does not expose timestamps.

The filesystem regression tests are in `source/test/fs/testCdrom.cpp`. Their
embedded UDF fixture is generated independently by
`tools/input/generate_udf_fixture.py` using `genisoimage -udf -J`.

## Mounting and scope

Use the existing ISO picker or `-cdrom image.iso e`. UDF is detected automatically
and preferred on ISO/UDF hybrid images. Wine sees the usual read-only CD-ROM
drive, with Linux's UDF filesystem type and 2048-byte sectors. No image conversion
or host filesystem mount is required.

This integration targets read-only data discs. UDF 1.02 is covered by the fixture
and a real 16,235,560,960-byte image containing 16,808 files and 1,020 directories.
Native Windows verification included Wine directory/volume queries and matching
file hashes at image offsets above 15 GB. It does not establish game installation
or playback compatibility, or browser support for uploading such large images.

The three UDF tests cover UDF-only/hybrid mounts, UTF-8 names, empty files, seeks,
fragmented/inline/sparse allocation, file lengths over 4 GiB, read-only access,
disc replacement/ejection, and rejection of malformed or cyclic descriptors.
The existing ISO/CUE regressions run alongside them. File timestamps and
fragmented directory streams are the limitations described above; UDF writing
and encrypted-disc handling are outside this integration.
