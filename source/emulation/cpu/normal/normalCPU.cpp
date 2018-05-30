#include "boxedwine.h"

#include "../decoder.h"
#include "normalCPU.h"
#include "../../source/emulation/softmmu/soft_code_page.h"

#define eaa1(cpu, op) cpu->seg[op->base].address + (U16)(cpu->reg[op->rm].u16 + (S16)cpu->reg[op->sibIndex].u16 + op->disp)
#define eaa3(cpu, op) cpu->seg[op->base].address + cpu->reg[op->rm].u32 + (cpu->reg[op->sibIndex].u32 << + op->sibScale) + op->disp
#define eaa(cpu, op) (op->ea16)?(eaa1(cpu, op)):(eaa3(cpu, op))

#include "instructions.h"
#include "normal_arith.h"
#include "normal_conditions.h"
#include "normal_incdec.h"
#include "normal_pushpop.h"
#include "normal_setcc.h"
#include "normal_strings.h"
#include "normal_strings_op.h"
#include "normal_shift.h"
#include "normal_shift_op.h"
#include "normal_xchg.h"
#include "normal_bit.h"
#include "normal_mmx.h"
#include "normal_fpu.h"
#include "normal_other.h"
#include "normal_jump.h"
#include "normal_move.h"

typedef void (OPCALL *OpCallback)(CPU* cpu, DecodedOp* op);

static OpCallback normalOps[NUMBER_OF_OPS];
static U32 normalOpsInitialized;

static void initNormalOps() {
    if (normalOpsInitialized)
        return;
    normalOpsInitialized = 1;   
    for (int i=0;i<InstructionCount;i++) {
        normalOps[i] = normal_invalid;
    }
#define INIT_CPU(e, f) normalOps[e] = f;
#include "normal_cpu_init.h"
#include "normal_cpu_init_mmx.h"
#include "normal_cpu_init_fpu.h"
#undef INIT_CPU    
    
    normalOps[SLDTReg] = 0; 
    normalOps[SLDTE16] = 0;
    normalOps[STRReg] = 0; 
    normalOps[STRE16] = 0;
    normalOps[LLDTR16] = 0; 
    normalOps[LLDTE16] = 0;
    normalOps[LTRR16] = 0; 
    normalOps[LTRE16] = 0;
    normalOps[VERRR16] = 0; 
    normalOps[VERRE16] = 0;
    normalOps[VERWR16] = 0; 
    normalOps[VERWE16] = 0;
    normalOps[SGDT] = 0;
    normalOps[SIDT] = 0;
    normalOps[LGDT] = 0;
    normalOps[LIDT] = 0;
    normalOps[SMSWRreg] = 0; 
    normalOps[SMSW] = 0;
    normalOps[LMSWRreg] = 0; 
    normalOps[LMSW] = 0;
    normalOps[INVLPG] = 0;
}

NormalCPU::NormalCPU() {
    initNormalOps();
}

U8 NormalCPU::Block::fetchByte() {
    return readb(this->eip++);
}

void NormalCPU::run() {
    Block data;
    DecodedOp* op = NULL;
    U32 startIp = this->eip.u32 + this->seg[CS].address;
    static U32 lastStartIP;

    Page* page = this->thread->memory->mmu[startIp >> PAGE_SHIFT];
    if (page->type == Page::Type::Code_Page) {
        CodePage* codePage = (CodePage*)page;
        op = codePage->getCode(startIp);
    }
    if (!op) {
        data.eip = startIp;
        data.thread = this->thread;
        decodeBlock(&data, this->big, 0, 1);
        op = data.ops;

        // might have changed after a read
        page = this->thread->memory->mmu[startIp >> PAGE_SHIFT];

        CodePage* codePage; 
        if (page->type == Page::Type::Code_Page) {
            codePage = (CodePage*)page;
        } else {
            if (page->type == Page::Type::RO_Page || page->type == Page::Type::RW_Page || page->type == Page::Type::Copy_On_Write_Page) {
                RWPage* p = (RWPage*)page;
                codePage = CodePage::alloc(p->page, p->address, p->flags);
                this->thread->memory->mmu[startIp >> PAGE_SHIFT] = codePage;
                p->close();
            } else {
                kpanic("Unhandled code caching page type: %d", page->type);
            }
        }
        codePage->addCode(startIp, op, data.bytes);
    }
    while (op->next) {
        op->log(this);
        normalOps[op->inst](this, op);
        this->eip.u32+=op->len;
        op = op->next;
    }
    op->log(this);
    normalOps[op->inst](this, op); // the last op in the block is responsible for updating eip
    lastStartIP = startIp;
    this->blockInstructionCount+=data.opCount;
}