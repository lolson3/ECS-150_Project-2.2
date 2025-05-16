#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "private.h"
#include "sem.h"

// Initializing semaphore struct
struct semaphore {
	int internal_count;
	queue_t blocked_threads;
};

/* Creates new semaphore and initializes internal values */
sem_t sem_create(size_t count) {
	// Allocate memory for sem struct
	sem_t sem = malloc(sizeof(*sem));
	// Check to make sure sem is not NULL
	if (sem == NULL) {
		return NULL; // If memory allocation fails, return NULL
	}
	sem->internal_count = count; // Set internal sem count to count
	sem->blocked_threads = queue_create(); // Creates queue for blocked threads
	
	// Check to see if blocked_threads queue failed to allocate space
	if (sem->blocked_threads == NULL) {
		free(sem); // Frees semaphore if failure is detected
		return NULL;
	}
	// Returns created semaphore
	return sem;
}

/* Destroys a semaphore if its queue is empty or it is NULL */
int sem_destroy(sem_t sem) {
	// Check if sem is NULL or queue is not empty
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

	// Check to make sure sem is not NULL
    if (sem == NULL) {
		return -1;
	}

	// Disable preemption while we change sem counts and queues
	preempt_disable();

	// Wait for resources to become available
	while (sem->internal_count == 0) {
		// If thread tries to use sem_down when no resources are available, block it and yield
		queue_enqueue(sem->blocked_threads, curr);
		uthread_block();

		// Critical section complete, enable preemption
		preempt_enable();

		uthread_yield();
	}

	// Disable preemption while we change sem counts and queues
	preempt_disable();
	
	// Decrement internal count of resources when done waiting
	sem->internal_count--;
	
	// Critical section complete, enable preemption
	preempt_enable();

    return 0;
}

/* Releases a semaphore; unblocks next thread in blocked queue, increases internal count */
int sem_up(sem_t sem) {
	// Check to make sure sem is not NULL
	if (sem == NULL) {
		return -1;
	}

	// Disable preemption while we change sem counts and queues
	preempt_disable();

	// Increment internal count of sem when resources are available
	sem->internal_count++;
	
	// Dequeues next blocked thread when resources become available
	struct uthread_tcb *next_thread_tcb;
	if (queue_dequeue(sem->blocked_threads, (void**)&next_thread_tcb) == 0) {
		uthread_unblock(next_thread_tcb);
		uthread_yield(); // Yielding for fairness
	}

	// Critical section complete, enable preemption
	preempt_enable();

	return 0;
}