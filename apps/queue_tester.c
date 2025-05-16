#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

#define TEST_ASSERT(assert)				\
do {									\
	printf("ASSERT: " #assert " ... ");	\
	if (assert) {						\
		printf("PASS\n");				\
	} else	{							\
		printf("FAIL\n");				\
		exit(1);						\
	}									\
} while(0)

/* Test queue creation */
void test_create(void)
{
	fprintf(stderr, "*** TEST create ***\n");

	TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
	int data = 3, *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST queue_simple ***\n");

	q = queue_create();
    TEST_ASSERT(queue_enqueue(q, NULL) == -1);
	TEST_ASSERT(queue_enqueue(q, &data) == 0);
    TEST_ASSERT(queue_dequeue(q, NULL) == -1);
	TEST_ASSERT(queue_dequeue(q, (void**)&ptr) == 0);
	TEST_ASSERT(ptr == &data);
}

/* Callback function that increments items */
static void iterator_inc(queue_t q, void *data)
{
    int *a = (int*)data;

    if (*a == 42)
        queue_delete(q, data);
    else
        *a += 1;
}

void test_queue_iterator(void)
{
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    size_t i;

    fprintf(stderr, "*** TEST queue_iterator ***\n");

    /* Initialize the queue and enqueue items */
    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    /* Increment every item of the queue, delete item '42' */
    queue_iterate(q, iterator_inc);
    TEST_ASSERT(data[0] == 2);
    TEST_ASSERT(data[1] == 3);
    TEST_ASSERT(data[9] == 10);
    TEST_ASSERT(queue_length(q) == 9);
}

void test_queue_destroy(void) {
	queue_t q;
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    size_t i;

	fprintf(stderr, "*** TEST queue_destroy ***\n");

    // Test destroying basic empty queue
	q = queue_create();
	TEST_ASSERT(queue_destroy(q) == 0);

    // Test destroying with NULL arg
	TEST_ASSERT(queue_destroy(NULL) == -1);

    // Test destroying a full queue
    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++) {
        queue_enqueue(q, &data[i]);
    }
    TEST_ASSERT(queue_destroy(q) == -1);
}

void test_queue_delete(void)
{
	int dataset[] = {1, 2, 3, 4, 5};
	int *ptr;
	queue_t q;
	
	fprintf(stderr, "*** TEST queue_delete ***\n");
	
	q = queue_create();
	
	for (int i = 0; i < 5; i++) {
		queue_enqueue(q, &dataset[i]);
	}
	
	// Delete middle item
	TEST_ASSERT(queue_delete(q, &dataset[2]) == 0);
	TEST_ASSERT(queue_length(q) == 4);
	
	// Delete first item
	TEST_ASSERT(queue_delete(q, &dataset[0]) == 0);
	TEST_ASSERT(queue_length(q) == 3);
	
	// Delete last item
	TEST_ASSERT(queue_delete(q, &dataset[4]) == 0);
	TEST_ASSERT(queue_length(q) == 2);
	
	// Delete already deleted item
	TEST_ASSERT(queue_delete(q, &dataset[0]) == -1);
	
	// Delete NULL parameters
	TEST_ASSERT(queue_delete(NULL, &dataset[1]) == -1);
	TEST_ASSERT(queue_delete(q, NULL) == -1);
	
	// Dequeue remaining items
	queue_dequeue(q, (void**)&ptr);
	queue_dequeue(q, (void**)&ptr);
}

int main(void) {
    test_create();
    test_queue_iterator();
    test_queue_simple();
	test_queue_destroy();
	test_queue_delete();
    return 0;
}