#pragma once

Pool_t* createPool(int threadCount, void (func)(Args_t*), void* args, void* ret);
int joinPool(Pool_t* pool);
int* destroyPool(Pool_t* pool);

#ifdef _WIN32
#include <windows.h>
typedef HANDLE Uthread_t;
typedef HANDLE Umutex;
#define createUthread(dst,func,arg) dst = CreateThread(NULL,0,func,arg,0,NULL)
#define joinUthread(thread) (int)WaitForSingleObject(thread,INFINITE)
#define closeUthread(thread) CloseHandle(thread)
#endif

#ifdef __linux__
#include <pthread.h>
typedef pthread_t Uthread_t;
typedef pthread_mutex_t* Umutex;
#define createUthread(dst,func,arg) pthread_create(&dst,NULL,func,arg)
#define joinUthread(thread) pthread_join(thread,NULL)
#define closeUthread(thread) pthread_detach(thread)
#endif

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

typedef struct Pool {
	int numThreads;
	Uthread_t* threads;	      // Array of threads
	ThreadData_t* pDataArray; // Array of thread data structures
} Pool_t;

#ifdef job_debug
typedef struct jobList {
	int* jobID;		   // Number of jobs in the list
	Uthread_t jobThread; // thread handle for job
	ThreadData_t* job; // Array of job data structures
	struct jobList* next; // Pointer to the next job in the list
} jobList_t;

typedef struct jobPool {
	Uthread_t managerThread; // Handle for the manager thread
	int cleanupFrequency;  // Frequency of cleanup operations
	int cleanupWait;       // wait time for cleanup operations
	int jobCount;		   //number of jobs in the pool
	char settings;         // Settings for the job pool (x,x,x,x,x,x,x,freeID)
	Umutex jobMutex;      // Mutex for synchronizing access to the job list
	jobList_t* jobList;    // List of jobs in the pool
} jobPool_t;
#endif