#pragma once

#include "parralellize.h"

#ifdef _WIN32
#include <windows.h>

Pool_t* createPool(int threadCount, void (*func)(Args_t*), void* args, void* ret);
DWORD*  destroyPool(Pool_t* pool, DWORD join);
DWORD   joinPool(Pool_t* pool, DWORD WaitTime);
int addJobToPool(jobPool_t* jobPool, void (*func)(Args_t*), void* args, int* ID);

#endif

#ifdef __linux__
#include <pthread.h>

Pool_t* createPool(int threadCount, void (*func)(Args_t*), void* args, void* ret);
int*    destroyPool(Pool_t* pool, int join);
int     joinPool(Pool_t* pool, int WaitTime);
int     addJobToPool(jobPool_t* jobPool, void (*func)(Args_t*), void* args, int* ID);

#endif