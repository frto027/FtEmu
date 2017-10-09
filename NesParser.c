#include "NesParser.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "BlockDisplay.h"
#include "CpuEmulate.h"
#include "PpuEmulate.h"

#include "CpuInstruction.h"

#include "GlobalClocks.h"

struct RomHead rom_head;
uint8_t mem_trainer[512];
uint8_t (*mem_prg_rom)[0x4000];
uint8_t (*mem_chr_rom)[0x2000];

//是否已经载入
int nes_loaded = 0;

void initCPU();

void UnknownMap(){
    fprintf(stderr,"unknown mapper:%d",rom_head.mapper_number);
    ScreenTip("unsupport mapper",3);
}

void ErrorFormat(const char * filepath){
    fprintf(stderr,"File format error:%s",filepath);
    ScreenTip("file format error",3);
}
void ErrorOpenfile(const char * filepath){
    fprintf(stderr,"Can't open file :%s",filepath);
    ScreenTip("failed open file",3);
}
void ErrorEOF(const char * filepath){
    fprintf(stderr,"File format error(EOF):%s",filepath);
    ScreenTip("file format error(EOF)",3);
}

void PrintHeadInfo(){
    printf("Nes head read success:\n"
           "rom page num:%d\n"
           "character rom page num:%d\n"
           "mapper number:%d\n"
           "four screen mode:%c\n"
           "trainer:%d\n"
           "battery back sram:%c\n"
           "mirror:%d\n"
           "nes_2.0:%d\n"
           "playchoice10:%d\n"
           "vs unisystem:%d\n"
           "----\n"
           ,rom_head.rom_page_num,rom_head.crom_page_num,rom_head.mapper_number,rom_head.four_screen_mode?'T':'F',
           rom_head.trainer,rom_head.battery_back_sram?'T':'F',rom_head.mirroring,rom_head.nes_2_0_rule,rom_head.playchoice10,rom_head.unisystem_vs);
}

#define MAPPER_NROM 0


//解析函数
void ParseNesFile(const char * filepath){
    if(nes_loaded)
        UnloadNes();
    FILE * romfile = fopen(filepath,"rb");
    if(romfile == NULL){
        ErrorOpenfile(filepath);
        return;
    }
    //暂存头部
    uint8_t head[16];
    //读取文件头
    if(fread(head,sizeof(uint8_t),sizeof(head)/sizeof(uint8_t),romfile) != sizeof(head)/sizeof(uint8_t)){
        ErrorFormat(filepath);
    }
    //校验格式
    for(int i=0;i<4;i++){
        if(head[i] != "NES\x1A"[i]){
            ErrorFormat(filepath);
            fclose(romfile);
            return;
        }
    }

    ScreenTip("success",1);

    //解析头部
    rom_head.rom_page_num = head[4];
    rom_head.crom_page_num = head[5];
    rom_head.mapper_number =
            ((head[6]>>4)& 0x0F) | (head[7]&0xF0);
    rom_head.four_screen_mode = (head[6]>>3)&1;
    rom_head.trainer = (head[6]>>2)&1;
    rom_head.battery_back_sram = (head[6]>>1)&1;
    rom_head.mirroring = head[6]&1;
    rom_head.nes_2_0_rule = ((head[7]&0xF0)>>2) == 2;
    rom_head.playchoice10 = (head[7]>>1)&1;
    rom_head.unisystem_vs = head[7]&1;
    //8-15位的Nes为0，否则不在此模拟器模拟范围之内
    for(int i=8;i<16;i++){
        if(head[i] != 0){
            ScreenTip("head format not current",1);
        }
    }
    PrintHeadInfo();
    //装载trainer
    if(rom_head.trainer){
        fread(mem_trainer,sizeof(uint8_t),
              sizeof(mem_trainer)/sizeof(uint8_t),romfile);
    }
    //根据Mapper装载iNES文件
    //http://wiki.nesdev.com/w/index.php/Mapper
    switch (rom_head.mapper_number) {
    case MAPPER_NROM://mapper 0
        //read prg_rom
        mem_prg_rom = malloc(sizeof(*mem_prg_rom) * rom_head.rom_page_num);
        for(int i=0;i<rom_head.rom_page_num;i++){
            size_t read = fread(mem_prg_rom[i],sizeof(uint8_t),
                  sizeof(mem_prg_rom[i])/sizeof(uint8_t),romfile);
            if(read != sizeof(mem_prg_rom[i]))
                ErrorEOF(filepath);
        }
        //read chr_rom
        mem_chr_rom = malloc(sizeof(*mem_chr_rom) * rom_head.crom_page_num);
        for(int i=0;i<rom_head.crom_page_num;i++){
            size_t read = fread(mem_chr_rom[i],sizeof(uint8_t),
                  sizeof(mem_chr_rom[i])/sizeof(uint8_t),romfile);
           if(read != sizeof(mem_chr_rom[i]))
                ErrorEOF(filepath);
        }
        break;
    default:
        UnknownMap();
        return;
    }

    fclose(romfile);

    initCPU();

    nes_loaded = 1;


    return;
}
void UnloadNes(){
    if(!nes_loaded)
        return;
    switch (rom_head.mapper_number) {
    case MAPPER_NROM:
        free(mem_prg_rom);
        free(mem_chr_rom);
        memset(cpu_memory,0,CPU_MEM_SIZE);
        break;
    default:
        UnknownMap();
        break;
    }
    nes_loaded = 0;
}

uint8_t cpuTestZeroPage[] = {

};

uint8_t cpuTestContent[] = {
    0x20,0x09,0x06,0x20,0x0c,
    0x06,0x20,0x12,0x06,0xa2,
    0x00,0x60,0xe8,0xe0,0x05,
    0xd0,0xfb,0x60,0x00

};

void initCPU(){
    cpu_r_pc = 0;
    cpu_r_sp = 0xFF;
    cpu_r_x = 0;
    cpu_r_y = 0;
    cpu_r_a = 0;
    cpu_r_p = 0;
    cpu_flag_set(1<<5);
    cpu_instruct_init();
    cpu_runningtime = clock();
    switch (rom_head.mapper_number) {
    case MAPPER_NROM:
        //用mem_prg_rom装填cpu mem
        switch (rom_head.rom_page_num) {
        case 0:
            fprintf(stderr,"load error:no PRG ROM find");
            break;
        case 1:
            //0x8000 - 0xc000 0xc000-0x10000 内容一致
            memcpy(cpu_memory + CPU_ADDR_PRG_L_BEG,mem_prg_rom,CPU_ADDR_PRG_LEN);
            memcpy(cpu_memory + CPU_ADDR_PRG_H_BEG,mem_prg_rom,CPU_ADDR_PRG_LEN);
            break;
        case 2:
            memcpy(cpu_memory + CPU_ADDR_PRG_L_BEG,mem_prg_rom,CPU_ADDR_PRG_LEN);
            memcpy(cpu_memory + CPU_ADDR_PRG_H_BEG,mem_prg_rom + 1,CPU_ADDR_PRG_LEN);
            break;
        default:
            fprintf(stderr,"load error:PRG ROM num error");
            break;
        }
        //用CHR_ROM装填PPU的pattern table
        if(rom_head.crom_page_num){
            memcpy(ppu_memory,mem_chr_rom[0],0x2000 * sizeof(uint8_t));
        }
        //这里装载cpu测试内容
/*
        for(int i=0;i<sizeof(cpuTestZeroPage);i++)
            cpu_memory[i]= cpuTestZeroPage[i];

        for(int i=0;i<sizeof(cpuTestContent);i++)
            cpu_memory[0x0600 + i]= cpuTestContent[i];
        cpu_memory[0xFFFD]=0x06;
        cpu_memory[0xFFFC]=0x00;
/**/
        //这里要触发CPU的reset中断
        cpu_interrupt(CPU_INTERRUPT_RESET);

        break;
    default:
        UnknownMap();
        break;
    }
}
