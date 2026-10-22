/**
 * ============================================================================
 * heap.c - Array-Based Min-Heap Priority Queue Implementation
 * SRM IST Campus Navigation & Route Planner
 * ============================================================================
 */

#include "heap.h"

/**
 * Purpose:        Swap two heap nodes.
 * Parameters:     a, b - pointers to HeapNode elements
 * Return:         void
 * Time Complexity: O(1)
 */
static void swapNodes(HeapNode *a, HeapNode *b) {
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}

/**
 * Purpose:        Allocate a dynamic array-based min-heap.
 * Parameters:     capacity - initial number of elements to support
 * Return:         Pointer to MinHeap, or NULL on allocation error
 * Time Complexity: O(1)
 */
MinHeap* createMinHeap(int capacity) {
    if (capacity <= 0) capacity = 64;

    MinHeap *h = (MinHeap *)malloc(sizeof(MinHeap));
    if (!h) {
        fprintf(stderr, "Error: Memory allocation failed for MinHeap struct.\n");
        return NULL;
    }

    h->array = (HeapNode *)malloc(capacity * sizeof(HeapNode));
    if (!h->array) {
        fprintf(stderr, "Error: Memory allocation failed for MinHeap array.\n");
        free(h);
        return NULL;
    }

    h->size = 0;
    h->capacity = capacity;
    return h;
}

/**
 * Purpose:        Restore min-heap invariant upwards from idx to root.
 * Parameters:     h   - pointer to MinHeap
 *                 idx - index of newly inserted element
 * Return:         void
 * Time Complexity: O(log N)
 */
static void heapifyUp(MinHeap *h, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (h->array[idx].dist < h->array[parent].dist) {
            swapNodes(&h->array[idx], &h->array[parent]);
            idx = parent;
        } else {
            break;
        }
    }
}

/**
 * Purpose:        Restore min-heap invariant downwards from idx to leaves.
 * Parameters:     h   - pointer to MinHeap
 *                 idx - index to bubble down
 * Return:         void
 * Time Complexity: O(log N)
 */
static void heapifyDown(MinHeap *h, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < h->size && h->array[left].dist < h->array[smallest].dist) {
        smallest = left;
    }
    if (right < h->size && h->array[right].dist < h->array[smallest].dist) {
        smallest = right;
    }

    if (smallest != idx) {
        swapNodes(&h->array[idx], &h->array[smallest]);
        heapifyDown(h, smallest);
    }
}

/**
 * Purpose:        Push a node and tentative distance into the min-heap.
 * Parameters:     h    - pointer to MinHeap
 *                 node - graph node ID
 *                 dist - current shortest tentative distance
 * Return:         void
 * Time Complexity: O(log N)
 */
void pushHeap(MinHeap *h, int node, int dist) {
    if (!h) return;

    /* Dynamic array resizing if capacity exceeded */
    if (h->size >= h->capacity) {
        int newCap = h->capacity * 2;
        HeapNode *newArr = (HeapNode *)realloc(h->array, newCap * sizeof(HeapNode));
        if (!newArr) {
            fprintf(stderr, "Error: MinHeap reallocation failed.\n");
            return;
        }
        h->array = newArr;
        h->capacity = newCap;
    }

    /* Insert at end and bubble up */
    h->array[h->size].node = node;
    h->array[h->size].dist = dist;
    h->size++;
    heapifyUp(h, h->size - 1);
}

/**
 * Purpose:        Extract and remove the minimum element (root) from the heap.
 * Parameters:     h - pointer to MinHeap
 * Return:         HeapNode with lowest distance (node = -1 if empty)
 * Time Complexity: O(log N)
 */
HeapNode popHeap(MinHeap *h) {
    HeapNode emptyNode = {-1, INF};
    if (!h || h->size <= 0) {
        return emptyNode;
    }

    HeapNode minNode = h->array[0];
    h->array[0] = h->array[h->size - 1];
    h->size--;

    if (h->size > 0) {
        heapifyDown(h, 0);
    }

    return minNode;
}

/**
 * Purpose:        Check if min-heap has zero elements.
 * Parameters:     h - pointer to MinHeap
 * Return:         1 if empty, 0 otherwise
 * Time Complexity: O(1)
 */
int isHeapEmpty(const MinHeap *h) {
    return (!h || h->size == 0);
}

/**
 * Purpose:        Free all allocated memory for the min-heap.
 * Parameters:     h - pointer to MinHeap
 * Return:         void
 * Time Complexity: O(1)
 */
void freeHeap(MinHeap *h) {
    if (!h) return;
    if (h->array) {
        free(h->array);
        h->array = NULL;
    }
    free(h);
}
