#ifndef GLOBALCLOCKS_H
#define GLOBALCLOCKS_H

#include <pthread.h>

extern pthread_mutex_t clock_mutex;

//cpu_ppu线程
void * clocks_update_main(void *);
void clocks_stop();
#endif // GLOBALCLOCKS_H
