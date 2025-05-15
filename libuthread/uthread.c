// #include <assert.h>
// #include <signal.h>
// #include <stddef.h>
// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/time.h>

// #include "private.h"
// #include "uthread.h"
// #include "queue.h"

// // Thread state variable
// enum thread_state {
// 	READY,
// 	RUNNING,
// 	BLOCKED,
// 	ZOMBIE
// };

// static queue_t ready_queue;

// /* Struct that should hold context of a thread, info about its stack, info about its state. */
// struct uthread_tcb {
// 	void *stack;
// 	uthread_ctx_t context;
// 	enum thread_state state;
// };

// static struct uthread_tcb *current_thread = NULL;

// struct uthread_tcb *uthread_current(void)
// {
// 	return current_thread;
// }

// void uthread_yield(void)
// {
// 	// preempt_disable();

// 	// Gets currently running thread and adds it back to ready queue if it is running
// 	struct uthread_tcb *curr = uthread_current();
// 	if (curr->state == RUNNING) {
// 		curr->state = READY;
// 		queue_enqueue(ready_queue, curr);
// 	}

// 	struct uthread_tcb *next;
// 	if (queue_dequeue(ready_queue, (void**)&next) < 0) {
// 		exit(0); // If no more threads are left
// 	}
// 	next->state = RUNNING;
// 	uthread_ctx_switch(&curr->context, &next->context);
// }

// void uthread_exit(void)
// {
// 	current_thread->state = ZOMBIE;

// 	if (current_thread->stack != NULL) {
// 		uthread_ctx_destroy_stack(current_thread->stack);
// 		free(current_thread);
// 	}

// 	struct uthread_tcb *next_thread;
// 	if (queue_dequeue(ready_queue, (void**)&next_thread) < 0) {
// 		exit(0);
// 	}

// 	next_thread->state = RUNNING;
// 	current_thread = next_thread;

// 	setcontext(&current_thread->context);
// 	assert(0);
// }

// // void uthread_exit(void)
// // {
// // 	struct uthread_tcb *curr = uthread_current();
// // 	curr->state = ZOMBIE;

// // 	struct uthread_tcb *next_thread;
// // 	if (queue_dequeue(ready_queue, (void**)&next_thread) < 0) {
// // 		exit(0);
// // 	}

// // 	next_thread->state = RUNNING;
// // 	current_thread = next_thread;

	
// // 	if (current_thread->stack != NULL) {
// // 		uthread_ctx_switch(&curr->context, &next_thread->context);
// // 		uthread_ctx_destroy_stack(curr->stack);
// // 		free(current_thread);
// // 	}
// // }

// /* Creates a thread with a function for the thread to run (and args) */
// int uthread_create(uthread_func_t func, void *arg)
// {
// 	// Allocates memory for thread control block
// 	struct uthread_tcb *tcb = malloc(sizeof(*tcb));
// 	if (tcb == NULL) {
// 		return -1;
// 	}
	
// 	// Allocates memory for thread stack
// 	tcb->stack = uthread_ctx_alloc_stack();
// 	if (tcb->stack == NULL) {
// 		free(tcb); // If stack allocation fails, free memory allocated to TCB
// 		return -1;
// 	}

// 	// Takes args (uthread_ctx_t *uctx, void *top_of_stack, uthread_func_t func, void *arg)
// 	uthread_ctx_init(&tcb->context, tcb->stack, func, arg);
// 	tcb->state = READY;
	
// 	// Stops if ready queue somehow wasn't initialized
// 	if (ready_queue == NULL) {
// 		return -1;
// 	}

// 	// Adds thread to ready queue
// 	queue_enqueue(ready_queue, tcb);
	
// 	return 0;
// }

// /* Creates first user thread */
// int uthread_run(bool preempt, uthread_func_t func, void *arg)
// {
// 	// if the thread is already running
// 	if (ready_queue != NULL) {
// 		return -1;
// 	}

// 	ready_queue = queue_create();
// 	if (!ready_queue) {
// 		return -1; // if queue_create() fails
// 	}

// 	struct uthread_tcb main_thread;
// 	main_thread.stack = NULL;
// 	main_thread.state = RUNNING;
// 	getcontext(&main_thread.context); // saving main context

// 	current_thread = &main_thread;

// 	if (uthread_create(func, arg) < 0) {
// 		return -1;
// 	}

// 	if (preempt) {
// 		//preempt_start(true);
// 	}

// 	while (queue_length(ready_queue) > 0) {
// 		uthread_yield();
// 	}

// 	if (preempt) {
// 		//preempt_stop();
// 	}

// 	ready_queue = NULL;
// 	current_thread = NULL;

// 	return 0;
// }

// void uthread_block(void)
// {
// 	struct uthread_tcb *curr = uthread_current();
// 	curr->state = BLOCKED;
// 	uthread_yield();
// }

// void uthread_unblock(struct uthread_tcb *uthread)
// {
// 	uthread->state = READY;
// 	queue_enqueue(ready_queue, uthread);
// }




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

struct uthread_tcb *uthread_current(void)
{
	return current_thread;
}

void uthread_yield(void)
{
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

void uthread_exit(void)
{
    struct uthread_tcb *exiting_thread = current_thread;
    exiting_thread->state = ZOMBIE;
    
    // Add to zombie queue for later cleanup
    if (zombie_queue) {
        queue_enqueue(zombie_queue, exiting_thread);
    }
    
    // Find next thread to run
    struct uthread_tcb *next_thread;
    if (queue_dequeue(ready_queue, (void**)&next_thread) < 0) {
        // No more threads - clean up everything and exit
        if (exiting_thread->stack != NULL) {
            uthread_ctx_destroy_stack(exiting_thread->stack);
        }
        free(exiting_thread);
        
        // Clean up any remaining zombies
        struct uthread_tcb *zombie;
        while (zombie_queue && queue_dequeue(zombie_queue, (void**)&zombie) == 0) {
            if (zombie->stack != NULL) {
                uthread_ctx_destroy_stack(zombie->stack);
            }
            free(zombie);
        }
        
        exit(0);
    }
    
    // Switch to next thread
    next_thread->state = RUNNING;
    current_thread = next_thread;
    
    // This doesn't return - we're switching away from the exiting thread
    setcontext(&next_thread->context);
    assert(0);  // Should never reach here
}

/* Creates a thread with a function for the thread to run (and args) */
int uthread_create(uthread_func_t func, void *arg)
{
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

/* Creates first user thread */
int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
    if (ready_queue != NULL) {
        return -1;
    }
    
    ready_queue = queue_create();
    zombie_queue = queue_create();  // Create zombie queue
    if (!ready_queue || !zombie_queue) {
        return -1;
    }
    
    // Allocate main thread dynamically
    struct uthread_tcb *main_thread = malloc(sizeof(struct uthread_tcb));
    if (!main_thread) {
        return -1;
    }
    
    main_thread->stack = NULL;  // Main thread uses system stack
    main_thread->state = RUNNING;
    getcontext(&main_thread->context);
    
    current_thread = main_thread;
    
    if (uthread_create(func, arg) < 0) {
        free(main_thread);
        return -1;
    }
    
    if (preempt) {
        //preempt_start(true);
    }
    
    while (queue_length(ready_queue) > 0) {
        uthread_yield();
    }
    
    if (preempt) {
        //preempt_stop();
    }
    
    // Clean up zombies
    struct uthread_tcb *zombie;
    while (queue_dequeue(zombie_queue, (void**)&zombie) == 0) {
        if (zombie->stack != NULL) {
            uthread_ctx_destroy_stack(zombie->stack);
        }
        free(zombie);
    }
    
    // Clean up main thread
    free(main_thread);
    
    queue_destroy(ready_queue);
    queue_destroy(zombie_queue);
    ready_queue = NULL;
    zombie_queue = NULL;
    current_thread = NULL;
    
    return 0;
}

void uthread_block(void)
{
	struct uthread_tcb *curr = uthread_current();
	curr->state = BLOCKED;
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	uthread->state = READY;
	queue_enqueue(ready_queue, uthread);
}

