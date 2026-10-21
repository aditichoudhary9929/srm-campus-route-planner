/**
 * ============================================================================
 * trie.c - Enhanced Multi-Token Prefix Tree Implementation
 * SRM IST Delhi-NCR Campus Navigation & Route Planner
 * ============================================================================
 */

#include "trie.h"

/**
 * Allocate and initialize a single Trie node.
 */
TrieNode* createTrieNode(void) {
    TrieNode *node = (TrieNode *)malloc(sizeof(TrieNode));
    if (!node) {
        fprintf(stderr, "Error: Memory allocation failed for TrieNode.\n");
        return NULL;
    }

    node->isEnd = 0;
    node->nodeId = -1;
    node->idCount = 0;
    for (int i = 0; i < MAX_TRIE_IDS; i++) {
        node->nodeIds[i] = -1;
    }
    for (int i = 0; i < 128; i++) {
        node->child[i] = NULL;
    }

    return node;
}

/**
 * Helper to insert a single keyword token into the Trie.
 */
static void insertToken(TrieNode *root, const char *token, int nodeId) {
    if (!root || !token || token[0] == '\0') return;

    TrieNode *curr = root;
    for (size_t i = 0; i < strlen(token); i++) {
        unsigned char c = (unsigned char)tolower((unsigned char)token[i]);
        if (c >= 128) continue;

        if (!curr->child[c]) {
            curr->child[c] = createTrieNode();
            if (!curr->child[c]) return;
        }
        curr = curr->child[c];
    }

    curr->isEnd = 1;
    if (curr->nodeId == -1) {
        curr->nodeId = nodeId;
    }

    /* Add to nodeIds array if not already present */
    int exists = 0;
    for (int i = 0; i < curr->idCount; i++) {
        if (curr->nodeIds[i] == nodeId) {
            exists = 1;
            break;
        }
    }
    if (!exists && curr->idCount < MAX_TRIE_IDS) {
        curr->nodeIds[curr->idCount++] = nodeId;
    }
}

/**
 * Insert a campus location into the Trie with smart multi-word indexing.
 * E.g., "Tagore_Boys_Hostel" is indexed as "tagore_boys_hostel", "tagore", "boys", and "hostel".
 */
void insertTrie(TrieNode *root, const char *name, int nodeId) {
    if (!root || !name || name[0] == '\0') return;

    /* 1. Insert full place name */
    insertToken(root, name, nodeId);

    /* 2. Tokenize by underscores and insert individual words */
    char copy[NAME_LEN];
    strncpy(copy, name, NAME_LEN - 1);
    copy[NAME_LEN - 1] = '\0';

    char *token = strtok(copy, "_- ");
    while (token) {
        if (strlen(token) >= 2) { /* Index words with 2 or more characters */
            insertToken(root, token, nodeId);
        }
        token = strtok(NULL, "_- ");
    }
}

/**
 * Helper to check if an ID is already present in matchedIds.
 */
static int isIdPresent(const int *matchedIds, int count, int id) {
    for (int i = 0; i < count; i++) {
        if (matchedIds[i] == id) return 1;
    }
    return 0;
}

/**
 * Recursive helper to collect unique matching IDs via DFS.
 */
static void collectMatches(const TrieNode *node, int *matchedIds, int *count, int maxMatches) {
    if (!node || *count >= maxMatches) return;

    if (node->isEnd) {
        /* Primary ID */
        if (node->nodeId >= 0 && !isIdPresent(matchedIds, *count, node->nodeId)) {
            matchedIds[*count] = node->nodeId;
            (*count)++;
            if (*count >= maxMatches) return;
        }
        /* Multi-match IDs */
        for (int i = 0; i < node->idCount; i++) {
            int id = node->nodeIds[i];
            if (id >= 0 && !isIdPresent(matchedIds, *count, id)) {
                matchedIds[*count] = id;
                (*count)++;
                if (*count >= maxMatches) return;
            }
        }
    }

    for (int i = 0; i < 128; i++) {
        if (node->child[i]) {
            collectMatches(node->child[i], matchedIds, count, maxMatches);
            if (*count >= maxMatches) return;
        }
    }
}

/**
 * Search all place names that start with a given prefix.
 */
int searchPrefix(const TrieNode *root, const char *prefix, int *matchedIds, int maxMatches) {
    if (!root || !matchedIds || maxMatches <= 0) return 0;

    int matchCount = 0;
    const TrieNode *curr = root;

    if (!prefix || prefix[0] == '\0') {
        collectMatches(curr, matchedIds, &matchCount, maxMatches);
        return matchCount;
    }

    for (size_t i = 0; i < strlen(prefix); i++) {
        unsigned char c = (unsigned char)tolower((unsigned char)prefix[i]);
        if (c >= 128 || !curr->child[c]) {
            return 0;
        }
        curr = curr->child[c];
    }

    collectMatches(curr, matchedIds, &matchCount, maxMatches);
    return matchCount;
}

/**
 * Helper to unmark a single keyword token in the Trie.
 */
static int unmarkSingleToken(TrieNode *root, const char *token) {
    if (!root || !token || token[0] == '\0') return 0;

    TrieNode *curr = root;
    for (size_t i = 0; i < strlen(token); i++) {
        unsigned char c = (unsigned char)tolower((unsigned char)token[i]);
        if (c >= 128 || !curr->child[c]) return 0;
        curr = curr->child[c];
    }

    if (curr->isEnd) {
        curr->isEnd = 0;
        curr->nodeId = -1;
        curr->idCount = 0;
        return 1;
    }
    return 0;
}

/**
 * Unmark a location and all its associated tokens from the Trie.
 */
int removeFromTrie(TrieNode *root, const char *name) {
    if (!root || !name || name[0] == '\0') return 0;

    int res = unmarkSingleToken(root, name);

    char copy[NAME_LEN];
    strncpy(copy, name, NAME_LEN - 1);
    copy[NAME_LEN - 1] = '\0';

    char *token = strtok(copy, "_- ");
    while (token) {
        unmarkSingleToken(root, token);
        token = strtok(NULL, "_- ");
    }

    return res;
}

/**
 * Free all dynamically allocated nodes in the Trie.
 */
void freeTrie(TrieNode *root) {
    if (!root) return;

    for (int i = 0; i < 128; i++) {
        if (root->child[i]) {
            freeTrie(root->child[i]);
            root->child[i] = NULL;
        }
    }

    free(root);
}

/**
 * Display formatted autocomplete suggestions in the console.
 */
void printAutocompleteSuggestions(const TrieNode *root, const Graph *g, const char *prefix) {
    if (!root || !g) return;

    int matches[MAX_NODES];
    int count = searchPrefix(root, prefix, matches, 10);

    if (count == 0) {
        printf("  %sNo campus locations match '%s'%s\n", COLOR_YELLOW, prefix, COLOR_RESET);
        return;
    }

    printf("  %sMatches for '%s' (%d found):%s\n", COLOR_CYAN, prefix, count, COLOR_RESET);
    for (int i = 0; i < count; i++) {
        int id = matches[i];
        if (id >= 0 && id < MAX_NODES && g->nodes[id].exists) {
            printf("    %s[%d]%s %-26s %s(%s)%s\n",
                   COLOR_GREEN, i + 1, COLOR_RESET,
                   g->nodes[id].name,
                   COLOR_GRAY, g->nodes[id].type, COLOR_RESET);
        }
    }
}
