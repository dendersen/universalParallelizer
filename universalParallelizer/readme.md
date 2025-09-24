# universal parralellizer

## create pool

to create a work pool (SIMD) simply use

``` C

#include "parralellize_Thread.c"
createPool(threadCount, function, functionArgs, returnValues)

```

all thread will resieve almost the same input
the only difference being the thread ID

this uses the custom Uthread system that will be described later.

## multi threading compatible functions

to actually use multithreading the function you write must be compatible with multithreading, no additional safties are included in the library at this point

``` C

struct Args {
	int threadCount; // Number of threads to create
	int ID;          // Thread ID
	void* args;      // Arguments to pass to the function
	void* ret;       // Pointer to store the return value
} Args_t;

```

to be able to the function it must recieve a pointre to the struct called Args_t

this will be the only input given to the thread upon launch

upon launch the function should get the args pointer, this pointer is the same as the one provided to "createPool()"

do remember that this memory block is not different for each thread, so all thread safety must be implimentet by you

it is generally a good idea to either provide the "args_t" as a single struct or a well known array.
then uppon use cast it to the actual used datatype.

looking in "main.c" you can see an example of 2 array pointers and a size variable being provided to the threads

## join pool, await finish

when running a compute load on multiple threads it is important to check if the compute is completed before you can do anything with the data.

for this you join the pool. as in you disable the current thread until all other threads are done exited.

``` C

int joinPool(Pool_t* pool);

```

this runs very simply, provide the correct pool and once the function ends you can asume that the pool is done processsing.

it is a good idea to check the return value as any non zero value indicates that something went wrong while looking into the threads and the output may no be correct
