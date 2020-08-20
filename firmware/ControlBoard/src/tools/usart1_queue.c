//  ***************************************************************************
/// @file    usart1_queue.c
/// @author  NeoProg
//  ***************************************************************************
#include "usart1_queue.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define NODES_COUNT             (10)


typedef struct {
	uint8_t data;   // Node data
	void*   next;   // Pointer to next node
} node_t;

typedef struct {
	node_t   nodes[NODES_COUNT];    // Node list
	node_t*  head;                  // Pointer to head of queue
	node_t*  tail;                  // Pointer to tail of queue
} queue_t;


static queue_t queue = {0};


//  ***************************************************************************
/// @brief  Queue initialization
/// @param  none
/// @return none
//  ***************************************************************************
void usart1_queue_init(void) {

	// Make nodes loop: [0]->[1]->[...]->[N]->[0]
	for (uint32_t i = 0; i < NODES_COUNT - 1; ++i) {
		queue.nodes[i].data = 0;
		queue.nodes[i].next = &queue.nodes[i + 1];
	}
	queue.nodes[NODES_COUNT - 1].data = 0;
	queue.nodes[NODES_COUNT - 1].next = &queue.nodes[0];

	// Initialization head and tail
	queue.head = NULL;
	queue.tail = NULL;
}

//  ***************************************************************************
/// @brief  Enqueue item to queue
/// @param  data: data for enqueue
/// @return true - enqueue success, false - queue is full
//  ***************************************************************************
bool usart1_queue_enqueue(uint8_t data) {

	if (queue.tail != NULL && queue.tail->next == queue.head) {
		return false; // Queue is overflow
	}

	if (queue.head == NULL) { // Insert first item as head of queue
		queue.head = &queue.nodes[0];
		queue.head->data = data;
	}
	else if (queue.tail == NULL) { // Insert second as tail of queue
		queue.tail = (node_t*)queue.head->next;
		queue.tail->data = data;
	}
	else {
		queue.tail = (node_t*)queue.tail->next;
		queue.tail->data = data;
	}
	return true;
}

//  ***************************************************************************
/// @brief  Dequeue item from queue
/// @param  data: buffer for data
/// @return true - dequeue success, false - queue is empty
//  ***************************************************************************
bool usart1_queue_dequeue(uint8_t* data) {

	if (queue.head == NULL) {
		return false; // Queue is empty
	}

	// Read data from queue and clear this node
	(*data) = queue.head->data;
	queue.head->data = 0;
	queue.head = (node_t*)queue.head->next;

	if (queue.head == queue.tail) {
		queue.tail = NULL; // Remaining last item as head - remove tail
	}
	else if (queue.tail == NULL) {
		queue.head = NULL; // We read last item - remove head
	}
	return true;
}

//  ***************************************************************************
/// @brief  Check queue empty
/// @param  none
/// @return true - queue is empty, false - otherwise
//  ***************************************************************************
bool usart1_queue_is_empty(void) {
	return queue.head == NULL;
}

//  ***************************************************************************
/// @brief  Clear queue
/// @param  none
/// @return none
//  ***************************************************************************
void usart1_queue_clear(void) {
	usart1_queue_init();
}
