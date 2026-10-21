/**
 * ============================================================================
 * hash.c - Hash Table Implementation with djb2 Algorithm & Chaining
 * SRM IST Campus Navigation & Route Planner
 * ============================================================================
 */

#include "hash.h"

/**
 * Purpose:        Case-insensitive string comparison helper (pure C11).
 * Parameters:     s1, s2 - strings to compare
 * Return:         0 if identical (case-insensitive), non-zero otherwise
 * Time Complexity: O(min(len1, len2))
 */
static int caseInsensitiveCompare(const char *s1, const char *s2) {
    if (!s1 || !s2) return -1;
    while (*s1 && *s2) {
        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

/**
 * Purpose:        djb2 hash algorithm (Dan Bernstein) with case-folding.
 * Parameters:     str - location name string
 * Return:         Bucket index in range [0, HASH_BUCKETS - 1]
 * Time Complexity: O(L) where L is length of name
 */
unsigned long hashFunc(const char *str) {
    if (!str) return 0;
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + tolower((unsigned char)c); /* hash * 33 + c */
    }

    return hash % HASH_BUCKETS;
}

/**
 * Purpose:        Initialize all hash table buckets to NULL.
 * Parameters:     ht - pointer to HashTable
 * Return:         void
 * Time Complexity: O(HASH_BUCKETS)
 */
void initHashTable(HashTable *ht) {
    if (!ht) return;
    for (int i = 0; i < HASH_BUCKETS; i++) {
        ht->buckets[i] = NULL;
    }
}

/**
 * Purpose:        Insert or update a location name -> ID mapping.
 * Parameters:     ht   - pointer to HashTable
 *                 name - location name string
 *                 id   - corresponding graph node ID
 * Return:         1 on success, 0 on failure
 * Time Complexity: Average O(1), Worst O(Chain Length)
 */
int insertName(HashTable *ht, const char *name, int id) {
    if (!ht || !name || name[0] == '\0') return 0;

    unsigned long index = hashFunc(name);

    /* Check if already present in this bucket's chain (update id) */
    HEntry *curr = ht->buckets[index];
    while (curr) {
        if (caseInsensitiveCompare(curr->name, name) == 0) {
            curr->id = id;
            return 1;
        }
        curr = curr->next;
    }

    /* Allocate new entry node */
    HEntry *entry = (HEntry *)malloc(sizeof(HEntry));
    if (!entry) {
        fprintf(stderr, "Error: Memory allocation failed for HashTable entry.\n");
        return 0;
    }

    strncpy(entry->name, name, NAME_LEN - 1);
    entry->name[NAME_LEN - 1] = '\0';
    entry->id = id;

    /* Prepend to bucket list (Separate Chaining) */
    entry->next = ht->buckets[index];
    ht->buckets[index] = entry;

    return 1;
}

/**
 * Purpose:        Lookup location node ID by name (case-insensitive).
 * Parameters:     ht   - pointer to HashTable
 *                 name - location name to search
 * Return:         Node ID (>= 0) if found, or -1 if not found
 * Time Complexity: Average O(1), Worst O(Chain Length)
 */
int getId(const HashTable *ht, const char *name) {
    if (!ht || !name || name[0] == '\0') return -1;

    unsigned long index = hashFunc(name);
    HEntry *curr = ht->buckets[index];

    while (curr) {
        if (caseInsensitiveCompare(curr->name, name) == 0) {
            return curr->id;
        }
        curr = curr->next;
    }

    return -1; /* Not found */
}

/**
 * Purpose:        Remove a name mapping from the hash table.
 * Parameters:     ht   - pointer to HashTable
 *                 name - location name to delete
 * Return:         1 if found & deleted, 0 if not found
 * Time Complexity: Average O(1), Worst O(Chain Length)
 */
int removeName(HashTable *ht, const char *name) {
    if (!ht || !name || name[0] == '\0') return 0;

    unsigned long index = hashFunc(name);
    HEntry *curr = ht->buckets[index];
    HEntry *prev = NULL;

    while (curr) {
        if (caseInsensitiveCompare(curr->name, name) == 0) {
            if (prev == NULL) {
                ht->buckets[index] = curr->next;
            } else {
                prev->next = curr->next;
            }
            free(curr);
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }

    return 0; /* Not found */
}

/**
 * Purpose:        Free all allocated entries in the hash table.
 * Parameters:     ht - pointer to HashTable
 * Return:         void
 * Time Complexity: O(HASH_BUCKETS + N) where N is total items
 */
void freeHashTable(HashTable *ht) {
    if (!ht) return;

    for (int i = 0; i < HASH_BUCKETS; i++) {
        HEntry *curr = ht->buckets[i];
        while (curr) {
            HEntry *next = curr->next;
            free(curr);
            curr = next;
        }
        ht->buckets[i] = NULL;
    }
}

/**
 * Purpose:        Calculate collision metrics for viva examination defense.
 * Parameters:     ht            - pointer to HashTable
 *                 outCollisions - receives count of collisions (bucket items > 1)
 *                 outMaxChain   - receives length of the longest bucket chain
 * Return:         void
 * Time Complexity: O(HASH_BUCKETS + N)
 */
void getHashStats(const HashTable *ht, int *outCollisions, int *outMaxChain) {
    if (!ht) return;
    int collisions = 0;
    int maxChain = 0;

    for (int i = 0; i < HASH_BUCKETS; i++) {
        int chainLen = 0;
        HEntry *curr = ht->buckets[i];
        while (curr) {
            chainLen++;
            curr = curr->next;
        }
        if (chainLen > 1) {
            collisions += (chainLen - 1);
        }
        if (chainLen > maxChain) {
            maxChain = chainLen;
        }
    }

    if (outCollisions) *outCollisions = collisions;
    if (outMaxChain) *outMaxChain = maxChain;
}
