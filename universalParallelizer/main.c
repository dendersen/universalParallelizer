#include "parralellize.h"
#include "parralellize_Thread.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

//compiling in linux terminal gcc main.c parralellize.h parralellize_Thread.h parralellize_Thread.c

void parallel_add(Args_t* arg) {

	size_t* src1 = ((size_t**)arg->args)[0];
	size_t* src2 = ((size_t**)arg->args)[1];
	size_t size = *((size_t**)arg->args)[2];

	size_t amount = size / arg->threadCount;
	size_t min =  arg->ID      * amount;
	size_t max = (arg->ID + 1) * amount;


	for (size_t i = min; i < max; i++) {
		((size_t*)arg->ret)[i] = src1[i] + src2[i];
	}
	
}

int main(){
	printf("intializing:\n");
	long long size = 1000000; // ~1.5 billion elements in one second ish
	printf("\tmalloc src1\n");
	long long* src1 = (long long*)malloc(size * sizeof(long long));
	if (!src1){ // Check for memory allocation failure
		printf("failed\n");
		return -1;
	}
	printf("\tmalloc src2\n");
	long long* src2 = (long long*)malloc(size * sizeof(long long));
	if( !src2) {
		printf("failed\n");
		free(src1); // Free previously allocated memory before returning
		return -1; // Check for memory allocation failure
	}
	printf("\tmalloc dst\n");
	long long* dst = (long long*)malloc(size * sizeof(long long));
	if( !dst) {
		printf("failed\n");
		free(src1); // Free previously allocated memory before returning
		free(src2);
		return -1; // Check for memory allocation failure
	}
	printf("\tarray contents\n");
	for (long long i = 0; i < size; i++) {
		src1[i] = i;
		src2[i] = i * 2;
		dst[i] = 0;
	}

	printf("starting program\n");
	clock_t begin = clock();
	Pool_t* pool = createPool(4, parallel_add, (void* []) { (void*)src1, (void*)src2, (void*)&size }, dst);
	printf("\tprogram started\n");
	int res = joinPool(pool,0);
	clock_t end = clock();
	printf("program done\n");

	if(res != 0) {
		printf("Thread pool failed with error code %d\n", res);
	}
	
	for (long long i = 0; i < size; i++) {
		if (dst[i] != i * 3) {
			printf("Error at index %lld: expected %lld, got %lld\n", i, i * 3, dst[i]);
		}
	}

	printf("Completed successfully\n");
	double spent = ((double)end / (double)CLOCKS_PER_SEC) - ((double)begin / (double)CLOCKS_PER_SEC);

	printf("Time taken: %.4lf seconds\n", spent);
	printf("Throughput: %.4lf operations/second\n", size / spent);

	return 0;
}

#ifdef __cplusplus
}
#endif