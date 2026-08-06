/*
 * Tiny 32-bit Linux read-only LAN share agent for Boxedwine networking.
 *
 * This combines the share host and join roles: each peer advertises and serves
 * one local read-only folder while also discovering and mirroring remote shares
 * into automatically assigned Wine drive letters.
 */

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

#define SYS_EXIT 1
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_UNLINK 10
#define SYS_LSEEK 19
#define SYS_RMDIR 40
#define SYS_MKDIR 39
#define SYS_FCNTL 55
#define SYS_SYMLINK 83
#define SYS_SOCKETCALL 102
#define SYS_NANOSLEEP 162
#define SYS_GETDENTS 141

#define SC_SOCKET 1
#define SC_BIND 2
#define SC_CONNECT 3
#define SC_LISTEN 4
#define SC_ACCEPT 5
#define SC_SEND 9
#define SC_RECV 10
#define SC_RECVFROM 12
#define SC_SENDTO 11

#define AF_INET 2
#define SOCK_STREAM 1
#define SOCK_DGRAM 2
#define IPPROTO_UDP 17
#define F_SETFL 4
#define O_RDONLY 0
#define O_WRONLY 1
#define O_CREAT 0x40
#define O_TRUNC 0x200
#define O_NONBLOCK 0x800
#define SEEK_SET 0
#define SEEK_END 2
#define MAX_REMOTES 8
#define ARCHIVE_BUFFER_SIZE (8 * 1024 * 1024)
#define MANIFEST_BUFFER_SIZE (256 * 1024)
#define RECV_CHUNK_SIZE 4096

typedef struct RemoteShare {
    int used;
    char raw_name[64];
    char safe_name[64];
    char mirror_root[256];
    u8 ip[4];
    u32 port;
    char drive;
    int first_sync;
    int have_hash;
    u32 last_hash;
    int poll_now;
    u32 defer_ticks;
} RemoteShare;

static const char default_share_root[] = "/home/username/.wine/dosdevices/c:/host";
static const char default_share_name[] = "c-host";
static const char default_share_drive[] = "c";
static const char default_share_path[] = "host";
static const char default_share_mode[] = "read-only";
static const char default_mirror_base[] = "/home/username/.wine/dosdevices/c:/share-mirror";
static const char drive_prefix[] = "/home/username/.wine/dosdevices/";
static const char archive_request[] = "GET archive BW-SHARE/1\n\n";
static const char manifest_request[] = "GET manifest BW-SHARE/1\n\n";
static const char file_data_request_prefix[] = "GET file-data path=";
static const u32 MANIFEST_RECV_CHUNK_SIZE = 256;

static const char* share_root = default_share_root;
static const char* share_name = default_share_name;
static const char* share_drive = default_share_drive;
static const char* share_path = default_share_path;
static const char* share_mode = default_share_mode;
static const char* mirror_base = default_mirror_base;
static u32 listen_port = 19200;
static u32 beacon_port = 19201;
static u32 poll_seconds = 30;
static u8 broadcast_ip[4] = {10, 0, 3, 255};
static u32 serve_cooldown_ticks = 0;
static const u32 file_serve_cooldown_ticks = 4;

static RemoteShare remotes[MAX_REMOTES];
static char dents_buffer[4096];
static char probe_buffer[256];
static char file_buffer[8192];
static char line_buffer[1024];
static char path_buffer[768];
static char second_path_buffer[768];
static char manifest_buffer[MANIFEST_BUFFER_SIZE];
static char archive_buffer[ARCHIVE_BUFFER_SIZE];
static int manifest_len = 0;
static int manifest_build_truncated = 0;

static int sys1(int n, int a) {
    int r;
    __asm__ volatile("int $0x80" : "=a"(r) : "a"(n), "b"(a) : "memory");
    return r;
}

static int sys2(int n, int a, int b) {
    int r;
    __asm__ volatile("int $0x80" : "=a"(r) : "a"(n), "b"(a), "c"(b) : "memory");
    return r;
}

static int sys3(int n, int a, int b, int c) {
    int r;
    __asm__ volatile("int $0x80" : "=a"(r) : "a"(n), "b"(a), "c"(b), "d"(c) : "memory");
    return r;
}

static int socketcall(int call, u32* args) {
    return sys2(SYS_SOCKETCALL, call, (int)args);
}

#include "network-share-entry.h"

static int strlen0(const char* s) {
    int n = 0;
    while (s[n]) n++;
    return n;
}

static int streq(const char* a, const char* b) {
    int i = 0;
    while (a[i] && b[i] && a[i] == b[i]) i++;
    return a[i] == b[i];
}

static int strcmp0(const char* a, const char* b) {
    int i = 0;
    while (a[i] && b[i] && a[i] == b[i]) i++;
    return ((int)(unsigned char)a[i]) - ((int)(unsigned char)b[i]);
}

static int starts_with(const char* s, const char* prefix) {
    int i = 0;
    while (prefix[i]) {
        if (s[i] != prefix[i]) return 0;
        i++;
    }
    return 1;
}

static int find_token(const char* s, const char* token) {
    int i = 0;
    int j;
    while (s[i]) {
        j = 0;
        while (token[j] && s[i + j] == token[j]) j++;
        if (!token[j]) return i;
        i++;
    }
    return -1;
}

static int skip_dirent_name(const char* name) {
    return !name[0] || streq(name, ".") || streq(name, "..");
}

static void print(const char* s) {
    sys3(SYS_WRITE, 1, (int)s, strlen0(s));
}

static void append_char(char* dst, int* pos, char c) {
    dst[*pos] = c;
    *pos = *pos + 1;
}

static void append_str(char* dst, int* pos, const char* src) {
    int i = 0;
    while (src[i]) {
        append_char(dst, pos, src[i]);
        i++;
    }
}

static void append_uint(char* dst, int* pos, u32 value) {
    char tmp[12];
    int n = 0;
    if (value == 0) {
        append_char(dst, pos, '0');
        return;
    }
    while (value) {
        tmp[n++] = (char)('0' + (value % 10));
        value /= 10;
    }
    while (n) append_char(dst, pos, tmp[--n]);
}

static void print_uint(u32 value) {
    char tmp[12];
    int pos = 0;
    append_uint(tmp, &pos, value);
    tmp[pos] = 0;
    print(tmp);
}

static u32 parse_uint(const char* s) {
    u32 value = 0;
    while (*s >= '0' && *s <= '9') {
        value = value * 10 + (u32)(*s - '0');
        s++;
    }
    return value;
}

static int parse_ip(const char* s, u8 out[4]) {
    int part = 0;
    int saw_digit = 0;
    u32 value = 0;
    while (*s) {
        if (*s >= '0' && *s <= '9') {
            value = value * 10 + (u32)(*s - '0');
            if (value > 255) return 0;
            saw_digit = 1;
        } else if (*s == '.') {
            if (!saw_digit || part >= 3) return 0;
            out[part++] = (u8)value;
            value = 0;
            saw_digit = 0;
        } else {
            return 0;
        }
        s++;
    }
    if (!saw_digit || part != 3) return 0;
    out[part] = (u8)value;
    return 1;
}

static int option_value(int* index, int argc, char** argv, const char* option, const char** value) {
    const char* arg = argv[*index];
    int len = strlen0(option);
    if (streq(arg, option)) {
        if (*index + 1 >= argc) return 0;
        *index = *index + 1;
        *value = argv[*index];
        return 1;
    }
    if (starts_with(arg, option) && arg[len] == '=') {
        *value = arg + len + 1;
        return 1;
    }
    return 0;
}

static void parse_args(int argc, char** argv) {
    int i;
    for (i = 1; i < argc; i++) {
        const char* value = 0;
        if (option_value(&i, argc, argv, "--root", &value) || option_value(&i, argc, argv, "--share-root", &value)) {
            share_root = value;
        } else if (option_value(&i, argc, argv, "--name", &value) || option_value(&i, argc, argv, "--share-name", &value)) {
            share_name = value;
        } else if (option_value(&i, argc, argv, "--drive", &value)) {
            share_drive = value;
        } else if (option_value(&i, argc, argv, "--path", &value)) {
            share_path = value;
        } else if (option_value(&i, argc, argv, "--mode", &value)) {
            share_mode = value;
        } else if (option_value(&i, argc, argv, "--port", &value) || option_value(&i, argc, argv, "--listen-port", &value)) {
            listen_port = parse_uint(value);
        } else if (option_value(&i, argc, argv, "--beacon-port", &value)) {
            beacon_port = parse_uint(value);
        } else if (option_value(&i, argc, argv, "--broadcast", &value)) {
            parse_ip(value, broadcast_ip);
        } else if (option_value(&i, argc, argv, "--mirror-base", &value) || option_value(&i, argc, argv, "--target", &value)) {
            mirror_base = value;
        } else if (option_value(&i, argc, argv, "--poll-seconds", &value) || option_value(&i, argc, argv, "--poll", &value)) {
            poll_seconds = parse_uint(value);
        }
    }
}

static void copy_str(char* dst, const char* src) {
    int i = 0;
    while (src[i]) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;
}

static void copy_n(char* dst, const char* src, int n) {
    int i;
    for (i = 0; i < n; i++) dst[i] = src[i];
    dst[n] = 0;
}

static void join_path(char* dst, const char* base, const char* rel) {
    int pos = 0;
    append_str(dst, &pos, base);
    if (pos > 0 && dst[pos - 1] != '/') append_char(dst, &pos, '/');
    append_str(dst, &pos, rel);
    dst[pos] = 0;
}

static void make_dir(const char* path) {
    sys3(SYS_MKDIR, (int)path, 0777, 0);
}

static void make_parent_dirs(const char* path) {
    char tmp[768];
    int i = 0;
    int last_slash = -1;
    while (path[i]) {
        tmp[i] = path[i];
        if (path[i] == '/') last_slash = i;
        i++;
    }
    tmp[i] = 0;
    if (last_slash <= 0) return;
    tmp[last_slash] = 0;
    for (i = 1; tmp[i]; i++) {
        if (tmp[i] == '/') {
            tmp[i] = 0;
            make_dir(tmp);
            tmp[i] = '/';
        }
    }
    make_dir(tmp);
}

static int path_is_dir(const char* path) {
    int fd = sys3(SYS_OPEN, (int)path, O_RDONLY, 0);
    int result;
    if (fd < 0) return 0;
    result = sys3(SYS_GETDENTS, fd, (int)probe_buffer, (int)sizeof(probe_buffer));
    sys1(SYS_CLOSE, fd);
    return result >= 0;
}

static void remove_tree(const char* path, int depth);

static u32 fnv1a_update(u32 hash, const char* data, int len) {
    int i;
    for (i = 0; i < len; i++) {
        hash ^= (u8)data[i];
        hash *= 16777619u;
    }
    return hash;
}

static void hash_text(u32* hash, const char* text) {
    *hash = fnv1a_update(*hash, text, strlen0(text));
}

static void hash_uint(u32* hash, u32 value) {
    char tmp[12];
    int pos = 0;
    append_uint(tmp, &pos, value);
    *hash = fnv1a_update(*hash, tmp, pos);
}

static u32 calculate_file_hash(const char* full_path, u32* size_out) {
    u32 hash = 2166136261u;
    int fd = sys3(SYS_OPEN, (int)full_path, O_RDONLY, 0);
    int size;
    int remaining;
    *size_out = 0;
    if (fd < 0) return hash;
    size = sys3(SYS_LSEEK, fd, 0, SEEK_END);
    if (size < 0) {
        sys1(SYS_CLOSE, fd);
        return hash;
    }
    *size_out = (u32)size;
    sys3(SYS_LSEEK, fd, 0, SEEK_SET);
    remaining = size;
    while (remaining > 0) {
        int want = remaining > (int)sizeof(file_buffer) ? (int)sizeof(file_buffer) : remaining;
        int got = sys3(SYS_READ, fd, (int)file_buffer, want);
        if (got <= 0) break;
        hash = fnv1a_update(hash, file_buffer, got);
        remaining -= got;
    }
    sys1(SYS_CLOSE, fd);
    return hash;
}

static void hash_file(u32* hash, const char* full_path, const char* rel_path) {
    u32 size;
    u32 file_hash = calculate_file_hash(full_path, &size);
    hash_text(hash, "file:");
    hash_text(hash, rel_path);
    hash_text(hash, ":");
    hash_uint(hash, size);
    hash_text(hash, ":");
    hash_uint(hash, file_hash);
    hash_text(hash, "\n");
}

static int send_all(int fd, const char* data, int len) {
    int sent = 0;
    while (sent < len) {
        u32 args[4];
        int r;
        args[0] = (u32)fd;
        args[1] = (u32)(data + sent);
        args[2] = (u32)(len - sent);
        args[3] = 0;
        r = socketcall(SC_SEND, args);
        if (r <= 0) return r;
        sent += r;
    }
    return sent;
}

static void send_text(int fd, const char* text) {
    send_all(fd, text, strlen0(text));
}

static void send_file(int client_fd, const char* full_path, const char* rel_path) {
    int fd = sys3(SYS_OPEN, (int)full_path, O_RDONLY, 0);
    int size;
    int pos;
    if (fd < 0) return;
    size = sys3(SYS_LSEEK, fd, 0, SEEK_END);
    if (size < 0) {
        sys1(SYS_CLOSE, fd);
        return;
    }
    sys3(SYS_LSEEK, fd, 0, SEEK_SET);
    pos = 0;
    append_str(line_buffer, &pos, "file path=");
    append_str(line_buffer, &pos, rel_path);
    append_str(line_buffer, &pos, " size=");
    append_uint(line_buffer, &pos, (u32)size);
    append_char(line_buffer, &pos, '\n');
    line_buffer[pos] = 0;
    send_all(client_fd, line_buffer, pos);
    while (size > 0) {
        int want = size > RECV_CHUNK_SIZE ? RECV_CHUNK_SIZE : size;
        int got = sys3(SYS_READ, fd, (int)file_buffer, want);
        if (got <= 0) break;
        send_all(client_fd, file_buffer, got);
        size -= got;
    }
    send_text(client_fd, "\nendfile\n");
    sys1(SYS_CLOSE, fd);
}

static void hash_dir_tree(u32* hash, const char* full_dir, const char* rel_dir, int depth) {
    char local_dents[4096];
    int fd;
    if (depth > 6) return;
    fd = sys3(SYS_OPEN, (int)full_dir, O_RDONLY, 0);
    if (fd < 0) return;
    if (rel_dir[0]) {
        hash_text(hash, "dir:");
        hash_text(hash, rel_dir);
        hash_text(hash, "\n");
    }
    for (;;) {
        int nread = sys3(SYS_GETDENTS, fd, (int)local_dents, (int)sizeof(local_dents));
        int offset = 0;
        if (nread <= 0) break;
        while (offset < nread) {
            u16 reclen = *(u16*)(local_dents + offset + 8);
            char* name = local_dents + offset + 10;
            char child_full[512];
            char child_rel[512];
            if (!reclen) break;
            if (!skip_dirent_name(name)) {
                join_path(child_full, full_dir, name);
                if (rel_dir[0]) join_path(child_rel, rel_dir, name);
                else copy_str(child_rel, name);
                if (path_is_dir(child_full)) hash_dir_tree(hash, child_full, child_rel, depth + 1);
                else hash_file(hash, child_full, child_rel);
            }
            offset += reclen;
        }
    }
    sys1(SYS_CLOSE, fd);
}

static u32 calculate_share_hash(void) {
    u32 hash = 2166136261u;
    hash_text(&hash, share_name);
    hash_text(&hash, "\n");
    hash_dir_tree(&hash, share_root, "", 0);
    return hash;
}

static void walk_dir(int client_fd, const char* full_dir, const char* rel_dir, int depth) {
    int fd;
    if (depth > 6) return;
    fd = sys3(SYS_OPEN, (int)full_dir, O_RDONLY, 0);
    if (fd < 0) return;
    for (;;) {
        int nread = sys3(SYS_GETDENTS, fd, (int)dents_buffer, (int)sizeof(dents_buffer));
        int offset = 0;
        if (nread <= 0) break;
        while (offset < nread) {
            u16 reclen = *(u16*)(dents_buffer + offset + 8);
            char* name = dents_buffer + offset + 10;
            char child_full[512];
            char child_rel[512];
            if (!reclen) break;
            if (!skip_dirent_name(name)) {
                join_path(child_full, full_dir, name);
                if (rel_dir[0]) join_path(child_rel, rel_dir, name);
                else copy_str(child_rel, name);
                if (!path_is_dir(child_full)) send_file(client_fd, child_full, child_rel);
            }
            offset += reclen;
        }
    }
    sys1(SYS_CLOSE, fd);
    fd = sys3(SYS_OPEN, (int)full_dir, O_RDONLY, 0);
    if (fd < 0) return;
    for (;;) {
        int nread = sys3(SYS_GETDENTS, fd, (int)dents_buffer, (int)sizeof(dents_buffer));
        int offset = 0;
        if (nread <= 0) break;
        while (offset < nread) {
            u16 reclen = *(u16*)(dents_buffer + offset + 8);
            char* name = dents_buffer + offset + 10;
            char child_full[512];
            char child_rel[512];
            if (!reclen) break;
            if (!skip_dirent_name(name)) {
                join_path(child_full, full_dir, name);
                if (rel_dir[0]) join_path(child_rel, rel_dir, name);
                else copy_str(child_rel, name);
                if (path_is_dir(child_full)) {
                    int pos = 0;
                    append_str(line_buffer, &pos, "dir path=");
                    append_str(line_buffer, &pos, child_rel);
                    append_char(line_buffer, &pos, '\n');
                    line_buffer[pos] = 0;
                    send_all(client_fd, line_buffer, pos);
                    walk_dir(client_fd, child_full, child_rel, depth + 1);
                }
            }
            offset += reclen;
        }
    }
    sys1(SYS_CLOSE, fd);
}

static void send_manifest_file(int client_fd, const char* full_path, const char* rel_path) {
    int pos = 0;
    u32 size;
    u32 hash = calculate_file_hash(full_path, &size);
    append_str(line_buffer, &pos, "file path=");
    append_str(line_buffer, &pos, rel_path);
    append_str(line_buffer, &pos, " size=");
    append_uint(line_buffer, &pos, size);
    append_str(line_buffer, &pos, " hash=");
    append_uint(line_buffer, &pos, hash);
    append_char(line_buffer, &pos, '\n');
    line_buffer[pos] = 0;
    send_all(client_fd, line_buffer, pos);
}

static void send_manifest_entries(int client_fd, const char* full_dir, const char* rel_dir, int depth) {
    int fd;
    if (depth > 6) return;
    fd = sys3(SYS_OPEN, (int)full_dir, O_RDONLY, 0);
    if (fd < 0) return;
    for (;;) {
        int nread = sys3(SYS_GETDENTS, fd, (int)dents_buffer, (int)sizeof(dents_buffer));
        int offset = 0;
        if (nread <= 0) break;
        while (offset < nread) {
            u16 reclen = *(u16*)(dents_buffer + offset + 8);
            char* name = dents_buffer + offset + 10;
            char child_full[512];
            char child_rel[512];
            if (!reclen) break;
            if (!skip_dirent_name(name)) {
                join_path(child_full, full_dir, name);
                if (rel_dir[0]) join_path(child_rel, rel_dir, name);
                else copy_str(child_rel, name);
                if (!path_is_dir(child_full)) send_manifest_file(client_fd, child_full, child_rel);
            }
            offset += reclen;
        }
    }
    sys1(SYS_CLOSE, fd);
    fd = sys3(SYS_OPEN, (int)full_dir, O_RDONLY, 0);
    if (fd < 0) return;
    for (;;) {
        int nread = sys3(SYS_GETDENTS, fd, (int)dents_buffer, (int)sizeof(dents_buffer));
        int offset = 0;
        if (nread <= 0) break;
        while (offset < nread) {
            u16 reclen = *(u16*)(dents_buffer + offset + 8);
            char* name = dents_buffer + offset + 10;
            char child_full[512];
            char child_rel[512];
            if (!reclen) break;
            if (!skip_dirent_name(name)) {
                join_path(child_full, full_dir, name);
                if (rel_dir[0]) join_path(child_rel, rel_dir, name);
                else copy_str(child_rel, name);
                if (path_is_dir(child_full)) {
                    int pos = 0;
                    append_str(line_buffer, &pos, "dir path=");
                    append_str(line_buffer, &pos, child_rel);
                    append_char(line_buffer, &pos, '\n');
                    line_buffer[pos] = 0;
                    send_all(client_fd, line_buffer, pos);
                    send_manifest_entries(client_fd, child_full, child_rel, depth + 1);
                }
            }
            offset += reclen;
        }
    }
    sys1(SYS_CLOSE, fd);
}

static void append_manifest_char(char c) {
    if (manifest_len < (int)sizeof(manifest_buffer) - 1) {
        manifest_buffer[manifest_len++] = c;
        manifest_buffer[manifest_len] = 0;
    } else {
        manifest_build_truncated = 1;
    }
}

static void append_manifest_str(const char* src) {
    int i = 0;
    while (src[i]) append_manifest_char(src[i++]);
}

static void append_manifest_uint(u32 value) {
    char tmp[12];
    int pos = 0;
    append_uint(tmp, &pos, value);
    tmp[pos] = 0;
    append_manifest_str(tmp);
}

static void build_manifest_file(const char* full_path, const char* rel_path) {
    u32 size;
    u32 hash = calculate_file_hash(full_path, &size);
    append_manifest_str("file path=");
    append_manifest_str(rel_path);
    append_manifest_str(" size=");
    append_manifest_uint(size);
    append_manifest_str(" hash=");
    append_manifest_uint(hash);
    append_manifest_char('\n');
}

static void build_manifest_entries(const char* full_dir, const char* rel_dir, int depth) {
    int fd;
    if (depth > 6 || manifest_build_truncated) return;
    fd = sys3(SYS_OPEN, (int)full_dir, O_RDONLY, 0);
    if (fd < 0) return;
    for (;;) {
        int nread = sys3(SYS_GETDENTS, fd, (int)dents_buffer, (int)sizeof(dents_buffer));
        int offset = 0;
        if (nread <= 0) break;
        while (offset < nread) {
            u16 reclen = *(u16*)(dents_buffer + offset + 8);
            char* name = dents_buffer + offset + 10;
            char child_full[512];
            char child_rel[512];
            if (!reclen || manifest_build_truncated) break;
            if (!skip_dirent_name(name)) {
                join_path(child_full, full_dir, name);
                if (rel_dir[0]) join_path(child_rel, rel_dir, name);
                else copy_str(child_rel, name);
                if (!path_is_dir(child_full)) build_manifest_file(child_full, child_rel);
            }
            offset += reclen;
        }
    }
    sys1(SYS_CLOSE, fd);
    fd = sys3(SYS_OPEN, (int)full_dir, O_RDONLY, 0);
    if (fd < 0) return;
    for (;;) {
        int nread = sys3(SYS_GETDENTS, fd, (int)dents_buffer, (int)sizeof(dents_buffer));
        int offset = 0;
        if (nread <= 0) break;
        while (offset < nread) {
            u16 reclen = *(u16*)(dents_buffer + offset + 8);
            char* name = dents_buffer + offset + 10;
            char child_full[512];
            char child_rel[512];
            if (!reclen || manifest_build_truncated) break;
            if (!skip_dirent_name(name)) {
                join_path(child_full, full_dir, name);
                if (rel_dir[0]) join_path(child_rel, rel_dir, name);
                else copy_str(child_rel, name);
                if (path_is_dir(child_full)) {
                    append_manifest_str("dir path=");
                    append_manifest_str(child_rel);
                    append_manifest_char('\n');
                    build_manifest_entries(child_full, child_rel, depth + 1);
                }
            }
            offset += reclen;
        }
    }
    sys1(SYS_CLOSE, fd);
}

static void send_archive_header(int client_fd) {
    send_text(client_fd, "BW-SHARE-ARCHIVE/1\nname=");
    send_text(client_fd, share_name);
    send_text(client_fd, "\nroot=");
    send_text(client_fd, share_root);
    send_text(client_fd, "\ndrive=");
    send_text(client_fd, share_drive);
    send_text(client_fd, "\npath=");
    send_text(client_fd, share_path);
    send_text(client_fd, "\nmode=");
    send_text(client_fd, share_mode);
    send_text(client_fd, "\nentries=recursive\n\n");
}

static void send_archive(int client_fd) {
    send_archive_header(client_fd);
    walk_dir(client_fd, share_root, "", 0);
    send_text(client_fd, "END\n");
}

static int safe_rel_path(const char* path) {
    int i = 0;
    if (!path[0] || path[0] == '/') return 0;
    while (path[i]) {
        if (path[i] == '.' && path[i + 1] == '.' && (!i || path[i - 1] == '/') && (!path[i + 2] || path[i + 2] == '/')) return 0;
        i++;
    }
    return 1;
}

static void send_one_file_archive(int client_fd, const char* rel_path) {
    char clean_path[512];
    int i = 0;
    while (rel_path[i] && rel_path[i] != '\r' && rel_path[i] != '\n' && i < (int)sizeof(clean_path) - 1) {
        clean_path[i] = rel_path[i];
        i++;
    }
    clean_path[i] = 0;
    send_archive_header(client_fd);
    if (safe_rel_path(clean_path)) {
        join_path(path_buffer, share_root, clean_path);
        if (!path_is_dir(path_buffer)) send_file(client_fd, path_buffer, clean_path);
    }
    send_text(client_fd, "END\n");
}

static void send_file_data(int client_fd, const char* rel_path) {
    char clean_path[512];
    int i = 0;
    int fd;
    int size;
    int pos = 0;
    while (rel_path[i] && rel_path[i] != '\r' && rel_path[i] != '\n' && i < (int)sizeof(clean_path) - 1) {
        clean_path[i] = rel_path[i];
        i++;
    }
    clean_path[i] = 0;
    if (!safe_rel_path(clean_path)) {
        send_text(client_fd, "BW-SHARE-FILE/1\nstatus=bad-path\nsize=0\n\n");
        return;
    }
    join_path(path_buffer, share_root, clean_path);
    if (path_is_dir(path_buffer)) {
        send_text(client_fd, "BW-SHARE-FILE/1\nstatus=not-file\nsize=0\n\n");
        return;
    }
    fd = sys3(SYS_OPEN, (int)path_buffer, O_RDONLY, 0);
    if (fd < 0) {
        send_text(client_fd, "BW-SHARE-FILE/1\nstatus=missing\nsize=0\n\n");
        return;
    }
    size = sys3(SYS_LSEEK, fd, 0, SEEK_END);
    if (size < 0) {
        sys1(SYS_CLOSE, fd);
        send_text(client_fd, "BW-SHARE-FILE/1\nstatus=stat-failed\nsize=0\n\n");
        return;
    }
    sys3(SYS_LSEEK, fd, 0, SEEK_SET);
    append_str(line_buffer, &pos, "BW-SHARE-FILE/1\nstatus=ok\npath=");
    append_str(line_buffer, &pos, clean_path);
    append_str(line_buffer, &pos, "\nsize=");
    append_uint(line_buffer, &pos, (u32)size);
    append_str(line_buffer, &pos, "\n\n");
    line_buffer[pos] = 0;
    send_all(client_fd, line_buffer, pos);
    while (size > 0) {
        int want = size > RECV_CHUNK_SIZE ? RECV_CHUNK_SIZE : size;
        int got = sys3(SYS_READ, fd, (int)file_buffer, want);
        if (got <= 0) break;
        send_all(client_fd, file_buffer, got);
        size -= got;
    }
    sys1(SYS_CLOSE, fd);
}

static void send_manifest(int client_fd) {
    int pos = 0;
    u32 hash = calculate_share_hash();
    manifest_len = 0;
    manifest_build_truncated = 0;
    append_manifest_str("BW-SHARE-MANIFEST/1\nname=");
    append_manifest_str(share_name);
    append_manifest_str("\nhash=");
    append_manifest_uint(hash);
    append_manifest_str("\nentries=recursive\n\n");
    build_manifest_entries(share_root, "", 0);
    append_manifest_str("END\n");
    append_str(line_buffer, &pos, "BW-SHARE-MANIFEST-DATA/1\nsize=");
    append_uint(line_buffer, &pos, (u32)manifest_len);
    append_str(line_buffer, &pos, "\n\n");
    line_buffer[pos] = 0;
    send_all(client_fd, line_buffer, pos);
    send_all(client_fd, manifest_buffer, manifest_len);
}

static void remove_tree(const char* path, int depth) {
    char local_dents[4096];
    int fd;
    if (depth > 8) {
        sys1(SYS_UNLINK, (int)path);
        sys1(SYS_RMDIR, (int)path);
        return;
    }
    if (!path_is_dir(path)) {
        sys1(SYS_UNLINK, (int)path);
        return;
    }
    fd = sys3(SYS_OPEN, (int)path, O_RDONLY, 0);
    if (fd >= 0) {
        for (;;) {
            int nread = sys3(SYS_GETDENTS, fd, (int)local_dents, (int)sizeof(local_dents));
            int offset = 0;
            if (nread <= 0) break;
            while (offset < nread) {
                u16 reclen = *(u16*)(local_dents + offset + 8);
                char* name = local_dents + offset + 10;
                char child_path[768];
                if (!reclen) break;
                if (!skip_dirent_name(name)) {
                    join_path(child_path, path, name);
                    remove_tree(child_path, depth + 1);
                }
                offset += reclen;
            }
        }
        sys1(SYS_CLOSE, fd);
    }
    sys1(SYS_RMDIR, (int)path);
}

static char* next_line(char** cursor, char* end) {
    char* line = *cursor;
    char* p = line;
    if (line >= end) return 0;
    while (p < end && *p != '\n') p++;
    if (p < end) {
        *p = 0;
        *cursor = p + 1;
    } else {
        *cursor = end;
    }
    return line;
}

static int buffer_has_end_marker(const char* data, int len) {
    if (len < 4) return 0;
    if (data[len - 4] != 'E' || data[len - 3] != 'N' || data[len - 2] != 'D' || data[len - 1] != '\n') return 0;
    return len == 4 || data[len - 5] == '\n';
}

static int path_field_matches(const char* line, const char* prefix, const char* rel_path) {
    int i = 0;
    int start = strlen0(prefix);
    if (!starts_with(line, prefix)) return 0;
    while (rel_path[i] && line[start + i] == rel_path[i]) i++;
    if (rel_path[i]) return 0;
    return line[start + i] == 0 || line[start + i] == '\n' || line[start + i] == ' ';
}

static int manifest_contains_path(const char* rel_path, int is_dir) {
    char* cursor = manifest_buffer;
    char* end = manifest_buffer + manifest_len;
    const char* prefix = is_dir ? "dir path=" : "file path=";
    while (cursor < end) {
        char* line = cursor;
        while (cursor < end && *cursor && *cursor != '\n') cursor++;
        if (path_field_matches(line, prefix, rel_path)) return 1;
        cursor++;
    }
    return 0;
}

static void prune_deleted_entries(RemoteShare* remote, const char* full_dir, const char* rel_dir, int depth) {
    char local_dents[4096];
    int fd;
    if (depth > 8) return;
    fd = sys3(SYS_OPEN, (int)full_dir, O_RDONLY, 0);
    if (fd < 0) return;
    for (;;) {
        int nread = sys3(SYS_GETDENTS, fd, (int)local_dents, (int)sizeof(local_dents));
        int offset = 0;
        if (nread <= 0) break;
        while (offset < nread) {
            u16 reclen = *(u16*)(local_dents + offset + 8);
            char* name = local_dents + offset + 10;
            char child_full[768];
            char child_rel[512];
            int is_dir;
            if (!reclen) break;
            if (!skip_dirent_name(name)) {
                join_path(child_full, full_dir, name);
                if (rel_dir[0]) join_path(child_rel, rel_dir, name);
                else copy_str(child_rel, name);
                is_dir = path_is_dir(child_full);
                if (!manifest_contains_path(child_rel, is_dir)) {
                    remove_tree(child_full, 0);
                    print("network-share-agent: pruned ");
                    print(remote->safe_name);
                    print("/");
                    print(child_rel);
                    print("\n");
                } else if (is_dir) {
                    prune_deleted_entries(remote, child_full, child_rel, depth + 1);
                }
            }
            offset += reclen;
        }
    }
    sys1(SYS_CLOSE, fd);
}

static void parse_archive_into(RemoteShare* remote, int len) {
    char* cursor = archive_buffer;
    char* end = archive_buffer + len;
    char* line;
    make_dir("/home/username/.wine/dosdevices/c:");
    make_parent_dirs(remote->mirror_root);
    make_dir(remote->mirror_root);
    line = next_line(&cursor, end);
    if (!line || !starts_with(line, "BW-SHARE-ARCHIVE/1")) {
        print("network-share-agent: archive header missing\n");
        return;
    }
    while ((line = next_line(&cursor, end))) {
        if (!line[0]) break;
    }
    while ((line = next_line(&cursor, end))) {
        if (starts_with(line, "END")) break;
        if (starts_with(line, "dir path=")) {
            join_path(path_buffer, remote->mirror_root, line + 9);
            make_parent_dirs(path_buffer);
            make_dir(path_buffer);
        } else if (starts_with(line, "file path=")) {
            int size_pos = find_token(line, " size=");
            if (size_pos > 10) {
                char rel_path[512];
                u32 size;
                int fd;
                copy_n(rel_path, line + 10, size_pos - 10);
                size = parse_uint(line + size_pos + 6);
                if (cursor + size <= end) {
                    join_path(path_buffer, remote->mirror_root, rel_path);
                    make_parent_dirs(path_buffer);
                    fd = sys3(SYS_OPEN, (int)path_buffer, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    if (fd >= 0) {
                        u32 written = 0;
                        while (written < size) {
                            int r = sys3(SYS_WRITE, fd, (int)(cursor + written), (int)(size - written));
                            if (r <= 0) break;
                            written += (u32)r;
                        }
                        sys1(SYS_CLOSE, fd);
                        print("network-share-agent: mirrored ");
                        print(remote->safe_name);
                        print("/");
                        print(rel_path);
                        print("\n");
                    }
                    cursor += size;
                    if (cursor < end && *cursor == '\n') cursor++;
                    line = next_line(&cursor, end);
                }
            }
        }
    }
}

static void set_sockaddr_port(u8* sockaddr, u32 port) {
    sockaddr[2] = (u8)((port >> 8) & 0xff);
    sockaddr[3] = (u8)(port & 0xff);
}

static void set_sockaddr_ip(u8* sockaddr, const u8 ip[4]) {
    sockaddr[4] = ip[0];
    sockaddr[5] = ip[1];
    sockaddr[6] = ip[2];
    sockaddr[7] = ip[3];
}

static int connect_to_remote(RemoteShare* remote) {
    u32 socket_args[3] = {AF_INET, SOCK_STREAM, 0};
    u8 sockaddr[16] = {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    u32 connect_args[3];
    int fd = socketcall(SC_SOCKET, socket_args);
    if (fd < 0) return fd;
    set_sockaddr_port(sockaddr, remote->port);
    set_sockaddr_ip(sockaddr, remote->ip);
    connect_args[0] = (u32)fd;
    connect_args[1] = (u32)sockaddr;
    connect_args[2] = 16;
    if (socketcall(SC_CONNECT, connect_args) < 0) {
        sys1(SYS_CLOSE, fd);
        return -1;
    }
    return fd;
}

static int fetch_archive_with_request(RemoteShare* remote, const char* request) {
    int fd;
    int total = 0;
    fd = connect_to_remote(remote);
    if (fd < 0) {
        print("network-share-agent: connect failed ");
        print(remote->safe_name);
        print("\n");
        return -1;
    }
    send_all(fd, request, strlen0(request));
    while (total < (int)sizeof(archive_buffer)) {
        int want = (int)sizeof(archive_buffer) - total;
        u32 recv_args[4];
        int got;
        if (want > RECV_CHUNK_SIZE) want = RECV_CHUNK_SIZE;
        recv_args[0] = (u32)fd;
        recv_args[1] = (u32)(archive_buffer + total);
        recv_args[2] = (u32)want;
        recv_args[3] = 0;
        got = socketcall(SC_RECV, recv_args);
        if (got <= 0) break;
        total += got;
        if (buffer_has_end_marker(archive_buffer, total)) break;
    }
    sys1(SYS_CLOSE, fd);
    if (!buffer_has_end_marker(archive_buffer, total)) {
        print("network-share-agent: archive incomplete ");
        print(remote->safe_name);
        print("\n");
        return -1;
    }
    return total;
}

static int recv_one_byte(int fd, char* c) {
    u32 recv_args[4];
    recv_args[0] = (u32)fd;
    recv_args[1] = (u32)c;
    recv_args[2] = 1;
    recv_args[3] = 0;
    return socketcall(SC_RECV, recv_args);
}

static int recv_response_header(int fd, char* dst, int max_len);

static int recv_exact_to_buffer(int fd, char* dst, u32 size) {
    u32 total = 0;
    while (total < size) {
        u32 recv_args[4];
        int want = (size - total) > MANIFEST_RECV_CHUNK_SIZE ? (int)MANIFEST_RECV_CHUNK_SIZE : (int)(size - total);
        int got;
        recv_args[0] = (u32)fd;
        recv_args[1] = (u32)(dst + total);
        recv_args[2] = (u32)want;
        recv_args[3] = 0;
        got = socketcall(SC_RECV, recv_args);
        if (got <= 0) return 0;
        total += (u32)got;
    }
    return 1;
}

static int fetch_manifest(RemoteShare* remote, u32* manifest_hash) {
    int fd;
    int total = 0;
    int pos;
    int header_len;
    int size_pos;
    u32 advertised_size;
    fd = connect_to_remote(remote);
    if (fd < 0) {
        print("network-share-agent: manifest connect failed ");
        print(remote->safe_name);
        print("\n");
        return -1;
    }
    send_all(fd, manifest_request, sizeof(manifest_request) - 1);
    header_len = recv_response_header(fd, line_buffer, sizeof(line_buffer));
    if (header_len > 0 && starts_with(line_buffer, "BW-SHARE-MANIFEST-DATA/1")) {
        size_pos = find_token(line_buffer, "size=");
        if (size_pos < 0) {
            sys1(SYS_CLOSE, fd);
            print("network-share-agent: manifest size missing ");
            print(remote->safe_name);
            print("\n");
            return -1;
        }
        advertised_size = parse_uint(line_buffer + size_pos + 5);
        if (advertised_size >= (u32)sizeof(manifest_buffer)) {
            sys1(SYS_CLOSE, fd);
            print("network-share-agent: manifest too large ");
            print(remote->safe_name);
            print("\n");
            return -1;
        }
        if (!recv_exact_to_buffer(fd, manifest_buffer, advertised_size)) {
            sys1(SYS_CLOSE, fd);
            print("network-share-agent: manifest receive failed ");
            print(remote->safe_name);
            print("\n");
            return -1;
        }
        total = (int)advertised_size;
    } else if (header_len > 0 && starts_with(line_buffer, "BW-SHARE-MANIFEST/1")) {
        int i;
        for (i = 0; i < header_len && i < (int)sizeof(manifest_buffer) - 1; i++) manifest_buffer[i] = line_buffer[i];
        total = header_len;
        while (total < (int)sizeof(manifest_buffer) - 1) {
            char c;
            int got;
            got = recv_one_byte(fd, &c);
            if (got <= 0) break;
            manifest_buffer[total++] = c;
            if (buffer_has_end_marker(manifest_buffer, total)) break;
        }
    } else {
        sys1(SYS_CLOSE, fd);
        print("network-share-agent: manifest header missing ");
        print(remote->safe_name);
        print("\n");
        return -1;
    }
    manifest_len = total;
    manifest_buffer[total] = 0;
    if (!buffer_has_end_marker(manifest_buffer, total) || !starts_with(manifest_buffer, "BW-SHARE-MANIFEST/1")) {
        sys1(SYS_CLOSE, fd);
        print("network-share-agent: manifest incomplete ");
        print(remote->safe_name);
        print("\n");
        return -1;
    }
    pos = find_token(manifest_buffer, "hash=");
    if (pos < 0) {
        sys1(SYS_CLOSE, fd);
        print("network-share-agent: manifest hash missing ");
        print(remote->safe_name);
        print("\n");
        return -1;
    }
    *manifest_hash = parse_uint(manifest_buffer + pos + 5);
    return 0;
}

static int local_file_matches(RemoteShare* remote, const char* rel_path, u32 expected_size, u32 expected_hash) {
    u32 size;
    u32 hash;
    join_path(path_buffer, remote->mirror_root, rel_path);
    if (path_is_dir(path_buffer)) return 0;
    hash = calculate_file_hash(path_buffer, &size);
    return size == expected_size && hash == expected_hash;
}

static int fetch_file_archive(RemoteShare* remote, const char* rel_path) {
    int pos = 0;
    append_str(line_buffer, &pos, "GET file path=");
    append_str(line_buffer, &pos, rel_path);
    append_str(line_buffer, &pos, "\n\n");
    line_buffer[pos] = 0;
    return fetch_archive_with_request(remote, line_buffer);
}

static int recv_response_header(int fd, char* dst, int max_len) {
    int pos = 0;
    while (pos < max_len - 1) {
        char c;
        int got = recv_one_byte(fd, &c);
        if (got <= 0) break;
        dst[pos++] = c;
        if (pos >= 2 && dst[pos - 2] == '\n' && dst[pos - 1] == '\n') break;
        if (pos >= 4 && dst[pos - 4] == '\r' && dst[pos - 3] == '\n' && dst[pos - 2] == '\r' && dst[pos - 1] == '\n') break;
    }
    dst[pos] = 0;
    return pos;
}

static int recv_exact_to_file(int fd, int out_fd, u32 size) {
    u32 remaining = size;
    while (remaining > 0) {
        u32 recv_args[4];
        int want = remaining > RECV_CHUNK_SIZE ? RECV_CHUNK_SIZE : (int)remaining;
        int got;
        recv_args[0] = (u32)fd;
        recv_args[1] = (u32)file_buffer;
        recv_args[2] = (u32)want;
        recv_args[3] = 0;
        got = socketcall(SC_RECV, recv_args);
        if (got <= 0) return 0;
        if (sys3(SYS_WRITE, out_fd, (int)file_buffer, got) != got) return 0;
        remaining -= (u32)got;
    }
    return 1;
}

static int fetch_file_data(RemoteShare* remote, const char* rel_path, u32 expected_size) {
    int fd;
    int out_fd;
    int header_len;
    int size_pos;
    u32 size;
    int pos = 0;
    append_str(line_buffer, &pos, file_data_request_prefix);
    append_str(line_buffer, &pos, rel_path);
    append_str(line_buffer, &pos, "\n\n");
    line_buffer[pos] = 0;
    fd = connect_to_remote(remote);
    if (fd < 0) {
        print("network-share-agent: file connect failed ");
        print(remote->safe_name);
        print("/");
        print(rel_path);
        print("\n");
        return 0;
    }
    send_all(fd, line_buffer, pos);
    header_len = recv_response_header(fd, line_buffer, sizeof(line_buffer));
    if (header_len <= 0 || !starts_with(line_buffer, "BW-SHARE-FILE/1")) {
        sys1(SYS_CLOSE, fd);
        print("network-share-agent: file header missing ");
        print(remote->safe_name);
        print("/");
        print(rel_path);
        print("\n");
        return 0;
    }
    if (find_token(line_buffer, "status=ok") < 0) {
        sys1(SYS_CLOSE, fd);
        print("network-share-agent: file unavailable ");
        print(remote->safe_name);
        print("/");
        print(rel_path);
        print("\n");
        return 0;
    }
    size_pos = find_token(line_buffer, "size=");
    if (size_pos < 0) {
        sys1(SYS_CLOSE, fd);
        print("network-share-agent: file size missing ");
        print(remote->safe_name);
        print("/");
        print(rel_path);
        print("\n");
        return 0;
    }
    size = parse_uint(line_buffer + size_pos + 5);
    if (size != expected_size) {
        sys1(SYS_CLOSE, fd);
        print("network-share-agent: file size changed ");
        print(remote->safe_name);
        print("/");
        print(rel_path);
        print("\n");
        return 0;
    }
    join_path(path_buffer, remote->mirror_root, rel_path);
    make_parent_dirs(path_buffer);
    out_fd = sys3(SYS_OPEN, (int)path_buffer, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (out_fd < 0) {
        sys1(SYS_CLOSE, fd);
        print("network-share-agent: file create failed ");
        print(remote->safe_name);
        print("/");
        print(rel_path);
        print("\n");
        return 0;
    }
    if (!recv_exact_to_file(fd, out_fd, size)) {
        sys1(SYS_CLOSE, out_fd);
        sys1(SYS_CLOSE, fd);
        print("network-share-agent: file receive failed ");
        print(remote->safe_name);
        print("/");
        print(rel_path);
        print("\n");
        return 0;
    }
    sys1(SYS_CLOSE, out_fd);
    sys1(SYS_CLOSE, fd);
    return 1;
}

static int apply_manifest_delta(RemoteShare* remote, int prune_existing) {
    char* cursor = manifest_buffer;
    char* end = manifest_buffer + manifest_len;
    char* line;
    int changed = 0;
    int skipped = 0;
    make_parent_dirs(remote->mirror_root);
    make_dir(remote->mirror_root);
    if (prune_existing) {
        prune_deleted_entries(remote, remote->mirror_root, "", 0);
    }
    line = next_line(&cursor, end);
    if (!line || !starts_with(line, "BW-SHARE-MANIFEST/1")) return 0;
    while ((line = next_line(&cursor, end))) {
        if (!line[0]) break;
    }
    while ((line = next_line(&cursor, end))) {
        if (starts_with(line, "END")) break;
        if (starts_with(line, "dir path=")) {
            join_path(path_buffer, remote->mirror_root, line + 9);
            make_parent_dirs(path_buffer);
            make_dir(path_buffer);
        } else if (starts_with(line, "file path=")) {
            int size_pos = find_token(line, " size=");
            int hash_pos = find_token(line, " hash=");
            if (size_pos > 10 && hash_pos > size_pos) {
                char rel_path[512];
                u32 size;
                u32 hash;
                copy_n(rel_path, line + 10, size_pos - 10);
                size = parse_uint(line + size_pos + 6);
                hash = parse_uint(line + hash_pos + 6);
                if (local_file_matches(remote, rel_path, size, hash)) {
                    skipped++;
                } else {
                    if (!fetch_file_data(remote, rel_path, size)) return 0;
                    changed++;
                }
            }
        }
    }
    print("network-share-agent: synced ");
    print(remote->safe_name);
    print(" changed ");
    print_uint((u32)changed);
    print(" skipped ");
    print_uint((u32)skipped);
    print("\n");
    return 1;
}

static void build_drive_link(char drive) {
    int pos = 0;
    append_str(second_path_buffer, &pos, drive_prefix);
    append_char(second_path_buffer, &pos, drive);
    append_char(second_path_buffer, &pos, ':');
    second_path_buffer[pos] = 0;
}

static int drive_allocated(char drive) {
    int i;
    for (i = 0; i < MAX_REMOTES; i++) {
        if (remotes[i].used && remotes[i].drive == drive) return 1;
    }
    return 0;
}

static int drive_exists(char drive) {
    int fd;
    build_drive_link(drive);
    fd = sys3(SYS_OPEN, (int)second_path_buffer, O_RDONLY, 0);
    if (fd >= 0) {
        sys1(SYS_CLOSE, fd);
        return 1;
    }
    return 0;
}

static char allocate_drive(void) {
    char drive;
    for (drive = 'y'; drive >= 'e'; drive--) {
        if (drive == 'z' || drive == 'c' || drive == 'd') continue;
        if (drive_allocated(drive)) continue;
        if (drive_exists(drive)) continue;
        return drive;
    }
    return 0;
}

static int is_allowed_name_char(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-';
}

static char lower_char(char c) {
    if (c >= 'A' && c <= 'Z') return (char)(c + 32);
    return c;
}

static int reserved_name(const char* name) {
    char lower[16];
    int i = 0;
    while (name[i] && i < 15) {
        lower[i] = lower_char(name[i]);
        i++;
    }
    lower[i] = 0;
    if (streq(lower, "con") || streq(lower, "prn") || streq(lower, "aux") || streq(lower, "nul")) return 1;
    if (lower[0] == 'c' && lower[1] == 'o' && lower[2] == 'm' && lower[3] >= '1' && lower[3] <= '9' && lower[4] == 0) return 1;
    if (lower[0] == 'l' && lower[1] == 'p' && lower[2] == 't' && lower[3] >= '1' && lower[3] <= '9' && lower[4] == 0) return 1;
    return 0;
}

static void sanitize_share_name(char* dst, const char* src) {
    int in = 0;
    int out = 0;
    int last_us = 0;
    while (src[in] && out < 48) {
        char c = src[in++];
        if (is_allowed_name_char(c)) {
            dst[out++] = c;
            last_us = c == '_';
        } else if (!last_us && out > 0) {
            dst[out++] = '_';
            last_us = 1;
        }
    }
    while (out > 0 && dst[out - 1] == '_') out--;
    dst[out] = 0;
    if (!dst[0] || reserved_name(dst) || streq(dst, ".") || streq(dst, "..")) copy_str(dst, "share");
}

static int safe_name_exists(const char* safe, int except_index) {
    int i;
    for (i = 0; i < MAX_REMOTES; i++) {
        if (i != except_index && remotes[i].used && streq(remotes[i].safe_name, safe)) return 1;
    }
    return 0;
}

static void make_unique_safe_name(int index) {
    char base[64];
    char candidate[64];
    u32 suffix = 2;
    copy_str(base, remotes[index].safe_name);
    while (safe_name_exists(remotes[index].safe_name, index)) {
        int pos = 0;
        append_str(candidate, &pos, base);
        append_char(candidate, &pos, '-');
        append_uint(candidate, &pos, suffix++);
        candidate[pos] = 0;
        copy_str(remotes[index].safe_name, candidate);
    }
}

static void build_remote_mirror_root(RemoteShare* remote) {
    join_path(remote->mirror_root, mirror_base, remote->safe_name);
}

static void map_remote_drive(RemoteShare* remote) {
    int result;
    build_drive_link(remote->drive);
    sys1(SYS_UNLINK, (int)second_path_buffer);
    result = sys2(SYS_SYMLINK, (int)remote->mirror_root, (int)second_path_buffer);
    if (result == 0) {
        print("network-share-agent: mapped ");
        print(second_path_buffer);
        print(" to ");
        print(remote->safe_name);
        print("\n");
    } else {
        print("network-share-agent: map failed ");
        print(second_path_buffer);
        print(" to ");
        print(remote->safe_name);
        print(" result ");
        print_uint((u32)(0 - result));
        print("\n");
    }
}

static int same_remote(RemoteShare* remote, u8 ip[4], u32 port, const char* name) {
    return remote->used && remote->port == port && streq(remote->raw_name, name) &&
        remote->ip[0] == ip[0] && remote->ip[1] == ip[1] && remote->ip[2] == ip[2] && remote->ip[3] == ip[3];
}

static int find_remote_slot(u8 ip[4], u32 port, const char* name) {
    int i;
    int empty = -1;
    for (i = 0; i < MAX_REMOTES; i++) {
        if (same_remote(&remotes[i], ip, port, name)) return i;
        if (!remotes[i].used && empty < 0) empty = i;
    }
    return empty;
}

static void add_or_refresh_remote(u8 ip[4], u32 port, const char* name) {
    int slot;
    char drive;
    if (streq(name, share_name)) return;
    slot = find_remote_slot(ip, port, name);
    if (slot < 0) return;
    if (remotes[slot].used) {
        remotes[slot].ip[0] = ip[0];
        remotes[slot].ip[1] = ip[1];
        remotes[slot].ip[2] = ip[2];
        remotes[slot].ip[3] = ip[3];
        remotes[slot].port = port;
        return;
    }
    drive = allocate_drive();
    if (!drive) return;
    remotes[slot].used = 1;
    remotes[slot].ip[0] = ip[0];
    remotes[slot].ip[1] = ip[1];
    remotes[slot].ip[2] = ip[2];
    remotes[slot].ip[3] = ip[3];
    remotes[slot].port = port;
    remotes[slot].drive = drive;
    remotes[slot].first_sync = 1;
    remotes[slot].have_hash = 0;
    remotes[slot].poll_now = 1;
    remotes[slot].defer_ticks = strcmp0(share_name, name) > 0 ? 16 : 0;
    copy_str(remotes[slot].raw_name, name);
    sanitize_share_name(remotes[slot].safe_name, name);
    make_unique_safe_name(slot);
    build_remote_mirror_root(&remotes[slot]);
    print("network-share-agent: discovered ");
    print(name);
    print(" as ");
    print(remotes[slot].safe_name);
    print(" on ");
    line_buffer[0] = drive;
    line_buffer[1] = ':';
    line_buffer[2] = 0;
    print(line_buffer);
    if (remotes[slot].defer_ticks) print(" after delay");
    print("\n");
}

static void parse_beacon(char* text, u8 source_ip[4]) {
    char name[64];
    u32 port;
    int name_pos;
    int port_pos;
    int i = 0;
    if (!starts_with(text, "BW-SHARE/1")) return;
    name_pos = find_token(text, " name=");
    port_pos = find_token(text, " port=");
    if (name_pos < 0 || port_pos < 0) return;
    name_pos += 6;
    while (text[name_pos + i] && text[name_pos + i] != ' ' && text[name_pos + i] != '\r' && text[name_pos + i] != '\n' && i < 63) {
        name[i] = text[name_pos + i];
        i++;
    }
    name[i] = 0;
    port = parse_uint(text + port_pos + 6);
    if (name[0] && port) add_or_refresh_remote(source_ip, port, name);
}

static void process_beacons(int udp_fd) {
    for (;;) {
        u8 source_sockaddr[16];
        u32 source_len = 16;
        u32 recv_args[6];
        int received;
        recv_args[0] = (u32)udp_fd;
        recv_args[1] = (u32)line_buffer;
        recv_args[2] = sizeof(line_buffer) - 1;
        recv_args[3] = 0;
        recv_args[4] = (u32)source_sockaddr;
        recv_args[5] = (u32)&source_len;
        received = socketcall(SC_RECVFROM, recv_args);
        if (received <= 0) break;
        line_buffer[received] = 0;
        parse_beacon(line_buffer, source_sockaddr + 4);
    }
}

static void sync_remote(RemoteShare* remote) {
    u32 hash = 0;
    if (remote->first_sync) {
        if (remote->defer_ticks) {
            remote->defer_ticks--;
            return;
        }
        print("network-share-agent: first sync ");
        print(remote->safe_name);
        print("\n");
        if (fetch_manifest(remote, &hash) < 0) {
            int total;
            print("network-share-agent: first manifest failed, refreshing archive ");
            print(remote->safe_name);
            print("\n");
            total = fetch_archive_with_request(remote, archive_request);
            if (total < 0) {
                remote->defer_ticks = 16;
                return;
            }
            make_parent_dirs(remote->mirror_root);
            make_dir(remote->mirror_root);
            parse_archive_into(remote, total);
            remote->first_sync = 0;
            remote->poll_now = 0;
            map_remote_drive(remote);
            return;
        }
        if (!apply_manifest_delta(remote, 0)) {
            remote->defer_ticks = 16;
            return;
        }
        remote->first_sync = 0;
        remote->poll_now = 0;
        map_remote_drive(remote);
        remote->last_hash = hash;
        remote->have_hash = 1;
        return;
    }
    if (fetch_manifest(remote, &hash) < 0) {
        int total;
        print("network-share-agent: manifest failed, refreshing archive ");
        print(remote->safe_name);
        print("\n");
        total = fetch_archive_with_request(remote, archive_request);
        if (total < 0) {
            remote->defer_ticks = 16;
            return;
        }
        remove_tree(remote->mirror_root, 0);
        make_parent_dirs(remote->mirror_root);
        make_dir(remote->mirror_root);
        parse_archive_into(remote, total);
        map_remote_drive(remote);
        return;
    }
    if (remote->have_hash && !remote->first_sync && remote->last_hash == hash) return;
    if (!apply_manifest_delta(remote, 1)) {
        int total = fetch_archive_with_request(remote, archive_request);
        if (total < 0) {
            remote->defer_ticks = 16;
            return;
        }
        remove_tree(remote->mirror_root, 0);
        make_parent_dirs(remote->mirror_root);
        make_dir(remote->mirror_root);
        parse_archive_into(remote, total);
    }
    remote->last_hash = hash;
    remote->have_hash = 1;
    remote->poll_now = 0;
    map_remote_drive(remote);
}

static void poll_remotes(int force_all) {
    int i;
    for (i = 0; i < MAX_REMOTES; i++) {
        if (remotes[i].used && remotes[i].defer_ticks) remotes[i].defer_ticks--;
        if (!serve_cooldown_ticks && remotes[i].used && !remotes[i].defer_ticks && (force_all || remotes[i].poll_now)) sync_remote(&remotes[i]);
    }
}

static int create_tcp_listener(void) {
    u32 socket_args[3] = {AF_INET, SOCK_STREAM, 0};
    u8 sockaddr[16] = {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    u32 bind_args[3];
    u32 listen_args[2];
    int fd = socketcall(SC_SOCKET, socket_args);
    if (fd < 0) return fd;
    set_sockaddr_port(sockaddr, listen_port);
    bind_args[0] = (u32)fd;
    bind_args[1] = (u32)sockaddr;
    bind_args[2] = 16;
    if (socketcall(SC_BIND, bind_args) < 0) return -1;
    listen_args[0] = (u32)fd;
    listen_args[1] = 4;
    if (socketcall(SC_LISTEN, listen_args) < 0) return -1;
    sys3(SYS_FCNTL, fd, F_SETFL, O_NONBLOCK);
    return fd;
}

static int create_udp_beacon_socket(void) {
    u32 socket_args[3] = {AF_INET, SOCK_DGRAM, IPPROTO_UDP};
    u8 sockaddr[16] = {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    u32 bind_args[3];
    int fd = socketcall(SC_SOCKET, socket_args);
    if (fd < 0) return fd;
    set_sockaddr_port(sockaddr, beacon_port);
    bind_args[0] = (u32)fd;
    bind_args[1] = (u32)sockaddr;
    bind_args[2] = 16;
    if (socketcall(SC_BIND, bind_args) < 0) return -1;
    sys3(SYS_FCNTL, fd, F_SETFL, O_NONBLOCK);
    return fd;
}

static void advertise_once(int udp_fd) {
    u8 sockaddr[16] = {2, 0, 0, 0, 10, 0, 3, 255, 0, 0, 0, 0, 0, 0, 0, 0};
    u32 args[6];
    int pos = 0;
    set_sockaddr_port(sockaddr, beacon_port);
    sockaddr[4] = broadcast_ip[0];
    sockaddr[5] = broadcast_ip[1];
    sockaddr[6] = broadcast_ip[2];
    sockaddr[7] = broadcast_ip[3];
    append_str(line_buffer, &pos, "BW-SHARE/1 name=");
    append_str(line_buffer, &pos, share_name);
    append_str(line_buffer, &pos, " root=");
    append_str(line_buffer, &pos, share_root);
    append_str(line_buffer, &pos, " port=");
    append_uint(line_buffer, &pos, listen_port);
    append_str(line_buffer, &pos, " mode=");
    append_str(line_buffer, &pos, share_mode);
    append_char(line_buffer, &pos, '\n');
    line_buffer[pos] = 0;
    args[0] = (u32)udp_fd;
    args[1] = (u32)line_buffer;
    args[2] = (u32)pos;
    args[3] = 0;
    args[4] = (u32)sockaddr;
    args[5] = 16;
    socketcall(SC_SENDTO, args);
}

static void serve_pending_clients(int listener_fd) {
    for (;;) {
        u32 accept_args[3] = {(u32)listener_fd, 0, 0};
        int client_fd = socketcall(SC_ACCEPT, accept_args);
        if (client_fd < 0) break;
        {
            char request[768];
            u32 recv_args[4] = {(u32)client_fd, (u32)request, sizeof(request) - 1, 0};
            int received = socketcall(SC_RECV, recv_args);
            if (received > 0) request[received] = 0;
            else request[0] = 0;
            if (starts_with(request, "GET manifest")) {
                send_manifest(client_fd);
            } else if (starts_with(request, "GET file-data path=")) {
                send_file_data(client_fd, request + 19);
                serve_cooldown_ticks = file_serve_cooldown_ticks;
            } else if (starts_with(request, "GET file path=")) {
                send_one_file_archive(client_fd, request + 14);
                print("network-share-agent: file archive sent\n");
                serve_cooldown_ticks = file_serve_cooldown_ticks;
            } else {
                send_archive(client_fd);
                print("network-share-agent: archive sent\n");
                serve_cooldown_ticks = file_serve_cooldown_ticks;
            }
            sys1(SYS_CLOSE, client_fd);
        }
    }
}

static void sleep_tick(void) {
    u32 ts[2] = {0, 250000000};
    sys2(SYS_NANOSLEEP, (int)ts, 0);
}

static void agent_run(int argc, char** argv) {
    int listener_fd;
    int udp_fd;
    u32 ticks = 0;
    u32 poll_ticks;
    parse_args(argc, argv);
    poll_ticks = poll_seconds ? poll_seconds * 4 : 120;

    print("network-share-agent: start\n");
    print("network-share-agent: sharing ");
    print(share_root);
    print(" as ");
    print(share_name);
    print("\n");

    listener_fd = create_tcp_listener();
    if (listener_fd < 0) {
        print("network-share-agent: listen failed\n");
        sys1(SYS_EXIT, 1);
    }
    udp_fd = create_udp_beacon_socket();
    if (udp_fd < 0) {
        print("network-share-agent: udp failed\n");
        sys1(SYS_EXIT, 1);
    }

    for (;;) {
        advertise_once(udp_fd);
        process_beacons(udp_fd);
        serve_pending_clients(listener_fd);
        poll_remotes(ticks == 0 || (poll_ticks && ticks % poll_ticks == 0));
        if (serve_cooldown_ticks) serve_cooldown_ticks--;
        ticks++;
        sleep_tick();
    }
}

NETWORK_SHARE_ENTRY(agent_run, "network-share-agent.exe")
