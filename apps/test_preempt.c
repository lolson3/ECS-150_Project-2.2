/* 
Simple test for preemption
Spawns 3 threads which print the respective tick that 
they are on at the time up to a very large maximum value
to see if the threads are interleaving as the process runs
*/

#include <stdio.h>
#include <stdlib.h>

#include "private.h"

#define MAXCOUNT 500000

volatile int num_ticks = 0;

void t3_count(void *arg)
{
	(void)arg;
    while (num_ticks < MAXCOUNT) {
        printf("thread3: %d\n", num_ticks++);
    }
}

void t2_count(void *arg)
{
	(void)arg;
	uthread_create(t3_count, NULL);
    while (num_ticks < MAXCOUNT) {
	    printf("thread2: %d\n", num_ticks++);
    }
}

void t1_count(void *arg)
{
	(void)arg;
	uthread_create(t2_count, NULL);
    while (num_ticks < MAXCOUNT) {
        printf("thread1: %d\n", num_ticks++);
    }
}

int main(void) {
    uthread_run(true, t1_count, NULL);
    return 0;
}