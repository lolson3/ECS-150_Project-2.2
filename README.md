# ECS-150_Project-2.2
Similar to existing lightweight user-level thread libraries, your library must be able to:
- Create new threads
- Schedule the execution of threads in a round-robin fashion
- Be preemptive, that is to provide an interrupt-based scheduler
- Provide a thread synchronization API, namely semaphores

## Files to Complete
└── libuthread
    ├── Makefile*
    ├── preempt.c*
    ├── queue.c*
    ├── sem.c*
    └── uthread.c*

## Testing: Overview
- Phase 1: Queuing implementation, run queue_tester_example.c, create queue_tester.c
- Phase 2: Run the testers prefixed with uthread_
- Phase 3: Run the testers prefixed with sem_

## Phase 1: Overview
- Implement a simple FIFO queue using the interface defined in libuthread/queue.h by
    completing the code in libuthread/queue.c
- Complete the libuthread/Makefile so that it generates a static library archive named
    libuthread/libuthread.a
- At first only libuthread/queue.c should be included in your library

### Phase 1: Testing
- Create queue_tester.c in apps 
- Reference example from project_2.2.html

## Phase 2: Overview
- Implement most thread management
    - Interface defined in libuthread/uthread.h
    - Code defined in libuthread/uthread.c
- The first function an application has to call in order to kick off the uthread library and create the first user thread is uthread_run(). This function performs three actions:
    1) It registers the so-far single execution flow of the application as the idle thread that the library can later schedule for execution like any other threads.
    2) It creates a new thread, the initial thread, as specified by the arguments of the function.
    3) The function finally execute an infinite loop which,
        1. When there are no more threads which are ready to run in the system, stops the idle loop and returns.
        2. Or simply yields to next available thread.
        3. (It could also deal with threads that reached completion and destroys their associated TCB.)
- Nonpreemptive: Threads must call the function uthread_yield() in order to ask the library’s scheduler to pick and run the next available thread
- You will need a data structure that can store information about a single thread.

### Phase 2: Testing
- Two programs can help test this phase:
    - uthread_hello: creates a single thread that displays “Hello world!”
    - uthread_yield: creates three threads in cascade and test the yield feature of the scheduler

## Phase 3: Overview
- Implement Semaphore API
    - Interface defined in libuthread/sem.h
    - Code defined in libuthread/sem.c
- Semaphores are a way to control the access to common resources by multiple threads.

### Phase 3: Testing
- A few testing programs are available in order to test your semaphore implementation:
    - sem_simple: simple example of scheduling forced by semaphores
    - sem_count: alternate counting with two threads and two semaphores
    - sem_buffer: producer/consumer exchanging data in a buffer
    - sem_prime: prime sieve implemented with a growing pipeline of threads
- There is a specific corner case that must be considered for proper implementation 
    - Thread A calls down() on a semaphore with a count of 0, and gets blocked.
    - Thread B calls up() on the same semaphore, and gets thread A to be awaken
    - Before thread A can run again, thread C calls down() on the semaphore and snatch - the newly available resource.

- There are two difficulties with this scenario:
    - The semaphore should make sure that thread A will handle the situation correctly when finally resuming its execution. Theoretically, it should go back to sleep if the resource is not longer available by the time it gets to run. If the thread proceeds anyway, then the semaphore implementation is incorrect.
    - If this keeps happening, thread A will eventually be starving (i.e., it never gets access to the resource that it needs to proceed). Ideally, the semaphore implementation should prevent starvation from happening.

## Phase 4: Overview
- Implement preemption
    - Interface defined in libuthread/private.h
    - Code defined in libuthread/preempt.c
- Preemption API should stay completely transparent to user threads
- The function preempt_start() should be called when the uthread library is initializing in order to set up preemption, if required by the user.
- The setup is a two-step procedure:
    - Install a signal handler that receives alarm signals (of type SIGVTALRM)
    - Configure a timer which will fire an alarm (through a SIGVTALRM signal) a hundred times per second (i.e. 100 Hz)
- Your signal handler, which acts as the timer interrupt handler, will force the currently running thread to yield, so that another thread can be scheduled instead.
- If preemption is enabled, preempt_stop() should be called before uthread_run(), once the
multithreading phase of the application ends
- It should restore the previous signal action, and restore the previous timer configuration.
- The two other functions that you must implement are meant to enable or disable preemption. For that, you will need to be able to block or unblock signals of type SIGVTALRM.
- Preemption should be TEMPORARILY DISABLED when manipulating shared data structures so that they are guaranteed to be performed atomically

### Phase 4: Testing
- Add a new test program in the apps directory called test_preempt.c
- Does not have to be complicated

## Submission
- Source code of library in libuthread/
- Source code of testers in apps/
- libuthread/Makefile
    - Run with -Wall -Wextra -Werror
    - Use a clean rule