#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
typedef uint32_t DWORD;
typedef uint8_t BYTE;
typedef int BOOL;
typedef struct { DWORD Eax,Ecx,Edx,Ebx,Esp,Ebp,Esi,Edi,Eip,SegCs,SegDs,SegEs,SegFs,SegGs,SegSs; } CONTEXT;
typedef struct { DWORD base; struct { struct { unsigned Default_Big; } Bits; } HighWord; } LDT_ENTRY;
typedef struct { DWORD Selector; LDT_ENTRY Entry; } THREAD_DESCRIPTOR_INFORMATION;
#define EXCEPTION_INT_OVERFLOW 0xc0000095u
#define EXCEPTION_INT_DIVIDE_BY_ZERO 0xc0000094u
static BYTE code[32], data[65536], fsdata[65536];
static unsigned code_len, code32=1, checks;
static int GetCurrentThread(void) { return 0; }
static DWORD ldt_get_base(LDT_ENTRY e) { return e.base; }
static int get_thread_ldt_entry(int thread, THREAD_DESCRIPTOR_INFORMATION *desc, unsigned size)
{
    (void)thread;(void)size;
    if(desc->Selector==1) desc->Entry.base=(DWORD)code;
    else if(desc->Selector==2) desc->Entry.base=(DWORD)data;
    else if(desc->Selector==3) desc->Entry.base=(DWORD)fsdata;
    else return 1;
    desc->Entry.HighWord.Bits.Default_Big=code32;
    return 0;
}
static unsigned virtual_uninterrupted_read_memory(const void *src,void *dst,unsigned size)
{
    uintptr_t p=(uintptr_t)src;
    const BYTE *regions[]={code,data,fsdata};
    unsigned lengths[]={code_len,sizeof(data),sizeof(fsdata)},i;
    for(i=0;i<3;i++) if(p>=(uintptr_t)regions[i] && p<(uintptr_t)regions[i]+lengths[i]) {
        unsigned available=(uintptr_t)regions[i]+lengths[i]-p;
        if(size>available)size=available;
        memcpy(dst,src,size);return size;
    }
    return 0;
}
#include "integer_exception_impl.h"
static void check(CONTEXT *ctx,const BYTE *bytes,unsigned len,unsigned offset,unsigned size,int fs,int overflow)
{
    DWORD expected=overflow?EXCEPTION_INT_OVERFLOW:EXCEPTION_INT_DIVIDE_BY_ZERO;
    memset(code,0,sizeof(code));memcpy(code,bytes,len);code_len=len;
    memset(data,0,sizeof(data));memset(fsdata,0,sizeof(fsdata));
    if(offset<65536) {
        BYTE *p=(fs?fsdata:data)+offset;
        /* Nonzero only in the most significant byte checks operand width. */
        if(overflow)p[size-1]=0x80;
        if(size<4)p[size]=0x80; /* bytes outside the operand must be ignored */
    }
    if(get_integer_divide_code(ctx)!=expected) {
        fprintf(stderr,"failed case %u len=%u offset=%u size=%u fs=%d overflow=%d\n",checks,len,offset,size,fs,overflow);
        exit(1);
    }
    checks++;
}
int main(void)
{
    CONTEXT ctx={0};unsigned o,w,m;
    ctx.SegCs=1;ctx.SegDs=ctx.SegEs=ctx.SegSs=2;ctx.SegFs=3;
    ctx.Ebx=0x100;ctx.Esi=0x20;ctx.Edi=0x40;ctx.Ebp=0x180;ctx.Ecx=0x200;ctx.Edx=3;
    for(o=0;o<2;o++) for(w=0;w<3;w++) {
        BYTE reg[]={0xf7,0xf1},ind[]={0xf7,0x31},abs[]={0xf7,0x35,0x20,0,0,0};
        BYTE sib[]={0xf7,0x74,0x91,0xfc}, disp[]={0xf7,0xb1,0xf0,0xff,0xff,0xff};
        BYTE seg[]={0x64,0xf7,0x31},prefixed[16];
        BYTE *forms[]={reg,ind,abs,sib,disp,seg};
        unsigned lens[]={2,2,6,4,6,3},offsets[]={65536,0x200,0x20,0x208,0x1f0,0x200};
        unsigned size=w==0?1:w==1?2:4;
        for(m=0;m<6;m++) {
            unsigned skip=w==1,opcode=m==5?1:0;
            memcpy(prefixed+skip,forms[m],lens[m]);
            if(skip)prefixed[0]=0x66;
            if(!w)prefixed[skip+opcode]=0xf6;
            ctx.Ecx=m==0?(o?(1u<<(size*8-1)):0):0x200;
            check(&ctx,prefixed,lens[m]+skip,offsets[m],size,m==5,o);
        }
        ctx.Ecx=0x200;
        for(m=0;m<8;m++) {
            BYTE bytes[]={0x67,0xf7,(BYTE)(0x30|m),0x60,0};
            unsigned offsets16[]={0x120,0x140,0x1a0,0x1c0,0x20,0x40,0x60,0x100};
            unsigned len=m==6?5:3,skip=w==1;
            memcpy(prefixed+skip,bytes,len);
            if(skip)prefixed[0]=0x66;
            if(!w)prefixed[skip+1]=0xf6;
            check(&ctx,prefixed,len+skip,offsets16[m],size,0,o);
        }
    }
    ctx.Ecx=0x100;
    { const BYTE high[]={0xf6,0xf5};check(&ctx,high,sizeof(high),65536,1,0,1); }
    ctx.Ecx=0x10000;
    { const BYTE twice[]={0x66,0x66,0xf7,0xf1};check(&ctx,twice,sizeof(twice),65536,2,0,0); }
    code32=0;
    { const BYTE word[]={0xf7,0x36,0x20,0};check(&ctx,word,sizeof(word),0x20,2,0,1); }
    { const BYTE dword[]={0x66,0x67,0xf7,0x35,0x20,0,0,0};check(&ctx,dword,sizeof(dword),0x20,4,0,1); }
    code32=1;
    { const BYTE trunc[]={0xf7,0x35};check(&ctx,trunc,sizeof(trunc),65536,4,0,0); }
    { const BYTE invalid[]={0xf7,0xc1};check(&ctx,invalid,sizeof(invalid),65536,4,0,0); }
    { const BYTE badmem[]={0xf7,0x35,0,0,1,0};check(&ctx,badmem,sizeof(badmem),65536,4,0,0); }
    { const BYTE aam[]={0xd4,0};check(&ctx,aam,sizeof(aam),65536,1,0,0); }
    printf("%u integer-exception decoder checks passed\n",checks);
    return 0;
}
