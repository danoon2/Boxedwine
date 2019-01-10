#ifndef __X64OPS_H__
#define __X64OPS_H__

class X64Asm;

typedef U32 (*X64Decoder)(X64Asm* data);

extern X64Decoder x64Decoder[1024];

#endif