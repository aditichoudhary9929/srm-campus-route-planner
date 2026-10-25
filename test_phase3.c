/**
 * ============================================================================
 * test_phase3.c - Verification Harness for Hash Table and Multi-Token Trie
 * SRM IST Delhi-NCR Campus Navigation & Route Planner
 * ============================================================================
 */

#include "graph.h"
#include "file.h"
#include "hash.h"
#include "trie.h"
#include <assert.h>

int g_useColor = 1;

int main(void) {
    printf("[PHASE 3 TEST] Initializing Graph, HashTable, and Trie...\n");
    Graph g;
    HashTable ht;
    initHashTable(&ht);
    TrieNode *trie = createTrieNode();
    assert(trie != NULL);

    printf("[PHASE 3 TEST] Loading SRM Delhi-NCR map.txt with Trie & Hash Sync...\n");
    int loadOk = loadMap(&g, &ht, trie, "map.txt");
    assert(loadOk == 1);
    printf("Successfully synchronized %d nodes into Graph, Hash Table, and Trie.\n", g.count);
    assert(g.count >= 16);

    /* ========================================================================
     * 1. Test Hash Table Lookups (O(1) average time)
     * ======================================================================== */
    printf("\n[PHASE 3 TEST] 1. Testing Hash Table Lookups (Case-Insensitive)...\n");

    int idGate = getId(&ht, "Main_Gate");
    int idGateLower = getId(&ht, "main_gate");
    int idGateUpper = getId(&ht, "MAIN_GATE");
    assert(idGate == 0 && idGateLower == 0 && idGateUpper == 0);

    int idLib = getId(&ht, "central_library");
    assert(idLib == 3);

    int idMissing = getId(&ht, "Non_Existent_Building");
    assert(idMissing == -1);

    int collisions = 0, maxChain = 0;
    getHashStats(&ht, &collisions, &maxChain);
    printf("  Hash Table Stats: %d buckets, %d collisions, longest chain: %d\n",
           HASH_BUCKETS, collisions, maxChain);
    assert(maxChain >= 1 && maxChain < 5);

    printf("  %sHash Table tests passed successfully!%s\n", COLOR_GREEN, COLOR_RESET);

    /* ========================================================================
     * 2. Test Smart Multi-Token Trie Prefix Autocomplete
     * ======================================================================== */
    printf("\n[PHASE 3 TEST] 2. Testing Trie Prefix Autocomplete...\n");

    int matches[MAX_NODES];

    /* Test prefix 'acad' -> Academic_Block_A and Academic_Block_B */
    int c1 = searchPrefix(trie, "acad", matches, 10);
    assert(c1 >= 2);

    /* Test shared keyword 'hostel' -> matches Tagore, Raman, and Sarojini hostels! */
    int cHostel = searchPrefix(trie, "hostel", matches, 10);
    printf("  Prefix 'hostel' matched %d locations:\n", cHostel);
    for (int i = 0; i < cHostel; i++) {
        printf("    - %s\n", g.nodes[matches[i]].name);
    }
    assert(cHostel >= 3);

    /* Test shared keyword 'canteen' or 'cafe' */
    int cCafe = searchPrefix(trie, "cafe", matches, 10);
    assert(cCafe >= 1);

    /* Test uppercase prefix 'BASKET' */
    int c3 = searchPrefix(trie, "BASKET", matches, 10);
    assert(c3 == 1);
    assert(strcmp(g.nodes[matches[0]].name, "Basketball_Courts") == 0);

    /* Test no-match prefix */
    int c4 = searchPrefix(trie, "xyz_nowhere", matches, 10);
    assert(c4 == 0);

    /* Print formatted CLI suggestions enhancement */
    printf("\n[PHASE 3 TEST] Visual Autocomplete Demonstration:\n");
    printAutocompleteSuggestions(trie, &g, "hostel");
    printAutocompleteSuggestions(trie, &g, "basket");
    printAutocompleteSuggestions(trie, &g, "lib");

    /* ========================================================================
     * 3. Memory Cleanup & Zero-Leak Verification
     * ======================================================================== */
    printf("\n[PHASE 3 TEST] Freeing Trie, HashTable, and Graph memory...\n");
    freeTrie(trie);
    freeHashTable(&ht);
    freeGraph(&g);

    printf("\n%s[PHASE 3 TEST PASSED] All tests for Hash Table and Smart Trie passed!%s\n",
           COLOR_GREEN, COLOR_RESET);
    return 0;
}
