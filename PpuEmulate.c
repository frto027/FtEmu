#include "PpuEmulate.h"

#include "CpuEmulate.h"
#include <stdio.h>
#include <stdlib.h>

uint8_t ppu_memory[PPU_MEM_SIZE];//实际上0x4000之后的地方不会被访问
uint8_t ppu_spr_ram[0x100];

uint16_t ppu_value_scroll = 0;
uint16_t ppu_value_address = 0;
uint16_t ppu_addr_mirror(uint16_t address){
    //mirror
    if(address >= 0x4000)
        address %= 0x4000;
    if(address >=0x3000 && address <0x3F00)
        return address - 0x1000;
    if(address >=0x3F20 && address <0x4000)
        address = (address & 0xFF00) + ((address & 0x00FF)%0x20);
    if(address >= 0x3F10 && address < 0x3F20 && (address - 0x3F00)%4==0)
        address -= 0x10;

    return address;
}
uint16_t ppu_addr_mirror_read(uint16_t address){
    address = ppu_addr_mirror(address);
    return address;
}

uint8_t ppu_read(uint16_t address){
    //mirror
    address = ppu_addr_mirror_read(address);
    return ppu_memory[address];
}
void ppu_write(uint16_t address,uint8_t value){
    //mirror
    address = ppu_addr_mirror(address);
    ppu_memory[address] = value;
}

uint8_t ppu_oam_read(uint8_t addr){
    return ppu_spr_ram[addr];
}
void ppu_oam_write(uint8_t addr,uint8_t value){
    ppu_spr_ram[addr]=value;
}

uint8_t ppu_palette_color(uint16_t palette,uint8_t index){
    if(index % 4 == 0)
        index = 0;
    return ppu_read(palette + index)&0x3F;
}

void ppu_regw_scroll(uint8_t value){
    static int haveold = 0;
    static uint8_t old = 0;
    if(haveold){
        ppu_value_scroll = (((uint16_t)old << 8)&0xFF00)+value;
        haveold = 0;
    }else{
        old = value;
        haveold = 1;
    }
}
void ppu_regw_address(uint8_t addr){
    static int haveold = 0;
    static uint8_t old = 0;
    if(haveold){
        ppu_value_address = (((uint16_t)old << 8)&0xFF00)+addr;
        haveold = 0;
    }else{
        old = addr;
        haveold = 1;
    }
}
void ppu_regrw_add(){
    if(cpu_memory[PPU_REG_PPUCTRL]&(1<<2)){
        ppu_value_address+=32;
    }else{
        ppu_value_address+=1;
    }
}
void ppu_regw_data(uint8_t value){
    ppu_write(ppu_value_address,value);
    ppu_regrw_add();
}
uint8_t ppu_regr_data(){
    return ppu_read(ppu_value_address);
    ppu_regrw_add();
}

//DMA直接寻址
//当一个byte被写入CPU的$4014时，DMA被触发，若此时cpu$4014所代表的值为0xab,则$AB00-$ABFF即page $AB被写入spr_ram
void ppu_dma(uint8_t pageindex){
    for(uint16_t i = 0,j = ((pageindex)<<8);i<0x100;i++,j++){
        //ppu_spr_ram[i] = cpu_mem[j];
        ppu_spr_ram[i]=cpu_read(j);
    }
}
int ppu_nextcycle = 0;
void ppu_cycle(){
    if(ppu_nextcycle){
        ppu_nextcycle--;
        return;
    }
    ppu_nextcycle = 500;
    cpu_memory[PPU_REG_PPUSTATUS]|= 1<<7;
    /* debug only */
    static int set = 1;
    if(set){
            cpu_memory[PPU_REG_PPUSTATUS]|= 1 << 6;
    }else{
            cpu_memory[PPU_REG_PPUSTATUS]&= ~(1 << 6);
    }
    set = !set;
    if((cpu_memory[PPU_REG_PPUCTRL]>>7)&1){
        cpu_interrupt(CPU_INTERRUPT_NMI);
    }
}
