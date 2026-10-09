/* Resolve received paths relative to an opened mirror directory. These helpers
 * use i386 Linux syscalls in both the ELF and import-free Wine PE builds. */
#ifndef NETWORK_SHARE_PATH_H
#define NETWORK_SHARE_PATH_H

#define SHARE_O_DIRECTORY 0x10000
#define SHARE_O_NOFOLLOW 0x20000

static int share_sys4(int n, int a, int b, int c, int d) {
    int r;
    __asm__ volatile("int $0x80" : "=a"(r) : "a"(n), "b"(a), "c"(b), "d"(c), "S"(d) : "memory");
    return r;
}

static int share_valid_path(const char* path) {
    int start = 0;
    int i;
    for (i = 0; i < 512; i++) {
        char c = path[i];
        if (c == '\\' || c == ':' || c == '\n' || c == '\r') return 0;
        if (!c || c == '/') {
            int len = i - start;
            if (!len || len > 255 || (len == 1 && path[start] == '.') ||
                (len == 2 && path[start] == '.' && path[start + 1] == '.')) return 0;
            if (!c) return 1;
            start = i + 1;
        }
    }
    return 0;
}

/* Return an owned parent fd and the final component. Open each component with
 * O_NOFOLLOW so an existing symlink cannot redirect a received path. */
static int share_open_parent(const char* root, const char* path, int create, char leaf[256]) {
    int fd;
    int pos = 0;
    if (!share_valid_path(path)) return -22;
    fd = sys3(SYS_OPEN, (int)root, O_RDONLY | SHARE_O_DIRECTORY | SHARE_O_NOFOLLOW, 0);
    if (fd < 0) return fd;
    for (;;) {
        int n = 0;
        int next;
        while (path[pos] && path[pos] != '/') leaf[n++] = path[pos++];
        leaf[n] = 0;
        if (!path[pos]) return fd;
        pos++;
        next = share_sys4(295, fd, (int)leaf, O_RDONLY | SHARE_O_DIRECTORY | SHARE_O_NOFOLLOW, 0);
        if (next == -2 && create) {
            sys3(296, fd, (int)leaf, 0777);
            next = share_sys4(295, fd, (int)leaf, O_RDONLY | SHARE_O_DIRECTORY | SHARE_O_NOFOLLOW, 0);
        }
        sys1(SYS_CLOSE, fd);
        if (next < 0) return next;
        fd = next;
    }
}

static int share_open_file(const char* root, const char* path, int flags, int mode) {
    char leaf[256];
    int parent = share_open_parent(root, path, (flags & O_CREAT) != 0, leaf);
    int fd;
    if (parent < 0) return parent;
    fd = share_sys4(295, parent, (int)leaf, flags | SHARE_O_NOFOLLOW, mode);
    sys1(SYS_CLOSE, parent);
    return fd;
}

static int share_make_dir(const char* root, const char* path) {
    char leaf[256];
    int parent = share_open_parent(root, path, 1, leaf);
    int fd;
    if (parent < 0) return parent;
    sys3(296, parent, (int)leaf, 0777);
    fd = share_sys4(295, parent, (int)leaf, O_RDONLY | SHARE_O_DIRECTORY | SHARE_O_NOFOLLOW, 0);
    sys1(SYS_CLOSE, parent);
    if (fd < 0) return fd;
    sys1(SYS_CLOSE, fd);
    return 0;
}

#endif
