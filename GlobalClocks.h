#ifndef GLOBALCLOCKS_H
#define GLOBALCLOCKS_H

void clocks_update();
void clocks_flushtime();
int clocks_cpu_busy();//返回cpu是否忙，如果忙，不应绘图
#endif // GLOBALCLOCKS_H
