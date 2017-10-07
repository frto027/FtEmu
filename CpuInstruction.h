#ifndef CPUINSTRUCTION_H
#define CPUINSTRUCTION_H

#include "GlobalDefine.h"

//cpu指令结构
typedef struct CpuInstruction{
    uint8_t opcode;
    int bytes;
    int cycles;//指一般情况下的周期，特殊情况特殊对待
    int (*doOp)(uint8_t * args);//将指令传入这个函数然后执行，返回值是时钟周期修正量，也就是这条指令周期 = cycles + reutrn vlue
}CpuInstruction;

//CPU指令表
extern CpuInstruction cpu_instruction_table[0x100];
//初始化指令系统
void cpu_instruct_init();

#endif // CPUINSTRUCTION_H
