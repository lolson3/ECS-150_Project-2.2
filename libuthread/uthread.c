#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

// Thread state variable
enum thread_state {
	READY,
	RUNNING,
	BLOCKED,
	ZOMBIE
};

// Initializing ready_queue and zombie_queue
static queue_t ready_queue;
static queue_t zombie_queue;

/* Struct that should hold context of a thread, info about its stack, info about its state. */
struct uthread_tcb {
	void *stack;
	uthread_ctx_t context;
	enum thread_state state;
};

static struct uthread_tcb *current_thread = NULL;
static struct uthread_tcb *main_thread = NULL;

// Simple struct that returns pointer to current_thread
struct uthread_tcb *uthread_current(void) {
	return current_thread;
}

/* Yields to the next thread marked as READY */
void uthread_yield(void) {
    struct uthread_tcb *curr = current_thread;
    
    // Only re-queue if thread is RUNNING (not BLOCKED or ZOMBIE)
    if (curr->state == RUNNING) {
        curr->state = READY;
        queue_enqueue(ready_queue, curr);
    }
    
    // Initializes next thread, dequeues from the ready queue
    struct uthread_tcb *next;
    if (queue_dequeue(ready_queue, (void**)&next) < 0) {
        // If next thread is not READY, exit
        if (curr->state == ZOMBIE || curr->state == BLOCKED) {
            exit(0);  // Nothing else to run
        }
        return; // Continue running current thread
    }
    
    // Same thread, no need to switch
    if (next == curr) {
        return; 
    }
    
    // Changes state of next thread to running and moves it to current thread
    next->state = RUNNING;
    current_thread = next;
    uthread_ctx_switch(&curr->context, &next->context);
}

/* Exits from current thread and changes its state to ZOMBIE */
void uthread_exit(void) {
    // Gets current_thread and tracks it as exiting_thread, changes its state to ZOMBIE
    struct uthread_tcb *exiting_thread = current_thread;
    exiting_thread->state = ZOMBIE;

    // Checks if zombie_queue is available and adds exiting_thread to it if its stack is not empty
    if (zombie_queue && exiting_thread->stack != NULL) {
        queue_enqueue(zombie_queue, exiting_thread);
    }

    // Initializes variable for next_thread to switch to
    struct uthread_tcb *next_thread;
    if (queue_dequeue(ready_queue, (void**)&next_thread) < 0) {
        exit(0);
    }

    // Sets state of next_thread to RUNNING and sets it to current_thread
    next_thread->state = RUNNING;
    current_thread = next_thread;

    // Sets context of current thread to context of the thread it is switching to
    setcontext(&next_thread->context);
    assert(0);
}

/* Creates a thread with a function for the thread to run (and args) */
int uthread_create(uthread_func_t func, void *arg) {
	// Allocates memory for thread control block
	struct uthread_tcb *tcb = malloc(sizeof(*tcb));
	if (tcb == NULL) {
		return -1;
	}
	
	// Allocates memory for thread stack
	tcb->stack = uthread_ctx_alloc_stack();
	if (tcb->stack == NULL) {
		free(tcb); // If stack allocation fails, free memory allocated to TCB
		return -1;
	}

	// Takes args (uthread_ctx_t *uctx, void *top_of_stack, uthread_func_t func, void *arg)
	uthread_ctx_init(&tcb->context, tcb->stack, func, arg);
	tcb->state = READY;
	
	// Stops if ready queue somehow wasn't initialized
	if (ready_queue == NULL) {
		return -1;
	}

	// Adds thread to ready queue
	queue_enqueue(ready_queue, tcb);
	
	return 0;
}

// /* Creates first user thread */
int uthread_run(bool preempt, uthread_func_t func, void *arg) {
    if (preempt) {
        preempt_start(true);
    }

    // Creates ready and zombie queues
    ready_queue  = queue_create();
    zombie_queue = queue_create();

    // Checks to see if either queue failed to create, returns -1 if so
    if (!ready_queue || !zombie_queue) {
        return -1;
    }

    // Allocates memory for current_thread
    current_thread = malloc(sizeof(struct uthread_tcb));
    
    // Checks to see if current thread failed to create
    if (current_thread == NULL) {
        return -1;
    }

    // Initializes context of current_thread and gives it to main_thread
    current_thread->stack = NULL;
    getcontext(&current_thread->context);
    main_thread = current_thread;

    // Creates first user thread and checks for failure
    if (uthread_create(func, arg) < 0) {
        return -1;
    }
    
    // Running until there are no more ready threads
    while (queue_length(ready_queue) > 0) {
        uthread_yield();
    }

    // Iterate through zombie queue and free it and its stack
    struct uthread_tcb *zombie;
    while (queue_dequeue(zombie_queue, (void**)&zombie) == 0) {
        // Avoids freeing main_thread if it has been added to zombie queue
        if (zombie != main_thread) {
            uthread_ctx_destroy_stack(zombie->stack);
            free(zombie);
        }
    }

    // Frees the main_thread and current_thread when all other threads are finished
    free(main_thread);
    main_thread = NULL;
    current_thread = NULL;

    // Destroys both ready and zombie queues when they are empty
    queue_destroy(ready_queue);
    queue_destroy(zombie_queue);

    // Stops preemption when all threads are done
    if (preempt) {
        preempt_stop();
    }

    return 0;
}

/* Sets current thread's state to BLOCKED */
void uthread_block(void) {
	struct uthread_tcb *curr = uthread_current();
	curr->state = BLOCKED;
}

/* Sets current thread's state to READY */
void uthread_unblock(struct uthread_tcb *uthread) {
	uthread->state = READY;
	queue_enqueue(ready_queue, uthread);
}