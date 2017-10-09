#ifndef CPUEMULATE_H
#define CPUEMULATE_H

#include "GlobalDefine.h"
#include <time.h>
//模拟时不要直接读，用"cpuemulate.h"中的cpu_read和cpu_write
#define CPU_MEM_SIZE (0x10000)
extern uint8_t cpu_memory[CPU_MEM_SIZE];
//CPU运行时间
extern clock_t cpu_runningtime;

uint8_t cpu_read(uint16_t address);
void cpu_write(uint16_t address,uint8_t value);

#define CPU_ADDR_PRG_L_BEG 0x8000
#define CPU_ADDR_PRG_H_BEG 0xC000
#define CPU_ADDR_PRG_LEN 0x4000

//寄存器定义

extern uint16_t cpu_r_pc;//Program Counter程序计数器
extern uint8_t cpu_r_sp;//Stack Pointer堆栈寄存器 堆栈位于0x0100-0x01FF 寄存器记录栈顶后两位，初值FF，送入元素-1，栈溢出未定义

extern uint8_t cpu_r_a;//Accumulator算数寄存器
extern uint8_t cpu_r_x;//Index Register X
extern uint8_t cpu_r_y;
extern uint8_t cpu_r_p;//状态寄存器
/*
 * 有关cpu_r_p
 * 7 6 5 4 3 2 1 0
 * N V   B D I Z C
 * N Negative Flag
 * V Overflow Flag
 * B Break Command(标识BRK指令被执行)
 * D Decimal Mode 2A03不支持 (SED置位 CLD复位)
 * I Interrupt Disable禁止IRQs中断，SEI置位CLI复位
 * Z Zero Flag 上次计算结果为0
 * Carry Flag 进位标识，SEC置位CLC复位
 */
#define CPU_FLAG_N ((uint8_t)1<<7)
#define CPU_FLAG_V ((uint8_t)1<<6)
#define CPU_FLAG_B ((uint8_t)1<<4)
#define CPU_FLAG_D ((uint8_t)1<<3)
#define CPU_FLAG_I ((uint8_t)1<<2)
#define CPU_FLAG_Z ((uint8_t)1<<1)
#define CPU_FLAG_C ((uint8_t)1)

//中断类型定义(数字越大优先级越高)
#define CPU_INTERRUPT_RESET 3
#define CPU_INTERRUPT_NMI   2
#define CPU_INTERRUPT_IRQ   1
//CPU中断函数，当中断信号到达时调用来标记中断，中断将在下一个周期处理
void cpu_interrupt(int interrupt_type);
void cpu_flag_set(uint8_t flag);//置为1
void cpu_flag_clear(uint8_t flag);//置为0
int cpu_flag_get(uint8_t flag);//返回0或1
void cpu_flag_change(uint8_t flag,int istrue);//直接设置标志位为布尔量

void cpu_stack_push(uint8_t data);
uint8_t cpu_stack_pop();
//更新cpu若干个周期，返回值是建议下次传入的数值
int cpu_update(int count);

#endif // CPUEMULATE_H
