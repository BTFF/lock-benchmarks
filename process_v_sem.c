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

static void* shared = NULL;

static int* value = NULL;

static int semid = -1;

static pid_t* process = NULL;

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

	if(!(shared = mmap(NULL, sizeof(*value) + sizeof(*test) * n, PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0)))
		return NULL;
	else
	{
		void* p = shared;
		value = p; p += sizeof(*value);
		test = p;
	}

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
	struct sembuf sops = {0, -1, 0};
	semop(semid, &sops, 1);
}

void unlock(int i)
{
	struct sembuf sops = {0, 1, 0};
	semop(semid, &sops, 1);
}

