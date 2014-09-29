#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "test.h"

static int value[] = { 0 };

static volatile struct fifo
{
	volatile int in;
	volatile int out;
} fifo[1] = { { -1, 0 } };

static pthread_t* thread = NULL;

struct test* prepare(int n)
{
	struct test* test = NULL;
	int i;

	if(!(test = malloc(sizeof(*test) * n)))
		return NULL;
	if(!(thread = malloc(sizeof(*thread) * n)))
	{
		free(thread);
		return NULL;
	}
	
	for(i = 0; i < n; i++)
	{
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

void lock(int i)
{
    int ticket;
    do
    {
        ticket = fifo->in + 1;
    }while(!__sync_bool_compare_and_swap(&fifo->in, ticket - 1, ticket));
    while(fifo->out != ticket);
}

void unlock(int i)
{
	fifo->out++;
}

