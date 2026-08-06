/*
 * Entry point helpers for the tiny network share apps.
 *
 * The Linux build starts from the raw i386 process stack. The Win32 PE build is
 * intentionally import-free so Wine can load it while the app still uses the
 * Boxedwine Linux syscall path for files and sockets.
 */

#ifdef _WIN32
__attribute__((used, naked)) void _chkstk(void) {
    __asm__ volatile("ret");
}

static int pe_copy_command_line(char* dst, int cap) {
    void* peb = 0;
    void* params;
    u16 bytes;
    u16* wide;
    int chars;
    int i;

    __asm__ volatile("movl %%fs:0x30, %0" : "=r"(peb));
    if (!peb || cap <= 0) return 0;
    params = *(void**)((char*)peb + 0x10);
    if (!params) return 0;
    bytes = *(u16*)((char*)params + 0x40);
    wide = *(u16**)((char*)params + 0x44);
    if (!wide) return 0;

    chars = (int)(bytes / 2);
    if (chars >= cap) chars = cap - 1;
    for (i = 0; i < chars; i++) {
        u16 ch = wide[i];
        dst[i] = (ch >= 32 && ch < 127) ? (char)ch : ' ';
    }
    dst[chars] = 0;
    return chars;
}

static int pe_build_argv(char* cmd, int cmd_cap, char** argv, int argv_cap, char* fallback_argv0) {
    int argc = 0;
    char* src;
    char* dst;

    if (pe_copy_command_line(cmd, cmd_cap) <= 0) {
        argv[0] = fallback_argv0;
        return 1;
    }

    src = cmd;
    dst = cmd;
    while (*src && argc < argv_cap) {
        int quoted = 0;
        while (*src == ' ' || *src == '\t') src++;
        if (!*src) break;
        argv[argc++] = dst;
        while (*src) {
            if (*src == '"') {
                quoted = !quoted;
                src++;
                continue;
            }
            if (!quoted && (*src == ' ' || *src == '\t')) break;
            *dst++ = *src++;
        }
        *dst++ = 0;
        while (*src == ' ' || *src == '\t') src++;
    }
    if (argc == 0) {
        argv[0] = fallback_argv0;
        argc = 1;
    }
    return argc;
}

#define NETWORK_SHARE_ENTRY(run_fn, exe_name) \
    __attribute__((used)) void start(void) { \
        static char cmdline[2048]; \
        static char* argv[64]; \
        static char fallback[] = exe_name; \
        int argc = pe_build_argv(cmdline, (int)sizeof(cmdline), argv, (int)(sizeof(argv) / sizeof(argv[0])), fallback); \
        run_fn(argc, argv); \
        sys1(SYS_EXIT, 0); \
        for (;;) {} \
    }
#else
#define BW_CONCAT2(a, b) a##b
#define BW_CONCAT(a, b) BW_CONCAT2(a, b)
#define NETWORK_SHARE_ENTRY(run_fn, exe_name) \
    __attribute__((used, noinline)) void BW_CONCAT(run_fn, _linux_main)(u32* stack) { \
        int argc = (int)stack[0]; \
        char** argv = (char**)(&stack[1]); \
        run_fn(argc, argv); \
    } \
    __attribute__((naked)) void _start(void) { \
        __asm__ volatile( \
            "movl %esp, %eax\n" \
            "pushl %eax\n" \
            "call " #run_fn "_linux_main\n" \
            "hlt\n"); \
    }
#endif
