/**
 * ============================================================================
 * hash.h - Hash Table with djb2 Algorithm & Separate Chaining
 * SRM IST Campus Navigation & Route Planner
 * ============================================================================
 */

#ifndef HASH_H
#define HASH_H

#include "common.h"

/**
 * djb2 hash function by Dan Bernstein: hash = hash * 33 + c
 * Operates on lowercase string for case-insensitivity.
 * Returns bucket index modulo HASH_BUCKETS.
 */
unsigned long hashFunc(const char *str);

/**
 * Initialize all hash buckets to NULL.
 */
void initHashTable(HashTable *ht);

/**
 * Insert or update a location name -> ID mapping.
 * Returns 1 on success, 0 on memory allocation failure.
 */
int insertName(HashTable *ht, const char *name, int id);

/**
 * Lookup location ID by name (case-insensitive).
 * Returns node ID (>= 0) if found, or -1 if not found.
 */
int getId(const HashTable *ht, const char *name);

/**
 * Remove a place name mapping from the hash table.
 * Returns 1 if removed, 0 if not found.
 */
int removeName(HashTable *ht, const char *name);

/**
 * Free all allocated hash entries in all buckets.
 */
void freeHashTable(HashTable *ht);

/**
 * Alias for freeHashTable to match syllabus naming.
 */
static inline void freeTable(HashTable *ht) {
    freeHashTable(ht);
}

/**
 * Enhancement: Computes collision statistics and max chain length for viva defense.
 */
void getHashStats(const HashTable *ht, int *outCollisions, int *outMaxChain);

#endif /* HASH_H */
