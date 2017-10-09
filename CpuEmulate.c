#include "CpuEmulate.h"
#include "CpuInstruction.h"
#include "PpuEmulate.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/*
#define DEBUGLOG_CPU_WRITE
#define DEBUGLOG_CPU_READ
#define DEBUGLOG_CPU_OP_PRINT
#define DEBUGLOG_STACK_CHANGEADDR
*/
#define CHECK_STACK

uint8_t cpu_memory[CPU_MEM_SIZE];

clock_t cpu_runningtime = 0;

uint16_t cpu_r_pc = 0;//Program Counter程序计数器
uint8_t cpu_r_sp = 0;//Stack Pointer堆栈寄存器 堆栈位于0x0100-0x01FF 寄存器记录栈顶后两位，初值FF，送入元素-1，栈溢出未定义,直接溢出sp回到0xFF

uint8_t cpu_r_a = 0;//Accumulator算数寄存器
uint8_t cpu_r_x = 0;//Index Register X
uint8_t cpu_r_y = 0;
uint8_t cpu_r_p = 0;//状态寄存器
//三种中断向量的位置
#define CPU_INTERRUPT_VEC_NMI   0xFFFA
#define CPU_INTERRUPT_VEC_RESET 0xFFFC
#define CPU_INTERRUPT_VEC_IRQ   0xFFFE
int cpu_interrupt_mark = 0;
//执行指令剩余的时钟周期
unsigned int cycle_remain = 0;


uint16_t cpu_mirror(uint16_t address){
    if(address >= 0x2000 && address <0x4000){
        //0x2008 - 0x4000 mirror of 0x2000-0x2007(IO Reg)
        address = (address - 0x2000) % 8 + 0x2000;
    }else if(address >= 0x0800 && address < 0x2000){
        //0x0800-0x2000 mirror of 0x0000-0x0800
        address = address % 0x0800;
    }
    return address;
}

uint8_t cpu_read(uint16_t address){
    uint8_t ret;
    address = cpu_mirror(address);
    //判断是否为某些寄存器，获取结果

    //....
    ret = cpu_memory[address];
    switch (address) {
    case PPU_REG_PPUCTRL:
    case PPU_REG_PPUMASK:
    case PPU_REG_OAMADDR:
    case PPU_REG_PPUSCROLL:
    case PPU_REG_PPUADDR:
    case PPU_REG_OAMDMA:
        //fprintf(stderr,"tip:try to read write only ppu reg %04X\n",address);
        break;
    case PPU_REG_PPUSTATUS:
        cpu_memory[address]&= ~(1<<7);//清楚V位
        break;
    case PPU_REG_OAMDATA:
        ret = ppu_oam_read(cpu_memory[PPU_REG_OAMADDR]);
        break;
    case PPU_REG_PPUDATA:
        ret = ppu_regr_data();
        break;
    default:
        break;
    }
#ifdef DEBUGLOG_CPU_READ
    if(address < 0x8000)
        printf("cpu read at %04X is %04X\n",address,ret);
#endif
    return ret;
}

void cpu_write(uint16_t address,uint8_t value){
    address = cpu_mirror(address);
    //判断是否为某些特殊地址，执行函数
    switch (address) {
    case PPU_REG_PPUSTATUS:
        fprintf(stderr,"Error: try to write read only ppu reg $%04X",address);
        break;
    case PPU_REG_PPUCTRL:
        break;
    case PPU_REG_PPUMASK:
        break;
    case PPU_REG_OAMDATA:
        ppu_oam_write(cpu_memory[PPU_REG_OAMADDR],value);
    case PPU_REG_PPUSCROLL:
        ppu_regw_scroll(value);
        break;
    case PPU_REG_PPUADDR:
        ppu_regw_address(value);
        break;
    case PPU_REG_PPUDATA:
        ppu_regw_data(value);
        break;
    case PPU_REG_OAMDMA:
        ppu_dma(value);
        break;
    default:
        break;
    }
    cpu_memory[address] = value;
#ifdef DEBUGLOG_CPU_WRITE
    if(address < 0x8000)
        printf("cpu write:at %04X <= %02X\n",address,value);
#endif
}

void cpu_stack_push(uint8_t data){
#ifdef CHECK_STACK
    if(cpu_r_sp == 0x00){
        fprintf(stderr,"Stack push overflow\n");
        exit(-1);
    }
#endif
#ifdef DEBUGLOG_STACK_CHANGEADDR
    printf("stack push at %04X\n",0x0100 + cpu_r_sp);
#endif

    cpu_memory[0x0100 + (cpu_r_sp--)]=data;
}
uint8_t cpu_stack_pop(){
#ifdef CHECK_STACK
    if(cpu_r_sp == 0xFF){
        fprintf(stderr,"Stack pop overflow\n");
        exit(-1);
    }
#endif
    return cpu_memory[0x0100 + (++cpu_r_sp)];
}
void cpu_print_status(){

    printf("---<CPU>---\n");
    printf("A:$%X\n"
           "X:$%X\n"
           "Y:$%X\n"
           "PC:$%04X\n"
           "SP:$%X\n",cpu_r_a,cpu_r_x,cpu_r_y,cpu_r_pc,cpu_r_sp);
    printf("CZIDBVN\n%d%d%d%d%d%d%d\n",
           cpu_flag_get(CPU_FLAG_C),
           cpu_flag_get(CPU_FLAG_Z),
           cpu_flag_get(CPU_FLAG_I),
           cpu_flag_get(CPU_FLAG_D),
           cpu_flag_get(CPU_FLAG_B),
           cpu_flag_get(CPU_FLAG_V),
           cpu_flag_get(CPU_FLAG_N));
}

//一个时钟周期调用一次，时间错过了要补回来
void cpu_cycle(){
    uint8_t instructionBuff[10];//指令缓冲，要求大于所有指令的最大长度
    if(cycle_remain){
        cycle_remain--;
        return;
    }
    //检查中断标记

    if(cpu_interrupt_mark){
        //cycle_remain = 7;//处理中断需要7个时钟周期
        //这里处理中断
        /*
         * 当中断发生时
         * (完成当前周期)
         * 将PC和状态寄存器入栈
         * 设置禁止中断标志位
         * 将中断向量表(vector table)中的地址送入PC
         * 正常执行
         * ...
         * 执行RTI指令：
         * 将PC和状态寄存器从堆栈取出
         * 恢复执行
         * ...
         * */
        switch (cpu_interrupt_mark) {
        case CPU_INTERRUPT_RESET:
            cpu_r_pc = (uint16_t)cpu_memory[CPU_INTERRUPT_VEC_RESET] +
                    (((uint16_t)cpu_memory[CPU_INTERRUPT_VEC_RESET + 1])<<8);
            break;
        case CPU_INTERRUPT_NMI:
            cpu_stack_push((uint8_t)((cpu_r_pc>>8)&0xFF));
            cpu_stack_push((uint8_t)(cpu_r_pc&0xFF));//未证实：中断发生，先push PC高位，再push PC底位，然后是状态寄存器
            cpu_stack_push(cpu_r_p);
            cpu_r_pc = (uint16_t)cpu_memory[CPU_INTERRUPT_VEC_NMI] +
                    (((uint16_t)cpu_memory[CPU_INTERRUPT_VEC_NMI + 1])<<8);
            cpu_flag_set(CPU_FLAG_I);
            break;
        case CPU_INTERRUPT_IRQ:
            cpu_stack_push((uint8_t)((cpu_r_pc>>8)&0xFF));
            cpu_stack_push((uint8_t)(cpu_r_pc&0xFF));//未证实：中断发生，先push PC高位，再push PC底位，然后是状态寄存器
            cpu_stack_push(cpu_r_p);
            cpu_r_pc = (uint16_t)cpu_memory[CPU_INTERRUPT_VEC_IRQ] +
                    (((uint16_t)cpu_memory[CPU_INTERRUPT_VEC_IRQ + 1])<<8);
            cpu_flag_set(CPU_FLAG_I);
            break;
        default:
            fprintf(stderr,"Unknown CPU interrupt type:%d",cpu_interrupt_mark);
            break;
        }
        cpu_interrupt_mark = 0;
        //return;
    }

    //cpu_print_status();
    //执行指令
    uint16_t oldpc = cpu_r_pc;
    uint8_t opcode = cpu_read(oldpc);
    if(cpu_instruction_table[opcode].doOp == NULL){
        fprintf(stderr,"Unknown instruction:0x%X at 0x%X\n",opcode,oldpc);
        exit(-1);
        return;
    }
#ifdef DEBUGLOG_CPU_OP_PRINT
    if(opcode != 0x4C)
        printf("do instruction:0x%X 0x%X at 0x%X\n",opcode,cpu_read(oldpc + 1),oldpc);
#endif
    cpu_r_pc += cpu_instruction_table[opcode].bytes;
    for(int i=0;i<cpu_instruction_table[opcode].bytes;i++){
        instructionBuff[i] = cpu_read(oldpc + i) ;
    }
    cycle_remain = cpu_instruction_table[opcode].cycles + cpu_instruction_table[opcode].doOp(instructionBuff);
}

int cpu_update(int count){
    if(cycle_remain >= count){
        cycle_remain -= count;
    }else{
        count -= cycle_remain;
        cycle_remain = 0;
        for(int i=0;i<count;i++)
            cpu_cycle();
    }
    return cycle_remain + 1;
}

void cpu_flag_set(uint8_t flag){
    cpu_r_p |= flag;
}

void cpu_flag_clear(uint8_t flag){
    cpu_r_p &= ~flag;
}

int cpu_flag_get(uint8_t flag){
    if(cpu_r_p & flag)
        return 1;
    else
        return 0;
}

void cpu_flag_change(uint8_t flag,int istrue){
    if(istrue)
        cpu_flag_set(flag);
    else
        cpu_flag_clear(flag);
}

//中断标记函数
void cpu_interrupt(int interrupt_type){
    /*
    if(interrupt_type == CPU_INTERRUPT_IRQ){
        printf("Break!");
        exit(0);
    }*/
    //如果中断标志位禁止，且中断类型为IRQ，则不标记中断
    if(interrupt_type == CPU_INTERRUPT_RESET && cpu_flag_get(CPU_FLAG_I)){
        return;
    }
    //如果PPU控制寄存器（cpu $2000）7bit被清除，则INTERRUPT_NMI不会被触发
    if(cpu_interrupt_mark < interrupt_type)
        cpu_interrupt_mark = interrupt_type;
}
