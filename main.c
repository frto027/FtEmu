#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "GLFW/glfw3.h"
#include "GL/gl.h"

#include "ScreenReshape.h"
#include "BlockDisplay.h"
#include "NesParser.h"
#include "CpuEmulate.h"

#include "PpuEmulate.h"

#include "TestDisplay.h"
#include "TestJoystickDisplay.h"

#include "GlobalClocks.h"

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n",error, description);
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods){
    //printf("%c %d %d %d\n",(char)key,key, action,mods);
    UNUSED(window);
    UNUSED(scancode);
    UNUSED(mods);
    if(action == 1){
        if(key == GLFW_KEY_M){
            ScreenTip("m Pressed",1);
        }
        if(key == GLFW_KEY_O){
//            ParseNesFile("F:\\tutor.nes");
            ParseNesFile("/home/frto027/Things/Mario.nes");
//            ParseNesFile("F:\\Mario.nes");
//            ParseNesFile("/home/frto027/Downloads/nestest.nes");
        }
        if(key == GLFW_KEY_D){
            for(uint16_t i=0x3F00;i<0x3F20;i++){
                printf("%04X:%X\n",i,ppu_read(i));
            }
        }
        if(key == GLFW_KEY_S){
            cpu_memory[PPU_REG_PPUSTATUS]|= 1 << 6;
        }
        if(key == GLFW_KEY_U){
            cpu_memory[PPU_REG_PPUSTATUS]&= ~(1 << 6);
        }
    }
    if(key == GLFW_KEY_N){
        cpu_memory[PPU_REG_PPUCTRL]|= 1 << 7;
        cpu_interrupt(CPU_INTERRUPT_NMI);
    }
}

void draw(GLFWwindow * window){
    static int maxtime = CLOCKS_PER_SEC/FPS_LIMIT;
    static clock_t lasttick = 0;
    clock_t tktime = clock();
    if(tktime - lasttick < maxtime)
        return;
    lasttick = tktime;
    glClear(GL_COLOR_BUFFER_BIT);
    BlockDisplay();
//    TestJoystickDisplay(GLFW_JOYSTICK_1);
    glfwSwapBuffers(window);

}

char title[1024]="Emulator";
char * titleapp;

void ShowIps(GLFWwindow * window){
    static int count = 0;
    static time_t lasttime = 0;
    time_t nowtime = time(NULL);
    if(nowtime != lasttime){
        sprintf(titleapp," Ips:%d",(int)count);
        glfwSetWindowTitle(window,title);
        lasttime = nowtime;
        count = 0;
    }
    count++;
}

void WindowResize(GLFWwindow * window,int w,int h){
    UNUSED(window);
    ResizeScreenCallback(w,h);
}




int main()
{
//    freopen("/home/frto027/logout","w",stdout);
    //为Fps显示做准备
    for(titleapp = title;*titleapp;titleapp++)
        continue;
    if(!glfwInit()){
        fprintf(stderr,"glfwInit fail!");
        exit(-1);
    }
    glfwSetErrorCallback(error_callback);
    GLFWwindow * window = glfwCreateWindow(800,480,title,NULL,NULL);
    if(!window){
        fprintf(stderr,"window create fail");
        glfwTerminate();
        exit(-1);
    }

    glfwSetKeyCallback(window,keyCallBack);
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window,WindowResize);
    glfwSetWindowSize(window,ORI_WIDTH*2,ORI_HEIGHT*2);

    while(!glfwWindowShouldClose(window)){
        //主循环
        //绘图

        draw(window);
        //调整大小
        //WindowResize();
        //处理事件
        glfwPollEvents();
        //统计Ips
        ShowIps(window);
        //处理cpu
        if(nes_loaded){
            clocks_update();
        }
        //usleep(1);
    }
    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}
