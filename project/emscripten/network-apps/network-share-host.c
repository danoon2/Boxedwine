/*
 * Tiny 32-bit Linux LAN share host for Boxedwine networking.
 *
 * This freestanding i386 ELF scans a configured Boxedwine directory,
 * advertises it over the virtual LAN, and serves a simple archive stream over
 * TCP to network-share-join.
 */

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

#define SYS_EXIT 1
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_LSEEK 19
#define SYS_FCNTL 55
#define SYS_SOCKETCALL 102
#define SYS_NANOSLEEP 162
#define SYS_GETDENTS 141

#define SC_SOCKET 1
#define SC_BIND 2
#define SC_LISTEN 4
#define SC_ACCEPT 5
#define SC_SEND 9
#define SC_RECV 10
#define SC_SENDTO 11

#define AF_INET 2
#define SOCK_STREAM 1
#define SOCK_DGRAM 2
#define IPPROTO_UDP 17
#define F_SETFL 4
#define O_RDONLY 0
#define O_NONBLOCK 0x800
#define SEEK_SET 0
#define SEEK_END 2

static const char default_share_root[] = "/home/username/.wine/dosdevices/c:/host";
static const char default_share_name[] = "c-host";
static const char default_share_drive[] = "c";
static const char default_share_path[] = "host";
static const char default_share_mode[] = "read-only";

static const char* share_root = default_share_root;
static const char* share_name = default_share_name;
static const char* share_drive = default_share_drive;
static const char* share_path = default_share_path;
static const char* share_mode = default_share_mode;
static u32 listen_port = 19200;
static u32 beacon_port = 19201;
static u8 broadcast_ip[4] = {10, 0, 3, 255};

static char dents_buffer[4096];
static char probe_buffer[256];
static char file_buffer[8192];
static char line_buffer[768];

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
        }
    }
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
    append_uint(line_buffer, &pos, value);
    line_buffer[pos] = 0;
    print(line_buffer);
}

static void copy_str(char* dst, const char* src) {
    int i = 0;
    while (src[i]) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;
}

static void join_path(char* dst, const char* base, const char* name) {
    int pos = 0;
    append_str(dst, &pos, base);
    if (pos > 0 && dst[pos - 1] != '/') append_char(dst, &pos, '/');
    append_str(dst, &pos, name);
    dst[pos] = 0;
}

static void set_sockaddr_port(u8* sockaddr, u32 port) {
    sockaddr[2] = (u8)((port >> 8) & 0xff);
    sockaddr[3] = (u8)(port & 0xff);
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

    print("network-share-host: sending file ");
    print(rel_path);
    print("\n");

    pos = 0;
    append_str(line_buffer, &pos, "file path=");
    append_str(line_buffer, &pos, rel_path);
    append_str(line_buffer, &pos, " size=");
    append_uint(line_buffer, &pos, (u32)size);
    append_char(line_buffer, &pos, '\n');
    line_buffer[pos] = 0;
    send_all(client_fd, line_buffer, pos);

    while (size > 0) {
        int want = size > (int)sizeof(file_buffer) ? (int)sizeof(file_buffer) : size;
        int got = sys3(SYS_READ, fd, (int)file_buffer, want);
        if (got <= 0) break;
        send_all(client_fd, file_buffer, got);
        size -= got;
    }
    send_text(client_fd, "\nendfile\n");
    sys1(SYS_CLOSE, fd);
}

static int path_is_dir(const char* path) {
    int fd = sys3(SYS_OPEN, (int)path, O_RDONLY, 0);
    int result;
    if (fd < 0) return 0;
    result = sys3(SYS_GETDENTS, fd, (int)probe_buffer, (int)sizeof(probe_buffer));
    sys1(SYS_CLOSE, fd);
    return result >= 0;
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
                if (!path_is_dir(child_full)) {
                    send_file(client_fd, child_full, child_rel);
                }
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
                if (!path_is_dir(child_full)) {
                    send_manifest_file(client_fd, child_full, child_rel);
                }
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
    char full_path[768];
    char clean_path[512];
    int i = 0;
    while (rel_path[i] && rel_path[i] != '\r' && rel_path[i] != '\n' && i < (int)sizeof(clean_path) - 1) {
        clean_path[i] = rel_path[i];
        i++;
    }
    clean_path[i] = 0;
    send_archive_header(client_fd);
    if (safe_rel_path(clean_path)) {
        join_path(full_path, share_root, clean_path);
        if (!path_is_dir(full_path)) send_file(client_fd, full_path, clean_path);
    }
    send_text(client_fd, "END\n");
}

static void send_manifest(int client_fd) {
    int pos = 0;
    u32 hash = calculate_share_hash();
    append_str(line_buffer, &pos, "BW-SHARE-MANIFEST/1\nname=");
    append_str(line_buffer, &pos, share_name);
    append_str(line_buffer, &pos, "\nhash=");
    append_uint(line_buffer, &pos, hash);
    append_str(line_buffer, &pos, "\nentries=recursive\n\n");
    line_buffer[pos] = 0;
    send_all(client_fd, line_buffer, pos);
    send_manifest_entries(client_fd, share_root, "", 0);
    send_text(client_fd, "END\n");
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
    listen_args[1] = 1;
    if (socketcall(SC_LISTEN, listen_args) < 0) return -1;
    sys3(SYS_FCNTL, fd, F_SETFL, O_NONBLOCK);
    return fd;
}

static int create_udp_socket(void) {
    u32 socket_args[3] = {AF_INET, SOCK_DGRAM, IPPROTO_UDP};
    return socketcall(SC_SOCKET, socket_args);
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

static void sleep_beacon(void) {
    u32 ts[2] = {0, 250000000};
    sys2(SYS_NANOSLEEP, (int)ts, 0);
}

static void host_run(int argc, char** argv) {
    int listener_fd;
    int udp_fd;
    parse_args(argc, argv);

    print("network-share-host: start\n");
    print("network-share-host: sharing ");
    print(share_root);
    print("\n");

    listener_fd = create_tcp_listener();
    if (listener_fd < 0) {
        print("network-share-host: listen failed\n");
        sys1(SYS_EXIT, 1);
    }
    print("network-share-host: listening on 0.0.0.0:");
    print_uint(listen_port);
    print("\n");

    udp_fd = create_udp_socket();
    if (udp_fd < 0) {
        print("network-share-host: udp socket failed\n");
        sys1(SYS_EXIT, 1);
    }
    print("network-share-host: advertising share ");
    print(share_name);
    print("\n");

    for (;;) {
        u32 accept_args[3] = {(u32)listener_fd, 0, 0};
        int client_fd;
        advertise_once(udp_fd);
        client_fd = socketcall(SC_ACCEPT, accept_args);
        if (client_fd >= 0) {
            char request[768];
            u32 recv_args[4] = {(u32)client_fd, (u32)request, sizeof(request), 0};
            int received;
            print("network-share-host: client connected\n");
            recv_args[2] = sizeof(request) - 1;
            received = socketcall(SC_RECV, recv_args);
            if (received > 0) request[received] = 0;
            else request[0] = 0;
            if (starts_with(request, "GET manifest")) {
                send_manifest(client_fd);
                print("network-share-host: manifest sent\n");
            } else if (starts_with(request, "GET file path=")) {
                send_one_file_archive(client_fd, request + 14);
                print("network-share-host: file archive sent\n");
            } else {
                send_archive(client_fd);
                print("network-share-host: archive sent\n");
            }
            sys1(SYS_CLOSE, client_fd);
        }
        sleep_beacon();
    }
    sys1(SYS_CLOSE, udp_fd);
    sys1(SYS_CLOSE, listener_fd);
    sys1(SYS_EXIT, 0);
}

NETWORK_SHARE_ENTRY(host_run, "network-share-host.exe")
