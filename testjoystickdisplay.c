#include "TestJoystickDisplay.h"

#include "GlobalDefine.h"

#include "GLFW/glfw3.h"
//glfw move to glut
void TestJoystickDisplay(int joy){
    UNUSED(joy);
    /*
    int count;
    const float * value;
    value = glfwGetJoystickAxes(joy,&count);
    for(int i=0;i<count;i++){
        glBegin(GL_LINE_LOOP);
        glVertex2d(100,(i+1)*10);
        glVertex2d(100 + value[i]*100,(i+1)*10);
        glEnd();
    }
    const unsigned char * bts = glfwGetJoystickButtons(joy,&count);
    glBegin(GL_LINES);
    for(int i=0;i<count;i++){
       glVertex2d(i*10+1,70 + bts[i]*5);
       glVertex2d((i+1)*10-1,70 + bts[i]);
    }
    glEnd();
    */
}
