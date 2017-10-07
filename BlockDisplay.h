#ifndef BLOCKDISPLAY_H
#define BLOCKDISPLAY_H

#include "GlobalDefine.h"

#include <time.h>

//这个函数用于绘制图形
void BlockDisplay();
//这个函数传入需要绘制的PPU内存数组，显示前传入一次就可以，地址改变后需要重新传入
void setPPUArray(uint8_t (*_ppu)[0x10000]);

//在屏幕上显示某句话一段时间
void ScreenTip(const char * tip,time_t time);

#endif // BLOCKDISPLAY_H
