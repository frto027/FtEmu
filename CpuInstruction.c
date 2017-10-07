#include "CpuInstruction.h"
#include "CpuEmulate.h"

#include <memory.h>
#include <stdio.h>
/*
#define INSLOG_JSR
#define INSLOG_RTS
*/
CpuInstruction cpu_instruction_table[0x100];

#define CPU_INS(opcodes,bytess,cycle,function) do{\
    cpu_instruction_table[opcodes].opcode=opcodes;\
    cpu_instruction_table[opcodes].bytes=bytess;\
    cpu_instruction_table[opcodes].cycles=cycle;\
    cpu_instruction_table[opcodes].doOp=function;\
    }while(0)

int ((*(NormTable[0x100])))(void);

void Change_empty(uint8_t unused){UNUSED(unused);}
void Change_Acc(uint8_t op){
    cpu_r_a = op;
}

uint16_t changeOpAddr = 0;
void Change_Addr(uint8_t op){
    cpu_write(changeOpAddr,op);
}
uint8_t mem_input;

//当指令需要修改操作数时调用这个函数来操作
void (*ChangeFunc)(uint8_t);
//如果需要读取内存就加入这个宏
#define NEED_MEM uint8_t mem;{if(ChangeFunc == Change_Addr)mem=cpu_read(changeOpAddr);else mem=mem_input;}

//定义寻址方式的函数，返回值是函数周期偏移
#define ADDR_MODE(funcname) int funcname(uint8_t * op)

ADDR_MODE(Implicit){//0
    ChangeFunc = Change_empty;
    return NormTable[*op]();
}
ADDR_MODE(Accumulator){//1
    ChangeFunc = Change_Acc;
    mem_input = cpu_r_a;
    return NormTable[*op]();
}
ADDR_MODE(Immediate){//2
    ChangeFunc = Change_empty;
    mem_input = op[1];
    return NormTable[*op]();
}
ADDR_MODE(ZeroPage){//3
    changeOpAddr = op[1]&0x00FF;
    ChangeFunc = Change_Addr;
    return NormTable[*op]();
}
ADDR_MODE(ZeroPageX){//4
    changeOpAddr = (op[1] + cpu_r_x) & 0x00FF;
    ChangeFunc = Change_Addr;
    return NormTable[*op]();
}
ADDR_MODE(ZeroPageY){//5
    changeOpAddr = (op[1] + cpu_r_y) & 0x00FF;
    ChangeFunc = Change_Addr;
    return NormTable[*op]();
}
ADDR_MODE(Relative){//6
    ChangeFunc = Change_empty;
    mem_input = op[1];
    return NormTable[*op]();//只会在分支语句用到，直接PC加操作数就可以
}
ADDR_MODE(Absolute){//7
    changeOpAddr = ((((uint16_t)op[2])<<8)&0xFF00) + ((((uint16_t)op[1]))&0x00FF);
    ChangeFunc = Change_Addr;
    return NormTable[*op]();
}
ADDR_MODE(AbsoluteX){//8
    changeOpAddr = ((((uint16_t)op[2])<<8)&0xFF00) + ((((uint16_t)op[1]))&0x00FF) + (uint16_t)cpu_r_x;
    ChangeFunc = Change_Addr;
    return NormTable[*op]();
}
ADDR_MODE(AbsoluteY){//9
    changeOpAddr = ((((uint16_t)op[2])<<8)&0xFF00) + ((((uint16_t)op[1]))&0x00FF) + (uint16_t)cpu_r_y;
    ChangeFunc = Change_Addr;
    return NormTable[*op]();
}
ADDR_MODE(Indirect){//10
    changeOpAddr = (0x00FF&(uint16_t)cpu_read((((uint16_t)op[1])&0xFF) + (((uint16_t)op[2] & 0xFF)<<8))) +
            (0xFF00&((uint16_t)cpu_read((((uint16_t)op[1])&0xFF) + (((uint16_t)op[2] & 0xFF)<<8) + 1) << 8));
    ChangeFunc = Change_empty;
    return NormTable[*op]();
}
ADDR_MODE(IndexIndirect){//11 X
    changeOpAddr = cpu_read(((uint16_t)(op[1]+cpu_r_x))&0xFF) + (((uint16_t)cpu_read((uint16_t)(op[1] + cpu_r_x) & 0xFF) + 1)<<8);
    ChangeFunc = Change_Addr;
    return NormTable[*op]();
}
ADDR_MODE(IndirectIndex){//12 Y
    changeOpAddr = cpu_r_y + (0xFF & (uint16_t)cpu_read(0x00FF & (uint16_t)op[1])) + (0xFF00 & ((uint16_t)cpu_read((0x00FF & (uint16_t)op[1]) + 1) << 8));
    ChangeFunc = Change_Addr;
    return NormTable[*op]();
}
#define ADDR_IMPLICT        0
#define ADDR_ACCUMULATOR    1
#define ADDR_IMMEDIATE      2
#define ADDR_ZEROPAGE       3
#define ADDR_ZEROPAGEX      4
#define ADDR_ZEROPAGEY      5
#define ADDR_RELATIVE       6
#define ADDR_ABSOLUTE       7
#define ADDR_ABSOLUTEX      8
#define ADDR_ABSOLUTEY      9
#define ADDR_INDIRECT       10
#define ADDR_INDEX_INDRECT_X  11
#define ADDR_INDIRECT_INDEX_Y 12

int (*(addr_func[]))(uint8_t *) = {
        Implicit,Accumulator,Immediate,ZeroPage,
        ZeroPageX,ZeroPageY,Relative,Absolute,
        AbsoluteX,AbsoluteY,Indirect,IndexIndirect,IndirectIndex
        };
uint8_t addr_byte[] = {
    1,1,2,2,
    2,2,2,3,
    3,3,3,2,2
};
/*
 * 指令开发说明：
 * Implied指令：
 * 1.一个函数，输入void，返回int为周期标准周期偏移
 * 2.cpu_ins_init_assistent列表中加入函数说明，操作码，函数，类型必须ADDR_IMPLICT，周期为标准周期
 * 其他指令：
 * 1.一个函数，输入uint8_t为操作值，返回int为周期标准周期偏移
 * 2.cpu_ins_init_assistent列表中加入函数说明，操作码，函数，类型，周期为标准周期
 * 3.如果要修改操作数，所代表的位置，直接调用ChangeFunc指针指向的函数，传入新的数值即可。
 * 4.如果有必要(如JMP)，检查changeOpAddr，这是操作数的地址
 */
#define INS_FUNC(funcname) int funcname()
#define INS_FUNC_IMP INS_FUNC

INS_FUNC(i_adc){
    NEED_MEM
    uint16_t a = cpu_r_a;
    a+=mem;
    a+=cpu_flag_get(CPU_FLAG_C)?1:0;

    cpu_flag_change(CPU_FLAG_C,(a>>8)&1);
    cpu_flag_change(CPU_FLAG_Z,a==0);
    cpu_flag_change(CPU_FLAG_V,((a>>7)&1)!=((cpu_r_a>>7)&1));
    cpu_flag_change(CPU_FLAG_N,(a>>7)&1);

    cpu_r_a = a & 0xFF;
    return 0;
}
INS_FUNC(i_and){
    NEED_MEM
    cpu_r_a &= mem;
    cpu_flag_change(CPU_FLAG_Z,cpu_r_a == 0);
    cpu_flag_change(CPU_FLAG_N,(cpu_r_a>>7)&1);
    return 0;
}
INS_FUNC(i_asl){
    NEED_MEM
    cpu_flag_change(CPU_FLAG_C,(mem>>7)&1);
    mem<<=1;
    cpu_flag_change(CPU_FLAG_Z,mem==0);
    cpu_flag_change(CPU_FLAG_N,(mem>>7)&1);
    ChangeFunc(mem);
    return 0;
}
INS_FUNC(i_bcc){
    NEED_MEM
    if(cpu_flag_get(CPU_FLAG_C))
        return 0;
    else{
        uint8_t oldpage = cpu_r_pc>>8;
        cpu_r_pc += mem;
        if((mem>>7)&1)
            cpu_r_pc -=0x100;
        return (cpu_r_pc>>8)==oldpage ? 1 : 2;
    }
}
INS_FUNC(i_bcs){
    NEED_MEM
    if(cpu_flag_get(CPU_FLAG_C)){
        uint8_t oldpage = cpu_r_pc>>8;
        cpu_r_pc += mem;
        if((mem>>7)&1)
            cpu_r_pc -=0x100;
        return (cpu_r_pc>>8)==oldpage ? 1 : 2;
    }else
        return 0;
}
INS_FUNC(i_beq){
    NEED_MEM
    if(cpu_flag_get(CPU_FLAG_Z)){
        uint8_t oldpage = cpu_r_pc>>8;
        cpu_r_pc += mem;
        if((mem>>7)&1)
            cpu_r_pc -=0x100;
        return (cpu_r_pc>>8)==oldpage ? 1 : 2;
    }else
        return 0;
}
INS_FUNC(i_bit){
    NEED_MEM
    uint8_t tmp = cpu_r_a & mem;
    cpu_flag_change(CPU_FLAG_Z,tmp==0);
    cpu_flag_change(CPU_FLAG_V,(tmp>>6)&1);
    cpu_flag_change(CPU_FLAG_N,(tmp>>7)&1);
    return 0;
}
INS_FUNC(i_bmi){
    NEED_MEM
    if(cpu_flag_get(CPU_FLAG_N)){
        uint8_t oldpage = cpu_r_pc>>8;
        cpu_r_pc += mem;
        if((mem>>7)&1)
            cpu_r_pc -=0x100;
        return (cpu_r_pc>>8)==oldpage ? 1 : 2;
    }else
        return 0;
}
INS_FUNC(i_bne){
    NEED_MEM
    if(cpu_flag_get(CPU_FLAG_Z))
        return 0;
    else{
        uint8_t oldpage = cpu_r_pc>>8;
        cpu_r_pc += mem;
        if((mem>>7)&1)
            cpu_r_pc -=0x100;
        return (cpu_r_pc>>8)==oldpage ? 1 : 2;
    }
}
INS_FUNC(i_bpl){
    NEED_MEM
    if(cpu_flag_get(CPU_FLAG_N))
        return 0;
    else{
        uint8_t oldpage = cpu_r_pc>>8;
        cpu_r_pc += mem;
        if((mem>>7)&1)
            cpu_r_pc -=0x100;
        return (cpu_r_pc>>8)==oldpage ? 1 : 2;
    }
}
INS_FUNC_IMP(i_brk){
    cpu_flag_set(CPU_FLAG_B);
    cpu_interrupt(CPU_INTERRUPT_IRQ);
    return 0;
}
INS_FUNC(i_bvc){
    NEED_MEM
    if(cpu_flag_get(CPU_FLAG_V))
        return 0;
    else{
        uint8_t oldpage = cpu_r_pc>>8;
        cpu_r_pc += mem;
        if((mem>>7)&1)
            cpu_r_pc -=0x100;
        return (cpu_r_pc>>8)==oldpage ? 1 : 2;
    }
}
INS_FUNC(i_bvs){
    NEED_MEM
    if(cpu_flag_get(CPU_FLAG_V)){
        uint8_t oldpage = cpu_r_pc>>8;
        cpu_r_pc += mem;
        if((mem>>7)&1)
            cpu_r_pc -=0x100;
        return (cpu_r_pc>>8)==oldpage ? 1 : 2;
    }else
        return 0;
}
INS_FUNC_IMP(i_clc){
    cpu_flag_clear(CPU_FLAG_C);
    return 0;
}
INS_FUNC_IMP(i_cld){
    cpu_flag_clear(CPU_FLAG_D);
    return 0;
}
INS_FUNC_IMP(i_cli){
    cpu_flag_clear(CPU_FLAG_I);
    return 0;
}
INS_FUNC_IMP(i_clv){
    cpu_flag_clear(CPU_FLAG_V);
    return 0;
}
INS_FUNC(i_cmp){
    NEED_MEM
    cpu_flag_change(CPU_FLAG_C,cpu_r_a >= mem);
    cpu_flag_change(CPU_FLAG_Z,cpu_r_a == mem);
    cpu_flag_change(CPU_FLAG_N,((cpu_r_a - mem)>>7)&1);
    return 0;
}
INS_FUNC(i_cpx){
    NEED_MEM
    cpu_flag_change(CPU_FLAG_C,cpu_r_x >= mem);
    cpu_flag_change(CPU_FLAG_Z,cpu_r_x == mem);
    cpu_flag_change(CPU_FLAG_N,((cpu_r_x - mem)>>7)&1);
    return 0;
}
INS_FUNC(i_cpy){
    NEED_MEM
    cpu_flag_change(CPU_FLAG_C,cpu_r_y >= mem);
    cpu_flag_change(CPU_FLAG_Z,cpu_r_y == mem);
    cpu_flag_change(CPU_FLAG_N,((cpu_r_y - mem)>>7)&1);
    return 0;
}
INS_FUNC(i_dec){
    NEED_MEM
    uint8_t result = mem - 1;
    cpu_flag_change(CPU_FLAG_Z,result == 0);
    cpu_flag_change(CPU_FLAG_N,(result>>7)&1);
    ChangeFunc(result);
    return 0;
}
INS_FUNC_IMP(i_dex){
    cpu_r_x = cpu_r_x - 1;
    cpu_flag_change(CPU_FLAG_Z,cpu_r_x == 0);
    cpu_flag_change(CPU_FLAG_N,(cpu_r_x>>7)&1);
    return 0;
}
INS_FUNC_IMP(i_dey){
    cpu_r_y = cpu_r_y - 1;
    cpu_flag_change(CPU_FLAG_Z,cpu_r_y == 0);
    cpu_flag_change(CPU_FLAG_N,(cpu_r_y>>7)&1);
    return 0;
}
INS_FUNC(i_eor){
    NEED_MEM
    cpu_r_a = cpu_r_a ^ mem;
    cpu_flag_change(CPU_FLAG_Z,cpu_r_a == 0);
    cpu_flag_change(CPU_FLAG_N,(cpu_r_a>>7)&1);
    return 0;
}
INS_FUNC(i_inc){
    NEED_MEM
    uint8_t result = mem + 1;
    cpu_flag_change(CPU_FLAG_Z,result == 0);
    cpu_flag_change(CPU_FLAG_N,(result>>7)&1);
    ChangeFunc(result);
    return 0;
}
INS_FUNC_IMP(i_inx){
    cpu_r_x++;
    cpu_flag_change(CPU_FLAG_Z,cpu_r_x == 0);
    cpu_flag_change(CPU_FLAG_N,(cpu_r_x>>7)&1);
    return 0;
}
INS_FUNC_IMP(i_iny){
    cpu_r_y++;
    cpu_flag_change(CPU_FLAG_Z,cpu_r_y == 0);
    cpu_flag_change(CPU_FLAG_N,(cpu_r_y>>7)&1);
    return 0;
}
INS_FUNC(i_jmp){
    cpu_r_pc = changeOpAddr;//用到了mem地址，比较特殊
    return 0;
}
INS_FUNC(i_jsr){
    cpu_r_pc--;
#ifdef INSLOG_JSR
    printf("jsr %02X->\n",cpu_r_sp);
#endif
    cpu_stack_push((uint8_t)(cpu_r_pc>>8)&0xFF);
    cpu_stack_push((uint8_t)(cpu_r_pc&0xFF));
    cpu_r_pc = changeOpAddr;
    return 0;
}
INS_FUNC(i_lda){
    NEED_MEM
    cpu_r_a = mem;
    cpu_flag_change(CPU_FLAG_Z,mem == 0);
    cpu_flag_change(CPU_FLAG_N,(mem>>7)&1);
    return 0;
}
INS_FUNC(i_ldx){
    NEED_MEM
    cpu_r_x = mem;
    cpu_flag_change(CPU_FLAG_Z,mem == 0);
    cpu_flag_change(CPU_FLAG_N, (mem>>7)&1);
    return 0;
}
INS_FUNC(i_ldy){
    NEED_MEM
    cpu_r_y = mem;
    cpu_flag_change(CPU_FLAG_Z,mem == 0);
    cpu_flag_change(CPU_FLAG_N,(mem>>7)&1);
    return 0;
}
INS_FUNC(i_lsr){
    NEED_MEM
    cpu_flag_change(CPU_FLAG_C,mem&1);
    uint8_t result = mem/2;
    ChangeFunc(result);
    cpu_flag_change(CPU_FLAG_N,(result>>7)&1);
    return 0;
}
INS_FUNC_IMP(i_nop){
    return 0;
}
INS_FUNC(i_ora){
    NEED_MEM
    cpu_r_a |= mem;
    cpu_flag_change(CPU_FLAG_Z,cpu_r_a==0);
    cpu_flag_change(CPU_FLAG_N,(cpu_r_a>>7)&1);
    return 0;
}
INS_FUNC_IMP(i_pha){
    cpu_stack_push(cpu_r_a);
    return 0;
}
INS_FUNC_IMP(i_php){
    cpu_stack_push(cpu_r_p);
    return 0;
}
INS_FUNC_IMP(i_pla){
    cpu_r_a = cpu_stack_pop();
    cpu_flag_change(CPU_FLAG_Z,cpu_r_a == 0);
    return 0;
}
INS_FUNC_IMP(i_plp){
    cpu_r_p = cpu_stack_pop();
    return 0;
}
INS_FUNC(i_rol){
    NEED_MEM
    uint8_t result = mem << 1;
    if(cpu_flag_get(CPU_FLAG_C))
        result |= 0x01;
    else
        result &= ~(uint8_t)0x01;
    cpu_flag_change(CPU_FLAG_C,(mem>>7)&1);
    ChangeFunc(result);
    return 0;
}
INS_FUNC(i_ror){
    NEED_MEM
    uint8_t result = mem>>1;
    if(cpu_flag_get(CPU_FLAG_C))
        result |= ((uint8_t)1<<7);
    else
        result &= ~((uint8_t)1<<7);
    cpu_flag_change(CPU_FLAG_C,mem&1);
    ChangeFunc(result);
    return 0;
}
INS_FUNC_IMP(i_rti){
    cpu_r_p = cpu_stack_pop();
    //先pop低位，再pop高位
    uint8_t pcL = cpu_stack_pop();
    uint8_t pcH = cpu_stack_pop();
    cpu_r_pc = ((((uint16_t)pcH)<<8)&0xFF00) + pcL;
    return 0;
}
INS_FUNC_IMP(i_rts){
    uint8_t pcL = cpu_stack_pop();
    uint8_t pcH = cpu_stack_pop();
#ifdef INSLOG_RTS
    printf("rts %02X<-\n",cpu_r_sp);
#endif
    cpu_r_pc = ((((uint16_t)pcH)<<8)&0xFF00) + pcL + 1;
    return 0;
}
INS_FUNC(i_sbc){
    NEED_MEM
    mem = mem + cpu_flag_get(CPU_FLAG_C);
    uint8_t result = cpu_r_a - mem;
    uint8_t beforeA = (cpu_r_a>>7)&1,
            beforeB = (mem>>7)&1,
            afterA = (result>>7)&1;
    cpu_flag_change(CPU_FLAG_C,cpu_r_a < mem);
    cpu_flag_change(CPU_FLAG_Z,cpu_r_a == mem);
    cpu_flag_change(CPU_FLAG_V,(beforeA != beforeB && afterA != beforeA));
    cpu_flag_change(CPU_FLAG_N,afterA);
    cpu_r_a = result;
    return 0;
}
INS_FUNC_IMP(i_sec){
    cpu_flag_set(CPU_FLAG_C);
    return 0;
}
INS_FUNC_IMP(i_sed){
    cpu_flag_set(CPU_FLAG_D);
    return 0;
}
INS_FUNC_IMP(i_sei){
    cpu_flag_set(CPU_FLAG_I);
    return 0;
}
INS_FUNC(i_sta){
    ChangeFunc(cpu_r_a);
    return 0;
}
INS_FUNC(i_stx){
    ChangeFunc(cpu_r_x);
    return 0;
}
INS_FUNC(i_sty){
    ChangeFunc(cpu_r_y);
    return 0;
}
INS_FUNC_IMP(i_tax){
    cpu_r_x = cpu_r_a;
    cpu_flag_change(CPU_FLAG_Z,cpu_r_x == 0);
    return 0;
}
INS_FUNC_IMP(i_tay){
    cpu_r_y = cpu_r_a;
    cpu_flag_change(CPU_FLAG_Z,cpu_r_y == 0);
    return 0;
}
INS_FUNC_IMP(i_tsx){
    cpu_r_x = cpu_r_sp;
    cpu_flag_change(CPU_FLAG_Z,cpu_r_x == 0);
    return 0;
}
INS_FUNC_IMP(i_txa){
    cpu_r_a = cpu_r_x;
    cpu_flag_change(CPU_FLAG_Z,cpu_r_a == 0);
    return 0;
}
INS_FUNC_IMP(i_txs){
    cpu_r_sp = cpu_r_x;
    return 0;
}
INS_FUNC_IMP(i_tya){
    cpu_r_a = cpu_r_y;
    cpu_flag_change(CPU_FLAG_Z,cpu_r_a == 0);
    cpu_flag_change(CPU_FLAG_N,(cpu_r_a>>7)&1);
    return 0;
}
struct{
    uint8_t opcode;
    int (*func)();
    int addr_mode;
    int cycles;
}cpu_ins_init_assistent[]={
    // 注意这个列表如果是ADDR_IMPLICT类型寻址，则必须传入uint8_t (*)(void) 否则传入 uint8_t (*)(uint8 op)
{0x69,i_adc,ADDR_IMMEDIATE,2},
{0x65,i_adc,ADDR_ZEROPAGE,3},
{0x75,i_adc,ADDR_ZEROPAGEX,4},
{0x6D,i_adc,ADDR_ABSOLUTE,4},
{0x7D,i_adc,ADDR_ABSOLUTEX,4},
{0x79,i_adc,ADDR_ABSOLUTEY,4},
{0x61,i_adc,ADDR_INDEX_INDRECT_X,6},
{0x71,i_adc,ADDR_INDIRECT_INDEX_Y,5},

{0x29,i_and,ADDR_IMMEDIATE,2},
{0x25,i_and,ADDR_ZEROPAGE,3},
{0x35,i_and,ADDR_ZEROPAGEX,4},
{0x2D,i_and,ADDR_ABSOLUTE,4},
{0x3D,i_and,ADDR_ABSOLUTEX,4},
{0x39,i_and,ADDR_ABSOLUTEY,4},
{0x21,i_and,ADDR_INDEX_INDRECT_X,6},
{0x31,i_and,ADDR_INDIRECT_INDEX_Y,5},

{0x0A,i_asl,ADDR_ACCUMULATOR,2},
{0x06,i_asl,ADDR_ZEROPAGE,5},
{0x16,i_asl,ADDR_ZEROPAGEX,6},
{0x0E,i_asl,ADDR_ABSOLUTE,6},
{0x1E,i_asl,ADDR_ABSOLUTEX,7},

{0x90,i_bcc,ADDR_RELATIVE,2},
{0xB0,i_bcs,ADDR_RELATIVE,2},
{0xF0,i_beq,ADDR_RELATIVE,2},

{0x24,i_bit,ADDR_ZEROPAGE,3},
{0x2C,i_bit,ADDR_ABSOLUTE,4},

{0x30,i_bmi,ADDR_RELATIVE,2},
{0xD0,i_bne,ADDR_RELATIVE,2},
{0x10,i_bpl,ADDR_RELATIVE,2},

{0x00,i_brk,ADDR_IMPLICT,7},
{0x50,i_bvc,ADDR_RELATIVE,2},
{0x70,i_bvs,ADDR_RELATIVE,2},

{0x18,i_clc,ADDR_IMPLICT,2},
{0xD8,i_cld,ADDR_IMPLICT,2},
{0x58,i_cli,ADDR_IMPLICT,2},
{0xB8,i_clv,ADDR_IMPLICT,2},

{0xC9,i_cmp,ADDR_IMMEDIATE,2},
{0xC5,i_cmp,ADDR_ZEROPAGE,3},
{0xD5,i_cmp,ADDR_ZEROPAGEX,4},
{0xCD,i_cmp,ADDR_ABSOLUTE,4},
{0xDD,i_cmp,ADDR_ABSOLUTEX,4},
{0xD9,i_cmp,ADDR_ABSOLUTEY,4},
{0xC1,i_cmp,ADDR_INDEX_INDRECT_X,6},
{0xD1,i_cmp,ADDR_INDIRECT_INDEX_Y,5},

{0xE0,i_cpx,ADDR_IMMEDIATE,2},
{0xE4,i_cpx,ADDR_ZEROPAGE,3},
{0xEC,i_cpx,ADDR_ABSOLUTE,4},
{0xC0,i_cpy,ADDR_IMMEDIATE,2},
{0xC4,i_cpy,ADDR_ZEROPAGE,3},
{0xCC,i_cpy,ADDR_ABSOLUTE,4},

{0xC6,i_dec,ADDR_ZEROPAGE,5},
{0xD6,i_dec,ADDR_ZEROPAGEX,6},
{0xCE,i_dec,ADDR_ABSOLUTE,6},
{0xDE,i_dec,ADDR_ABSOLUTEX,7},
{0xCA,i_dex,ADDR_IMPLICT,2},
{0x88,i_dey,ADDR_IMPLICT,2},

{0x49,i_eor,ADDR_IMMEDIATE,2},
{0x45,i_eor,ADDR_ZEROPAGE,3},
{0x55,i_eor,ADDR_ZEROPAGEX,4},
{0x4D,i_eor,ADDR_ABSOLUTE,4},
{0x5D,i_eor,ADDR_ABSOLUTEX,4},
{0x59,i_eor,ADDR_ABSOLUTEY,4},
{0x41,i_eor,ADDR_INDEX_INDRECT_X,6},
{0x51,i_eor,ADDR_INDIRECT_INDEX_Y,5},

{0xE6,i_inc,ADDR_ZEROPAGE,5},
{0xF6,i_inc,ADDR_ZEROPAGEX,6},
{0xEE,i_inc,ADDR_ABSOLUTE,6},
{0xFE,i_inc,ADDR_ABSOLUTEX,7},

{0xE8,i_inx,ADDR_IMPLICT,2},
{0xC8,i_iny,ADDR_IMPLICT,2},

{0x4C,i_jmp,ADDR_ABSOLUTE,3},
{0x6C,i_jmp,ADDR_INDIRECT,5},
{0x20,i_jsr,ADDR_ABSOLUTE,6},

{0xA9,i_lda,ADDR_IMMEDIATE,2},
{0xA5,i_lda,ADDR_ZEROPAGE,3},
{0xB5,i_lda,ADDR_ZEROPAGEX,4},
{0xAD,i_lda,ADDR_ABSOLUTE,4},
{0xBD,i_lda,ADDR_ABSOLUTEX,4},
{0xB9,i_lda,ADDR_ABSOLUTEY,4},
{0xA1,i_lda,ADDR_INDEX_INDRECT_X,6},
{0xB1,i_lda,ADDR_INDIRECT_INDEX_Y,5},

{0xA2,i_ldx,ADDR_IMMEDIATE,2},
{0xA6,i_ldx,ADDR_ZEROPAGE,3},
{0xB6,i_ldx,ADDR_ZEROPAGEY,4},
{0xAE,i_ldx,ADDR_ABSOLUTE,4},
{0xBE,i_ldx,ADDR_ABSOLUTEY,4},

{0xA0,i_ldy,ADDR_IMMEDIATE,2},
{0xA4,i_ldy,ADDR_ZEROPAGE,3},
{0xB4,i_ldy,ADDR_ZEROPAGEX,4},
{0xAC,i_ldy,ADDR_ABSOLUTE,4},
{0xBC,i_ldy,ADDR_ABSOLUTEX,4},

{0x4A,i_lsr,ADDR_ACCUMULATOR,2},
{0x46,i_lsr,ADDR_ZEROPAGE,5},
{0x56,i_lsr,ADDR_ZEROPAGEX,6},
{0x4E,i_lsr,ADDR_ABSOLUTE,6},
{0x5E,i_lsr,ADDR_ABSOLUTEX,7},

{0xEA,i_nop,ADDR_IMPLICT,2},

{0x09,i_ora,ADDR_IMMEDIATE,2},
{0x05,i_ora,ADDR_ZEROPAGE,3},
{0x15,i_ora,ADDR_ZEROPAGEX,4},
{0x0D,i_ora,ADDR_ABSOLUTE,3},
{0x1D,i_ora,ADDR_ABSOLUTEX,4},
{0x19,i_ora,ADDR_ABSOLUTEY,4},
{0x01,i_ora,ADDR_INDEX_INDRECT_X,2},
{0x11,i_ora,ADDR_INDIRECT_INDEX_Y,2},

{0x48,i_pha,ADDR_IMPLICT,3},
{0x08,i_php,ADDR_IMPLICT,3},
{0x68,i_pla,ADDR_IMPLICT,4},
{0x28,i_plp,ADDR_IMPLICT,4},

{0x2A,i_rol,ADDR_ACCUMULATOR,2},
{0x26,i_rol,ADDR_ZEROPAGE,5},
{0x36,i_rol,ADDR_ZEROPAGEX,6},
{0x2E,i_rol,ADDR_ABSOLUTE,6},
{0x3E,i_rol,ADDR_ABSOLUTEX,7},

{0x6A,i_ror,ADDR_ACCUMULATOR,2},
{0x66,i_ror,ADDR_ZEROPAGE,5},
{0x76,i_ror,ADDR_ZEROPAGEX,6},
{0x6E,i_ror,ADDR_ABSOLUTE,6},
{0x7E,i_ror,ADDR_ABSOLUTEX,7},

{0x40,i_rti,ADDR_IMPLICT,6},
{0x60,i_rts,ADDR_IMPLICT,6},

{0xE9,i_sbc,ADDR_IMMEDIATE,2},
{0xE5,i_sbc,ADDR_ZEROPAGE,3},
{0xF5,i_sbc,ADDR_ZEROPAGEX,4},
{0xED,i_sbc,ADDR_ABSOLUTE,4},
{0xFD,i_sbc,ADDR_ABSOLUTEX,4},
{0xF9,i_sbc,ADDR_ABSOLUTEY,4},
{0xE1,i_sbc,ADDR_INDEX_INDRECT_X,6},
{0xF1,i_sbc,ADDR_INDIRECT_INDEX_Y,5},

{0x38,i_sec,ADDR_IMPLICT,2},
{0xF8,i_sed,ADDR_IMPLICT,2},
{0x78,i_sei,ADDR_IMPLICT,2},

{0x85,i_sta,ADDR_ZEROPAGE,3},
{0x95,i_sta,ADDR_ZEROPAGEX,4},
{0x8D,i_sta,ADDR_ABSOLUTE,4},
{0x9D,i_sta,ADDR_ABSOLUTEX,5},
{0x99,i_sta,ADDR_ABSOLUTEY,5},
{0x81,i_sta,ADDR_INDEX_INDRECT_X,6},
{0x91,i_sta,ADDR_INDIRECT_INDEX_Y,6},

{0x86,i_stx,ADDR_ZEROPAGE,3},
{0x96,i_stx,ADDR_ZEROPAGEY,4},
{0x8E,i_stx,ADDR_ABSOLUTE,4},

{0x84,i_sty,ADDR_ZEROPAGE,3},
{0x94,i_sty,ADDR_ZEROPAGEX,4},
{0x8C,i_sty,ADDR_ABSOLUTE,4},

{0xAA,i_tax,ADDR_IMPLICT,2},
{0xA8,i_tay,ADDR_IMPLICT,2},
{0xBA,i_tsx,ADDR_IMPLICT,2},
{0x8A,i_txa,ADDR_IMPLICT,2},
{0x9A,i_txs,ADDR_IMPLICT,2},
{0x98,i_tya,ADDR_IMPLICT,2},
};


void cpu_instruct_init(){
    for(size_t i=0;i<sizeof(cpu_instruction_table)/sizeof(*cpu_instruction_table);i++)
        cpu_instruction_table[i].doOp = NULL;//当 doOp == NULL 认为这个指令不存在
    for(size_t i=0;i<sizeof(cpu_ins_init_assistent)/sizeof(*cpu_ins_init_assistent);i++){
        cpu_instruction_table[cpu_ins_init_assistent[i].opcode].opcode=cpu_ins_init_assistent[i].opcode;
        cpu_instruction_table[cpu_ins_init_assistent[i].opcode].bytes=addr_byte[cpu_ins_init_assistent[i].addr_mode];
        cpu_instruction_table[cpu_ins_init_assistent[i].opcode].cycles=cpu_ins_init_assistent[i].cycles;
        cpu_instruction_table[cpu_ins_init_assistent[i].opcode].doOp=addr_func[cpu_ins_init_assistent[i].addr_mode];

        NormTable[cpu_ins_init_assistent[i].opcode]=cpu_ins_init_assistent[i].func;

    }
}
