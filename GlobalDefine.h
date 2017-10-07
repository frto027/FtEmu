#ifndef _GLOBAL_DEF_H
#define _GLOBAL_DEF_H

#include <stdint.h>

//屏幕显示方式
#define	VIEW_RESHAPE_ORIGION 0			//原始左上
#define VIEW_RESHAPE_SCALE 1			//拉伸缩放
#define VIEW_RESHAPE_CENTER 2			//原始居中
#define VIEW_RESHAPE_SCALE_CENTER 3		//自动适应

//原始分辨率
#define ORI_WIDTH 256
#define ORI_HEIGHT 240
//调色板有多少种颜色 BlockDisplay.c中的颜色定义要跟上
#define COLOR_COUNT 0x40

#define FPS_LIMIT 60
//cpu时钟频率 单位Hz
#define CPU_FREQUENC 21477272

extern int nes_loaded;

struct RomHead{
    int rom_page_num;//1 byte
    int crom_page_num;//1 byte
    //flags6 - 7 - 8
    int mapper_number;//lower 4 bits of the mapper number
    //Flags6
    int four_screen_mode;//boolean
    //useless
    int trainer;//boolean 0 or 1 no trainer/512 byte trainer at 0x7000-0x71FF
    int battery_back_sram;//boolean
    int mirroring;//0 horizontal 1 vertical
    //Flags7
    int nes_2_0_rule;
    int playchoice10;//boolean
    int unisystem_vs;//boolean
    //byte 8 - 15 zero
    //(pop)
};
extern struct RomHead rom_head;

extern uint8_t mem_trainer[512];
extern uint8_t (*mem_prg_rom)[0x4000];
extern uint8_t (*mem_chr_rom)[0x2000];

#define UNUSED(x) ((void)(x))
#endif // !_GLOBAL_DEF_H
