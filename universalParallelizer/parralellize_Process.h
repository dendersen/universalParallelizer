#pragma once

//this is not necessary at the moment and is therfore paused temporarily
#ifdef job_debug

#include <stdlib.h>
#include "parralellize.h"

typedef struct ProcessorPool {
	int numProcessors;
	HANDLE* processes;
} ProcessorPool_t;

typedef struct ProcesseArgs {
	void* ownedArgs; // Pointer to the arguments owned by the process
	void* sharedArgs; // Pointer to the arguments shared among processes
	int processIndex; // Index of the process in the pool
	int processCount; // Total number of processes in the pool
	void* returnValue; // Pointer to store the return value of the process (predefinedSize)
} ProcesseArgs_t;

#endif