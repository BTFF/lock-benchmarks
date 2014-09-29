#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include "test.h"
#include "spin_lock.h"

static void* shared = NULL;

static int* value = NULL;

static volatile struct
{
	int lock;
	struct test* head;
	struct test* tail;
}* list;

static pthread_t* thread = NULL;

static struct test* test = NULL;

struct test* prepare(int n)
{
	int i;

	if(!(shared = mmap(NULL, sizeof(*value) + sizeof(*list) + sizeof(*test) * n, PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0)))
		return NULL;
	else
	{
		void* p = shared;
		value = p; p += sizeof(*value);
		list = p; p += sizeof(*list);
		test = p;
	}

	*value = 0;
	list->lock = 0;
	list->head = NULL;
	list->tail = NULL;

	if(!(thread = malloc(sizeof(*thread) * n)))
		return NULL;
	
	for(i = 0; i < n; i++)
	{
		if(-1 == pipe(test[i].pipe))
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

static unsigned char buffer[1];

#define pipe_read pipe[0]
#define pipe_write pipe[1]

void lock(int i)
{
	struct test* self = test + i;
	int wait = -1;

	self->next = NULL;
	spin_lock(&list->lock);
	if(!list->head)
	{
		list->head = self;
		list->tail = self;
	}
	else
	{
		list->tail->next = self;
		list->tail = self;
		wait = self->pipe_read;
	}
	spin_unlock(&list->lock);

	if(-1 != wait)
		read(wait, buffer, 1);
}

void unlock(int i)
{
	int wait = -1;

	spin_lock(&list->lock);
	if(list->head == list->tail)
	{
		list->head = NULL;
		list->tail = NULL;
	}
	else
	{
		struct test* head;
		head = list->head->next;
		list->head = head;
		wait = head->pipe_write;
	}
	spin_unlock(&list->lock);

	if(-1 != wait)
		write(wait, buffer, 1);
}

