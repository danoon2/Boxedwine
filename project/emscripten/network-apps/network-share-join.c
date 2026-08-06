/*
 * Tiny 32-bit Linux LAN share joiner for Boxedwine networking.
 *
 * Discovers network-share-host, fetches its archive stream, and mirrors files
 * into a configured Wine-visible directory.
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
#define SYS_SYMLINK 83
#define SYS_SOCKETCALL 102
#define SYS_NANOSLEEP 162
#define SYS_GETDENTS 141

#define SC_SOCKET 1
#define SC_BIND 2
#define SC_CONNECT 3
#define SC_SEND 9
#define SC_RECV 10
#define SC_RECVFROM 12

#define AF_INET 2
#define SOCK_STREAM 1
#define SOCK_DGRAM 2
#define IPPROTO_UDP 17
#define O_RDONLY 0
#define O_WRONLY 1
#define O_CREAT 0x40
#define O_TRUNC 0x200
#define SEEK_SET 0
#define SEEK_END 2
#define ARCHIVE_BUFFER_SIZE (8 * 1024 * 1024)
#define MANIFEST_BUFFER_SIZE (256 * 1024)

static const char default_mirror_root[] = "/home/username/.wine/dosdevices/c:/share-mirror";
static const char default_drive[] = "y";
static const char archive_request[] = "GET archive BW-SHARE/1\n\n";
static const char manifest_request[] = "GET manifest BW-SHARE/1\n\n";
static const char drive_prefix[] = "/home/username/.wine/dosdevices/";

static const char* mirror_root = default_mirror_root;
static const char* mirror_drive = default_drive;
static u32 beacon_port = 19201;
static u32 connect_port = 19200;
static u32 poll_seconds = 30;
static int connect_port_overridden = 0;
static int direct_host = 0;
static int run_once = 0;
static int have_manifest_hash = 0;
static u32 last_manifest_hash = 0;
static int manifest_len = 0;
static u8 direct_host_ip[4] = {0, 0, 0, 0};

static char beacon_buffer[1024];
static char manifest_buffer[MANIFEST_BUFFER_SIZE];
static char archive_buffer[ARCHIVE_BUFFER_SIZE];
static char path_buffer[768];
static char drive_link_buffer[256];

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

static int starts_with(const char* s, const char* prefix) {
    int i = 0;
    while (prefix[i]) {
        if (s[i] != prefix[i]) return 0;
        i++;
    }
    return 1;
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
    int pos = 0;
    append_uint(path_buffer, &pos, value);
    path_buffer[pos] = 0;
    print(path_buffer);
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
        if (option_value(&i, argc, argv, "--mirror-root", &value) || option_value(&i, argc, argv, "--target", &value)) {
            mirror_root = value;
        } else if (option_value(&i, argc, argv, "--drive", &value)) {
            mirror_drive = value;
        } else if (option_value(&i, argc, argv, "--beacon-port", &value) || option_value(&i, argc, argv, "--listen-port", &value)) {
            beacon_port = parse_uint(value);
        } else if (option_value(&i, argc, argv, "--port", &value) || option_value(&i, argc, argv, "--connect-port", &value)) {
            connect_port = parse_uint(value);
            connect_port_overridden = 1;
        } else if (option_value(&i, argc, argv, "--host", &value)) {
            direct_host = parse_ip(value, direct_host_ip);
        } else if (option_value(&i, argc, argv, "--poll-seconds", &value) || option_value(&i, argc, argv, "--poll", &value)) {
            poll_seconds = parse_uint(value);
        } else if (streq(argv[i], "--once")) {
            run_once = 1;
        }
    }
}

static int buffer_has_end_marker(const char* data, int len) {
    if (len < 4) return 0;
    return data[len - 4] == 'E' && data[len - 3] == 'N' && data[len - 2] == 'D' && data[len - 1] == '\n';
}

static void copy_n(char* dst, const char* src, int n) {
    int i;
    for (i = 0; i < n; i++) dst[i] = src[i];
    dst[n] = 0;
}

static void join_path(char* dst, const char* base, const char* rel) {
    int pos = 0;
    append_str(dst, &pos, base);
    if (pos > 0 && dst[pos - 1] != '/') dst[pos++] = '/';
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
    char probe_buffer[256];
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

static u32 calculate_file_hash(const char* full_path, u32* size_out) {
    char hash_buffer[4096];
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
        int want = remaining > (int)sizeof(hash_buffer) ? (int)sizeof(hash_buffer) : remaining;
        int got = sys3(SYS_READ, fd, (int)hash_buffer, want);
        if (got <= 0) break;
        hash = fnv1a_update(hash, hash_buffer, got);
        remaining -= got;
    }
    sys1(SYS_CLOSE, fd);
    return hash;
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

static void prune_deleted_entries(const char* full_dir, const char* rel_dir, int depth) {
    char dents_buffer[4096];
    int fd;
    if (depth > 8) return;
    fd = sys3(SYS_OPEN, (int)full_dir, O_RDONLY, 0);
    if (fd < 0) return;

    for (;;) {
        int nread = sys3(SYS_GETDENTS, fd, (int)dents_buffer, (int)sizeof(dents_buffer));
        int offset = 0;
        if (nread <= 0) break;
        while (offset < nread) {
            u16 reclen = *(u16*)(dents_buffer + offset + 8);
            char* name = dents_buffer + offset + 10;
            char child_full[768];
            char child_rel[512];
            int is_dir;
            if (!reclen) break;
            if (!skip_dirent_name(name)) {
                join_path(child_full, full_dir, name);
                if (rel_dir[0]) join_path(child_rel, rel_dir, name);
                else copy_n(child_rel, name, strlen0(name));
                is_dir = path_is_dir(child_full);
                if (!manifest_contains_path(child_rel, is_dir)) {
                    remove_tree(child_full, 0);
                    print("network-share-join: pruned ");
                    print(child_rel);
                    print("\n");
                } else if (is_dir) {
                    prune_deleted_entries(child_full, child_rel, depth + 1);
                }
            }
            offset += reclen;
        }
    }
    sys1(SYS_CLOSE, fd);
}

static void remove_tree(const char* path, int depth) {
    char dents_buffer[4096];
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
            int nread = sys3(SYS_GETDENTS, fd, (int)dents_buffer, (int)sizeof(dents_buffer));
            int offset = 0;
            if (nread <= 0) break;
            while (offset < nread) {
                u16 reclen = *(u16*)(dents_buffer + offset + 8);
                char* name = dents_buffer + offset + 10;
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

static void prune_mirror_root(void) {
    char dents_buffer[4096];
    int fd;
    make_parent_dirs(mirror_root);
    make_dir(mirror_root);
    fd = sys3(SYS_OPEN, (int)mirror_root, O_RDONLY, 0);
    if (fd < 0) return;

    print("network-share-join: pruning mirror root\n");
    for (;;) {
        int nread = sys3(SYS_GETDENTS, fd, (int)dents_buffer, (int)sizeof(dents_buffer));
        int offset = 0;
        if (nread <= 0) break;
        while (offset < nread) {
            u16 reclen = *(u16*)(dents_buffer + offset + 8);
            char* name = dents_buffer + offset + 10;
            char child_path[768];
            if (!reclen) break;
            if (!skip_dirent_name(name)) {
                join_path(child_path, mirror_root, name);
                remove_tree(child_path, 0);
            }
            offset += reclen;
        }
    }
    sys1(SYS_CLOSE, fd);
}

static void mirror_file(const char* rel_path, const char* data, u32 size) {
    int fd;
    join_path(path_buffer, mirror_root, rel_path);
    make_parent_dirs(path_buffer);
    fd = sys3(SYS_OPEN, (int)path_buffer, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd >= 0) {
        u32 written = 0;
        while (written < size) {
            int r = sys3(SYS_WRITE, fd, (int)(data + written), (int)(size - written));
            if (r <= 0) break;
            written += (u32)r;
        }
        sys1(SYS_CLOSE, fd);
        print("network-share-join: mirrored file ");
        print(rel_path);
        print("\n");
    }
}

static void build_drive_link(void) {
    int pos = 0;
    append_str(drive_link_buffer, &pos, drive_prefix);
    append_char(drive_link_buffer, &pos, mirror_drive[0]);
    append_char(drive_link_buffer, &pos, ':');
    drive_link_buffer[pos] = 0;
}

static void map_mirror_drive(void) {
    int result;
    build_drive_link();
    sys1(SYS_UNLINK, (int)drive_link_buffer);
    result = sys2(SYS_SYMLINK, (int)mirror_root, (int)drive_link_buffer);
    if (result == 0) {
        print("network-share-join: mapped ");
        print(drive_link_buffer);
        print(" to ");
        print(mirror_root);
        print("\n");
    } else {
        print("network-share-join: failed to map drive\n");
    }
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

static void parse_archive(int len) {
    char* cursor = archive_buffer;
    char* end = archive_buffer + len;
    char* line;
    make_dir("/home/username/.wine/dosdevices/c:");
    make_dir(mirror_root);
    print("network-share-join: mirror root ");
    print(mirror_root);
    print("\n");

    line = next_line(&cursor, end);
    if (!line || !starts_with(line, "BW-SHARE-ARCHIVE/1")) {
        print("network-share-join: archive header missing\n");
        return;
    }

    while ((line = next_line(&cursor, end))) {
        if (!line[0]) break;
    }

    while ((line = next_line(&cursor, end))) {
        if (starts_with(line, "END")) break;
        if (starts_with(line, "dir path=")) {
            join_path(path_buffer, mirror_root, line + 9);
            make_parent_dirs(path_buffer);
            make_dir(path_buffer);
            print("network-share-join: mirrored dir ");
            print(line + 9);
            print("\n");
        } else if (starts_with(line, "file path=")) {
            int size_pos = find_token(line, " size=");
            if (size_pos > 10) {
                char rel_path[512];
                u32 size;
                copy_n(rel_path, line + 10, size_pos - 10);
                size = parse_uint(line + size_pos + 6);
                if (cursor + size <= end) {
                    mirror_file(rel_path, cursor, size);
                    cursor += size;
                    if (cursor < end && *cursor == '\n') cursor++;
                    line = next_line(&cursor, end);
                }
            }
        }
    }
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

static void read_port_from_beacon(void) {
    int pos;
    if (connect_port_overridden) return;
    pos = find_token(beacon_buffer, " port=");
    if (pos >= 0) connect_port = parse_uint(beacon_buffer + pos + 6);
}

static int discover_host(u8* server_sockaddr) {
    u32 socket_args[3] = {AF_INET, SOCK_DGRAM, IPPROTO_UDP};
    u8 listen_sockaddr[16] = {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    u8 source_sockaddr[16];
    u32 source_len = 16;
    u32 bind_args[3];
    u32 recv_args[6];
    int received;
    int fd = socketcall(SC_SOCKET, socket_args);
    if (fd < 0) return fd;
    set_sockaddr_port(listen_sockaddr, beacon_port);
    bind_args[0] = (u32)fd;
    bind_args[1] = (u32)listen_sockaddr;
    bind_args[2] = 16;
    if (socketcall(SC_BIND, bind_args) < 0) return -1;
    print("network-share-join: waiting for share beacon on 0.0.0.0:");
    print_uint(beacon_port);
    print("\n");
    recv_args[0] = (u32)fd;
    recv_args[1] = (u32)beacon_buffer;
    recv_args[2] = sizeof(beacon_buffer) - 1;
    recv_args[3] = 0;
    recv_args[4] = (u32)source_sockaddr;
    recv_args[5] = (u32)&source_len;
    received = socketcall(SC_RECVFROM, recv_args);
    if (received <= 0) return -1;
    beacon_buffer[received] = 0;
    server_sockaddr[4] = source_sockaddr[4];
    server_sockaddr[5] = source_sockaddr[5];
    server_sockaddr[6] = source_sockaddr[6];
    server_sockaddr[7] = source_sockaddr[7];
    read_port_from_beacon();
    print("network-share-join: beacon received\n");
    sys1(SYS_CLOSE, fd);
    return 0;
}

static int connect_to_host(u8* server_sockaddr) {
    u32 socket_args[3] = {AF_INET, SOCK_STREAM, 0};
    u32 connect_args[3];
    int fd = socketcall(SC_SOCKET, socket_args);
    if (fd < 0) return fd;
    set_sockaddr_port(server_sockaddr, connect_port);
    connect_args[0] = (u32)fd;
    connect_args[1] = (u32)server_sockaddr;
    connect_args[2] = 16;
    print("network-share-join: connecting to advertised host:");
    print_uint(connect_port);
    print("\n");
    if (socketcall(SC_CONNECT, connect_args) < 0) return -1;
    print("network-share-join: connect ok\n");
    return fd;
}

static void sleep_poll_interval(void) {
    u32 ts[2];
    if (poll_seconds == 0) return;
    ts[0] = poll_seconds;
    ts[1] = 0;
    sys2(SYS_NANOSLEEP, (int)ts, 0);
}

static int fetch_archive_with_request(u8* server_sockaddr, const char* request, const char* sent_message) {
    int fd;
    int total = 0;
    fd = connect_to_host(server_sockaddr);
    if (fd < 0) {
        print("network-share-join: connect failed\n");
        return -1;
    }
    send_all(fd, request, strlen0(request));
    print(sent_message);
    while (total < (int)sizeof(archive_buffer)) {
        u32 recv_args[4] = {(u32)fd, (u32)(archive_buffer + total), (u32)(sizeof(archive_buffer) - total), 0};
        int got = socketcall(SC_RECV, recv_args);
        if (got <= 0) break;
        total += got;
        if (buffer_has_end_marker(archive_buffer, total)) break;
    }
    sys1(SYS_CLOSE, fd);
    if (!buffer_has_end_marker(archive_buffer, total)) {
        print("network-share-join: archive incomplete or too large\n");
        return -1;
    }
    print("network-share-join: archive received\n");
    return total;
}

static int fetch_archive(u8* server_sockaddr) {
    return fetch_archive_with_request(server_sockaddr, archive_request, "network-share-join: archive request sent\n");
}

static int fetch_file_archive(u8* server_sockaddr, const char* rel_path) {
    int pos = 0;
    append_str(path_buffer, &pos, "GET file path=");
    append_str(path_buffer, &pos, rel_path);
    append_str(path_buffer, &pos, "\n\n");
    path_buffer[pos] = 0;
    print("network-share-join: fetching changed file ");
    print(rel_path);
    print("\n");
    return fetch_archive_with_request(server_sockaddr, path_buffer, "network-share-join: file request sent\n");
}

static int fetch_manifest(u8* server_sockaddr, u32* manifest_hash) {
    int fd;
    int total = 0;
    int pos;
    fd = connect_to_host(server_sockaddr);
    if (fd < 0) {
        print("network-share-join: manifest connect failed\n");
        return -1;
    }
    send_all(fd, manifest_request, sizeof(manifest_request) - 1);
    print("network-share-join: manifest request sent\n");
    while (total < (int)sizeof(manifest_buffer) - 1) {
        u32 recv_args[4] = {(u32)fd, (u32)(manifest_buffer + total), (u32)(sizeof(manifest_buffer) - 1 - total), 0};
        int got = socketcall(SC_RECV, recv_args);
        if (got <= 0) break;
        total += got;
        if (buffer_has_end_marker(manifest_buffer, total)) break;
    }
    sys1(SYS_CLOSE, fd);
    manifest_len = total;
    manifest_buffer[total] = 0;
    if (!buffer_has_end_marker(manifest_buffer, total) || !starts_with(manifest_buffer, "BW-SHARE-MANIFEST/1")) {
        print("network-share-join: manifest incomplete\n");
        return -1;
    }
    pos = find_token(manifest_buffer, "hash=");
    if (pos < 0) {
        print("network-share-join: manifest hash missing\n");
        return -1;
    }
    *manifest_hash = parse_uint(manifest_buffer + pos + 5);
    print("network-share-join: manifest hash ");
    print_uint(*manifest_hash);
    print("\n");
    return 0;
}

static int local_file_matches(const char* rel_path, u32 expected_size, u32 expected_hash) {
    u32 size;
    u32 hash;
    join_path(path_buffer, mirror_root, rel_path);
    if (path_is_dir(path_buffer)) return 0;
    hash = calculate_file_hash(path_buffer, &size);
    return size == expected_size && hash == expected_hash;
}

static int apply_manifest_delta(u8* server_sockaddr) {
    char* cursor = manifest_buffer;
    char* end = manifest_buffer + manifest_len;
    char* line;
    int changed_files = 0;
    int skipped_files = 0;
    make_dir("/home/username/.wine/dosdevices/c:");
    make_parent_dirs(mirror_root);
    make_dir(mirror_root);
    prune_deleted_entries(mirror_root, "", 0);

    line = next_line(&cursor, end);
    if (!line || !starts_with(line, "BW-SHARE-MANIFEST/1")) {
        print("network-share-join: manifest header missing\n");
        return 0;
    }

    while ((line = next_line(&cursor, end))) {
        if (!line[0]) break;
    }

    while ((line = next_line(&cursor, end))) {
        if (starts_with(line, "END")) break;
        if (starts_with(line, "dir path=")) {
            join_path(path_buffer, mirror_root, line + 9);
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
                if (local_file_matches(rel_path, size, hash)) {
                    skipped_files++;
                } else {
                    int total = fetch_file_archive(server_sockaddr, rel_path);
                    if (total < 0) return 0;
                    parse_archive(total);
                    changed_files++;
                }
            }
        }
    }
    map_mirror_drive();
    print("network-share-join: delta changed files ");
    print_uint((u32)changed_files);
    print(" skipped ");
    print_uint((u32)skipped_files);
    print("\n");
    return 1;
}

static void join_run(int argc, char** argv) {
    u8 server_sockaddr[16] = {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int first_sync = 1;
    parse_args(argc, argv);

    print("network-share-join: start\n");
    if (direct_host) {
        set_sockaddr_ip(server_sockaddr, direct_host_ip);
        print("network-share-join: using direct host\n");
    } else if (discover_host(server_sockaddr) < 0) {
        print("network-share-join: discovery failed\n");
        sys1(SYS_EXIT, 1);
    }

    for (;;) {
        u32 manifest_hash = 0;
        int manifest_ok = fetch_manifest(server_sockaddr, &manifest_hash) == 0;
        int fetch_needed = 1;
        if (manifest_ok && have_manifest_hash && !first_sync && manifest_hash == last_manifest_hash) {
            print("network-share-join: no share changes\n");
            fetch_needed = 0;
        }
        if (fetch_needed && !first_sync && manifest_ok) {
            if (apply_manifest_delta(server_sockaddr)) {
                last_manifest_hash = manifest_hash;
                have_manifest_hash = 1;
                first_sync = 0;
                print("network-share-join: sync complete\n");
            } else {
                print("network-share-join: delta failed, falling back to archive\n");
                fetch_needed = 1;
                manifest_ok = 0;
            }
        }
        if (fetch_needed && (first_sync || !manifest_ok)) {
            int total = fetch_archive(server_sockaddr);
            if (total < 0) {
                if (first_sync) sys1(SYS_EXIT, 1);
            } else {
                prune_mirror_root();
                parse_archive(total);
                map_mirror_drive();
                first_sync = 0;
                if (manifest_ok) {
                    last_manifest_hash = manifest_hash;
                    have_manifest_hash = 1;
                } else {
                    have_manifest_hash = 0;
                }
                print("network-share-join: sync complete\n");
            }
        } else {
            first_sync = 0;
        }
        if (run_once || poll_seconds == 0) break;
        print("network-share-join: sleeping ");
        print_uint(poll_seconds);
        print(" seconds\n");
        sleep_poll_interval();
    }
    sys1(SYS_EXIT, 0);
}

NETWORK_SHARE_ENTRY(join_run, "network-share-join.exe")
