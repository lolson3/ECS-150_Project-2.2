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


struct uthread_tcb *uthread_current(void) {
	return current_thread;
}

void uthread_yield(void) {
    struct uthread_tcb *curr = current_thread;
    
    // Only re-queue if thread is RUNNING (not BLOCKED or ZOMBIE)
    if (curr->state == RUNNING) {
        curr->state = READY;
        queue_enqueue(ready_queue, curr);
    }
    
    struct uthread_tcb *next;
    if (queue_dequeue(ready_queue, (void**)&next) < 0) {
        // No ready threads
        if (curr->state == ZOMBIE || curr->state == BLOCKED) {
            exit(0);  // Nothing else to run
        }
        return;  // Continue running current thread
    }
    
    if (next == curr) {
        return;  // Same thread, no need to switch
    }
    
    next->state = RUNNING;
    current_thread = next;
    uthread_ctx_switch(&curr->context, &next->context);
}

void uthread_exit(void) {
    struct uthread_tcb *exiting_thread = current_thread;
    exiting_thread->state = ZOMBIE;

    if (zombie_queue && exiting_thread->stack != NULL) {
        queue_enqueue(zombie_queue, exiting_thread);
    }

    struct uthread_tcb *next_thread;
    if (queue_dequeue(ready_queue, (void**)&next_thread) < 0) {
        exit(0);
    }

    next_thread->state = RUNNING;
    current_thread = next_thread;

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
        preempt_start(true)
    }

    // 1) Initialize queues
    ready_queue  = queue_create();
    zombie_queue = queue_create();
    if (!ready_queue || !zombie_queue) return -1;

    // 2) Create and track the main thread (uses system stack)
    current_thread = malloc(sizeof(struct uthread_tcb));
    if (!current_thread) return -1;
    current_thread->stack = NULL;
    getcontext(&current_thread->context);
    main_thread = current_thread;

    // 3) Create the first user thread; uthread_create enqueues it on ready_queue
    if (uthread_create(func, arg) < 0) return -1;

    // 4) Run until no more ready threads
    while (queue_length(ready_queue) > 0) {
        uthread_yield();
    }

    // 5) Clean up all zombie threads except the main thread
    struct uthread_tcb *zombie;
    while (queue_dequeue(zombie_queue, (void**)&zombie) == 0) {
        if (zombie != main_thread) {
            uthread_ctx_destroy_stack(zombie->stack);
            free(zombie);
        }
    }

    // 6) Finally, free the main thread TCB (its stack is system-managed)
    free(main_thread);
    main_thread   = NULL;
    current_thread = NULL;

    // 7) Destroy the queues (they should now be empty)
    queue_destroy(ready_queue);
    queue_destroy(zombie_queue);

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

