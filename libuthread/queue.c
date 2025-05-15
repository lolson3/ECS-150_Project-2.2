#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

typedef struct node {
	void *data;
	struct node *next;
} node_t;

struct queue {
	node_t *head;
	node_t *tail;
	int length;
};

queue_t queue_create(void)
{
	// Allocating space for the queue structure.
	queue_t q = malloc(sizeof(*q));
	if (q == NULL) {
		return NULL;
	}
	// Initialize queue values
	q->head = NULL;
	q->tail = NULL;
	q->length = 0;

	return q;
}

// int queue_destroy(queue_t queue)
// {
// 	// If queue is NULL/empty, return.
// 	if (queue == NULL || queue->length > 0) {
// 		return -1;
// 	}
// 	// Free memory for queue itself.
// 	free(queue);
// 	return 0;
// }

int queue_destroy(queue_t queue)
{
	if (queue == NULL) {
		return -1;
	}

	// Free all nodes, but do NOT free the actual data pointers (owned by user)
	node_t *curr = queue->head;
	while (curr != NULL) {
		node_t *temp = curr;
		curr = curr->next;
		free(temp);
	}

	free(queue);
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
	// If queue or data are NULL/empty, return.
	if (queue == NULL || data == NULL) {
		return -1;
	}

	// Allocate memory for the new node. If it is empty, return.
	node_t *new_node = malloc(sizeof(node_t));
	if (new_node == NULL) {
		return -1;
	}
	
	// Fills new node with data
	new_node->data = data;
	new_node->next = NULL;

	if (queue->tail == NULL) {
		queue->head = new_node;
		queue->tail = new_node;
	} else {
		queue->tail->next = new_node;
		queue->tail = new_node;
	}

	queue->length++;
	return 0;
}

/* Dequeues item at beginning of list (FIFO) */
int queue_dequeue(queue_t queue, void **data)
{
	// If queue or data are NULL/empty, return. If queue empty, return.
	if (queue == NULL || data == NULL || queue->length == 0) {
		return -1;
	}

	// If queue does not have a head, return.
	if (queue->head == NULL) {
		*data = NULL;
		return -1;
	}

	node_t *old_head = queue->head;
	*data = old_head->data;

	// Sets head as next in queue.
	queue->head = old_head->next;
	if (queue->head == NULL) {
		queue->tail = NULL;
	}
	
	// Clears data from dequeued head from memory.
	free(old_head);
	queue->length--;

	return 0;
}

/* Finds a node with specificed data in the queue list and deletes it */
int queue_delete(queue_t queue, void *data)
{
	// If either queue or data are null/empty, return.
	if (queue == NULL || data == NULL) {
		return -1;
	}
	
	node_t *curr = queue->head;
	node_t *prev = NULL;

	// Iterate through list while nodes remain.
	while (curr != NULL) {
		if (curr->data == data) {
			// If head is a match, simply set next node as head and free its memory.
			if (prev == NULL) {
				queue->head = curr->next;
			} else {
				prev->next = curr->next;
			}
			// If tail is a match, set the previous node as tail and free its memory.
			if (curr == queue->tail) {
				queue->tail = prev;
			}
			// Frees memory for current (deleted) node and decrements length.
			free(curr);
			queue->length--;
			return 0;
		}
		// Continues through list.
		prev = curr;
		curr = curr->next;
	}
	return -1;
}

/* Provides a generic way to call a custom function on each item currently enqueued in the queue */
int queue_iterate(queue_t queue, queue_func_t func)
{
	if (queue == NULL || func == NULL) {
		return -1;
	}
	// Iterate through list and apply function to each node.
	node_t *curr = queue->head;
	while (curr != NULL) {
		node_t *next = curr->next;
		func(queue, curr->data);
		curr = next;
	}
	return 0;
}

/* If queue is not null, returns length of queue */
int queue_length(queue_t queue)
{
	if (queue == NULL) {
		return -1;
	}
	return queue->length;
}