#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <stdbool.h>

#include "private.h"

volatile int num_ticks = 0;

void tester(int signum) {
    if (signum == SIGVTALRM) {
        num_ticks++;
    }
    // num_ticks++;
}

int main(void) {
    signal(SIGVTALRM, tester);

    preempt_start(true);
    sleep(1);

    preempt_stop();
    int last_tick_count = num_ticks;
    sleep(1);

    printf("Ticks: %d\n", num_ticks);
    printf("Last tick count: %d\n", last_tick_count);

    if(num_ticks == last_tick_count) {
        printf("Success\n");
    } else {
        printf("Fail\n");
    }

    return 0;
}