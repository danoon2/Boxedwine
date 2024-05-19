#include "boxedwine.h"

#include "instructions.h"

U32 div8(CPU* cpu, U8 src) {
    if (src==0) {
        cpu->prepareException(EXCEPTION_DIVIDE, 0);
        return 0;
    }

    U16 quo = AX / src;
    U8 rem = AX % src;

    if (quo > 255) {
        cpu->prepareException(EXCEPTION_DIVIDE, 1);
        return 0;
    }
    AL = (U8)quo;
    AH = rem;
    return 1;
}

U32 idiv8(CPU* cpu, S8 src) {
    if (src==0) {
        cpu->prepareException(EXCEPTION_DIVIDE, 0);
        return 0;
    }

    S16 quo = (S16)AX / src;
    S8 quo8 = (S8)quo;
    S8 rem = (S16)AX % src;

    if (quo != quo8) {
        cpu->prepareException(EXCEPTION_DIVIDE, 1);
        return 0;
    }
    AL = quo8;
    AH = rem;
    return 1;
}

U32 div16(CPU* cpu, U16 src) {	
    U32 num = ((U32)DX << 16) | AX;

    if (src==0) {	
        cpu->prepareException(EXCEPTION_DIVIDE, 0);
        return 0;
    }
    U32 quo=num/src;
    U16 rem=(U16)(num % src);
    U16 quo16=(U16)quo;
    if (quo!=(U32)quo16) {
        cpu->prepareException(EXCEPTION_DIVIDE, 1);
        return 0;
    }
    DX=rem;
    AX=quo16;
    return 1;
}

U32 idiv16(CPU* cpu, S16 src) {
    S32 num = (S32)(((U32)DX << 16) | AX);

    if (src==0) {
        cpu->prepareException(EXCEPTION_DIVIDE, 0);
        return 0;
    }
    S32 quo=num/src;
    S16 rem=(S16)(num % src);
    S16 quo16s=(S16)quo;
    if (quo!=(S32)quo16s) {
        cpu->prepareException(EXCEPTION_DIVIDE, 1);
        return 0;
    }
    DX=rem;
    AX=quo16s;
    return 1;
}

U32 div32(CPU* cpu, U32 src) {	
    U64 num = ((U64)EDX << 32) | EAX;

    if (src==0)	{
        cpu->prepareException(EXCEPTION_DIVIDE, 0);
        return 0;
    }

    U64 quo=num/src;
    U32 rem=(U32)(num % src);
    U32 quo32=(U32)quo;
    if (quo!=(U64)quo32) {
        cpu->prepareException(EXCEPTION_DIVIDE, 1);
        return 0;
    }
    EDX=rem;
    EAX=quo32;
    return 1;
}

U32 idiv32(CPU* cpu, S32 src) {
    S64 num = (S64)(((U64)EDX << 32) | EAX);

    if (src==0) {
        cpu->prepareException(EXCEPTION_DIVIDE, 0);
        return 0;
    }

    S64 quo=num/src;
    S32 rem=(S32)(num % src);
    S32 quo32s=(S32)quo;
    if (quo!=(S64)quo32s) {
        cpu->prepareException(EXCEPTION_DIVIDE, 1);
        return 0;
    }
    EDX=rem;
    EAX=quo32s;
    return 1;
}

void dshlr16r16(CPU* cpu, U32 reg, U32 rm, U32 imm) {
    cpu->src.u32 = imm;
    cpu->dst.u32 = cpu->reg[reg].u16;
    cpu->dst2.u32 = cpu->reg[rm].u16;
    U32 tmp = (cpu->dst.u32<<16)|cpu->dst2.u32;
    U32 result=tmp << cpu->src.u8;
    if (imm>16) {
        //klog("dshlr16r16: imm=%x",imm);
        result |= ((U32)(cpu->reg[rm].u16) << (imm - 16));
    }
    cpu->result.u16=(U16)(result >> 16);
    cpu->reg[reg].u16 = cpu->result.u16;
    cpu->lazyFlags=FLAGS_DSHL16;
}

void dshle16r16(CPU* cpu, U32 reg, U32 address, U32 imm) {
    cpu->src.u32 = imm;
    cpu->dst.u32 = cpu->memory->readw(address);
    cpu->dst2.u32 = cpu->reg[reg].u16;
    U32 tmp = (cpu->dst.u32<<16)|cpu->dst2.u32;
    U32 result=tmp << cpu->src.u8;
    if (imm>16) {
        //klog("dshle16r16: imm=%x",imm);
        result |= ((U32)(cpu->reg[reg].u16) << (imm - 16));
    }
    cpu->result.u16=(U16)(result >> 16);
    cpu->memory->writew(address, cpu->result.u16);
    cpu->lazyFlags=FLAGS_DSHL16;
}

void dshlr32r32(CPU* cpu, U32 reg, U32 rm, U32 imm) {
    cpu->src.u32=imm;
    cpu->dst.u32=cpu->reg[reg].u32;
    cpu->result.u32=(cpu->reg[reg].u32 << imm) | (cpu->reg[rm].u32 >> (32-imm));
    cpu->reg[reg].u32 = cpu->result.u32;	
    cpu->lazyFlags=FLAGS_DSHL32;
}

void dshle32r32(CPU* cpu, U32 reg, U32 address, U32 imm) {
    cpu->src.u32=imm;
    cpu->dst.u32= cpu->memory->readd(address);
    cpu->result.u32=(cpu->dst.u32 << imm) | (cpu->reg[reg].u32 >> (32-imm));
    cpu->memory->writed(address, cpu->result.u32);
    cpu->lazyFlags=FLAGS_DSHL32;
}

void dshlclr16r16(CPU* cpu, U32 reg, U32 rm) {
    if (CL & 0x1f) {
        cpu->src.u32 = CL & 0x1f;
        cpu->dst.u32 = cpu->reg[reg].u16;
        cpu->dst2.u32 = cpu->reg[rm].u16;
        U32 tmp = (cpu->dst.u32<<16)|cpu->dst2.u32;
        U32 result=tmp << cpu->src.u8;
        if (cpu->src.u32>16) {
            //klog("error: dshlclr16r16 cl=%x",CL);
            result |= ((U32)(cpu->reg[rm].u16) << (cpu->src.u32 - 16));
        }
        cpu->result.u16=(U16)(result >> 16);
        cpu->reg[reg].u16 = cpu->result.u16;
        cpu->lazyFlags=FLAGS_DSHL16;
    }
}

void dshlcle16r16(CPU* cpu, U32 reg, U32 address) {
    if (CL & 0x1f) {
        cpu->src.u32 = CL & 0x1f;
        cpu->dst.u32 = cpu->memory->readw(address);
        cpu->dst2.u32 = cpu->reg[reg].u16;
        U32 tmp = (cpu->dst.u32<<16)|cpu->dst2.u32;
        U32 result=tmp << cpu->src.u8;
        if (cpu->src.u32>16) {
            //klog("error: dshlcle16r16 cl=%x",CL);
            result |= ((U32)(cpu->reg[reg].u16) << (cpu->src.u32 - 16));
        }
        cpu->result.u16=(U16)(result >> 16);
        cpu->memory->writew(address, cpu->result.u16);
        cpu->lazyFlags=FLAGS_DSHL16;
    }
}

void dshlclr32r32(CPU* cpu, U32 reg, U32 rm) {
    if (CL & 0x1f) {
        cpu->src.u32=CL & 0x1f;
        cpu->dst.u32=cpu->reg[reg].u32;
        cpu->result.u32=(cpu->dst.u32 << cpu->src.u32);
        cpu->result.u32|=(cpu->reg[rm].u32 >> (32-cpu->src.u32));
        cpu->reg[reg].u32 = cpu->result.u32;	
        cpu->lazyFlags=FLAGS_DSHL32;
    }
}

void dshlcle32r32(CPU* cpu, U32 reg, U32 address) {
    if (CL & 0x1f) {
        cpu->src.u32=CL & 0x1f;
        cpu->dst.u32= cpu->memory->readd(address);
        cpu->result.u32=(cpu->dst.u32 << cpu->src.u32);
        cpu->result.u32|=(cpu->reg[reg].u32 >> (32-cpu->src.u32));
        cpu->memory->writed(address, cpu->result.u32);
        cpu->lazyFlags=FLAGS_DSHL32;
    }
}

void dshrr16r16(CPU* cpu, U32 reg, U32 rm, U32 imm) {
    cpu->src.u32 = imm;
    cpu->dst.u32 = (cpu->reg[reg].u16)|((U32)(cpu->reg[rm].u16)<<16);
    U32 result=cpu->dst.u32 >> cpu->src.u8;
    if (imm>16) {
        //klog("error: dshrr16r16 imm=%x",imm);
        result |= ((U32)(cpu->reg[rm].u16) << (32 - imm));
    }
    cpu->result.u16=(U16)result;
    cpu->reg[reg].u16 = cpu->result.u16;
    cpu->lazyFlags=FLAGS_DSHR16;
}

void dshre16r16(CPU* cpu, U32 reg, U32 address, U32 imm) {
    cpu->src.u32 = imm;
    cpu->dst.u32 = cpu->memory->readw(address)|((U32)(cpu->reg[reg].u16)<<16);
    U32 result=cpu->dst.u32 >> cpu->src.u8;
    if (imm>16) {
        //klog("error: dshre16r16 imm=%x",imm);
        result |= ((U32)(cpu->reg[reg].u16) << (32 - imm));
    }
    cpu->result.u16=(U16)result;
    cpu->memory->writew(address, cpu->result.u16);
    cpu->lazyFlags=FLAGS_DSHR16;
}

void dshrr32r32(CPU* cpu, U32 reg, U32 rm, U32 imm) {
    cpu->src.u32=imm;
    cpu->dst.u32=cpu->reg[reg].u32;
    cpu->result.u32=(cpu->reg[reg].u32 >> imm) | (cpu->reg[rm].u32 << (32-imm));
    cpu->reg[reg].u32 = cpu->result.u32;	
    cpu->lazyFlags=FLAGS_DSHR32;
}

void dshre32r32(CPU* cpu, U32 reg, U32 address, U32 imm) {
    cpu->src.u32=imm;
    cpu->dst.u32= cpu->memory->readd(address);
    cpu->result.u32=(cpu->dst.u32 >> imm) | (cpu->reg[reg].u32 << (32-imm));
    cpu->memory->writed(address, cpu->result.u32);
    cpu->lazyFlags=FLAGS_DSHR32;
}

void dshrclr16r16(CPU* cpu, U32 reg, U32 rm) {
    if (CL & 0x1f) {
        cpu->src.u32 = CL & 0x1f;
        cpu->dst.u32 = (cpu->reg[reg].u16)|((U32)(cpu->reg[rm].u16)<<16);
        U32 result=cpu->dst.u32 >> cpu->src.u8;
        if (cpu->src.u32>16) {
            //klog("error: dshrclr16r16 cl=%x",CL);
            result |= ((U32)(cpu->reg[rm].u16) << (32 - cpu->src.u32));
        }
        cpu->result.u16=(U16)result;
        cpu->reg[reg].u16 = cpu->result.u16;
        cpu->lazyFlags=FLAGS_DSHR16;
    }
}

void dshrcle16r16(CPU* cpu, U32 reg, U32 address) {
    if (CL & 0x1f) {
        cpu->src.u32 = CL & 0x1f;
        cpu->dst.u32 = cpu->memory->readw(address)|((U32)(cpu->reg[reg].u16)<<16);
        U32 result=cpu->dst.u32 >> cpu->src.u8;
        if (cpu->src.u32>16) {
            //klog("error: dshrcle16r16 cl=%x",CL);
            result |= ((U32)(cpu->reg[reg].u16) << (32 - cpu->src.u32));
        }
        cpu->result.u16=(U16)result;
        cpu->memory->writew(address, cpu->result.u16);
        cpu->lazyFlags=FLAGS_DSHR16;
    }
}

void dshrclr32r32(CPU* cpu, U32 reg, U32 rm) {
    if (CL & 0x1f) {
        cpu->src.u32=CL & 0x1f;
        cpu->dst.u32=cpu->reg[reg].u32;
        cpu->result.u32=(cpu->dst.u32 >> cpu->src.u32);
        cpu->result.u32 |= (cpu->reg[rm].u32 << (32-cpu->src.u32));
        cpu->reg[reg].u32 = cpu->result.u32;	
        cpu->lazyFlags=FLAGS_DSHR32;
    }
}

void dshrcle32r32(CPU* cpu, U32 reg, U32 address) {
    if (CL & 0x1f) {
        cpu->src.u32=CL & 0x1f;
        cpu->dst.u32= cpu->memory->readd(address);
        cpu->result.u32=(cpu->dst.u32 >> cpu->src.u32);
        cpu->result.u32 |= (cpu->reg[reg].u32 << (32-cpu->src.u32));
        cpu->memory->writed(address, cpu->result.u32);
        cpu->lazyFlags=FLAGS_DSHR32;
    }
}

void daa(CPU* cpu) {
    cpu->lazyFlags=FLAGS_NONE;
    if (((AL & 0x0F)>0x09) || cpu->getAF()) {
        if ((AL > 0x99) || cpu->getCF()) {
            AL+=0x60;
            cpu->addCF();
        } else {
            cpu->removeCF();
        }
        AL+=0x06;
        cpu->addAF();
    } else {
        if ((AL > 0x99) || cpu->getCF()) {
            AL+=0x60;
            cpu->addCF();
        } else {
            cpu->removeCF();
        }
        cpu->removeAF();
    }
    cpu->setSF(AL & 0x80);
    cpu->setZF(AL == 0);
    cpu->setPFonValue(AL);    
}

void das(CPU* cpu) {
    cpu->lazyFlags=FLAGS_NONE;
    U8 osigned=AL & 0x80;
    if (((AL & 0x0f) > 9) || cpu->getAF()) {
        if ((AL>0x99) || cpu->getCF()) {
            AL-=0x60;
            cpu->addCF();
        } else {
            cpu->setCF(AL<=0x05);
        }
        AL-=6;
        cpu->addAF();
    } else {
        if ((AL>0x99) || cpu->getCF()) {
            AL-=0x60;
            cpu->addCF();
        } else {
            cpu->removeCF();
        }
        cpu->removeAF();
    }
    cpu->setOF(osigned && ((AL & 0x80)==0));
    cpu->setSF(AL & 0x80);
    cpu->setZF(AL==0);
    cpu->setPFonValue(AL);    
}

void aaa(CPU* cpu) {
    cpu->lazyFlags=FLAGS_NONE;
    cpu->setSF((AL>=0x7a) && (AL<=0xf9));
    if ((AL & 0xf) > 9) {
        cpu->setOF((AL & 0xf0)==0x70);
        AX += 0x106;
        cpu->addCF();
        cpu->setZF(AL == 0);
        cpu->addAF();
    } else if (cpu->getAF()) {
        AX += 0x106;
        cpu->removeOF();
        cpu->addCF();
        cpu->removeZF();
        cpu->addAF();
    } else {
        cpu->removeOF();
        cpu->removeCF();
        cpu->setZF(AL == 0);
        cpu->removeAF();
    }
    cpu->setPFonValue(AL);
    AL &= 0x0F;    
}

void aas(CPU* cpu) {
    cpu->lazyFlags=FLAGS_NONE;
    if ((AL & 0x0f)>9) {
        cpu->setSF(AL>0x85);
        AX -= 0x106;
        cpu->removeOF();
        cpu->addCF();
        cpu->addAF();
    } else if (cpu->getAF()) {
        cpu->setOF((AL>=0x80) && (AL<=0x85));
        cpu->setSF((AL<0x06) || (AL>0x85));
        AX -= 0x106;
        cpu->addCF();
        cpu->addAF();
    } else {
        cpu->setSF(AL>=0x80);
        cpu->removeOF();
        cpu->removeCF();
        cpu->removeAF();
    }
    cpu->setZF(AL == 0);
    cpu->setPFonValue(AL);
    AL &= 0x0F;    
}

void aad(CPU* cpu, U32 value) {
    AL = AH * value + AL;
    AH = 0;
    cpu->lazyFlags = FLAGS_NONE;
    cpu->setSF(AL & 0x80);
    cpu->setZF(AL == 0);		
    cpu->setPFonValue(AL);
    cpu->removeCF();
    cpu->removeOF();
    cpu->removeAF();    
}

U32 aam(CPU* cpu, U32 value) {
    if (value) {
        AH = AL / value;
        AL = AL % value;
        cpu->lazyFlags = FLAGS_NONE;
        cpu->setSF(AL & 0x80);
        cpu->setZF(AL == 0);		
        cpu->setPFonValue(AL);
        cpu->removeCF();
        cpu->removeOF();
        cpu->removeAF();        
        return 1;
    } else {
        cpu->prepareException(EXCEPTION_DIVIDE, 0);
        return 0;
    } 
}