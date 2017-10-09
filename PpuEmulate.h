#ifndef PPUEMULATE_H
#define PPUEMULATE_H

#include "GlobalDefine.h"

#define PPU_MEM_SIZE (0x10000)
//在模拟时不要直接读，用"ppuemulate.h"中的ppu_read和ppu_write
//mirror区域不可读写
extern uint8_t ppu_memory[PPU_MEM_SIZE];
extern uint8_t ppu_spr_ram[0x100];

uint8_t ppu_read(uint16_t address);
void ppu_write(uint16_t address,uint8_t value);

uint8_t ppu_oam_read(uint8_t addr);
void ppu_oam_write(uint8_t addr,uint8_t value);

void ppu_regw_scroll(uint8_t value);
void ppu_regw_address(uint8_t addr);
void ppu_regw_data(uint8_t value);
uint8_t ppu_regr_data();

//DMA直接寻址
//当一个byte被写入CPU的$4014时，DMA被触发，若此时cpu$4014所代表的值为0xAB,则$AB00-$ABFF即page $AB被写入spr_ram
void ppu_dma(uint8_t pageindex);

#define PPU_ADDR_PATTERN_TABLE_0 0x0000
#define PPU_ADDR_PATTERN_TABLE_1 0x1000
#define PPU_ADDR_NAME_TABLE_0 0x2000
#define PPU_ADDR_NAME_TABLE_1 0x2400
#define PPU_ADDR_NAME_TABLE_2 0x2800
#define PPU_ADDR_NAME_TABLE_3 0x2C00
#define PPU_ADDR_ATTR_TABLE_0 0x23C0
#define PPU_ADDR_ATTR_TABLE_1 0x27C0
#define PPU_ADDR_ATTR_TABLE_2 0x2BC0
#define PPU_ADDR_ATTR_TABLE_3 0x2FC0

#define PPU_PALETTE_IMAGE 0x3F00
#define PPU_PALETTE_SPRITE 0x3F10

#define PPU_REG_PPUCTRL     0x2000
#define PPU_REG_PPUMASK     0x2001
#define PPU_REG_PPUSTATUS   0x2002
#define PPU_REG_OAMADDR     0x2003
#define PPU_REG_OAMDATA     0x2004
#define PPU_REG_PPUSCROLL   0x2005
#define PPU_REG_PPUADDR     0x2006
#define PPU_REG_PPUDATA     0x2007
#define PPU_REG_OAMDMA      0x4014
//返回palette某索引的颜色索引
uint8_t ppu_palette_color(uint16_t palette,uint8_t index);

void ppu_cycle();
//计算ppu当前显示的颜色，然后填充到colorarr[]中
void ppu_putcolor(uint8_t colorarr[ORI_HEIGHT*ORI_WIDTH]);

#define PPU_FREQUENC (CPU_FREQUENC/4)

#endif // PPUEMULATE_H
