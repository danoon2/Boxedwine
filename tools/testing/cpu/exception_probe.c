/* Compare 32-bit x86 exceptions on Windows (VEH), Linux (sigaction), and Wine.
 * gcc -m32 -O0 exception_probe.c -o exception_probe
 * i686-w64-mingw32-gcc -O0 exception_probe.c -o exception_probe.exe
 * An optional case name runs just that case. No game or installer is involved.
 */
#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <signal.h>
#include <ucontext.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <asm/ldt.h>
#endif

static unsigned char *code;
static unsigned char *protected_page;
static uintptr_t resume_ip;
static volatile unsigned count, code_value, signal_value, trap_value, error_value, flags_value, errno_value;
static volatile uintptr_t event_ip, address, fault_address;
#ifdef _WIN32
static LONG CALLBACK handler(EXCEPTION_POINTERS *p)
{
    ++count;
    code_value=p->ExceptionRecord->ExceptionCode;
    event_ip=p->ContextRecord->Eip;
    address=(uintptr_t)p->ExceptionRecord->ExceptionAddress;
    flags_value=p->ContextRecord->EFlags;
    if (code_value == EXCEPTION_ACCESS_VIOLATION && p->ExceptionRecord->NumberParameters >= 2)
        fault_address=p->ExceptionRecord->ExceptionInformation[1];
    p->ContextRecord->Eip=resume_ip;
    p->ContextRecord->EFlags &= ~0x100; /* Do not single step the harness. */
    return EXCEPTION_CONTINUE_EXECUTION;
}
#else
static void handler(int sig,siginfo_t *si,void *ptr)
{
    ucontext_t *ctx=ptr;
    ++count; signal_value=sig; code_value=si->si_code; errno_value=si->si_errno;
    event_ip=ctx->uc_mcontext.gregs[REG_EIP]; address=(uintptr_t)si->si_addr;
    trap_value=ctx->uc_mcontext.gregs[REG_TRAPNO];
    error_value=ctx->uc_mcontext.gregs[REG_ERR];
    flags_value=ctx->uc_mcontext.gregs[REG_EFL];
    fault_address=ctx->uc_mcontext.cr2;
    ctx->uc_mcontext.gregs[REG_EIP]=resume_ip;
    ctx->uc_mcontext.gregs[REG_EFL] &= ~0x100;
}
#endif
static const struct { const char *name; unsigned char bytes[32]; unsigned length, offset; } cases[]={
    {"int4_of0",{0x31,0xc0,0xcd,0x04},4,2},
    {"int4_of1",{0xb0,0x7f,0x04,0x01,0xcd,0x04},6,4},
    {"int4_prefix",{0x31,0xc0,0x66,0xcd,0x04},5,2},
    {"int3",{0x31,0xc0,0xcc},3,2},
    {"int3_long",{0x31,0xc0,0xcd,0x03},4,2},
    {"int3_prefix",{0x31,0xc0,0x66,0xcc},4,2},
    {"int1",{0x31,0xc0,0xcd,0x01},4,2},
    {"int5",{0x31,0xc0,0xcd,0x05},4,2},
    {"into_of0",{0x31,0xc0,0xce},3,2},
    {"into_of1",{0xb0,0x7f,0x04,0x01,0xce},5,4},
    {"icebp",{0x31,0xc0,0xf1},3,2},
    {"single_step",{0x9c,0x81,0x0c,0x24,0,1,0,0,0x9d,0x90},10,9},
    {"tf_ud2",{0x9c,0x81,0x0c,0x24,0,1,0,0,0x9d,0x0f,0x0b},11,9},
    {"tf_int3",{0x9c,0x81,0x0c,0x24,0,1,0,0,0x9d,0xcc},10,9},
    {"ud2",{0x31,0xc0,0x0f,0x0b},4,2},
    {"ud1",{0x31,0xc0,0x0f,0xb9,0xc0},5,2},
    {"hlt",{0x31,0xc0,0xf4},3,2},
    {"cli",{0x31,0xc0,0xfa},3,2},
    {"sti",{0x31,0xc0,0xfb},3,2},
    {"div8_zero",{0x31,0xc0,0x31,0xc9,0xf6,0xf1},6,4},
    {"aam_zero",{0x31,0xc0,0xd4,0},4,2},
    {"div8_overflow",{0xb8,0,1,0,0,0xb9,1,0,0,0,0xf6,0xf1},12,10},
    {"idiv8_overflow",{0xb8,0x80,0xff,0,0,0xb9,0xff,0,0,0,0xf6,0xf9},12,10},
    {"div16_zero",{0x31,0xc0,0x31,0xd2,0x31,0xc9,0x66,0xf7,0xf1},9,6},
    {"div16_overflow",{0x31,0xc0,0xba,1,0,0,0,0xb9,1,0,0,0,0x66,0xf7,0xf1},15,12},
    {"idiv16_overflow",{0xb8,0,0x80,0,0,0xba,0xff,0xff,0,0,0xb9,0xff,0xff,0,0,0x66,0xf7,0xf9},18,15},
    {"div32_zero",{0x31,0xc0,0x31,0xd2,0x31,0xc9,0xf7,0xf1},8,6},
    {"div32_overflow",{0x31,0xc0,0xba,1,0,0,0,0xb9,1,0,0,0,0xf7,0xf1},14,12},
    {"idiv32_overflow",{0xb8,0,0,0,0x80,0xba,0xff,0xff,0xff,0xff,0xb9,0xff,0xff,0xff,0xff,0xf7,0xf9},17,15},
    {"bound32",{0xb8,2,0,0,0,0xb9,0,0,0,0,0x62,0x01},12,10},
    {"bound16",{0xb8,2,0,0,0,0xb9,0,0,0,0,0x66,0x62,0x01},13,10},
    {"page_read",{0x31,0xc0,0xa1,0,0x20,0,0},7,2},
    {"page_write",{0x31,0xc0,0xa3,0,0x20,0,0},7,2},
#ifndef _WIN32
    {"segment_np",{0xb8,0x87,0,0,0,0x8e,0xd8},7,5},
    {"stack_np",{0xb8,0x87,0,0,0,0x8e,0xd0},7,5},
#endif
    {"page_noaccess_read",{0x31,0xc0,0xa1,0,0,0,0},7,2},
    {"page_noaccess_write",{0x31,0xc0,0xa3,0,0,0,0},7,2},
    {"page_readonly_write",{0x31,0xc0,0xa3,0,0,0,0},7,2},
};
int main(int argc,char **argv)
{
    unsigned i, mode;
    int32_t bounds[2]={0,1};
    if (argc > 2 && !freopen(argv[2],"w",stdout)) { perror("output");return 1; }
#ifdef _WIN32
    code=VirtualAlloc(NULL,4096,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE);
    protected_page=VirtualAlloc(NULL,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    AddVectoredExceptionHandler(1,handler);
#else
    struct sigaction sa={0}; int signals[]={SIGSEGV,SIGTRAP,SIGILL,SIGFPE,SIGBUS};
    code=mmap(NULL,4096,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
    protected_page=mmap(NULL,4096,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
    sa.sa_sigaction=handler;sa.sa_flags=SA_SIGINFO;
    for(i=0;i<sizeof(signals)/sizeof(signals[0]);i++) sigaction(signals[i],&sa,NULL);
    struct user_desc desc={0};
    desc.entry_number=16;desc.limit=0xfffff;desc.seg_32bit=1;
    desc.limit_in_pages=1;desc.seg_not_present=1;desc.useable=1;
    if (syscall(SYS_modify_ldt,0x11,&desc,sizeof(desc))) { perror("modify_ldt");return 1; }
#endif
    for(i=0;i<sizeof(cases)/sizeof(cases[0]);i++) for(mode=0;mode<4;mode++) {
        unsigned length=cases[i].length, offset=cases[i].offset;
        uint32_t divisor=strstr(cases[i].name,"overflow") ? (cases[i].name[0]=='i' ? 0xffffffff : 1) : 0;
        uintptr_t ptr=(uintptr_t)&divisor;
        char name[80];
        static const char *suffix[]={"","_absolute","_indirect","_sib_disp8"};
        if(mode && strncmp(cases[i].name,"div",3) && strncmp(cases[i].name,"idiv",4)) continue;
        snprintf(name,sizeof(name),"%s%s",cases[i].name,suffix[mode]);
        if(argc>1 && strcmp(argv[1],"*") && strcmp(argv[1],name)) continue;
        count=code_value=signal_value=trap_value=error_value=flags_value=errno_value=0;
        event_ip=address=fault_address=0;
        memcpy(code,cases[i].bytes,length);
        if(mode>1) {
            if(mode==3) ptr+=4;
            memmove(code+offset+5,code+offset,length-offset);
            code[offset]=0xb9;memcpy(code+offset+1,&ptr,4);offset+=5;length+=5;
        }
        if(mode==1) {code[length-1]=(code[length-1]&0x38)|0x05;memcpy(code+length,&ptr,4);length+=4;}
        if(mode==2) code[length-1]=(code[length-1]&0x38)|0x01;
        if(mode==3) {code[length-1]=(code[length-1]&0x38)|0x44;code[length++]=0x21;code[length++]=0xfc;}
        code[length]=0xc3;
        if (!strncmp(name,"page_noaccess",13) || !strcmp(name,"page_readonly_write")) {
            uintptr_t page=(uintptr_t)protected_page;
            int readonly=!strcmp(name,"page_readonly_write");
            protected_page[0]=0;
            memcpy(code+3,&page,4);
#ifdef _WIN32
            DWORD previous;
            VirtualProtect(protected_page,4096,readonly?PAGE_READONLY:PAGE_NOACCESS,&previous);
#else
            mprotect(protected_page,4096,readonly?PROT_READ:PROT_NONE);
#endif
        }
        if (!strncmp(cases[i].name,"bound",5)) {
            uintptr_t ptr=(uintptr_t)bounds;
            bounds[0]=0;bounds[1]=1;
            if (!strcmp(cases[i].name,"bound16")) bounds[0]=0x10000;
            memcpy(code+6,&ptr,4);
        }
        resume_ip=(uintptr_t)(code+length);
        ((void (*)(void))code)();
        printf("%s count=%u signal=%u code=%08x errno=%u trap=%u error=%x ip_offset=%ld addr_offset=%ld address=%08lx cr2=%08lx flags=%x page_offset=%ld\n",
               name,count,signal_value,code_value,errno_value,trap_value,error_value,
               event_ip?(long)(event_ip-(uintptr_t)(code+offset)):-99,
               address?(long)(address-(uintptr_t)(code+offset)):-99,
               (unsigned long)address,(unsigned long)fault_address,flags_value,(long)(fault_address-(uintptr_t)protected_page));
        fflush(stdout);
#ifdef _WIN32
        { DWORD previous; VirtualProtect(protected_page,4096,PAGE_READWRITE,&previous); }
#else
        mprotect(protected_page,4096,PROT_READ|PROT_WRITE);
#endif
    }
    return 0;
}
