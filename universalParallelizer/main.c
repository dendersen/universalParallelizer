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

void verify(long long* src1, long long* src2, long long* dst, long long size);
void setupArray(long long* src1, long long* src2, long long* dst, long long size);
void timeTaken(clock_t begin, clock_t end, long long size);

void parallel_add(Args_t* arg) {

	long long* src1 = ((long long**)arg->args)[0];
	long long* src2 = ((long long**)arg->args)[1];
	long long size = *((long long**)arg->args)[2];

	long long min =  (arg->ID      * size) / arg->threadCount;
	long long max = ((arg->ID + 1) * size) / arg->threadCount;


	for (long long i = min; i < max; i++) {
		((long long*)arg->ret)[i] = src1[i] + src2[i];
	}
	
}

parrallel_addFunc(long long* src1, long long* src2, long long* dst, long long size) {
	for (long long i = 0; i < size; i++) {
		dst[i] = src1[i] + src2[i];
	}
}

int init(long long size, long long** src1, long long** src2, long long** dst) {
	printf("\tmalloc src1\n");
	(*src1) = (long long*)malloc(size * sizeof(long long));
	if (!src1) { // Check for memory allocation failure
		printf("failed\n");
		return -1;
	}
	printf("\tmalloc src2\n");
	(*src2) = (long long*)malloc(size * sizeof(long long));
	if (!src2) {
		printf("failed\n");
		free(src1); // Free previously allocated memory before returning
		return -2; // Check for memory allocation failure
	}
	printf("\tmalloc dst\n");
	(*dst) = (long long*)malloc(size * sizeof(long long));
	if (!dst) {
		printf("failed\n");
		free(src1); // Free previously allocated memory before returning
		free(src2);
		return -3; // Check for memory allocation failure
	}
	printf("\tarray contents\n");
	setupArray(*src1, *src2, *dst, size);
	return 0;
}

int main(){
	int maxThreadCount = 4;
	printf("threadCount = %d\n", maxThreadCount);
	printf("intializing:\n");
	long long  size = 1000000000; // ~1.5 billion elements in one second ish
	long long* src1 = NULL;
	long long* src2 = NULL;
	long long* dst  = NULL;
	int fail = init(size, &src1, &src2, &dst);
	if(fail != 0) {
		printf("failed to initialize\n");
		return fail;
	}
	void* internalArg = (void* []){ (void*)src1, (void*)src2, (void*)&size };

	printf("starting program\n");
	clock_t begin = clock();
	Pool_t* pool = createPool(maxThreadCount, parallel_add, internalArg, dst);
	printf("\tprogram started\n");
	int res = joinPool(pool);
	clock_t end = clock();
	printf("program done\n");

	if(res != 0) {
		printf("Thread pool failed with error code %d\n", res);
	}
	
	timeTaken(begin, end, size);

	verify(src1, src2, dst, size);

	printf("\nprepparing single thread pool\n");

	printf("\tcleanup\n");
	free(src1); src1 = NULL;
	free(src2); src2 = NULL;
	free(dst);  dst = NULL;
	destroyPool(pool);

	fail = init(size, &src1, &src2, &dst);
	if (fail != 0) {
		printf("failed to initialize\n");
		return fail;
	}
	internalArg = (void* []){ (void*)src1, (void*)src2, (void*)&size };

	printf("starting single thread pool\n");

	Args_t arg = { .threadCount = 1, .ID = 0, .args = internalArg, .ret = dst };
	begin = clock();
	pool = createPool(1, parallel_add, internalArg, dst);
	printf("single thread pool running\n");
	joinPool(pool);
	end = clock();
	printf("single thread pool done\n");

	verify(src1, src2, dst, size);

	timeTaken(begin, end, size);

	printf("cleanup\n");
	free(src1); src1 = NULL;
	free(src2); src2 = NULL;
	free(dst);  dst = NULL;
	destroyPool(pool);

	printf("\nprepparing direct function call\n");

	fail = init(size, &src1, &src2, &dst);
	if (fail != 0) {
		printf("failed to initialize\n");
		return fail;
	}
	internalArg = (void* []){ (void*)src1, (void*)src2, (void*)&size };

	setupArray(src1, src2, dst, size);
	
	printf("calling funciton\n");
	begin = clock();
	parrallel_addFunc(src1, src2, dst, size);
	end = clock();
	printf("function returned\n");
	
	verify(src1, src2, dst, size);

	timeTaken(begin, end, size);

	printf("\ndemo complete\n");
	free(src1);
	free(src2);
	free(dst);
	destroyPool(pool);

	return 0;
}

void verify(long long* src1, long long* src2, long long* dst, long long size) {
	printf("verifying results\n");

	unsigned long long errors = 0;
	for (long long i = 0; i < size; i++) {
		if (dst[i] != i * 3) {
			printf("\tError at index %lld: dst modified,  expected %lld, got %lld\n", i, i * 3, dst[i]);
			errors++;
		}
		if (src1[i] != i) {
			printf("\tError at index %lld: src1 modified, expected %lld, got %lld\n", i, i, src1[i]);
			errors++;
		}
		if (src2[i] != i * 2) {
			printf("\tError at index %lld: src2 modified, expected %lld, got %lld\n", i, i * 2, src2[i]);
			errors++;
		}
	}
	if (errors == 0) {
		printf("completed with no errors\n");
	}
	else {
		printf("completed with %llu errors\n", errors);
	}
}
void setupArray(long long* src1, long long* src2, long long* dst, long long size) {
	for (long long i = 0; i < size; i++) {
		src1[i] = i;
		src2[i] = i * 2;
		dst[i] = 0;
	}
}

void timeTaken(clock_t begin, clock_t end, long long size) {
	double spent = ((double)end / (double)CLOCKS_PER_SEC) - ((double)begin / (double)CLOCKS_PER_SEC);

	printf("Time taken: %.4lf seconds\n", spent);
	printf("Throughput: %.4lf operations/second\n", size / spent);
}

#ifdef __cplusplus
}
#endif