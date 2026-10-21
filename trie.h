/**
 * ============================================================================
 * trie.h - Trie Prefix Tree Interface for Campus Autocomplete
 * SRM IST Campus Navigation & Route Planner
 * ============================================================================
 */

#ifndef TRIE_H
#define TRIE_H

#include "common.h"

/**
 * Allocate and initialize a new Trie node with 128 ASCII children.
 * Returns pointer to TrieNode or NULL on memory failure.
 */
TrieNode* createTrieNode(void);

/**
 * Insert a location name and its node ID into the Trie.
 * Stores names case-insensitively (converted to lowercase).
 */
void insertTrie(TrieNode *root, const char *name, int nodeId);

/**
 * Search all place names that start with a given prefix.
 * Fills matchedIds array with up to maxMatches node IDs.
 * Returns the count of matching locations found.
 */
int searchPrefix(const TrieNode *root, const char *prefix, int *matchedIds, int maxMatches);

/**
 * Unmark/remove a place name from the Trie.
 * Returns 1 if place existed and was unmarked, 0 otherwise.
 */
int removeFromTrie(TrieNode *root, const char *name);

/**
 * Free all dynamically allocated nodes in the Trie.
 */
void freeTrie(TrieNode *root);

/**
 * Enhancement: Print autocomplete suggestions formatted for the CLI.
 */
void printAutocompleteSuggestions(const TrieNode *root, const Graph *g, const char *prefix);

#endif /* TRIE_H */
