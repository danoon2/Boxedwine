/* Run with networking enabled and a room peer at 10.0.3.2 that has no TCP
 * listener on port 18659. Build using the share apps' freestanding ELF flags. */
typedef unsigned int u32;
typedef unsigned short u16;
#define SYS_EXIT 1
static int sys1(int n, int a) {
    int r; __asm__ volatile("int $0x80" : "=a"(r) : "a"(n), "b"(a) : "memory"); return r;
}
static int sys3(int n, int a, int b, int c) {
    int r; __asm__ volatile("int $0x80" : "=a"(r) : "a"(n), "b"(a), "c"(b), "d"(c) : "memory"); return r;
}
static int sys5(int n, int a, int b, int c, int d, int e) {
    int r; __asm__ volatile("int $0x80" : "=a"(r) : "a"(n), "b"(a), "c"(b), "d"(c), "S"(d), "D"(e) : "memory"); return r;
}
static void check(int ok, const char* message) {
    int n = 0;
    if (ok) return;
    while (message[n]) n++;
    sys3(4, 1, (int)message, n);
    sys1(SYS_EXIT, 1);
}
static void poll_test(int argc, char** argv) {
    static u32 args[5] = {2, 1, 0};
    static unsigned char address[16] = {2, 0, 72, 227, 10, 0, 3, 2};
    struct { int fd; short events, revents; } p;
    u32 reads, writes, error = 0, len = 4;
    static int timeout[2];
    int fd = sys3(102, 1, (int)args, 0);
    check(fd >= 0 && fd < 32, "FAIL socket\n");
    sys3(55, fd, 4, 0x800);
    args[0] = fd; args[1] = (u32)address; args[2] = 16;
    check(sys3(102, 3, (int)args, 0) == -115, "FAIL nonblocking connect\n");
    p.fd = fd; p.events = 4; p.revents = 0;
    check(sys3(168, (int)&p, 1, 3000) == 1 && (p.revents & 8), "FAIL poll refused connection\n");
    reads = 0; writes = 1u << fd;
    check(sys5(142, fd + 1, (int)&reads, (int)&writes, 0, (int)timeout) == 1 && reads == 0 && writes == (1u << fd), "FAIL select write set\n");
    reads = 1u << fd; writes = 0;
    check(sys5(142, fd + 1, (int)&reads, (int)&writes, 0, (int)timeout) == 1 && reads == (1u << fd) && writes == 0, "FAIL select read set\n");
    args[0] = fd; args[1] = 1; args[2] = 4; args[3] = (u32)&error; args[4] = (u32)&len;
    check(sys3(102, 15, (int)args, 0) == 0 && error == 111, "FAIL SO_ERROR\n");
    sys1(6, fd);
    sys3(4, 1, (int)"NETWORK POLL TEST PASSED\n", 25);
    sys1(SYS_EXIT, 0);
}
#include "network-share-entry.h"
NETWORK_SHARE_ENTRY(poll_test, "network-poll-test")
