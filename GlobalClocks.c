#include "GlobalDefine.h"
#include "GlobalClocks.h"

#include "GLFW/glfw3.h"

#include "CpuEmulate.h"
#include "PpuEmulate.h"

#include <stdio.h>
//cpu周期更新最小单位
#define UPDATE_RATE (0.001)

double lastupdate = 0;

int cpubusy = 0;

int clocks_cpu_busy(){
    return cpubusy;
}


void clocks_flushtime(){
    lastupdate = glfwGetTime() * SYSTEM_FREQUENCE;
}

//保证每秒调用此函数的nextcycle累计为SYSTEM_FREQUENCE
int clocks_cycle(int nextcycle){
    int cnext = cpu_update(nextcycle);
    int pnext = ppu_update(nextcycle/4)*4;
    nextcycle = cnext > pnext ? pnext:cnext;
    if(nextcycle == 0)
        nextcycle = 1;
    return nextcycle;
}

void clocks_update(){
    /* 以SYSTEM_FREQUENCE频率更新clocks_cycle */
}
