/**
 * ============================================================================
 * heap.h - Array-Based Min-Heap Priority Queue Interface
 * SRM IST Campus Navigation & Route Planner
 * ============================================================================
 */

#ifndef HEAP_H
#define HEAP_H

#include "common.h"

/**
 * Allocate and initialize a min-heap with given capacity.
 * Returns pointer to MinHeap, or NULL on memory allocation failure.
 */
MinHeap* createMinHeap(int capacity);

/**
 * Push a node and its tentative distance into the min-heap.
 * Automatically doubles capacity if heap is full.
 */
void pushHeap(MinHeap *h, int node, int dist);

/**
 * Extract and return the minimum element (node with smallest distance).
 * Precondition: Heap must not be empty.
 */
HeapNode popHeap(MinHeap *h);

/**
 * Check if the min-heap contains no active elements.
 * Returns 1 if empty, 0 otherwise.
 */
int isHeapEmpty(const MinHeap *h);

/**
 * Free the heap buffer and the heap structure itself.
 */
void freeHeap(MinHeap *h);

#endif /* HEAP_H */
