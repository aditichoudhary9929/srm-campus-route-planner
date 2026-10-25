/**
 * ============================================================================
 * tests/test_main.c - Comprehensive Assert-Based Test Suite
 * SRM IST Delhi-NCR Campus Navigation & Route Planner
 * Plain C11 Standard Library Verification
 * ============================================================================
 */

#include "common.h"
#include "graph.h"
#include "hash.h"
#include "trie.h"
#include "heap.h"
#include "algo.h"
#include "history.h"
#include "file.h"
#include <assert.h>

int g_useColor = 1;

/* ANSI Colors for Test Output */
#define TEST_PASS(msg) printf("  %s[PASS]%s %s\n", COLOR_GREEN, COLOR_RESET, msg)
#define TEST_SECTION(title) printf("\n%s=== %s ===%s\n", COLOR_BOLD, title, COLOR_RESET)

/**
 * 1. Test Min-Heap Priority Queue Ordering & Resizing
 */
static void testMinHeap(void) {
    TEST_SECTION("1. Testing Min-Heap Priority Queue");

    MinHeap *h = createMinHeap(2); /* Small initial capacity to trigger realloc */
    assert(h != NULL);
    assert(isHeapEmpty(h) == 1);

    pushHeap(h, 10, 150);
    pushHeap(h, 20, 40);
    pushHeap(h, 30, 80);
    pushHeap(h, 40, 10);
    pushHeap(h, 50, 200);

    assert(isHeapEmpty(h) == 0);

    /* Assert strict ascending order of extracted minimums */
    HeapNode n1 = popHeap(h);
    assert(n1.node == 40 && n1.dist == 10);

    HeapNode n2 = popHeap(h);
    assert(n2.node == 20 && n2.dist == 40);

    HeapNode n3 = popHeap(h);
    assert(n3.node == 30 && n3.dist == 80);

    HeapNode n4 = popHeap(h);
    assert(n4.node == 10 && n4.dist == 150);

    HeapNode n5 = popHeap(h);
    assert(n5.node == 50 && n5.dist == 200);

    assert(isHeapEmpty(h) == 1);
    freeHeap(h);
    TEST_PASS("Min-Heap dynamic resizing and priority order verified.");
}

/**
 * 2. Test Hash Table djb2 Algorithm & Case-Insensitive Lookups
 */
static void testHashTable(void) {
    TEST_SECTION("2. Testing djb2 Hash Table & Chaining");

    HashTable ht;
    initHashTable(&ht);

    /* Insert test keys */
    assert(insertName(&ht, "Main_Gate", 0) == 1);
    assert(insertName(&ht, "Academic_Block_A", 1) == 1);
    assert(insertName(&ht, "Central_Library", 3) == 1);
    assert(insertName(&ht, "Central_Cafeteria", 7) == 1);

    /* Case-insensitive retrieval */
    assert(getId(&ht, "Main_Gate") == 0);
    assert(getId(&ht, "main_gate") == 0);
    assert(getId(&ht, "MAIN_GATE") == 0);
    assert(getId(&ht, "aCaDeMiC_bLoCk_A") == 1);

    /* Non-existent key */
    assert(getId(&ht, "NonExistentPlace") == -1);

    /* Hash collision metrics */
    int collisions = 0, maxChain = 0;
    getHashStats(&ht, &collisions, &maxChain);
    assert(maxChain >= 1 && maxChain <= 5);

    /* Deletion */
    assert(removeName(&ht, "Central_Cafeteria") == 1);
    assert(getId(&ht, "Central_Cafeteria") == -1);
    assert(removeName(&ht, "Central_Cafeteria") == 0);

    freeHashTable(&ht);
    TEST_PASS("Hash Table O(1) lookups, case-insensitivity, and deletion verified.");
}

/**
 * 3. Test Trie Prefix Tree & Autocomplete
 */
static void testTrie(void) {
    TEST_SECTION("3. Testing Trie Prefix Matching & Autocomplete");

    TrieNode *trie = createTrieNode();
    assert(trie != NULL);

    insertTrie(trie, "Main_Gate", 0);
    insertTrie(trie, "Academic_Block_A", 1);
    insertTrie(trie, "Administrative_Block", 4);
    insertTrie(trie, "Basketball_Courts", 13);
    insertTrie(trie, "Tagore_Boys_Hostel", 9);
    insertTrie(trie, "Sarojini_Girls_Hostel", 11);

    int matches[MAX_NODES];

    /* Prefix 'acad' matches Academic_Block_A */
    int countAcad = searchPrefix(trie, "acad", matches, 10);
    assert(countAcad >= 1);

    /* Shared keyword 'hostel' matches BOTH Tagore_Boys_Hostel and Sarojini_Girls_Hostel */
    int countHostel = searchPrefix(trie, "hostel", matches, 10);
    assert(countHostel >= 2);

    /* Case-insensitive prefix 'BASKET' */
    int countBasket = searchPrefix(trie, "BASKET", matches, 10);
    assert(countBasket == 1);
    assert(matches[0] == 13);

    /* No-match prefix */
    int countNone = searchPrefix(trie, "xyz_not_real", matches, 10);
    assert(countNone == 0);

    /* Trie deletion / unmarking */
    assert(removeFromTrie(trie, "Basketball_Courts") == 1);
    int countAfterDel = searchPrefix(trie, "basketball", matches, 10);
    assert(countAfterDel == 0);

    freeTrie(trie);
    TEST_PASS("Trie multi-word prefix search, case folding, and unmarking verified.");
}

/**
 * 4. Test Dijkstra Shortest Path, Disconnected Nodes & Alternate Detours
 */
static void testDijkstraAndRouting(void) {
    TEST_SECTION("4. Testing Dijkstra Shortest Path, Disconnected Nodes & Detours");

    Graph g;
    HashTable ht;
    initHashTable(&ht);
    TrieNode *trie = createTrieNode();

    int ok = loadMap(&g, &ht, trie, "map.txt");
    assert(ok == 1);
    assert(g.count >= 20);

    int dist[MAX_NODES];
    int parent[MAX_NODES];

    /* Test 4a: Shortest path from Main_Gate (0) to Central_Library (3) */
    /* Optimal: 0 -> 1 (150m) + 1 -> 3 (80m) = 230m */
    int d1 = dijkstra(&g, 0, 3, dist, parent);
    assert(d1 == 230);
    assert(parent[3] == 1);
    assert(parent[1] == 0);
    TEST_PASS("Known path 0 -> 1 -> 3 correctly calculated at 230m.");

    /* Test 4b: Source is destination (0m) */
    int dSame = dijkstra(&g, 0, 0, dist, parent);
    assert(dSame == 0);
    TEST_PASS("Source == destination distance correctly evaluated to 0m.");

    /* Test 4c: Disconnected Node (Future_Expansion_Plot: 20 has no edges) */
    int dDisc = dijkstra(&g, 0, 20, dist, parent);
    assert(dDisc == INF);
    TEST_PASS("Disconnected node correctly returned distance INF.");

    /* Test 4d: Block road 0 <-> 1 and test alternate detour */
    /* Detour: 0 -> 4 (90m) + 4 -> 1 (110m) + 1 -> 3 (80m) = 280m */
    setRoadBlocked(&g, 0, 1, 1);
    int dDetour = dijkstra(&g, 0, 3, dist, parent);
    assert(dDetour == 280);
    assert(parent[3] == 1);
    assert(parent[1] == 4);
    assert(parent[4] == 0);
    TEST_PASS("Blocked road forced alternate detour at 280m (via Administrative_Block).");

    /* Reopen road */
    setRoadBlocked(&g, 0, 1, 0);
    int dReopen = dijkstra(&g, 0, 3, dist, parent);
    assert(dReopen == 230);
    TEST_PASS("Reopening road restored optimal path at 230m.");

    freeGraph(&g);
    freeHashTable(&ht);
    freeTrie(trie);
}

/**
 * 5. Test Circular History Queue (FIFO Overwrite on 6th & 7th Searches)
 */
static void testHistoryQueue(void) {
    TEST_SECTION("5. Testing Circular Queue History & FIFO Overwrite");

    HistoryQueue hq;
    initHistory(&hq);
    assert(getHistoryCount(&hq) == 0);

    /* Add 5 searches */
    addHistory(&hq, "Place_1", "Place_2", 100);
    addHistory(&hq, "Place_2", "Place_3", 200);
    addHistory(&hq, "Place_3", "Place_4", 300);
    addHistory(&hq, "Place_4", "Place_5", 400);
    addHistory(&hq, "Place_5", "Place_6", 500);
    assert(getHistoryCount(&hq) == 5);
    assert(strcmp(hq.items[hq.front].srcName, "Place_1") == 0);

    /* 6th search evicts Place_1 */
    addHistory(&hq, "Place_6", "Place_7", 600);
    assert(getHistoryCount(&hq) == 5);
    assert(strcmp(hq.items[hq.front].srcName, "Place_2") == 0);

    /* 7th search evicts Place_2 */
    addHistory(&hq, "Place_7", "Place_8", 700);
    assert(getHistoryCount(&hq) == 5);
    assert(strcmp(hq.items[hq.front].srcName, "Place_3") == 0);

    clearHistory(&hq);
    assert(getHistoryCount(&hq) == 0);
    TEST_PASS("Circular FIFO eviction after 6 and 7 searches verified.");
}

/**
 * 6. Test File Persistence Save and Load Round Trip
 */
static void testSaveLoadRoundTrip(void) {
    TEST_SECTION("6. Testing File Persistence Save/Load Roundtrip");

    Graph g1, g2;
    HashTable ht1, ht2;
    initHashTable(&ht1);
    initHashTable(&ht2);
    TrieNode *trie1 = createTrieNode();
    TrieNode *trie2 = createTrieNode();

    assert(loadMap(&g1, &ht1, trie1, "map.txt") == 1);

    const char *tmpFile = "test_roundtrip_temp.txt";
    assert(saveMap(&g1, tmpFile) == 1);

    assert(loadMap(&g2, &ht2, trie2, tmpFile) == 1);
    assert(g1.count == g2.count);

    /* Verify all nodes and their categories match exactly */
    for (int i = 0; i < MAX_NODES; i++) {
        if (g1.nodes[i].exists) {
            assert(g2.nodes[i].exists == 1);
            assert(strcmp(g1.nodes[i].name, g2.nodes[i].name) == 0);
            assert(strcmp(g1.nodes[i].type, g2.nodes[i].type) == 0);
            assert(getId(&ht1, g1.nodes[i].name) == getId(&ht2, g2.nodes[i].name));
        }
    }

    freeGraph(&g1);
    freeGraph(&g2);
    freeHashTable(&ht1);
    freeHashTable(&ht2);
    freeTrie(trie1);
    freeTrie(trie2);
    remove(tmpFile);

    TEST_PASS("Save and reload roundtrip verified with identical graph structures.");
}

/**
 * 7. Test Graph Mutation Synchronization (Add & Delete Location/Road)
 */
static void testGraphMutations(void) {
    TEST_SECTION("7. Testing Graph Mutations & Multi-Structure Synchronization");

    Graph g;
    HashTable ht;
    initHashTable(&ht);
    TrieNode *trie = createTrieNode();
    initGraph(&g);

    /* Add location */
    int newId = addLocation(&g, "Robotics_Club", "lab");
    assert(newId >= 0);
    insertName(&ht, "Robotics_Club", newId);
    insertTrie(trie, "Robotics_Club", newId);

    assert(getId(&ht, "Robotics_Club") == newId);

    int matches[MAX_NODES];
    assert(searchPrefix(trie, "robot", matches, 5) >= 1);

    /* Delete location and synchronize */
    removeFromTrie(trie, "Robotics_Club");
    removeName(&ht, "Robotics_Club");
    assert(removeLocation(&g, newId) == 1);

    assert(getId(&ht, "Robotics_Club") == -1);
    assert(searchPrefix(trie, "robot", matches, 5) == 0);
    assert(g.nodes[newId].exists == 0);

    freeGraph(&g);
    freeHashTable(&ht);
    freeTrie(trie);
    TEST_PASS("Add, delete, and three-way synchronization fully validated.");
}

int main(void) {
    printf("%s============================================================%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s   SRM IST CAMPUS ROUTE PLANNER - AUTOMATED TEST RUNNER     %s\n", COLOR_BOLD, COLOR_RESET);
    printf("%s============================================================%s\n", COLOR_CYAN, COLOR_RESET);

    testMinHeap();
    testHashTable();
    testTrie();
    testDijkstraAndRouting();
    testHistoryQueue();
    testSaveLoadRoundTrip();
    testGraphMutations();

    printf("\n%s============================================================%s\n", COLOR_GREEN, COLOR_RESET);
    printf("%s   ALL 7 TEST SUITES PASSED CLEANLY (100%% SUCCESS)        %s\n", COLOR_GREEN, COLOR_RESET);
    printf("%s============================================================%s\n\n", COLOR_GREEN, COLOR_RESET);

    return 0;
}
