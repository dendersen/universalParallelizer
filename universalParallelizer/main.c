#include "parralellize.h"
#include <assert.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void parallel_add(Args_t* arg) {

	int* src1 = ((int**)arg->args)[0];
	int* src2 = ((int**)arg->args)[1];
	size_t size = (size_t)((int**)arg->args)[2];

	int amount = size / arg->threadCount;
	int min =  arg->ID      * amount;
	int max = (arg->ID + 1) * amount;

	Sleep(100); // Simulate some work with a sleep

	for (int i = min; i < max; i++) {
		((int*)arg->ret)[i] = src1[i] + src2[i];
	}
	
}

int main(){
	size_t size = 10000;
	int* src1 = (int*)malloc(size * sizeof(int));
	if (!src1) return -1; // Check for memory allocation failure
	int* src2 = (int*)malloc(size * sizeof(int));
	if( !src2) {
		free(src1); // Free previously allocated memory before returning
		return -1; // Check for memory allocation failure
	}
	int* dst = (int*)malloc(size * sizeof(int));
	if( !dst) {
		free(src1); // Free previously allocated memory before returning
		free(src2);
		return -1; // Check for memory allocation failure
	}

	for (int i = 0; i < size; i++) {
		src1[i] = i;
		src2[i] = i * 2;
		dst[i] = 0;
	}

	Pool_t* pool = createPool(4, parallel_add, (void* []) { (void*)src1, (void*)src2, (void*)size }, dst);

	DWORD* res = joinPool(pool,0);

	for (size_t i = 0; i < pool->numThreads; i++) {
		if(res[i] != 0) {
			printf("Thread %zu failed with error code %lu\n", i, res[i]);
		}
	}
	
	for (int i = 0; i < size; i++) {
		if (dst[i] != i * 3) {
			printf("Error at index %d: expected %d, got %d\n", i, i * 3, dst[i]);
		}
	}

	return 0;
}

#ifdef __cplusplus
}
#endif