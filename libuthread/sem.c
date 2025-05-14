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
sem_t sem_create(size_t count)
{
	// Allocate memory for sem struct
	sem_t sem = malloc(sizeof(*sem));
	if (sem == NULL) {
		return NULL; // If memory allocation fails, return NULL
	}
	sem->internal_count = count; // Set internal sem count to count
	sem->blocked_threads = queue_create(); // Creates queue for blocked threads
	
	if (sem->blocked_threads == NULL) { // if blocked_threads fail to create
		sem_destroy(sem);
		return NULL;
	}

	return sem;
}

/* Destroys a semaphore if its queue is empty or it is NULL */
int sem_destroy(sem_t sem)
{
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

/* Takes a semaphore; adds current thread to blocked queue, decreases internal count */
int sem_down(sem_t sem)
{
	if (sem == NULL) {
		return -1;
	}

	// Waits for available resources
	while (sem->internal_count == 0) {
		if (queue_enqueue(sem->blocked_threads, uthread_current()) == -1) {
			return -1;
		}
		uthread_block();
	}
	sem->internal_count--;

	return 0;
}

/* Releases a semaphore; unblocks next thread in blocked queue, increases internal count */
int sem_up(sem_t sem)
{
	if (sem == NULL) {
		return -1;
	}

	struct uthread_tcb *next_thread_tcb;

	if (queue_dequeue(sem->blocked_threads, (void**)&next_thread_tcb) == 0) {
		uthread_unblock(next_thread_tcb);
	} else {
		sem->internal_count++;
	}

	return 0;
}

/* There is a very specific corner case that you need to consider in order to implement your semaphore correctly. Here is the scenario:

    Thread A calls down() on a semaphore with a count of 0, and gets blocked.
    Thread B calls up() on the same semaphore, and gets thread A to be awaken
    Before thread A can run again, thread C calls down() on the semaphore and snatch the newly available resource.

There are two difficulties with this scenario:

    The semaphore should make sure that thread A will handle the situation correctly when finally resuming its execution. Theoretically, it should go back to sleep if the resource is not longer available by the time it gets to run. If the thread proceeds anyway, then the semaphore implementation is incorrect.
    If this keeps happening, thread A will eventually be starving (i.e., it never gets access to the resource that it needs to proceed). Ideally, the semaphore implementation should prevent starvation from happening.
*/