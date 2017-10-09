#include "BlockDisplay.h"
#include "GlobalDefine.h"
#include "bmpfont.h"
#include "GL/gl.h"
#include "GLFW/glfw3.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "PpuEmulate.h"

GLubyte colorpoint[256*240*3];//显存大小为256*256

uint8_t colorBuffer[256*240];//以编号形式显示的颜色点阵

GLushort
red[COLOR_COUNT]=
{0x75,0x27,0x00,0x47,0x8F,0xAB,0xA7,0x7F,0x43,0x00,0x00,0x00,0x1B,0x00,0x00,0x00,
 0xBC,0x00,0x23,0x83,0xBF,0xE7,0xDB,0xCB,0x8B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0xFF,0x3F,0x5F,0xA7,0xF7,0xFF,0xFF,0xFF,0xF3,0x83,0x4F,0x58,0x00,0x00,0x00,0x00,
 0xFF,0xAB,0xC7,0xD7,0xFF,0xFF,0xFF,0xFF,0xFF,0xE3,0xAB,0xB3,0x9F,0x00,0x00,0x00,},
green[COLOR_COUNT]=
{0x75,0x1B,0x00,0x00,0x00,0x00,0x00,0x0B,0x2F,0x47,0x51,0x3F,0x3F,0x00,0x00,0x00,
 0xBC,0x73,0x3B,0x00,0x00,0x00,0x2B,0x4F,0x73,0x97,0xAB,0x93,0x83,0x00,0x00,0x00,
 0xFF,0xBF,0x97,0x8B,0x7B,0x77,0x77,0x9B,0xBF,0xD3,0xDF,0xF8,0xEB,0x00,0x00,0x00,
 0xFF,0xE7,0xD7,0xCB,0xC7,0xC7,0xBF,0xDB,0xE7,0xFF,0xF3,0xFF,0xFF,0x00,0x00,0x00,},
blue[COLOR_COUNT]=
{0x75,0x8F,0xAB,0x9F,0x77,0x13,0x00,0x00,0x00,0x00,0x00,0x17,0x5F,0x00,0x00,0x00,
 0xBC,0xEF,0xEF,0xF3,0xBF,0x5B,0x00,0x0F,0x00,0x00,0x00,0x3B,0x8B,0x00,0x00,0x00,
 0xFF,0xFF,0xFF,0xFD,0xFF,0xB7,0x63,0x3B,0x3F,0x13,0x4B,0x98,0xDB,0x00,0x00,0x00,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xDB,0xB3,0xAB,0xA3,0xA3,0xBF,0xCF,0xF3,0x00,0x00,0x00};

#define SET(x,y,i) do{\
    int p = ((ORI_HEIGHT - 1  - (y)) * ORI_WIDTH + (x))*3;\
    if(p<0)break;\
    colorpoint[p] = red[i % COLOR_COUNT];\
    colorpoint[++p] = green[i % COLOR_COUNT];\
    colorpoint[++p] = blue[i % COLOR_COUNT];\
    }while(0)
/* 设置xy点的颜色索引
void setxy(int x,int y,int i){
    if(x >= 256 || y >= 256){
        fprintf(stderr,"Graph Memory overflow.");
        exit(-1);
    }
    colorpoint[(y * ORI_WIDTH + x)*3] = red[i % COLOR_COUNT];
    colorpoint[(y * ORI_WIDTH + x)*3+1] = green[i % COLOR_COUNT];
    colorpoint[(y * ORI_WIDTH + x)*3+2] = blue[i % COLOR_COUNT];
}*/
//测试显色效果的函数
void randData(){
    int step = 15;
    memset(colorpoint,0,sizeof(colorpoint));

    for(int i=0;i<4;i++){
        for(int j=0;j<0x10;j++){
            for(int ki = 0;ki < step;ki++){
                for(int kj = 0;kj < step;kj++){
                    SET(j * step + kj,i * step + ki,i * 0x10+ j);
                }
            }
        }
    }
    for(int i=100;i<150;i++){
        for(int j=100;j<150;j++){
            SET(i,j,rand());
        }
    }
    for(int i =0;i<ORI_WIDTH;i++){
        SET(i,0,0);
        SET(i,ORI_HEIGHT-1,1);
    }
    for(int i =0;i<ORI_HEIGHT;i++){
        SET(0,i,2);
        SET(ORI_WIDTH-1,i,3);
    }
}

//DebugOnly 绘制patternTable到屏幕
void showPatternTable(int x,int y){
    uint8_t table1[8],table2[8];
    int xorg = x;
    for(int i=0,count = 0;i<0x2000;i+=16,count++){
        if(count == 24){
            x = xorg;
            y += 9;
            count = 0;
        }
        for(int j=0;j<8;j++){
            table1[j] = ppu_memory[i + j];
            table2[j] = ppu_memory[i+j+8];
        }
        for(int xo=0;xo<8;xo++){
            for(int yo=0;yo<8;yo++){
                int color = ((table1[yo] >> (7 - xo))&1)+((((table2[yo] >> (7 - xo))&1))<<1);
                SET(x+xo,y+yo,color*5);
            }
        }
        x+=9;
    }
}

//显示Palette
void showPalette(int x,int y){
    int xorg = x;
    int step = 8;
    for(int i=0;i<16;i++){
        int color = ppu_palette_color(PPU_PALETTE_IMAGE,i);//palette只有低6位有效
        for(int xo=0;xo<step;xo++)
            for(int yo=0;yo<step;yo++){
                SET(x+xo,y+yo,color);
            }
        x+=step + 1;
    }
    x = xorg;
    y += step + 1;
    for(int i=0;i<16;i++){
        int color = ppu_palette_color(PPU_PALETTE_SPRITE,i);//palette只有低6位有效
        for(int xo=0;xo<step;xo++)
            for(int yo=0;yo<step;yo++){
                SET(x+xo,y+yo,color);
            }
        x+=step + 1;
    }
}
//FPS统计+显示
void ShowFpsInGame(){
    static char tip[30] = "Fps ";
    static char * flg = tip + 4;
    static int count = 0;
    static int lasttime = 0;
    int nowtime = (int)glfwGetTime();
    if(nowtime != lasttime){
        sprintf(flg,"%d",count);
        lasttime = nowtime;
        count = 0;
    }
    count++;
    FillBitmap(colorpoint,10,8,tip);
}

//屏幕上要显示的tip
char tip[1024];
time_t exTime = 0;
void ShowTip(){
    if(exTime){
        FillBitmap(colorpoint,10,ORI_HEIGHT - 40,tip);
        if(time(NULL) > exTime){
            exTime = 0;
        }
    }
}

void ScreenTip(const char * _tip,time_t _time){
    strcpy(tip,_tip);
    exTime = time(NULL) + _time;
}

void clearColor(){
    memset(colorpoint,0,sizeof(colorpoint));
    for(size_t i=0;i<sizeof(colorBuffer)/sizeof(*colorBuffer);i++){
        colorBuffer[i]=255;//getPaletteColor(PPU_PALETTE_SPRITE,0);
    }
    /*
    for(int i=0;i<ORI_WIDTH;i++)
        for(int j=0;j<ORI_HEIGHT;j++){
            SET(i,j,ppu_palette_color(PPU_PALETTE_IMAGE,0));
        }
    */
}
//将colorBuffer中的颜色索引记录转换为RGB到colorpoint数组
void DrawColorPoint(){
    for(int i=0;i<240;i++){
        for(int j=0;j<256;j++){
            SET(j,i,colorBuffer[i*256+j]);
        }
    }
}
//将对应pattern位置索引处图像的颜色矩阵（低两位）与heighcolor做|运算后放入arr数组
void PatternToArray(uint16_t pattern_addr,uint16_t paletteTable,uint8_t index,uint8_t arr[],int begx,int begy,int linewidth,uint8_t heighcolor){
    uint16_t begindex = pattern_addr + ((uint16_t)index)*16;//每16byte是一个图像
    uint8_t table[16];
    for(int i=0;i<16;i++){
        table[i]=ppu_read(begindex + i);
    }
    for(int y = begy;y<begy+8;y++){
        for(int x = begx;x<begx+8;x++){
            uint8_t color = ((table[y - begy]>>(7-(x-begx))) + ((table[y - begy + 8]>>(7-(x-begx)))<<1))&0x03;
            //if(color != 0)
                arr[y * linewidth + x]= ppu_palette_color(paletteTable,color | heighcolor);
        }
    }
}

//非线程安全(方法只读)
void ShowNameTable1(){

    for(int y=0;y<30;y++){
        for(int x=0;x<32;x++){
            uint16_t attr_offset = (y/4)*8 + x/4;
            int attr_rbit = ((((
                                   (y/2))&0x01)<<1)+((((x/2))&0x01)))*2;
            //int attr_index =((((y / 16)<<1)+(x/16))<<4)+(attr_indexarr[(x / 8) + (y / 8)*4]);
            PatternToArray(PPU_ADDR_PATTERN_TABLE_1,PPU_PALETTE_IMAGE,ppu_memory[PPU_ADDR_NAME_TABLE_0 + y * 32 + x],colorBuffer,x*8,y*8,ORI_WIDTH ,
                    ((ppu_memory[PPU_ADDR_ATTR_TABLE_0 + attr_offset]>>attr_rbit)&0x03) << 2);
        }

    }
}

void BlockDisplay(){
    clearColor();
    //randData();

    ShowNameTable1();// !

    ppu_putcolor(colorBuffer);
    DrawColorPoint();

    //showPatternTable(0,0);//在屏幕上叠加显示PatternTable
    showPalette(0,200);//多线程没加锁，仅供调试，只读

    ShowTip();//显示一段话
    ShowFpsInGame();//画面显示Fps
    glDrawPixels(ORI_WIDTH,ORI_HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,colorpoint);
}
