#include "GlobalDefine.h"
#include "GlobalClocks.h"

#include "GLFW/glfw3.h"

#include "CpuEmulate.h"
#include "PpuEmulate.h"

#include <stdio.h>

pthread_mutex_t clock_mutex;

int stopsignal = 0;

double lastupdate = 0;

//保证每秒调用此函数的nextcycle累计为SYSTEM_FREQUENCE
int clocks_cycle(int nextcycle){
    int cnext = cpu_update(nextcycle);
    int pnext = ppu_update(nextcycle/4)*4;
    nextcycle = cnext > pnext ? pnext:cnext;
    if(nextcycle == 0)
        nextcycle = 1;
    return nextcycle;
}

void delay(double time){
    double begt = glfwGetTime();
    while(!stopsignal && glfwGetTime() - begt < time)
        continue;
}
//clocks线程主函数
void * clocks_update_main(void * a){
    UNUSED(a);
    int nextup = 4;
    while(!stopsignal){
        if(!nes_loaded)
            continue;
        delay(nextup/(double)SYSTEM_FREQUENCE);

        pthread_mutex_lock(&clock_mutex);
        nextup = clocks_cycle(nextup);
        pthread_mutex_unlock(&clock_mutex);
    }
    return NULL;
}
void clocks_stop(){
    stopsignal = 1;
}
