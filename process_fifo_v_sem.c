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

static int* count = NULL;

static struct 
{
	struct test head;
	struct test* tail;
}* list = NULL;

static struct test* test = NULL;

static int semid = -1;

static pid_t* process = NULL;

struct test* prepare(int n)
{
	int i;

	if(-1 == (semid = semget(IPC_PRIVATE, n, 0600|IPC_CREAT)))
        return NULL;

	if(!(shared = mmap(NULL, sizeof(*value) + sizeof(*count) + sizeof(*list) + sizeof(*test) * n, PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0)))
		return NULL;
	else
	{
		void* p = shared;
		value = p; p += sizeof(*value);
		count = p; p += sizeof(*count);
		list = p; p += sizeof(*list);
		test = p;
	}

	*value = 0;
	*count = -1;
	list->head.next = NULL;
	list->tail = &list->head;

	if(!(process = malloc(sizeof(*process) * n)))
		return NULL;
	
	for(i = 0; i < n; i++)
	{
		union semun arg;
		arg.val = 0;
		if(-1 == semctl(semid, i, SETVAL, arg))
			return NULL;
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
		ticket = *count + 1;
	}while(!__sync_bool_compare_and_swap(count, ticket - 1, ticket));

	if(0 < ticket)
	{
		struct sembuf sops = {i, -1, 0};
		struct test* tail;
		test[i].next = NULL;
		do
		{
			tail = list->tail;
		}while(!__sync_bool_compare_and_swap(&tail->next, NULL, test + i));
		while(!__sync_bool_compare_and_swap(&list->tail, tail, test + i));
		semop(semid, &sops, 1);
	}
}

void unlock(int i)
{
	int ticket;
	do
	{
		ticket = *count - 1;
	}while(!__sync_bool_compare_and_swap(count, ticket + 1, ticket));

	if(0 <= ticket)
	{
		struct sembuf sops = {0, 1, 0};
		struct test* head;
		while(!(head = list->head.next));
		do
		{
			head = list->head.next;
		}while(!__sync_bool_compare_and_swap(&list->head.next, head, head->next));
		sops.sem_num = head->id;
		semop(semid, &sops, 1);
	}
}

