#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100
#define USEC 1000000 // Number of microseconds in a second

sigset_t block_alarm;
static struct sigaction old_sa;

/* Simple helper function that yields when our alarm signal reaches it */
static void signal_handler(int signum) {
	if (signum == SIGVTALRM) {
        uthread_yield();
    }
}

/* Disables preemption temporarily */
void preempt_disable(void) {
	// Uses SIG_BLOCK to disable the alarm with a mask
	sigprocmask(SIG_BLOCK, &block_alarm, NULL);
}

/* Enables preemption temporarily */
void preempt_enable(void) {
	// Uses SIG_UNBLOCK to enable the alarm again with a mask
	sigprocmask(SIG_UNBLOCK, &block_alarm, NULL);
}

/* Starts thread preemption and initializes timer and signal variables. If preempt is false, does nothing. */
void preempt_start(bool preempt) {
	if (preempt) {
		// Initialize the sigaction struct that will send the SIGVTALRM signal and trigger signal_handler
		struct sigaction sa;
		sa.sa_handler = signal_handler;
		sigemptyset(&sa.sa_mask); // Prevents other signals from being blocked
		sa.sa_flags = 0; 
		sigaction(SIGVTALRM, &sa, &old_sa);
		
		// Initialize the signal set for alarm blocking/unblocking
		sigemptyset(&block_alarm);
		sigaddset(&block_alarm, SIGVTALRM);
		
		// Initialize timer
		struct itimerval timer = {0};
		timer.it_interval.tv_usec = USEC/HZ; // firing 100 times per second
		timer.it_value.tv_usec = USEC/HZ; // previous
		setitimer(ITIMER_VIRTUAL, &timer, NULL); // (int which, const struct *value, struct itimerval *ovalue); // no old value
	} else {
		// Do nothing if preempt is false
		return;
	}
}

/* Stops thread preemption and restores previous timer config and signal*/
void preempt_stop(void) {
	// Resets sigaction
	sigaction(SIGVTALRM, &old_sa, NULL);

	// Resets timer behavior
	struct itimerval timer = {0};
	timer.it_interval.tv_usec = 0; // never firing
	timer.it_value.tv_usec = 0; // previous
	setitimer(ITIMER_VIRTUAL, &timer, NULL);
}