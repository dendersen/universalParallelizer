#pragma once

#include <windows.h>

typedef struct ThreadData {
	void (*func)(Args_t*);  // Function to execute in the thread
	Args_t* args;           // Arguments to pass to the function
} ThreadData_t;

typedef struct Args {
	int threadCount; // Number of threads to create
	int ID;          // Thread ID
	void* args;      // Arguments to pass to the function
	void* ret;       // Pointer to store the return value
} Args_t;

typedef struct Pool {
	int numThreads;
	HANDLE* threads;	      // Array of thread handles
	ThreadData_t* pDataArray; // Array of thread data structures
} Pool_t;

typedef struct jobList {
	int* jobID;		   // Number of jobs in the list
	HANDLE jobThread; // thread handle for job
	ThreadData_t* job; // Array of job data structures
	struct jobList* next; // Pointer to the next job in the list
} jobList_t;

typedef struct jobPool {
	HANDLE managerThread; // Handle for the manager thread
	int cleanupFrequency;  // Frequency of cleanup operations
	int cleanupWait;       // Maximum number of jobs in the pool
	int jobCount;		   //number of jobs in the pool
	char settings;         // Settings for the job pool (x,x,x,x,x,x,x,freeID)
	HANDLE jobMutex;      // Mutex for synchronizing access to the job list
	jobList_t* jobList;    // List of jobs in the pool
} jobPool_t;