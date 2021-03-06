#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "test.h"

static int value[] = { 0 };

static int spin = 0;

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
	int volatile* p = &spin;
	while(!__sync_bool_compare_and_swap(p, 0, 1))
	{
		//while(*p)
			//_mm_pause();
			//__sync_synchronize();
	}
}

void unlock(int i)
{
	int volatile* p = &spin;
	asm volatile ("");
	*p = 0;
}

