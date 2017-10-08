#include "GlobalDefine.h"
#include "GlobalClocks.h"
#include "time.h"

#include "CpuEmulate.h"
#include "PpuEmulate.h"

#include <stdio.h>

//保证此函数以SYSTEM_FREQUENCE的频率被调用
void clocks_cycle(){
    static int ppurun = 0;
    cpu_cycle();
    ppurun = (ppurun + 1)%4;
    if(ppurun==0)
        ppu_cycle();
}

void clocks_update(){
    static int tickperclock = SYSTEM_FREQUENCE/CLOCKS_PER_SEC;
    static clock_t lasttick = 0;
    static int runremain = 0;
    static int tickperrun = 1;
    static int runtime = 0;
    if(lasttick != clock()){
        lasttick = clock();
        //更新计算tickperrun
        if(runremain){
        int newtime = tickperclock/runtime;
        if(tickperrun < newtime)
            tickperrun = newtime;
        }
        runremain = tickperclock;
        runtime = 0;
    }
    if(runremain){
        runtime++;
        if(runremain <= tickperrun){
            for(int i=0;i<runremain;i++)
                clocks_cycle();
            runremain = 0;
        }else{
            runremain-=tickperrun;
            for(int i=0;i<tickperrun;i++)
                clocks_cycle();
        }
    }
}
