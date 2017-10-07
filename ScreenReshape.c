#include "ScreenReshape.h"
#include "GL/gl.h"
int view_reshape_mode = VIEW_RESHAPE_SCALE_CENTER;

void setResizeRule(int viewReshapeMode) {
    view_reshape_mode = viewReshapeMode;
}

double origion_scale_factor = 2.;

void setResizeFactor(double factor){
    origion_scale_factor = factor;
}

double mind(double a, double b) { return a < b ? a : b; }
void ResizeScreenCallback(int w, int h) {
    glLoadIdentity();
    double factor = 1;
    switch (view_reshape_mode)
    {
    default:
    case VIEW_RESHAPE_ORIGION:
        glViewport(0,h - ORI_HEIGHT * origion_scale_factor,ORI_WIDTH,ORI_HEIGHT);
        glPixelZoom(origion_scale_factor,origion_scale_factor);
        glRasterPos2d(-1,-1);
        break;
    case VIEW_RESHAPE_SCALE:
        glViewport(0,0,ORI_WIDTH,ORI_HEIGHT);
        glPixelZoom((float)w/ORI_WIDTH,(float)h/ORI_HEIGHT);
        glRasterPos2d(-1,-1);
        break;
    case VIEW_RESHAPE_CENTER:
        glViewport((w - ORI_WIDTH * origion_scale_factor)/2,(h - ORI_HEIGHT * origion_scale_factor)/2,ORI_WIDTH,ORI_HEIGHT);
        glPixelZoom(origion_scale_factor,origion_scale_factor);
        glRasterPos2d(-1,-1);
        break;
    case VIEW_RESHAPE_SCALE_CENTER:
        factor = mind((float)w/ORI_WIDTH,  (float)h/ORI_HEIGHT);
        glViewport((w - ORI_WIDTH * factor)/2,(h - ORI_HEIGHT * factor)/2,ORI_WIDTH,ORI_HEIGHT);
        glPixelZoom(factor,factor);
        glRasterPos2d(-1,-1);
        break;
    }
}
