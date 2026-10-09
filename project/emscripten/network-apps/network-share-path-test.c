/* Build as a freestanding i386 ELF with the same flags as the share apps.
 * Pass a fresh writable directory. Also compile with -DTEST_AGENT to exercise
 * the agent's inclusion of the common path helpers. */
#define _start unused_share_start
#ifdef TEST_AGENT
#include "network-share-agent.c"
#else
#include "network-share-join.c"
#endif
#undef _start

void* memcpy(void* dst, const void* src, u32 size) {
    u8* d = dst;
    const u8* s = src;
    u32 i;
    for (i = 0; i < size; i++) d[i] = s[i];
    return dst;
}

static void check(int ok, const char* message) {
    if (!ok) {
        print("FAIL: "); print(message); print("\n");
        sys1(SYS_EXIT, 1);
    }
}

static void path_test(int argc, char** argv) {
    const char* root = argc > 1 ? argv[1] : "/tmp/share-path-test";
    char path[1024];
    int fd;
    const char* invalid[] = { "", "/absolute", "../outside", "nested/../../outside",
        "nested/../outside", "./file", "dir//file", "dir/", "C:/file", "dir\\file" };
    int i;
    make_parent_dirs(root);
    make_dir(root);
    for (i = 0; i < (int)(sizeof(invalid) / sizeof(invalid[0])); i++) {
        check(share_open_file(root, invalid[i], O_WRONLY | O_CREAT | O_TRUNC, 0666) < 0, invalid[i]);
        check(share_make_dir(root, invalid[i]) < 0, invalid[i]);
    }
    check(share_make_dir(root, "nested/subdir") == 0, "nested directory");
    fd = share_open_file(root, "nested/subdir/good.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    check(fd >= 0, "valid file creation");
    check(sys3(SYS_WRITE, fd, (int)"OK", 2) == 2, "write valid file");
    sys1(SYS_CLOSE, fd);
    fd = share_open_file(root, "nested/subdir/good.txt", O_RDONLY, 0);
    check(fd >= 0, "valid file read");
    sys1(SYS_CLOSE, fd);
    join_path(path, (int)sizeof(path), root, "dir-link");
    sys2(SYS_SYMLINK, (int)"nested/subdir", (int)path);
    check(share_open_file(root, "dir-link/good.txt", O_WRONLY | O_TRUNC, 0) < 0, "directory symlink rejected");
    check(share_make_dir(root, "dir-link/new-dir") < 0, "mkdir through symlink rejected");
    join_path(path, (int)sizeof(path), root, "file-link");
    sys2(SYS_SYMLINK, (int)"nested/subdir/good.txt", (int)path);
    check(share_open_file(root, "file-link", O_WRONLY | O_TRUNC, 0) < 0, "file symlink rejected");
    fd = share_open_file(root, "nested/subdir/good.txt", O_RDONLY, 0);
    check(sys3(SYS_READ, fd, (int)path, 2) == 2 && path[0] == 'O' && path[1] == 'K', "symlink target preserved");
    sys1(SYS_CLOSE, fd);
    {
        const char archive[] = "BW-SHARE-ARCHIVE/1\n\n"
            "file path=../outside size=2\nNO\nendfile\n"
            "file path=file-link size=2\nNO\nendfile\n"
            "dir path=dir-link/rejected\n"
            "file path=nested/after.txt size=2\nOK\nendfile\nEND\n";
        memcpy(archive_buffer, archive, sizeof(archive));
#ifdef TEST_AGENT
        static RemoteShare remote;
        copy_n(remote.mirror_root, root, strlen0(root));
        parse_archive_into(&remote, sizeof(archive) - 1);
#else
        mirror_root = root;
        parse_archive(sizeof(archive) - 1);
#endif
        fd = share_open_file(root, "nested/after.txt", O_RDONLY, 0);
        check(fd >= 0, "archive continues after rejected entries");
        check(sys3(SYS_READ, fd, (int)path, 2) == 2 && path[0] == 'O' && path[1] == 'K', "archive payload remains aligned");
        sys1(SYS_CLOSE, fd);
        fd = share_open_file(root, "nested/subdir/good.txt", O_RDONLY, 0);
        check(fd >= 0 && sys3(SYS_READ, fd, (int)path, 2) == 2 && path[0] == 'O' && path[1] == 'K', "archive symlink target preserved");
        sys1(SYS_CLOSE, fd);
        check(share_open_file(root, "nested/subdir/rejected", O_RDONLY, 0) < 0, "archive directory symlink rejected");
    }
    print("SHARE PATH TEST PASSED\n");
    sys1(SYS_EXIT, 0);
}
NETWORK_SHARE_ENTRY(path_test, "network-share-path-test")
