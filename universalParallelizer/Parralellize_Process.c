#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include "parralellize.h"

typedef struct ProcessorPool {
	int numProcessors;
	HANDLE* processes
} ProcessorPool_t;

typedef struct ProcesseArgs {
	void* ownedArgs; // Pointer to the arguments owned by the process
	void* sharedArgs; // Pointer to the arguments shared among processes
	int processIndex; // Index of the process in the pool
	int processCount; // Total number of processes in the pool
	void* returnValue; // Pointer to store the return value of the process (predefinedSize)
} ProcesseArgs_t;

ProcessorPool_t* createMultiProcessor(int processCount, void (*func)(ProcesseArgs_t*)){
	ProcessorPool_t* pool = (ProcessorPool_t*)malloc(sizeof(ProcessorPool_t));
	if (pool == NULL) {
		return NULL; // Memory allocation failed
	}
	pool->numProcessors = processCount;
	pool->processes = (HANDLE*)malloc(sizeof(HANDLE) * processCount);
	if (pool->processes == NULL) {
		free(pool); // Clean up if allocation fails
		return NULL; // Memory allocation failed
	}
	for (int i = 0; i < processCount; i++) {
		pool->processes[i] = CreateProcess(
			NULL, // Application name
			NULL, // Command line arguments
			NULL, // Process attributes
			NULL, // Thread attributes
			FALSE, // Inherit handles
			0, // Creation flags
			NULL, // Environment block
			NULL, // Current directory
			NULL, // Startup info
			NULL  // Process information
		);
	}

}


#ifdef __cplusplus
}
#endif