// #include <stdio.h>
// #include <stdint.h>
// #include <stdlib.h>
// #include <string.h>
// #include <assert.h>

// #include "queue.h"

// void test_create(void)
// {
//     printf("Running test create\n");
//     queue_t q;

//     q = queue_create();
//     assert(q != NULL);
//     queue_destroy(q);

// }

// void test_queue_simple(void)
// {
//     queue_t q;
//     int data = 3, *ptr;

//     q = queue_create();
//     queue_enqueue(q, &data);
//     queue_dequeue(q, (void**)&ptr);
//     assert(ptr == &data);
//     queue_destroy(q);
// }

// int main(void) {
//     printf("Starting queue tests...\n\n");

//     test_create_destroy();
//     test_enqueue_dequeue_simple();

//     printf("\nAll specified queue tests completed.\n");
//     return 0;
// }




/* original code above */

// #include <stdio.h>     // For printf
// #include <stdint.h>
// #include <stdlib.h>
// #include <string.h>
// #include <assert.h>
// #include "queue.h"     // Your queue header file

// // Forward declarations for test functions
// void test_create(void);
// void test_enqueue_dequeue_simple(void);
// void test_delete_item(void);
// void test_iterate_items(void);
// void test_null_queue_operations(void);

// // Helper globals for iterator tests (if they are indeed global)
// // If they are only used within test_iterate_items, they can remain static inside or before it.
// // For simplicity and based on previous structure, let's assume they might be file-scoped static.
// static int iteration_sum;
// static int items_iterated_count;
// static int expected_values[5];

// // Callback function for queue_iterate tests - UPDATED SIGNATURE
// void iterator_test_callback(queue_t q, void *data) {
//     (void)q; // Mark as unused if not directly used

//     assert(data != NULL);
//     int value = *(int*)data;

//     assert(items_iterated_count < 5); // Ensure we don't go out of bounds
//     assert(value == expected_values[items_iterated_count]);

//     iteration_sum += value;
//     items_iterated_count++;
// }

// // Dummy callback for queue_iterate with NULL queue - UPDATED SIGNATURE
// // This can be defined within test_null_queue_operations if only used there,
// // or forward-declared if defined after main and needed by test_null_queue_operations.
// // For simplicity, let's assume test_null_queue_operations defines its own local dummy or uses a static one.
// // Or, if it's truly generic:
// // void dummy_callback(queue_t q_arg, void *data_param); // Forward declare if defined after main

// // Test function for creating and destroying a queue
// void test_create(void) {
//     printf("Running test_create...\n");
//     queue_t q;

//     q = queue_create();
//     assert(q != NULL);          // Queue was created
//     assert(queue_length(q) == 0); // Initial length should be 0

//     assert(queue_destroy(q) == 0); // Destroy should succeed
//     printf("test_create: PASSED\n");
// }

// // ... (the rest of your test function definitions: test_enqueue_dequeue_simple, test_delete_item, etc.) ...
// // ... Make sure test_iterate_items is defined before it is called, or its callback is defined/visible ...
// // ... Make sure test_null_queue_operations and its dummy_callback are handled ...

// // Then your main function:
// int main(void) {
//     printf("Starting queue tests...\n\n");

//     test_create();
//     test_enqueue_dequeue_simple();
//     test_delete_item();
//     test_iterate_items();
//     test_null_queue_operations();

//     printf("\nAll specified queue tests completed.\n");
//     return 0;
// }

// // If dummy_callback was forward declared and used by test_null_queue_operations, define it here:
// /*
// void dummy_callback(queue_t q_arg, void *data_param) {
//     (void)q_arg;
//     (void)data_param;
//     assert(0); // Should not be called
// }
// */











#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // For specific integer types if needed, though not directly here
#include <string.h> // For memset, if used
#include <assert.h>
#include "queue.h" // Make sure this path is correct for your project structure

// Forward declarations for test functions
void test_create_destroy(void);
void test_enqueue_dequeue_basic(void);
void test_enqueue_constraints(void);
void test_dequeue_constraints_and_state(void);
void test_length(void);
void test_delete(void);
void test_iteration(void);
void test_null_queue_operations(void);
void test_comprehensive_scenario(void);

// --- Main Test Runner ---
int main(void) {
    printf("Starting Queue Test Suite...\n");

    test_create_destroy();
    test_enqueue_dequeue_basic();
    test_enqueue_constraints();
    test_dequeue_constraints_and_state();
    test_length();
    test_delete();
    test_iteration();
    test_null_queue_operations();
    test_comprehensive_scenario();

    printf("\nAll queue tests completed successfully!\n");
    return 0;
}

// --- Test Function Definitions ---

void test_create_destroy(void) {
    printf("Running test_create_destroy...\n");
    queue_t q;

    // Test successful creation
    q = queue_create();
    // Note: Your queue_create has `malloc(sizeof(queue_t))`. If queue_t is `struct queue*`,
    // this should be `malloc(sizeof(struct queue))`. Assuming it's corrected or works for now.
    assert(q != NULL);
    assert(queue_length(q) == 0); // Initial length

    // Test successful destruction
    assert(queue_destroy(q) == 0);

    // Test destroying a NULL queue
    assert(queue_destroy(NULL) == -1);
    printf("test_create_destroy: PASSED\n");
}

void test_enqueue_dequeue_basic(void) {
    printf("Running test_enqueue_dequeue_basic...\n");
    queue_t q = queue_create();
    assert(q != NULL);

    int data1 = 10, data2 = 20, data3 = 30;
    int *ptr = NULL;

    // Enqueue one item
    assert(queue_enqueue(q, &data1) == 0);
    assert(queue_length(q) == 1);

    // Dequeue one item
    assert(queue_dequeue(q, (void**)&ptr) == 0);
    assert(ptr == &data1);
    assert(*ptr == 10);
    assert(queue_length(q) == 0);

    // Enqueue multiple items
    assert(queue_enqueue(q, &data1) == 0); // 10
    assert(queue_enqueue(q, &data2) == 0); // 10, 20
    assert(queue_enqueue(q, &data3) == 0); // 10, 20, 30
    assert(queue_length(q) == 3);

    // Dequeue multiple items in FIFO order
    assert(queue_dequeue(q, (void**)&ptr) == 0);
    assert(ptr == &data1 && *ptr == 10);
    assert(queue_length(q) == 2);

    assert(queue_dequeue(q, (void**)&ptr) == 0);
    assert(ptr == &data2 && *ptr == 20);
    assert(queue_length(q) == 1);

    assert(queue_dequeue(q, (void**)&ptr) == 0);
    assert(ptr == &data3 && *ptr == 30);
    assert(queue_length(q) == 0);

    assert(queue_destroy(q) == 0);
    printf("test_enqueue_dequeue_basic: PASSED\n");
}

void test_enqueue_constraints(void) {
    printf("Running test_enqueue_constraints...\n");
    queue_t q = queue_create();
    assert(q != NULL);
    int data = 5;

    // Test enqueuing NULL data (should fail based on your queue.c)
    assert(queue_enqueue(q, NULL) == -1);
    assert(queue_length(q) == 0);

    // Enqueue valid data afterwards
    assert(queue_enqueue(q, &data) == 0);
    assert(queue_length(q) == 1);

    assert(queue_destroy(q) == 0);
    printf("test_enqueue_constraints: PASSED\n");
}

void test_dequeue_constraints_and_state(void) {
    printf("Running test_dequeue_constraints_and_state...\n");
    queue_t q = queue_create();
    assert(q != NULL);
    int *ptr = NULL;
    int *initial_ptr_val = (int*)0xDEADBEEF; // A known non-NULL sentinel

    // Test dequeue from an empty queue
    ptr = initial_ptr_val; // Set to a known value
    assert(queue_dequeue(q, (void**)&ptr) == -1);
    // Your queue.c: if queue->length == 0, it returns -1 *without* modifying *ptr.
    assert(ptr == initial_ptr_val); // ptr should remain unchanged
    assert(queue_length(q) == 0);

    // Test dequeue with NULL as the data out-parameter
    int data1 = 10;
    assert(queue_enqueue(q, &data1) == 0);
    assert(queue_dequeue(q, NULL) == -1); // Should fail if `data` (the void**) is NULL
    assert(queue_length(q) == 1);         // Item should still be in queue

    assert(queue_destroy(q) == 0);
    printf("test_dequeue_constraints_and_state: PASSED\n");
}


void test_length(void) {
    printf("Running test_length...\n");
    queue_t q = queue_create();
    assert(q != NULL);
    int data = 1;

    assert(queue_length(q) == 0);
    queue_enqueue(q, &data);
    assert(queue_length(q) == 1);
    queue_enqueue(q, &data);
    assert(queue_length(q) == 2);

    int *ptr;
    queue_dequeue(q, (void**)&ptr);
    assert(queue_length(q) == 1);
    queue_dequeue(q, (void**)&ptr);
    assert(queue_length(q) == 0);

    // Test length of NULL queue
    assert(queue_length(NULL) == -1);

    assert(queue_destroy(q) == 0);
    printf("test_length: PASSED\n");
}

void test_delete(void) {
    printf("Running test_delete...\n");
    queue_t q = queue_create();
    assert(q != NULL);

    int d1=1, d2=2, d3=3, d4=4, d5=5;
    int *items[] = {&d1, &d2, &d3, &d4, &d5};
    int *ptr;

    // Delete from empty queue
    assert(queue_delete(q, items[0]) == -1);

    // Delete NULL data (should fail based on your queue.c)
    assert(queue_delete(q, NULL) == -1);

    // Enqueue: &d1, &d2, &d3, &d4, &d5
    for (int i = 0; i < 5; ++i) assert(queue_enqueue(q, items[i]) == 0);
    assert(queue_length(q) == 5);

    // Delete non-existent item
    int non_existent = 100;
    assert(queue_delete(q, &non_existent) == -1);
    assert(queue_length(q) == 5);

    // Delete head (&d1) -> queue: &d2, &d3, &d4, &d5
    assert(queue_delete(q, items[0]) == 0);
    assert(queue_length(q) == 4);
    assert(queue_dequeue(q, (void**)&ptr) == 0 && ptr == items[1]); // New head is &d2
    assert(queue_length(q) == 3); // Queue: &d3, &d4, &d5

    // Rebuild: &d1, &d2, &d3, &d4, &d5
    assert(queue_destroy(q) == 0);
    q = queue_create();
    for (int i = 0; i < 5; ++i) assert(queue_enqueue(q, items[i]) == 0);

    // Delete tail (&d5) -> queue: &d1, &d2, &d3, &d4
    assert(queue_delete(q, items[4]) == 0);
    assert(queue_length(q) == 4);
    // Verify by enqueuing new item and checking order
    int d6 = 6;
    assert(queue_enqueue(q, &d6) == 0); // Queue: &d1, &d2, &d3, &d4, &d6
    assert(queue_dequeue(q, (void**)&ptr) == 0 && ptr == items[0]);
    assert(queue_dequeue(q, (void**)&ptr) == 0 && ptr == items[1]);
    assert(queue_dequeue(q, (void**)&ptr) == 0 && ptr == items[2]);
    assert(queue_dequeue(q, (void**)&ptr) == 0 && ptr == items[3]); // Old tail's predecessor
    assert(queue_dequeue(q, (void**)&ptr) == 0 && ptr == &d6);    // New tail
    assert(queue_length(q) == 0);

    // Rebuild: &d1, &d2, &d3, &d4, &d5
    for (int i = 0; i < 5; ++i) assert(queue_enqueue(q, items[i]) == 0);

    // Delete middle (&d3) -> queue: &d1, &d2, &d4, &d5
    assert(queue_delete(q, items[2]) == 0);
    assert(queue_length(q) == 4);
    assert(queue_dequeue(q, (void**)&ptr) == 0 && ptr == items[0]);
    assert(queue_dequeue(q, (void**)&ptr) == 0 && ptr == items[1]);
    assert(queue_dequeue(q, (void**)&ptr) == 0 && ptr == items[3]);
    assert(queue_dequeue(q, (void**)&ptr) == 0 && ptr == items[4]);
    assert(queue_length(q) == 0);

    // Delete only item
    assert(queue_enqueue(q, items[0]) == 0);
    assert(queue_delete(q, items[0]) == 0);
    assert(queue_length(q) == 0);

    // Delete one of duplicate pointers
    // Queue: &d1, &d2, &d1, &d3
    assert(queue_enqueue(q, items[0]) == 0);
    assert(queue_enqueue(q, items[1]) == 0);
    assert(queue_enqueue(q, items[0]) == 0); // Enqueue &d1 again
    assert(queue_enqueue(q, items[2]) == 0);
    assert(queue_length(q) == 4);

    assert(queue_delete(q, items[0]) == 0); // Deletes first &d1
    assert(queue_length(q) == 3);           // Queue: &d2, &d1, &d3
    assert(queue_dequeue(q, (void**)&ptr) == 0 && ptr == items[1]); // &d2
    assert(queue_dequeue(q, (void**)&ptr) == 0 && ptr == items[0]); // second &d1
    assert(queue_dequeue(q, (void**)&ptr) == 0 && ptr == items[2]); // &d3

    assert(queue_destroy(q) == 0);
    printf("test_delete: PASSED\n");
}

// --- For test_iteration ---
static int iter_count;
static int iter_sum;
static int expected_iter_sequence[5]; // Max 5 items for this test
static queue_t queue_ref_in_callback;

void reset_iteration_globals(void) {
    iter_count = 0;
    iter_sum = 0;
    queue_ref_in_callback = NULL;
    for(int i=0; i<5; ++i) expected_iter_sequence[i] = 0;
}

// Callback for queue_iterate, matches void (*func)(queue_t, void*)
void iteration_callback(queue_t q, void *data) {
    assert(q != NULL); 
    assert(data != NULL);
    int val = *(int*)data;

    assert(q == queue_ref_in_callback); 

    if (iter_count < 5) { 
        assert(val == expected_iter_sequence[iter_count]);
    }
    iter_sum += val;
    iter_count++;
}


void test_iteration(void) {
    printf("Running test_iteration...\n");
    queue_t q = queue_create();
    assert(q != NULL);

    int d1=10, d2=20, d3=30;
    int *items[] = {&d1, &d2, &d3}; // items array has 3 elements

    // 1. Iterate on empty queue
    reset_iteration_globals(); // Reset before test
    queue_ref_in_callback = q; // Set expected queue ref (even if empty)
    assert(queue_iterate(q, iteration_callback) == 0);
    assert(iter_count == 0);
    assert(iter_sum == 0);

    // 2. Iterate on queue with one item
    assert(queue_enqueue(q, items[0]) == 0); // Enqueue &d1 (value 10)

    reset_iteration_globals();               // <<< CORRECT: Reset FIRST
    queue_ref_in_callback = q;
    expected_iter_sequence[0] = *items[0];   // <<< CORRECT: Then set expectations
                                             // iter_count is 0 from reset

    assert(queue_iterate(q, iteration_callback) == 0);
    assert(iter_count == 1);
    assert(iter_sum == *items[0]); // Sum should be 10

    // 3. Iterate on queue with multiple items
    // Queue 'q' currently contains: items[0] (10)
    assert(queue_enqueue(q, items[1]) == 0); // q: 10, 20
    assert(queue_enqueue(q, items[2]) == 0); // q: 10, 20, 30
    // Current items in queue (and their order for iteration): items[0], items[1], items[2]

    reset_iteration_globals();               // <<< CORRECT: Reset FIRST
    queue_ref_in_callback = q;
    expected_iter_sequence[0] = *items[0];   // <<< CORRECT: Then set expectations for 10
    expected_iter_sequence[1] = *items[1];   // for 20
    expected_iter_sequence[2] = *items[2];   // for 30
                                             // iter_count is 0 from reset

    assert(queue_iterate(q, iteration_callback) == 0);
    assert(iter_count == 3);
    assert(iter_sum == (*items[0] + *items[1] + *items[2])); // Sum should be 10+20+30=60

    assert(queue_destroy(q) == 0);
    printf("test_iteration: PASSED\n");
}

void test_null_queue_operations(void) {
    printf("Running test_null_queue_operations...\n");
    queue_t q_null = NULL;
    int data_val = 10;
    int *ptr = NULL;
    void *initial_ptr_val = (void*)0xDEADBEEF;

    assert(queue_destroy(q_null) == -1);
    assert(queue_enqueue(q_null, &data_val) == -1); // data_val is not NULL

    ptr = initial_ptr_val; // Reset for check
    assert(queue_dequeue(q_null, (void**)&ptr) == -1);
    assert(ptr == initial_ptr_val); // Should remain unchanged if q_null is NULL

    assert(queue_delete(q_null, &data_val) == -1); // data_val is not NULL
    assert(queue_iterate(q_null, iteration_callback) == -1); // Using previous callback
    assert(queue_length(q_null) == -1);

    // Test operations with NULL func pointer
    queue_t q_valid = queue_create();
    assert(q_valid != NULL);
    assert(queue_iterate(q_valid, NULL) == -1);
    assert(queue_destroy(q_valid) == 0);

    printf("test_null_queue_operations: PASSED\n");
}

void test_comprehensive_scenario(void) {
    printf("Running test_comprehensive_scenario...\n");
    queue_t q = queue_create();
    assert(q != NULL);

    int data_a=1, data_b=2, data_c=3, data_d=4, data_e=5;
    int *p;

    // Enqueue a few items: A, B, C
    assert(queue_enqueue(q, &data_a) == 0);
    assert(queue_enqueue(q, &data_b) == 0);
    assert(queue_enqueue(q, &data_c) == 0);
    assert(queue_length(q) == 3);

    // Dequeue one: A. Queue: B, C
    assert(queue_dequeue(q, (void**)&p) == 0 && p == &data_a);
    assert(queue_length(q) == 2);

    // Enqueue D: B, C, D
    assert(queue_enqueue(q, &data_d) == 0);
    assert(queue_length(q) == 3);

    // Delete B (head): C, D
    assert(queue_delete(q, &data_b) == 0);
    assert(queue_length(q) == 2);
    assert(queue_dequeue(q, (void**)&p) == 0 && p == &data_c); // C is new head
    assert(queue_enqueue(q, &data_c) == 0); // Put C back: D, C

    // Delete C (tail): D
    assert(queue_delete(q, &data_c) == 0);
    assert(queue_length(q) == 1);
    assert(queue_dequeue(q, (void**)&p) == 0 && p == &data_d); // D is only item
    assert(queue_length(q) == 0);

    // Enqueue: A, B, C, D, E
    assert(queue_enqueue(q, &data_a) == 0);
    assert(queue_enqueue(q, &data_b) == 0);
    assert(queue_enqueue(q, &data_c) == 0);
    assert(queue_enqueue(q, &data_d) == 0);
    assert(queue_enqueue(q, &data_e) == 0);
    assert(queue_length(q) == 5);

    // Delete C (middle): A, B, D, E
    assert(queue_delete(q, &data_c) == 0);
    assert(queue_length(q) == 4);

    // Iterate to check
    reset_iteration_globals();
    queue_ref_in_callback = q;
    expected_iter_sequence[0] = data_a;
    expected_iter_sequence[1] = data_b;
    expected_iter_sequence[2] = data_d;
    expected_iter_sequence[3] = data_e;
    assert(queue_iterate(q, iteration_callback) == 0);
    assert(iter_count == 4);
    assert(iter_sum == data_a + data_b + data_d + data_e);

    // Destroy non-empty queue
    assert(queue_destroy(q) == 0);

    printf("test_comprehensive_scenario: PASSED\n");
}