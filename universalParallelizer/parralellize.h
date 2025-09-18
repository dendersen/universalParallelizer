#pragma once

typedef struct Args {
	int threadCount; // Number of threads to create
	int ID;          // Thread ID
	void* args;      // Arguments to pass to the function
	void* ret;       // Pointer to store the return value
} Args_t;

typedef struct ThreadData {
	void (*func)(Args_t*);  // Function to execute in the thread
	Args_t* args;           // Arguments to pass to the function
} ThreadData_t;

#ifdef _WIN32
#include <windows.h>

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
	int cleanupWait;       // wait time for cleanup operations
	int jobCount;		   //number of jobs in the pool
	char settings;         // Settings for the job pool (x,x,x,x,x,x,x,freeID)
	HANDLE* jobMutex;      // Mutex for synchronizing access to the job list
	jobList_t* jobList;    // List of jobs in the pool
} jobPool_t;

#endif

#ifdef __linux__
#include <pthread.h>

typedef struct Pool {
	int numThreads;
	pthread_t* threads;	      // Array of thread handles
	ThreadData_t* pDataArray; // Array of thread data structures
} Pool_t;

typedef struct jobList {
	int* jobID;		   // Number of jobs in the list
	pthread_t jobThread; // thread handle for job
	ThreadData_t* job; // Array of job data structures
	struct jobList* next; // Pointer to the next job in the list
} jobList_t;

typedef struct jobPool {
	pthread_t managerThread; // Handle for the manager thread
	int cleanupFrequency;  // Frequency of cleanup operations
	int cleanupWait;       // wait time for cleanup operations
	int jobCount;		   //number of jobs in the pool
	char settings;         // Settings for the job pool (x,x,x,x,x,x,x,freeID)
	pthread_mutex_t* jobMutex;      // Mutex for synchronizing access to the job list
	jobList_t* jobList;    // List of jobs in the pool
} jobPool_t;

#endif