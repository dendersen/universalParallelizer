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

## destroy Pool

this will both wait for the pool to finish and then free all memory used by the pool
generally this should be the last thing you do with a pool

``` C

int* destroyPool(Pool_t* pool);

```

it will return a pointer to an array of RAW return values from the threads, these are specific to the operating system

# Uthread system

aside from adding the pool this library also adds a custom universal thread system

this is a very basic thread system that is designed to linearly map to the underlying operating system threading system to provide a universal interface

it is not designed to be a full threading library, but rather a simple way to create and manage threads in a universal way

## create thread

to create a thread simply use this macro

``` C

#define createUthread(dst,func,arg)

```

this will create a thread that runs the function "func" with the argument "arg"
the thread pointer/handle will be stored in "dst"

## join thread

to join a thread simply use this macro

``` C

#define joinUthread(thread)

```

this will block the current thread until the provided thread is done executing

## close thread

to end the execution of a thread and alow the freeing of resources use this macro

``` C

#define closeUthread(thread) CloseHandle(thread)

```

this will end the thread and free all resources the thread used

this will not free the memory used by the thread handle/pointer, or the data provided to the thread