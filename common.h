/**
 * ============================================================================
 * common.h - Core Data Structures, Shared Constants, and Prototypes
 * SRM IST Delhi-NCR Campus Navigation & Route Planner
 * Plain C11 Standard Library Implementation
 * ============================================================================
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* ============================================================================
 * Global Constants & Configurations
 * ============================================================================ */
#define MAX_NODES       100       /* Maximum number of campus locations */
#define NAME_LEN        64        /* Maximum length for place names */
#define TYPE_LEN        32        /* Maximum length for location category */
#define INF             1000000000/* Infinity representation for Dijkstra (10^9) */
#define HASH_BUCKETS    101       /* Prime number bucket count for djb2 hash */
#define HISTORY_SIZE    5         /* Circular queue capacity for recent searches */
#define WALKING_SPEED   1.4       /* Average student walking speed: 1.4 m/s */
#define MAX_TRIE_IDS    16        /* Max locations mapped to a shared keyword */
#define ADMIN_PIN       "srm123"  /* Default Administrative security passcode */

/* ============================================================================
 * ANSI Terminal Colors for Enhanced CLI UI (Toggle-able)
 * ============================================================================ */
extern int g_useColor;

#define COLOR_RESET     (g_useColor ? "\033[0m" : "")
#define COLOR_BOLD      (g_useColor ? "\033[1m" : "")
#define COLOR_RED       (g_useColor ? "\033[31m" : "")
#define COLOR_GREEN     (g_useColor ? "\033[32m" : "")
#define COLOR_YELLOW    (g_useColor ? "\033[33m" : "")
#define COLOR_BLUE      (g_useColor ? "\033[34m" : "")
#define COLOR_MAGENTA   (g_useColor ? "\033[35m" : "")
#define COLOR_CYAN      (g_useColor ? "\033[36m" : "")
#define COLOR_WHITE     (g_useColor ? "\033[37m" : "")
#define COLOR_GRAY      (g_useColor ? "\033[90m" : "")

/* ============================================================================
 * Graph Data Structures (Adjacency List)
 * ============================================================================ */

typedef struct Edge {
    int dest;
    int weight;
    int blocked;
    struct Edge *next;
} Edge;

typedef struct Node {
    int id;
    char name[NAME_LEN];
    char type[TYPE_LEN];
    int exists;
    Edge *head;
} Node;

typedef struct Graph {
    Node nodes[MAX_NODES];
    int count;
} Graph;

/* ============================================================================
 * Min-Heap Data Structure (Priority Queue for Dijkstra)
 * ============================================================================ */

typedef struct HeapNode {
    int node;
    int dist;
} HeapNode;

typedef struct MinHeap {
    HeapNode *array;
    int size;
    int capacity;
} MinHeap;

/* ============================================================================
 * Trie Data Structure (Multi-Token Prefix Search / Autocomplete)
 * ============================================================================ */

typedef struct TrieNode {
    struct TrieNode *child[128];
    int isEnd;
    int nodeId;
    int nodeIds[MAX_TRIE_IDS];
    int idCount;
} TrieNode;

/* ============================================================================
 * Hash Table Data Structure (djb2 with Chaining for Fast O(1) Lookup)
 * ============================================================================ */

typedef struct HEntry {
    char name[NAME_LEN];
    int id;
    struct HEntry *next;
} HEntry;

typedef struct HashTable {
    HEntry *buckets[HASH_BUCKETS];
} HashTable;

/* ============================================================================
 * Circular Queue Data Structure (Recent Search History)
 * ============================================================================ */

typedef struct HistoryEntry {
    char srcName[NAME_LEN];
    char destName[NAME_LEN];
    int distance;
} HistoryEntry;

typedef struct HistoryQueue {
    HistoryEntry items[HISTORY_SIZE];
    int front;
    int count;
} HistoryQueue;

/* ============================================================================
 * Inline Helper Utilities (Standard C11)
 * ============================================================================ */

static inline void toLowerCase(char *dest, const char *src) {
    if (!dest || !src) return;
    size_t i = 0;
    while (src[i] != '\0' && i < NAME_LEN - 1) {
        dest[i] = (char)tolower((unsigned char)src[i]);
        i++;
    }
    dest[i] = '\0';
}

static inline void trimNewline(char *str) {
    if (!str) return;
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r' || str[len - 1] == ' ' || str[len - 1] == '\t')) {
        str[len - 1] = '\0';
        len--;
    }
}

static inline int safeReadString(char *buffer, int maxLen) {
    if (!fgets(buffer, maxLen, stdin)) return 0;
    trimNewline(buffer);
    return (buffer[0] != '\0') ? 1 : 0;
}

/* ============================================================================
 * Module Prototypes
 * ============================================================================ */

/* graph.c */
void initGraph(Graph *g);
int addLocation(Graph *g, const char *name, const char *type);
int addRoad(Graph *g, int u, int v, int dist);
int removeRoad(Graph *g, int u, int v);
int removeLocation(Graph *g, int u);
int setRoadBlocked(Graph *g, int u, int v, int blocked);
void printGraph(const Graph *g);
void printAsciiMap(void);
void freeGraph(Graph *g);

/* heap.c */
MinHeap* createMinHeap(int capacity);
void pushHeap(MinHeap *h, int node, int dist);
HeapNode popHeap(MinHeap *h);
int isHeapEmpty(const MinHeap *h);
void freeHeap(MinHeap *h);

/* hash.c */
unsigned long hashFunc(const char *str);
void initHashTable(HashTable *ht);
int insertName(HashTable *ht, const char *name, int id);
int getId(const HashTable *ht, const char *name);
int removeName(HashTable *ht, const char *name);
void freeHashTable(HashTable *ht);
void getHashStats(const HashTable *ht, int *outCollisions, int *outMaxChain);

/* trie.c */
TrieNode* createTrieNode(void);
void insertTrie(TrieNode *root, const char *name, int nodeId);
int searchPrefix(const TrieNode *root, const char *prefix, int *matchedIds, int maxMatches);
int removeFromTrie(TrieNode *root, const char *name);
void freeTrie(TrieNode *root);
void printAutocompleteSuggestions(const TrieNode *root, const Graph *g, const char *prefix);

/* algo.c */
int dijkstra(const Graph *g, int src, int dest, int *dist, int *parent);
void printPath(const Graph *g, int src, int dest, const int *parent, const int *dist);
void bfs(const Graph *g, int startNode);
void dfs(const Graph *g, int startNode);
void nearestByType(const Graph *g, int src, const char *type);
void alternateRoute(Graph *g, int src, int dest, int blockU, int blockV);

/* history.c */
void initHistory(HistoryQueue *hq);
void addHistory(HistoryQueue *hq, const char *src, const char *dest, int dist);
void printHistory(const HistoryQueue *hq);
void clearHistory(HistoryQueue *hq);
int getHistoryCount(const HistoryQueue *hq);

/* file.c */
int loadMap(Graph *g, HashTable *ht, TrieNode *trie, const char *filename);
int saveMap(const Graph *g, const char *filename);

#endif /* COMMON_H */
