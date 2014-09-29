#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "test.h"

static void* shared = NULL;

static int* value = NULL;

static int* fifo_in = NULL;
static int* fifo_out = NULL;
static int* fifo = NULL;
static int fifo_size = 0;

static struct test* test = NULL;

static pthread_t* thread = NULL;

struct test* prepare(int n)
{
	int i;

	if(!(shared = mmap(NULL, sizeof(*value) + sizeof(*fifo_in) + sizeof(*fifo_out) + sizeof(*fifo) * (fifo_size = n + 2) + sizeof(*test) * n, PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0)))
		return NULL;
	else
	{
		void* p = shared;
		value = p; p += sizeof(*value);
		fifo_in = p; p += sizeof(*fifo_in);
		fifo_out = p; p += sizeof(*fifo_out);
		fifo = p; p += sizeof(*fifo) * fifo_size;
		test = p;
	}

	*value = 0;
	*fifo_in = 0;
	*fifo_out = 1;
	for(i = 0; i < fifo_size; i++)
		fifo[i] = -1;

	if(!(thread = malloc(sizeof(*thread) * n)))
		return NULL;
	
	for(i = 0; i < n; i++)
	{
		if(pipe(test[i].pipe))
			return NULL;
		test[i].id = i;
		test[i].value = value;
	}
	return test;
}

void startroutine(int id, void* (*routine)(void*), void* arg)
{
	pthread_create(thread + id, NULL, routine, arg);
}

void waitroutine(int id)
{
	pthread_join(thread[id], NULL);
}

static unsigned char buffer[1] = { '\0' };
#define pipe_read pipe[0]
#define pipe_write pipe[1]

void lock(int i) /* multiple threads */
{
	volatile int* in = fifo_in;
	volatile int* out = fifo_out;
	int ticket;
	int old;

	do
	{
		old = *in;
		if(fifo_size <= (ticket = old + 1))
			ticket = 0;
	}while(!__sync_bool_compare_and_swap(in, old, ticket));

	if(ticket != *out) /* wait */
	{
		fifo[ticket] = test[i].pipe_write;
		read(test[i].pipe_read, buffer, 1);
		fifo[ticket] = -1; /* only one thread */
	}
	/* ticket == *out */
	/* only one thread */
}

void unlock(int i) /* only one thread */
{
	volatile int* in = fifo_in;
	volatile int* out = fifo_out;
	int ticket;
	int old;

	old = *out;
	if(fifo_size <= (ticket = old + 1))
		ticket = 0;

	if(old != *in)
	{
		while(-1 == fifo[ticket])
			__sync_synchronize();
		*out = ticket;
		write(fifo[ticket], buffer, 1);
	}
	else
		*out = ticket;
}

