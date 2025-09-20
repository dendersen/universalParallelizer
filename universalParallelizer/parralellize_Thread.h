#pragma once

#include "parralellize.h"

Pool_t* createPool(int threadCount, void (func)(Args_t*), void* args, void* ret);
int joinPool(Pool_t* pool, int WaitTime);
int* destroyPool(Pool_t* pool);