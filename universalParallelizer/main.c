#include "parralellize.h"
#include "parralellize_Thread.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

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
	long long size = 1000000000; // ~1.5 billion elements in one second ish
	long long* src1 = (long long*)malloc(size * sizeof(long long));
	if (!src1) return -1; // Check for memory allocation failure
	long long* src2 = (long long*)malloc(size * sizeof(long long));
	if( !src2) {
		free(src1); // Free previously allocated memory before returning
		return -1; // Check for memory allocation failure
	}
	long long* dst = (long long*)malloc(size * sizeof(long long));
	if( !dst) {
		free(src1); // Free previously allocated memory before returning
		free(src2);
		return -1; // Check for memory allocation failure
	}

	for (long long i = 0; i < size; i++) {
		src1[i] = i;
		src2[i] = i * 2;
		dst[i] = 0;
	}

	clock_t begin = clock();

	Pool_t* pool = createPool(4, parallel_add, (void* []) { (void*)src1, (void*)src2, (void*)&size }, dst);
	int res = joinPool(pool,0);
	
	clock_t end = clock();

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