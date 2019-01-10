#include "boxedwine.h"
#include "x64Op.h"

U32 X64Op::writeOp(U8* buffer) {
    U32 g8 = 0;
    U32 g8Temp = 0;
    U32 tmpReg = x64_getTmpReg(data);

    // When reference 8-bit regs, 
    //
    // x86 will reference AL, CL, DL, BL, AH, CH, DH, BH
    //
    // x64 will reference R8, R9, R10, R11, R12, R13, R14, R15 if there is a rex prefix
    //
    // so if we are using a rex prefix, we to use a tmp reg
    if (data->rex && data->has_rm && isG8bit && G(data->rm)>=4) {        
        g8=G(data->rm);
        g8Temp = (g8==4?1:0);

        // push rax
        x64_pushNative(data, g8Temp, FALSE);

        // mov al, g8
        x64_writeToRegFromReg(data, g8Temp, FALSE, g8, FALSE, 1);

        // mov HOST_TMP2, eax
        x64_writeToRegFromReg(data, tmpReg, TRUE, g8Temp, FALSE, 4);

        // pop rax
        x64_popNative(data, g8Temp, FALSE);

        data->rex |= REX_BASE | REX_MOD_REG;
        data->rm &=~ (0x7 << 3);
        data->rm |= (tmpReg << 3);        
    }

    // LOCK, REP and OP SIZE prefixes, order does not matter
    if (data->lockPrefix)
        write8(data, 0xF0);
    if (data->cpu->big && data->operandPrefix)
        write8(data, 0x66);
    else if (!data->cpu->big && !data->operandPrefix)
        write8(data, 0x66);
    if (data->repZeroPrefix)
        write8(data, 0xF3);
    if (data->repNotZeroPrefix)
        write8(data, 0xF2);

    // REX must come after prefixes
    if (data->rex)
        write8(data, data->rex);           
    
    if (data->multiBytePrefix)
        write8(data, 0x0F);

    write8(data, data->op);
    if (data->has_rm) {
        write8(data, data->rm);
    }
    if (data->has_sib) {
        write8(data, data->sib);
    }
    if (data->has_disp8) {
        write8(data, data->disp8);
    } else if (data, data->has_disp16) {
        write16(data, data->disp16);
    } else if (data, data->has_disp32) {
        write32(data, data->disp32);
    }
    if (data->has_im8) {
        write8(data, data->im8);
    } else if (data, data->has_im16) {
        write16(data, data->im16);
    } else if (data, data->has_im32) {
        write32(data, data->im32);
    }
    if (isGWritten && g8) {
        // push rax
        x64_pushNative(data, g8Temp, FALSE);

        // mov eax, HOST_TMP2
        x64_writeToRegFromReg(data, g8Temp, FALSE, tmpReg, TRUE, 4);

        // mov g8, al
        x64_writeToRegFromReg(data, g8, FALSE, g8Temp, FALSE, 1);

        // pop rax
        x64_popNative(data, g8Temp, FALSE);
    }
    x64_releaseTmpReg(data, tmpReg);
}