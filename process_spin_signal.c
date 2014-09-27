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

static volatile int* spin = NULL;

static volatile pid_t* owner = NULL;

static volatile struct list** head = NULL;

struct list
{
	pid_t pid;
	struct list* next;
};

static volatile struct list* signal_list = NULL;

static sigset_t set;

static pid_t* process = NULL;

struct test* prepare(int n)
{
	struct test* test = NULL;
	void* p;
	int i;

	if(!(p = mmap(NULL, sizeof(*value) + sizeof(*spin) + sizeof(*owner) + sizeof(*head) + sizeof(*signal_list) * n, PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0)))
		return NULL;
	shared = p;
	value = p; p += sizeof(*value);
	spin = p; p += sizeof(*spin);
	owner = p; p += sizeof(*owner);
	head = p; p += sizeof(*head);
	signal_list = p;

	*value = 0;
	*spin = 0;
	*owner = -1;
	*head = NULL;

	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);

	if(!(test = malloc(sizeof(*test) * n)))
		return NULL;

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
    volatile int* p = spin;
	for(;;)
	{
		while(!__sync_bool_compare_and_swap(p, 0, 1));
			if(-1 == *owner)
				*owner = process[i];
			else
			{
				signal_list[i].pid = process[i];
				signal_list[i].next = *head;
				*head = signal_list + i;
			}
		asm volatile ("");
		*p = 0;

		if(*owner != process[i])
			sigsuspend(&set);
		else
			break;
	}
}

void unlock(int i)
{
	struct list* list;
    volatile int* p = spin;

    while(!__sync_bool_compare_and_swap(p, 0, 1));
		*owner = -1;
		if(list = *head)
			*head = list->next;
    asm volatile ("");
    *p = 0;

	if(list)
		kill(list->pid, SIGUSR1);
}

