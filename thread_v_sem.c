#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "test.h"

/* System V semaphore */

static int value[] = { 0 };

static int semid = -1;

static pthread_t* thread = NULL;

struct test* prepare(int n)
{
	struct test* test = NULL;
	int i;
	union semun arg;

	if(-1 == (semid = semget(IPC_PRIVATE, 1, 0600|IPC_CREAT)))
		return NULL;
	arg.val = 1;
	if(-1 == semctl(semid, 0, SETVAL, arg))
		return NULL;

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
	struct sembuf sops = {0, -1, 0};
	semop(semid, &sops, 1);
}

void unlock(int i)
{
	struct sembuf sops = {0, 1, 0};
	semop(semid, &sops, 1);
}

