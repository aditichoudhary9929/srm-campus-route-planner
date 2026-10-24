/**
 * ============================================================================
 * main.c - Enhanced Interactive CLI Menu & User Interface
 * SRM IST Delhi-NCR Campus Navigation & Route Planner
 * Plain C11 Standard Library Implementation (Zero External Dependencies)
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

/* Global color toggle state (1 = ANSI enabled, 0 = Plain text) */
int g_useColor = 1;

/* Administrative session authentication flag */
static int g_isAdminUnlocked = 0;

/* ============================================================================
 * Safe Console Input Helpers (No raw scanf)
 * ============================================================================ */

/**
 * Prompt user for a clean string line. Rejects empty input.
 */
static int promptString(const char *prompt, char *buffer, int maxLen) {
    while (1) {
        printf("%s", prompt);
        if (!fgets(buffer, maxLen, stdin)) {
            return 0;
        }
        trimNewline(buffer);
        if (buffer[0] != '\0') {
            return 1;
        }
        printf("%sInput cannot be empty. Please try again.%s\n", COLOR_YELLOW, COLOR_RESET);
    }
}

/**
 * Prompt user for an integer in range [minVal, maxVal].
 */
static int promptInt(const char *prompt, int minVal, int maxVal) {
    char line[64];
    while (1) {
        printf("%s", prompt);
        if (!fgets(line, sizeof(line), stdin)) {
            return minVal;
        }
        trimNewline(line);
        if (line[0] == '\0') {
            printf("%sPlease enter a number.%s\n", COLOR_YELLOW, COLOR_RESET);
            continue;
        }

        char *endptr;
        long val = strtol(line, &endptr, 10);
        if (*endptr == '\0' && val >= minVal && val <= maxVal) {
            return (int)val;
        }
        printf("%sInvalid number. Please enter a value between %d and %d.%s\n",
               COLOR_YELLOW, minVal, maxVal, COLOR_RESET);
    }
}

/**
 * Authenticate administrative security PIN for modification operations.
 */
static int verifyAdminPin(void) {
    if (g_isAdminUnlocked) return 1;

    char pin[32];
    printf("\n%s[ADMIN SECURITY ACCESS]%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("Administrative privileges required for this operation.\n");
    promptString("Enter Admin Passcode (default: srm123, or '0' to cancel): ", pin, sizeof(pin));

    if (strcmp(pin, "0") == 0) {
        printf("%sOperation cancelled.%s\n", COLOR_GRAY, COLOR_RESET);
        return 0;
    }

    if (strcmp(pin, ADMIN_PIN) == 0) {
        g_isAdminUnlocked = 1;
        printf("%sAdmin access granted for this session!%s\n", COLOR_GREEN, COLOR_RESET);
        return 1;
    }

    printf("%sAccess Denied: Incorrect passcode!%s\n", COLOR_RED, COLOR_RESET);
    return 0;
}

/**
 * Resolve place input by Name, Multi-Word Autocomplete, or ID.
 */
static int resolvePlace(const Graph *g, const HashTable *ht, const TrieNode *trie, const char *role) {
    char query[NAME_LEN];
    char prompt[128];
    snprintf(prompt, sizeof(prompt), "Enter %s place (name, prefix, or ID): ", role);

    while (1) {
        if (!promptString(prompt, query, sizeof(query))) {
            return -1;
        }

        /* 1. Direct integer ID */
        char *endptr;
        long numId = strtol(query, &endptr, 10);
        if (*endptr == '\0' && numId >= 0 && numId < MAX_NODES) {
            if (g->nodes[numId].exists) {
                return (int)numId;
            } else {
                printf("%sNo active location has ID %ld.%s\n", COLOR_YELLOW, numId, COLOR_RESET);
                continue;
            }
        }

        /* 2. Direct exact name in Hash Table (O(1)) */
        int exactId = getId(ht, query);
        if (exactId >= 0 && g->nodes[exactId].exists) {
            return exactId;
        }

        /* 3. Query Trie for prefix / keyword autocomplete */
        int matches[MAX_NODES];
        int count = searchPrefix(trie, query, matches, 10);

        if (count == 0) {
            printf("%sNo place found matching '%s'. Try again.%s\n", COLOR_RED, query, COLOR_RESET);
            continue;
        } else if (count == 1) {
            int selId = matches[0];
            printf("  %sAuto-selected:%s %s [%s]\n",
                   COLOR_GREEN, COLOR_RESET, g->nodes[selId].name, g->nodes[selId].type);
            return selId;
        } else {
            printf("  %sMultiple locations match '%s':%s\n", COLOR_CYAN, query, COLOR_RESET);
            for (int i = 0; i < count; i++) {
                int id = matches[i];
                printf("    %s[%d]%s %-26s (%s)\n",
                       COLOR_GREEN, i + 1, COLOR_RESET,
                       g->nodes[id].name, g->nodes[id].type);
            }
            int choice = promptInt("Select location number (or 0 to retry): ", 0, count);
            if (choice == 0) continue;
            return matches[choice - 1];
        }
    }
}

/* ============================================================================
 * Menu Handlers
 * ============================================================================ */

static void handleAddLocation(Graph *g, HashTable *ht, TrieNode *trie) {
    if (!verifyAdminPin()) return;

    char name[NAME_LEN];
    char type[TYPE_LEN];

    printf("\n%s--- ADD NEW CAMPUS LOCATION ---%s\n", COLOR_BOLD, COLOR_RESET);
    promptString("Enter place name (use underscores, e.g. New_Robotics_Lab): ", name, sizeof(name));

    if (getId(ht, name) != -1) {
        printf("%sError: A location named '%s' already exists!%s\n", COLOR_RED, name, COLOR_RESET);
        return;
    }

    promptString("Enter category (e.g. academic, canteen, lab, hostel, sports, facility): ", type, sizeof(type));

    int id = addLocation(g, name, type);
    if (id < 0) {
        printf("%sError: Campus graph is at maximum capacity (%d locations).%s\n",
               COLOR_RED, MAX_NODES, COLOR_RESET);
        return;
    }

    insertName(ht, name, id);
    insertTrie(trie, name, id);

    printf("%sSuccess! Location added with ID %d:%s %s [%s]\n",
           COLOR_GREEN, id, COLOR_RESET, name, type);
}

static void handleAddRoad(Graph *g, const HashTable *ht, const TrieNode *trie) {
    if (!verifyAdminPin()) return;

    printf("\n%s--- ADD ROAD CONNECTION ---%s\n", COLOR_BOLD, COLOR_RESET);
    int u = resolvePlace(g, ht, trie, "first");
    if (u < 0) return;

    int v = resolvePlace(g, ht, trie, "second");
    if (v < 0) return;

    if (u == v) {
        printf("%sError: Cannot add a road from a place to itself.%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    int dist = promptInt("Enter road walking distance in metres: ", 1, 5000);

    if (addRoad(g, u, v, dist)) {
        printf("%sRoad connected successfully:%s %s <---> %s (%dm)\n",
               COLOR_GREEN, COLOR_RESET, g->nodes[u].name, g->nodes[v].name, dist);
    } else {
        printf("%sError: Failed to add road connection.%s\n", COLOR_RED, COLOR_RESET);
    }
}

static void handleDelete(Graph *g, HashTable *ht, TrieNode *trie) {
    if (!verifyAdminPin()) return;

    printf("\n%s--- DELETE OPTIONS ---%s\n", COLOR_BOLD, COLOR_RESET);
    printf("1. Delete a Road\n");
    printf("2. Delete a Location\n");
    int choice = promptInt("Select action (1-2, 0 to cancel): ", 0, 2);

    if (choice == 1) {
        int u = resolvePlace(g, ht, trie, "first");
        int v = resolvePlace(g, ht, trie, "second");
        if (removeRoad(g, u, v)) {
            printf("%sRoad between %s and %s removed successfully.%s\n",
                   COLOR_GREEN, g->nodes[u].name, g->nodes[v].name, COLOR_RESET);
        } else {
            printf("%sNo road existed between %s and %s.%s\n",
                   COLOR_YELLOW, g->nodes[u].name, g->nodes[v].name, COLOR_RESET);
        }
    } else if (choice == 2) {
        int u = resolvePlace(g, ht, trie, "target");
        char delName[NAME_LEN];
        strncpy(delName, g->nodes[u].name, NAME_LEN - 1);
        delName[NAME_LEN - 1] = '\0';

        removeFromTrie(trie, delName);
        removeName(ht, delName);

        if (removeLocation(g, u)) {
            printf("%sLocation '%s' (ID %d) and all connected roads deleted.%s\n",
                   COLOR_GREEN, delName, u, COLOR_RESET);
        }
    }
}

static void handleShortestPath(const Graph *g, const HashTable *ht, const TrieNode *trie, HistoryQueue *hq) {
    printf("\n%s--- SHORTEST PATH NAVIGATION ---%s\n", COLOR_BOLD, COLOR_RESET);
    int src = resolvePlace(g, ht, trie, "starting");
    if (src < 0) return;

    int dest = resolvePlace(g, ht, trie, "destination");
    if (dest < 0) return;

    int dist[MAX_NODES];
    int parent[MAX_NODES];

    int shortestDist = dijkstra(g, src, dest, dist, parent);
    printPath(g, src, dest, parent, dist);

    if (shortestDist != INF && src != dest) {
        addHistory(hq, g->nodes[src].name, g->nodes[dest].name, shortestDist);
    }
}

static void handleSearchPlace(const Graph *g, const TrieNode *trie) {
    char prefix[NAME_LEN];
    printf("\n%s--- PLACE AUTOCOMPLETE SEARCH ---%s\n", COLOR_BOLD, COLOR_RESET);
    promptString("Enter search keyword or prefix: ", prefix, sizeof(prefix));

    int matches[MAX_NODES];
    int count = searchPrefix(trie, prefix, matches, 10);

    if (count == 0) {
        printf("%sNo places found matching '%s'.%s\n", COLOR_YELLOW, prefix, COLOR_RESET);
        return;
    }

    printf("\n%sFound %d matching location(s):%s\n", COLOR_CYAN, count, COLOR_RESET);
    for (int i = 0; i < count; i++) {
        int id = matches[i];
        printf("\n  %s[%d] %s%s (Type: %s)\n",
               COLOR_BOLD, id, g->nodes[id].name, COLOR_RESET, g->nodes[id].type);

        Edge *curr = g->nodes[id].head;
        if (!curr) {
            printf("       └── (Isolated - No connecting roads)\n");
        } else {
            while (curr) {
                printf("       ├──> %-26s [Dist: %3dm]%s\n",
                       g->nodes[curr->dest].name, curr->weight,
                       curr->blocked ? " (BLOCKED)" : "");
                curr = curr->next;
            }
        }
    }
}

static void handleNearestByType(const Graph *g, const HashTable *ht, const TrieNode *trie) {
    printf("\n%s--- FIND NEAREST PLACE BY CATEGORY ---%s\n", COLOR_BOLD, COLOR_RESET);
    int src = resolvePlace(g, ht, trie, "current");
    if (src < 0) return;

    printf("\nCommon categories: canteen, library, hostel, lab, sports, admin, facility, parking\n");
    char category[TYPE_LEN];
    promptString("Enter place category: ", category, sizeof(category));

    nearestByType(g, src, category);
}

static void handleBlockRoad(Graph *g, const HashTable *ht, const TrieNode *trie) {
    if (!verifyAdminPin()) return;

    printf("\n%s--- ROAD BLOCKAGE & DETOUR PLANNER ---%s\n", COLOR_BOLD, COLOR_RESET);
    printf("1. Block a Road (Simulate Construction / Event)\n");
    printf("2. Unblock a Road\n");
    printf("3. Calculate Alternate Route around a Blocked Road\n");
    int choice = promptInt("Select action (1-3, 0 to cancel): ", 0, 3);

    if (choice == 1) {
        int u = resolvePlace(g, ht, trie, "first");
        int v = resolvePlace(g, ht, trie, "second");
        if (setRoadBlocked(g, u, v, 1)) {
            printf("%sRoad between %s and %s is now BLOCKED.%s\n",
                   COLOR_RED, g->nodes[u].name, g->nodes[v].name, COLOR_RESET);
        } else {
            printf("%sNo road found between %s and %s.%s\n",
                   COLOR_YELLOW, g->nodes[u].name, g->nodes[v].name, COLOR_RESET);
        }
    } else if (choice == 2) {
        int u = resolvePlace(g, ht, trie, "first");
        int v = resolvePlace(g, ht, trie, "second");
        if (setRoadBlocked(g, u, v, 0)) {
            printf("%sRoad between %s and %s is now REOPENED.%s\n",
                   COLOR_GREEN, g->nodes[u].name, g->nodes[v].name, COLOR_RESET);
        } else {
            printf("%sNo road found between %s and %s.%s\n",
                   COLOR_YELLOW, g->nodes[u].name, g->nodes[v].name, COLOR_RESET);
        }
    } else if (choice == 3) {
        int src = resolvePlace(g, ht, trie, "start");
        int dest = resolvePlace(g, ht, trie, "destination");
        printf("Specify the road to temporarily block for detour:\n");
        int bu = resolvePlace(g, ht, trie, "blocked road endpoint 1");
        int bv = resolvePlace(g, ht, trie, "blocked road endpoint 2");
        alternateRoute(g, src, dest, bu, bv);
    }
}

static void handleReachability(const Graph *g, const HashTable *ht, const TrieNode *trie) {
    printf("\n%s--- REACHABILITY & CONNECTIVITY ---%s\n", COLOR_BOLD, COLOR_RESET);
    printf("1. Breadth-First Search (BFS level reachability)\n");
    printf("2. Depth-First Search (DFS connectivity walk)\n");
    printf("3. View Full Campus Adjacency List\n");
    printf("4. View ASCII Schematic Campus Map Layout\n");
    int choice = promptInt("Select action (1-4, 0 to cancel): ", 0, 4);

    if (choice == 1) {
        int start = resolvePlace(g, ht, trie, "start");
        bfs(g, start);
    } else if (choice == 2) {
        int start = resolvePlace(g, ht, trie, "start");
        dfs(g, start);
    } else if (choice == 3) {
        printGraph(g);
    } else if (choice == 4) {
        printAsciiMap();
    }
}

static void handleMultiStopRoute(const Graph *g, const HashTable *ht, const TrieNode *trie, HistoryQueue *hq) {
    printf("\n%s--- MULTI-STOP ROUTE PLANNER (A -> B -> C) ---%s\n", COLOR_BOLD, COLOR_RESET);
    int numStops = promptInt("How many stops in your journey? (2-5): ", 2, 5);

    int stops[5];
    for (int i = 0; i < numStops; i++) {
        char role[32];
        if (i == 0) snprintf(role, sizeof(role), "Start");
        else if (i == numStops - 1) snprintf(role, sizeof(role), "Final Destination");
        else snprintf(role, sizeof(role), "Stop #%d", i);

        stops[i] = resolvePlace(g, ht, trie, role);
    }

    printf("\n%s=== FULL MULTI-STOP CAMPUS ITINERARY ===%s\n", COLOR_BOLD, COLOR_RESET);
    int grandTotalDist = 0;

    for (int i = 0; i < numStops - 1; i++) {
        int u = stops[i];
        int v = stops[i + 1];

        printf("\n%s--- SEGMENT %d: %s to %s ---%s\n",
               COLOR_CYAN, i + 1, g->nodes[u].name, g->nodes[v].name, COLOR_RESET);

        int dist[MAX_NODES];
        int parent[MAX_NODES];
        int d = dijkstra(g, u, v, dist, parent);

        if (d == INF) {
            printf("%sCannot complete itinerary: No route between %s and %s!%s\n",
                   COLOR_RED, g->nodes[u].name, g->nodes[v].name, COLOR_RESET);
            return;
        }

        printPath(g, u, v, parent, dist);
        grandTotalDist += d;
    }

    double totalSecs = grandTotalDist / WALKING_SPEED;
    printf("\n%s>>> GRAND ITINERARY TOTAL: %dm (approx %d mins walking) <<<%s\n",
           COLOR_GREEN, grandTotalDist, (int)(totalSecs / 60), COLOR_RESET);

    addHistory(hq, g->nodes[stops[0]].name, g->nodes[stops[numStops - 1]].name, grandTotalDist);
}

/* ============================================================================
 * Main Application Entry Point
 * ============================================================================ */

int main(void) {
    Graph g;
    HashTable ht;
    initHashTable(&ht);
    TrieNode *trie = createTrieNode();
    HistoryQueue hq;
    initHistory(&hq);

    if (!trie) {
        fprintf(stderr, "Fatal: Could not initialize Trie autocomplete.\n");
        return 1;
    }

    const char *mapFile = "map.txt";
    if (!loadMap(&g, &ht, trie, mapFile)) {
        printf("%sWarning: Could not load '%s'. Starting with empty map.%s\n",
               COLOR_YELLOW, mapFile, COLOR_RESET);
    }

    printf("\n%s==================================================================%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s   SRM INSTITUTE OF SCIENCE AND TECHNOLOGY, DELHI-NCR CAMPUS      %s\n", COLOR_BOLD, COLOR_RESET);
    printf("%s                 Campus Navigation & Route Planner                %s\n", COLOR_BOLD, COLOR_RESET);
    printf("%s==================================================================%s\n", COLOR_CYAN, COLOR_RESET);
    printf("Loaded %s%d campus locations%s from '%s'.\n",
           COLOR_GREEN, g.count, COLOR_RESET, mapFile);

    int running = 1;
    while (running) {
        printf("\n%s===== SRM IST CAMPUS NAVIGATOR =====%s\n", COLOR_BOLD, COLOR_RESET);
        printf(" 1. Add location (Admin)  2. Add road (Admin)\n");
        printf(" 3. Delete location/road  4. Shortest path (with autocomplete)\n");
        printf(" 5. Search place          6. Nearest place by type\n");
        printf(" 7. Block road / detour   8. Reachable places (BFS/DFS/Map)\n");
        printf(" 9. Recent searches      10. Multi-stop route (A -> B -> C)\n");
        printf("11. Toggle ANSI Colors   12. Save & Exit\n");
        printf("%s=====================================%s\n", COLOR_BOLD, COLOR_RESET);

        int choice = promptInt("Select option (1-12): ", 1, 12);

        switch (choice) {
            case 1:
                handleAddLocation(&g, &ht, trie);
                break;
            case 2:
                handleAddRoad(&g, &ht, trie);
                break;
            case 3:
                handleDelete(&g, &ht, trie);
                break;
            case 4:
                handleShortestPath(&g, &ht, trie, &hq);
                break;
            case 5:
                handleSearchPlace(&g, trie);
                break;
            case 6:
                handleNearestByType(&g, &ht, trie);
                break;
            case 7:
                handleBlockRoad(&g, &ht, trie);
                break;
            case 8:
                handleReachability(&g, &ht, trie);
                break;
            case 9:
                printHistory(&hq);
                break;
            case 10:
                handleMultiStopRoute(&g, &ht, trie, &hq);
                break;
            case 11:
                g_useColor = !g_useColor;
                printf("\n%sTerminal ANSI Colors are now %s!%s\n",
                       COLOR_BOLD, g_useColor ? "ENABLED" : "DISABLED", COLOR_RESET);
                break;
            case 12:
                printf("\nSaving campus map to '%s'...\n", mapFile);
                if (saveMap(&g, mapFile)) {
                    printf("%sMap saved successfully!%s\n", COLOR_GREEN, COLOR_RESET);
                }
                running = 0;
                break;
            default:
                break;
        }
    }

    printf("Freeing graph and memory structures...\n");
    freeGraph(&g);
    freeHashTable(&ht);
    freeTrie(trie);

    printf("%sThank you for using SRM IST Campus Navigator. Goodbye!%s\n",
           COLOR_CYAN, COLOR_RESET);
    return 0;
}
