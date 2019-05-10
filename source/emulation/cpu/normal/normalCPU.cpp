#include "boxedwine.h"

#include "../decoder.h"
#include "normalCPU.h"
#include "../../softmmu/soft_code_page.h"
#include "../x32/x32CPU.h"

#ifdef _DEBUG
#define START_OP(cpu, op) op->log(cpu)
#else
#define START_OP(cpu, op)
#endif
#define NEXT() cpu->eip.u32+=op->len; op->next->pfn(cpu, op->next)
#define NEXT_DONE() cpu->nextBlock = cpu->getNextBlock();
#define NEXT_BRANCH1() cpu->eip.u32+=op->len; if (!DecodedBlock::currentBlock->next1) {DecodedBlock::currentBlock->next1 = cpu->getNextBlock(); DecodedBlock::currentBlock->next1->addReferenceFrom(DecodedBlock::currentBlock);} cpu->nextBlock = DecodedBlock::currentBlock->next1
#define NEXT_BRANCH2() cpu->eip.u32+=op->len; if (!DecodedBlock::currentBlock->next2) {DecodedBlock::currentBlock->next2 = cpu->getNextBlock(); DecodedBlock::currentBlock->next2->addReferenceFrom(DecodedBlock::currentBlock);} cpu->nextBlock = DecodedBlock::currentBlock->next2

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

static OpCallback normalOps[NUMBER_OF_OPS];
static U32 normalOpsInitialized;

void OPCALL normal_sidt(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);    
    U32 eaa = eaa(cpu, op);
    writew(eaa, 1023); // limit
    writed(eaa, 0); // base
#ifdef _DEBUG
    klog("sidt not implemented");
#endif
    NEXT();
}

static void initNormalOps() {
    if (normalOpsInitialized)
        return;
    normalOpsInitialized = 1;   
    for (int i=0;i<InstructionCount;i++) {
        normalOps[i] = normal_invalid;
    }
#define INIT_CPU(e, f) normalOps[e] = normal_##f;
#include "../common/cpu_init.h"
#include "../common/cpu_init_mmx.h"
#include "../common/cpu_init_fpu.h"
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
    normalOps[VERWR16] = 0; 
    normalOps[SGDT] = 0;
    normalOps[SIDT] = normal_sidt;
    normalOps[LGDT] = 0;
    normalOps[LIDT] = 0;
    normalOps[SMSWRreg] = 0; 
    normalOps[SMSW] = 0;
    normalOps[LMSWRreg] = 0; 
    normalOps[LMSW] = 0;
    normalOps[INVLPG] = 0;
    normalOps[Callback] = 0;
}

OpCallback NormalCPU::getFunctionForOp(DecodedOp* op) {
    initNormalOps();
    return normalOps[op->inst];
}

NormalCPU::NormalCPU() {   
    initNormalOps();
#ifdef BOXEDWINE_DYNAMIC32
    this->firstOp = firstX32Op;
#else
    this->firstOp = NULL;
#endif
}

U8 fetchByte(U32 *eip) {
    return readb((*eip)++);
}

class NormalBlock : public DecodedBlock {
public:
    static NormalBlock* alloc(NormalCPU* cpu);
    virtual void dealloc(bool delayed);  

    void run(CPU* cpu);

private:
    void init();
    NormalBlock* next;
};

void NormalBlock::run(CPU* cpu) {
#ifdef _DEBUG
    if (this==NULL || this->op==NULL || this->op->pfn==NULL) {
        kpanic("NormalBlock::run is about to crash");
    }
#endif  
    this->op->pfn(cpu, this->op);
    this->runCount++;
    cpu->blockInstructionCount+=this->opCount;
}

static NormalBlock* freeBlocks;

void NormalBlock::init() {
    this->next = 0;
    this->op = NULL;
    this->bytes = 0;
    this->opCount = 0;
    this->runCount = 0;
    this->next1 = NULL;
    this->next2 = NULL;
    this->referencedFrom = NULL;
}

NormalBlock* NormalBlock::alloc(NormalCPU* cpu) {
    NormalBlock* result;

    if (freeBlocks) {
        result = freeBlocks;
        freeBlocks = freeBlocks->next;
    } else {
        NormalBlock* blocks = new NormalBlock[1024];

        freeBlocks = &blocks[1];
        freeBlocks->next = 0;
        for (int i=2;i<1024;i++) {
            blocks[i].next = freeBlocks;
            freeBlocks = &blocks[i];            
        }
        result = &blocks[0];
    }
    result->init();
    return result;
}

void NormalBlock::dealloc(bool delayed) {
    CPU* cpu = KThread::currentThread()->cpu;
    if (cpu->delayedFreeBlock && cpu->delayedFreeBlock!=DecodedBlock::currentBlock) {
        DecodedBlock* b = cpu->delayedFreeBlock;        
        cpu->delayedFreeBlock = NULL;
        b->dealloc(false);
    }
    if ((delayed && !cpu->delayedFreeBlock) || this==DecodedBlock::currentBlock) {
        cpu->delayedFreeBlock = this;
    } else {
        this->op->dealloc(true);
        this->next = freeBlocks;
        this->op = NULL;
        freeBlocks = this;
    }
    if (this->next1) {
        this->next1->removeReferenceFrom(this);
        this->next1 = NULL;
    }
    if (this->next2) {
        this->next2->removeReferenceFrom(this);
        this->next2 = NULL;
    }
    DecodedBlockFromNode* from = this->referencedFrom;
    while (from) {
        DecodedBlockFromNode* n = from->next;
        if (from->block->next1 == this) {
            from->block->next1 = NULL;
        }
        if (from->block->next2 == this) {
            from->block->next2 = NULL;
        }
        from->dealloc();
        from = n;
    }
    this->referencedFrom = NULL;
}

DecodedBlock* NormalCPU::getNextBlock() {
    if (!this->thread->process) // exit was called, don't need to pre-cache the next block
        return NULL;

    U32 startIp = (this->big?this->eip.u32:this->eip.u16) + this->seg[CS].address;
    DecodedBlock* block = this->thread->memory->getCodeBlock(startIp);

    if (!block) {
        block = NormalBlock::alloc(this);
        decodeBlock(fetchByte, startIp, this->big, 0, K_PAGE_SIZE, 0, block);

        DecodedOp* op = block->op;
        while (op) {
            if (!op->pfn) // callback will be set by decoder
                op->pfn = normalOps[op->inst];
            op = op->next;
        }
        this->thread->memory->addCodeBlock(startIp, block);
        if (this->firstOp) {
            op = DecodedOp::alloc();
            op->inst = Custom1;
            op->pfn = this->firstOp;
            op->next = block->op;
            block->op = op;
        }
    }
    return block;
}

void NormalCPU::run() {    
    DecodedBlock::currentBlock = this->nextBlock;
    DecodedBlock::currentBlock->run(this);    
#ifdef _DEBUG
    if (!this->nextBlock && !this->yield) {
        kpanic("NormalCPU::run no block set");
    }
    DecodedBlock::currentBlock = NULL;
#endif
}
