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

#include <pthread.h>

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
            pthread_mutex_lock(&clock_mutex);

//            ParseNesFile("F:\\tutor.nes");
//            ParseNesFile("/home/frto027/Things/Mario.nes");
            ParseNesFile("F:\\Mario.nes");
//            ParseNesFile("/home/frto027/Downloads/nestest.nes");

            pthread_mutex_unlock(&clock_mutex);
        }
        if(key == GLFW_KEY_D){
            for(uint16_t i=0x3F00;i<0x3F20;i++){
                printf("%04X:%X\n",i,ppu_read(i));
            }
        }
        /*
        if(key == GLFW_KEY_S){
            cpu_memory[PPU_REG_PPUSTATUS]|= 1 << 6;
        }
        if(key == GLFW_KEY_U){
            cpu_memory[PPU_REG_PPUSTATUS]&= ~(1 << 6);
        }
        */
    }
    if(key == GLFW_KEY_N){
        cpu_memory[PPU_REG_PPUCTRL]|= 1 << 7;
        cpu_interrupt(CPU_INTERRUPT_NMI);
    }
}

void draw(GLFWwindow * window){
    static double lasttick = 0;

    double thistick = glfwGetTime();

    if((thistick - lasttick)*FPS_LIMIT < 1){
        return;
    }
    lasttick = thistick;
    glClear(GL_COLOR_BUFFER_BIT);
    BlockDisplay();
//    TestJoystickDisplay(GLFW_JOYSTICK_1);
    glfwSwapBuffers(window);
}

char title[1024]="Emulator";
char * titleapp;

void ShowIps(GLFWwindow * window){
    static int count = 0;
    static int lasttime = 0;
    int nowtime = (int)glfwGetTime();
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


void logtime(int i){
    printf("time % 2d %lf\n",i,glfwGetTime());
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

    //开启线程负责cpu计算
    pthread_t clocks_thread;
    {
        int err =
        pthread_create(&clocks_thread,NULL,clocks_update_main,NULL);
        if(err){
            fprintf(stderr,"thread create failed:%d\n",err);
            exit(-1);
        }
        pthread_mutex_init(&clock_mutex,NULL);
    }


    while(!glfwWindowShouldClose(window)){
        //主循环
        //绘图
        draw(window);
        //处理事件
        glfwPollEvents();
        //统计Ips
        //ShowIps(window);
    }

    glfwDestroyWindow(window);

    printf("waiting for clock thread %d end...\n",clocks_thread);
    clocks_stop();
    pthread_join(clocks_thread,NULL);
    printf("end.");

    glfwTerminate();


    return 0;
}
