/**
 * ============================================================================
 * file.h - Map File Persistence Interface (Save & Load)
 * SRM IST Campus Navigation & Route Planner
 * ============================================================================
 */

#ifndef FILE_H
#define FILE_H

#include "common.h"

/**
 * Load campus graph nodes and roads from text file.
 * Also synchronizes the Hash Table and Trie if pointers are non-NULL.
 * Returns 1 on success, 0 on failure.
 */
int loadMap(Graph *g, HashTable *ht, TrieNode *trie, const char *filename);

/**
 * Save campus graph nodes and roads to text file in standard format.
 * Returns 1 on success, 0 on failure.
 */
int saveMap(const Graph *g, const char *filename);

#endif /* FILE_H */
