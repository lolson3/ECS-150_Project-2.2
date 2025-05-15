#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "private.h"
#include "sem.h"

struct semaphore {
	int internal_count;
	queue_t blocked_threads;
};

/* Creates new semaphore and initializes internal values */
sem_t sem_create(size_t count) {
	// Allocate memory for sem struct
	sem_t sem = malloc(sizeof(*sem));
	if (sem == NULL) {
		return NULL; // If memory allocation fails, return NULL
	}
	sem->internal_count = count; // Set internal sem count to count
	sem->blocked_threads = queue_create(); // Creates queue for blocked threads
	
	if (sem->blocked_threads == NULL) { // if blocked_threads fail to create
		free(sem);
		return NULL;
	}

	return sem;
}

/* Destroys a semaphore if its queue is empty or it is NULL */
int sem_destroy(sem_t sem) {
	if (sem == NULL || queue_length(sem->blocked_threads) > 0) {
		return -1; // Failed to destroy semaphore because it is not empty
	}

	// Free memory allocated for the threads
	queue_destroy(sem->blocked_threads);
	sem->internal_count = 0;

	// Free memory allocated for the semaphore
	free(sem);
	return 0;
}

int sem_down(sem_t sem) {
	struct uthread_tcb *curr = uthread_current();

    if (sem == NULL) {
		return -1;
	}

	// This while loop handles the corner case correctly:
	// If a thread wakes up but the resource is no longer available,
	// it will re-check the condition and block again
	while (sem->internal_count == 0) {
		queue_enqueue(sem->blocked_threads, curr);
		uthread_block();
		uthread_yield();  // CRITICAL FIX: Actually yield control after blocking
	}
	sem->internal_count--;
    return 0;
}

/* Releases a semaphore; unblocks next thread in blocked queue, increases internal count */
int sem_up(sem_t sem) {
	if (sem == NULL) {
		return -1;
	}

	sem->internal_count++; // Always increment the count

	struct uthread_tcb *next_thread_tcb;
	if (queue_dequeue(sem->blocked_threads, (void**)&next_thread_tcb) == 0) {
		uthread_unblock(next_thread_tcb);
		uthread_yield(); // Optional: fairness
	}

	return 0;
}