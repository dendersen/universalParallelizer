#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include "parralellize_Thread.h"

DWORD WINAPI ThreadFunction(LPVOID lpParam);

Pool_t* createPool(int threadCount, void (*func)(Args_t*), void* args, void* ret) {
	Pool_t* pool = (Pool_t*)malloc(sizeof(Pool_t));
	if (pool == NULL) return NULL; // Memory allocation failed½
	pool->numThreads = threadCount;
	pool->threads = (HANDLE*)malloc(sizeof(HANDLE) * threadCount);
	if (pool->threads == NULL) {
		free(pool); // Clean up if allocation failed
		return NULL;
	}

	ThreadData_t* pDataArray = (ThreadData_t*)malloc(sizeof(ThreadData_t) * threadCount);
	if (pDataArray == NULL) {
		free(pool->threads);
		free(pool);
		return NULL; // Memory allocation failed
	}
	
	DWORD*		  dwThreadIdArray = (DWORD*)malloc(sizeof(DWORD) * threadCount);
	if (dwThreadIdArray == NULL) {
		free(pool->threads);
		free(pool);
		free(pDataArray);
		return NULL; // Memory allocation failed
	}
	for (int i = 0; i < threadCount; i++) {
		pDataArray[i].args = (Args_t*)malloc(sizeof(Args_t));
		if(pDataArray[i].args == NULL) {
			for (int j = 0; j < i; j++) {
				free(pDataArray[j].args); // Clean up previously allocated args
			}
			free(pDataArray);
			free(pool->threads);
			free(pool);
			return NULL; // Memory allocation failed
		}
		pDataArray[i].args->threadCount = threadCount;
		pDataArray[i].args->ID = i;
		pDataArray[i].args->args = args; // Pass the same args to all threads
		pDataArray[i].args->ret = ret;   // Pointer to store the return value
		pDataArray[i].func = func;        // Function to execute in the thread
	}

	for (size_t i = 0; i < threadCount; i++){
		pool->threads[i] = CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			ThreadFunction,			// thread function name
			&pDataArray[i],			// argument to thread function 
			0,                      // use default creation flags 
			&dwThreadIdArray[i]		// returns the thread identifier 
		);   
	}
	pool->pDataArray = pDataArray; // Store the thread data array in the pool
	return pool; // Return the created pool
}

DWORD WINAPI ThreadFunction(LPVOID lpParam) {
	ThreadData_t* pData = (ThreadData_t*)lpParam;
	if (pData == NULL || pData->func == NULL || pData->args == NULL) {
		return 1; // Error: Invalid parameters
	}
	pData->func(pData->args); // Execute the function and store the return value
	return 0; // Success
}

DWORD joinPool(Pool_t* pool, DWORD WaitTime) {
	if(WaitTime == 0) {
		WaitTime = INFINITE; // Default to wait indefinitely if no time is specified
	}
	DWORD waitResult = WaitForMultipleObjects(pool->numThreads, pool->threads, TRUE, WaitTime);
	return waitResult;// Success
}

DWORD* destroyPool(Pool_t* pool, DWORD join) {
	DWORD* waitResult = (DWORD*)malloc(sizeof(DWORD) * pool->numThreads);
	if (waitResult == NULL) return NULL; // Memory allocation failed

	for (int i = 0; i < pool->numThreads; i++) {
		if (join) {
			waitResult[i] = WaitForSingleObject(pool->threads[i], join); // Wait for each thread to finish
		}
		CloseHandle(pool->threads[i]); // Close thread handles
	}
	free((&pool->pDataArray[0])->args->args);
	free((&pool->pDataArray[0])->args->ret);
	for (int i = 0; i < pool->numThreads; i++) {
		free((&(pool->pDataArray[i]))->args); // Free the args structure
	}
	free(pool->pDataArray); // Free the thread data array
	free(pool->threads); // Free the thread handles array
	free(pool); // Free the pool structure
	return waitResult; // Return the wait results
}

DWORD WINAPI jobManager(LPVOID lpParam) {
	jobPool_t* jobPool = (jobPool_t*)lpParam;
	if (jobPool == NULL) return 1; // Error: Invalid job pool
	while (1) {
		Sleep(jobPool->cleanupFrequency); // Wait for the cleanup frequency
		WaitForSingleObject(jobPool->jobMutex, INFINITE); // Acquire the mutex to access the job list
		jobList_t* current = jobPool->jobList;
		while (current != NULL) {
			if(current->jobThread == NULL) {
				// Job thread is NULL, remove this job from the list
				jobList_t* toDelete = current;
				current = current->next; // Move to the next job
				free(toDelete); // Free the job list node
				continue; // Skip to the next iteration
			}
			DWORD response = WaitForSingleObject(current->jobThread, jobPool->cleanupWait); // Wait for the job to finish
			if (response == 0) {
				// Job finished, remove it from the list
				jobList_t* toDelete = current;
				current = current->next; // Move to the next job
				free(toDelete->job->args->args); // Free the job arguments
				free(toDelete->job->args->ret); // Free the job arguments
				free(toDelete->job->args); // Free the job arguments
				if(jobPool->settings & 0b00000001) {
					free(toDelete->jobID); // Free the job data
				} else {
					(*toDelete->jobID) = -1;
				}
				CloseHandle(toDelete->jobThread); // Close the thread handle
				free(toDelete); // Free the job list node
			}
			else {
				current = current->next; // Move to the next job if it is still running
			}
		}
		ReleaseMutex(jobPool->jobMutex); // Release the mutex to allow job addition
	}
}

DWORD WINAPI jobFunction(LPVOID lpParam) {
	ThreadData_t* pData = (ThreadData_t*)lpParam;
	if (pData == NULL || pData->func == NULL || pData->args == NULL) {
		return 1; // Error: Invalid parameters
	}
	pData->func(pData->args); // Execute the function and store the return value
	return 0; // Success
}

jobPool_t* createJobPool(int cleanupFrequency, int cleanupWait, int auroFreeID) {
	jobPool_t* jobPool = (jobPool_t*)malloc(sizeof(jobPool_t));
	if (jobPool == NULL) return NULL; // Memory allocation failed

	jobPool->cleanupFrequency = cleanupFrequency; // Set the cleanup frequency
	jobPool->cleanupWait = cleanupWait; // Set the cleanup wait time

	jobPool->jobCount = 0; // Initialize job count to zero
	jobPool->jobList = NULL; // Initialize job list to NULL

	jobPool->jobMutex = CreateMutex(NULL, FALSE, NULL); // Create a mutex for job synchronization
	if(jobPool->jobMutex == NULL) {
		free(jobPool); // Clean up if mutex creation failed
		return NULL; // Memory allocation failed
	}

	jobPool->settings = 0; // Set default settings (e.g., job IDs will be freed automatically)
	if (auroFreeID) {
		jobPool->settings |= 0b00000001; // Set the auto-free ID setting
	}

	jobPool->managerThread = CreateThread(
		NULL,                   // default security attributes
		0,                      // use default stack size  
		jobManager,       // thread function name
		jobPool,                // argument to thread function 
		0,                      // use default creation flags 
		NULL                    // returns the thread identifier 
	);
	if(jobPool->managerThread == NULL) {
		CloseHandle(jobPool->jobMutex); // Clean up if thread creation failed
		free(jobPool); // Free the job pool structure
		return NULL; // Memory allocation failed
	}

	return jobPool; // Return the created job pool
}

int addJobToPool(jobPool_t* jobPool, void (*func)(Args_t*), void* args, int* ID) {
	Args_t* jobArgs = (Args_t*)malloc(sizeof(Args_t));
	if (jobArgs == NULL) return -1; // Memory allocation failed
	jobArgs->args = args; // Set the job arguments
	jobArgs->ret = NULL; // Allocate memory for the return value
	jobArgs->threadCount = 1; // Set thread count to 1 for this job
	jobArgs->ID = 0; // Set the job ID
	
	ThreadData_t* jobData = (ThreadData_t*)malloc(sizeof(ThreadData_t));
	if (jobData == NULL) {
		free(jobArgs); // Clean up if allocation failed
		return -1; // Memory allocation failed
	}
	jobData->args = jobArgs; // Set the job arguments
	jobData->func = func; // Set the job function

	jobList_t* newJob = (jobList_t*)malloc(sizeof(jobList_t));
	if (newJob == NULL) {
		free(jobData->args); // Clean up if allocation failed
		free(jobData);
		return -2; // Memory allocation failed
	}
	newJob->job = jobData; // Set the job data
	if (ID == NULL) {
		if (jobPool->settings & 0b00000001) {
			ID = (int*)malloc(sizeof(int)); // Allocate memory for the job ID if not provided
		}
		if (ID == NULL) {
			free(newJob->job->args); // Clean up if allocation failed
			free(newJob->job);
			free(newJob);
			return -3; // Memory allocation failed
		}
	}
	WaitForSingleObject(jobPool->jobMutex, INFINITE); // Acquire the mutex to access the job list
	jobList_t* current = jobPool->jobList;
	for (int id = 0; jobPool->jobCount; id++) {
		int found = 0;
		while(current != NULL) {
			if (*current->jobID == id) {
				found = 1;
				break;
			} else {
				current = current->next; // Move to the next job
			}
		}
		if(!found) {
			*ID = id; // Set the job ID
			break; // Exit the loop if an available ID is found
		}
		current =jobPool->jobList; // Reset current to the start of the list
	}
	jobPool->jobCount++; // Increment job count
	newJob->jobID = ID; // Set the job ID
	newJob->next = jobPool->jobList; // Initialize the next pointer to NULL
	jobPool->jobList = newJob; // Add the new job to the job list

	newJob->jobThread = CreateThread(
		NULL,                   // default security attributes
		0,                      // use default stack size  
		jobFunction,        // thread function name
		jobData,               // argument to thread function 
		0,                      // use default creation flags 
		NULL                    // returns the thread identifier 
	);
	if(newJob->jobThread == NULL) {
		free(newJob->job->args); // Clean up if thread creation failed
		free(newJob->job);
		free(newJob);
		ReleaseMutex(jobPool->jobMutex); // Release the mutex before returning
		return -4; // Memory allocation failed
	}
	ReleaseMutex(jobPool->jobMutex); // Release the mutex after modifying the job list
	return 0; // Success
}

#ifdef __cplusplus
}
#endif