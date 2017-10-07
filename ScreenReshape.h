#ifndef _SCREEN_RESHAPE_H
#define _SCREEN_RESHAPE_H

#include "GlobalDefine.h"

/*
enum VIEW_RESHAPE_MODE {
#define	VIEW_RESHAPE_ORIGION 0			//原始左上
#define VIEW_RESHAPE_SCALE 1			//拉伸缩放
#define VIEW_RESHAPE_CENTER 2			//原始居中
#define VIEW_RESHAPE_SCALE_CENTER 3		//自动适应
};
*/

//glutReshapeFunc(ResizeScreenCallback);
void ResizeScreenCallback(int w,int h);

//viewResizeMode是VIEW_RESHAPE_MODE，即
//VIEW_RESHAPE_ORIGION VIEW_RESHAPE_SCALE VIEW_RESHAPE_CENTER VIEW_RESHAPE_SCALE_CENTER
//之一
void setResizeRule(int viewResizeMode);
//设置原大小显示时的缩放因数
void setResizeFactor(double _factor);
#endif // !_SCREEN_RESHAPE_H
