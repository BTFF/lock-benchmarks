#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "test.h"
#include "../peterson.h"

static int value[] = { 0 };

static void* flag_turn = NULL;
static struct peterson peterson; 

static pthread_t* thread = NULL;

struct test* prepare(int n)
{
	struct test* test = NULL;
	int i;

	if(!(flag_turn = malloc(peterson_flag_turn_size(n))))
		return NULL;
	peterson_initial(&peterson, n, flag_turn);

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
	peterson_lock(&peterson, i);
}

void unlock(int i)
{
	peterson_unlock(&peterson, i);
}

