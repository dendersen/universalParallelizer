#pragma once
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "parralellize.h"

DWORD*  destroyPool(Pool_t* pool, DWORD join);
DWORD   joinPool(Pool_t* pool, DWORD WaitTime);
Pool_t* createPool(int threadCount, void (*func)(Args_t*), void* args, void* ret);
int addJobToPool(jobPool_t* jobPool, void (*func)(Args_t*), void* args, int* ID);

#ifdef __cplusplus
}
#endif