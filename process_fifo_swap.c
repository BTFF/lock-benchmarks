#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include "test.h"

static void* shared = NULL;

static int* value = NULL;

static volatile struct fifo
{
	volatile int in;
	volatile int out;
}* fifo = NULL;

static pid_t* process = NULL;

struct test* prepare(int n)
{
	struct test* test = NULL;
	int i;

	if(!(shared = mmap(NULL, sizeof(*value) + sizeof(*fifo) + sizeof(*test) * n, PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0)))
		return NULL;
	else
	{
		void* p = shared;
		value = p; p += sizeof(*value);
		fifo = p; p += sizeof(*fifo);
		test = p;
	}

	*value = 0;
	fifo->in = -1;
	fifo->out = 0;

	if(!(process = malloc(sizeof(*process) * n)))
		return NULL;
	
	for(i = 0; i < n; i++)
	{
		process[i] = -1;
		test[i].id = i;
		test[i].value = value;
	}
	return test;
}

void startroutine(int id, void* (*routine)(void*), void* arg)
{
	process[id] = fork();
	if(0 == process[id]) /* child */
	{
		routine(arg);
		exit(0);
	}
	else
	if(0 > process[id])
		printf("startroutine failed\n");
}

void waitroutine(int id)
{
	waitpid(process[id], NULL, 0);
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
	if(!__sync_bool_compare_and_swap(&fifo->out, fifo->out, fifo->out + 1))
		printf("somthing's wrong.\n");
}

