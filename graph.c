/**
 * ============================================================================
 * graph.c - Adjacency List Implementation for Campus Map
 * SRM IST Campus Navigation & Route Planner
 * ============================================================================
 */

#include "graph.h"

/**
 * Purpose:        Initialize graph structure to a clean, empty state.
 * Parameters:     g - pointer to Graph
 * Return:         void
 * Time Complexity: O(MAX_NODES)
 */
void initGraph(Graph *g) {
    if (!g) return;
    g->count = 0;
    for (int i = 0; i < MAX_NODES; i++) {
        g->nodes[i].id = i;
        g->nodes[i].name[0] = '\0';
        g->nodes[i].type[0] = '\0';
        g->nodes[i].exists = 0;
        g->nodes[i].head = NULL;
    }
}

/**
 * Purpose:        Add a new campus location to the graph.
 * Parameters:     g    - pointer to Graph
 *                 name - unique place identifier (e.g., "Tech_Park")
 *                 type - place category (e.g., "academic", "canteen")
 * Return:         Assigned node ID (0 to MAX_NODES-1), or -1 if full/duplicate
 * Time Complexity: O(MAX_NODES)
 */
int addLocation(Graph *g, const char *name, const char *type) {
    if (!g || !name || !type || name[0] == '\0') {
        return -1;
    }

    /* Check for duplicate name (case-insensitive) */
    char lowerNew[NAME_LEN];
    toLowerCase(lowerNew, name);
    for (int i = 0; i < MAX_NODES; i++) {
        if (g->nodes[i].exists) {
            char lowerCurr[NAME_LEN];
            toLowerCase(lowerCurr, g->nodes[i].name);
            if (strcmp(lowerNew, lowerCurr) == 0) {
                return g->nodes[i].id; /* Already exists, return existing ID */
            }
        }
    }

    /* Find first available slot */
    int slot = -1;
    for (int i = 0; i < MAX_NODES; i++) {
        if (!g->nodes[i].exists) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        return -1; /* Graph is at maximum capacity */
    }

    /* Populate node data */
    g->nodes[slot].id = slot;
    strncpy(g->nodes[slot].name, name, NAME_LEN - 1);
    g->nodes[slot].name[NAME_LEN - 1] = '\0';
    strncpy(g->nodes[slot].type, type, TYPE_LEN - 1);
    g->nodes[slot].type[TYPE_LEN - 1] = '\0';
    g->nodes[slot].exists = 1;
    g->nodes[slot].head = NULL;
    g->count++;

    return slot;
}

/**
 * Purpose:        Internal helper to create a single directed edge node.
 * Parameters:     dest   - target node index
 *                 dist   - distance in metres
 * Return:         Allocated Edge pointer, or NULL on memory failure
 * Time Complexity: O(1)
 */
static Edge* createEdgeNode(int dest, int dist) {
    Edge *edge = (Edge *)malloc(sizeof(Edge));
    if (!edge) {
        fprintf(stderr, "Error: Memory allocation failed for graph edge.\n");
        return NULL;
    }
    edge->dest = dest;
    edge->weight = dist;
    edge->blocked = 0;
    edge->next = NULL;
    return edge;
}

/**
 * Purpose:        Add an undirected weighted road between two places.
 * Parameters:     g    - pointer to Graph
 *                 u    - first node ID
 *                 v    - second node ID
 *                 dist - distance in metres (must be > 0)
 * Return:         1 on success, 0 on failure
 * Time Complexity: O(deg(u) + deg(v))
 */
int addRoad(Graph *g, int u, int v, int dist) {
    if (!g || dist <= 0 || u == v) return 0;
    if (u < 0 || u >= MAX_NODES || !g->nodes[u].exists) return 0;
    if (v < 0 || v >= MAX_NODES || !g->nodes[v].exists) return 0;

    /* Check if edge already exists in u's list; if so, update distance */
    Edge *curr = g->nodes[u].head;
    while (curr) {
        if (curr->dest == v) {
            curr->weight = dist;
            /* Also update reverse edge in v's list */
            Edge *rev = g->nodes[v].head;
            while (rev) {
                if (rev->dest == u) {
                    rev->weight = dist;
                    break;
                }
                rev = rev->next;
            }
            return 1;
        }
        curr = curr->next;
    }

    /* Insert u -> v edge at head of u's adjacency list */
    Edge *e1 = createEdgeNode(v, dist);
    if (!e1) return 0;
    e1->next = g->nodes[u].head;
    g->nodes[u].head = e1;

    /* Insert v -> u edge at head of v's adjacency list */
    Edge *e2 = createEdgeNode(u, dist);
    if (!e2) {
        /* Roll back e1 on failure */
        g->nodes[u].head = e1->next;
        free(e1);
        return 0;
    }
    e2->next = g->nodes[v].head;
    g->nodes[v].head = e2;

    return 1;
}

/**
 * Purpose:        Internal helper to remove a single directed edge from list.
 * Parameters:     headRef - pointer to head pointer of adjacency list
 *                 dest    - target node ID to remove
 * Return:         1 if found and removed, 0 otherwise
 * Time Complexity: O(degree)
 */
static int removeSingleEdge(Edge **headRef, int dest) {
    if (!headRef || !*headRef) return 0;

    Edge *curr = *headRef;
    Edge *prev = NULL;

    while (curr) {
        if (curr->dest == dest) {
            if (prev == NULL) {
                *headRef = curr->next;
            } else {
                prev->next = curr->next;
            }
            free(curr);
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }
    return 0;
}

/**
 * Purpose:        Remove an undirected road between u and v in both directions.
 * Parameters:     g - pointer to Graph
 *                 u - first node ID
 *                 v - second node ID
 * Return:         1 if removed, 0 if road didn't exist
 * Time Complexity: O(deg(u) + deg(v))
 */
int removeRoad(Graph *g, int u, int v) {
    if (!g || u < 0 || u >= MAX_NODES || v < 0 || v >= MAX_NODES) return 0;
    if (!g->nodes[u].exists || !g->nodes[v].exists) return 0;

    int r1 = removeSingleEdge(&(g->nodes[u].head), v);
    int r2 = removeSingleEdge(&(g->nodes[v].head), u);

    return (r1 || r2) ? 1 : 0;
}

/**
 * Purpose:        Delete a location from the campus graph and remove all roads to it.
 * Parameters:     g - pointer to Graph
 *                 u - node ID to remove
 * Return:         1 on success, 0 if node was already absent
 * Time Complexity: O(V + E)
 */
int removeLocation(Graph *g, int u) {
    if (!g || u < 0 || u >= MAX_NODES || !g->nodes[u].exists) return 0;

    /* Free all outgoing edges from u */
    Edge *curr = g->nodes[u].head;
    while (curr) {
        Edge *next = curr->next;
        free(curr);
        curr = next;
    }
    g->nodes[u].head = NULL;

    /* Remove incoming edges pointing to u from all other nodes */
    for (int i = 0; i < MAX_NODES; i++) {
        if (g->nodes[i].exists && i != u) {
            removeSingleEdge(&(g->nodes[i].head), u);
        }
    }

    g->nodes[u].exists = 0;
    g->nodes[u].name[0] = '\0';
    g->nodes[u].type[0] = '\0';
    g->count--;

    return 1;
}

/**
 * Purpose:        Block or unblock a road for route recalculation.
 * Parameters:     g       - pointer to Graph
 *                 u, v    - endpoints of the road
 *                 blocked - 1 to block, 0 to reopen
 * Return:         1 if updated, 0 if road not found
 * Time Complexity: O(deg(u) + deg(v))
 */
int setRoadBlocked(Graph *g, int u, int v, int blocked) {
    if (!g || u < 0 || u >= MAX_NODES || v < 0 || v >= MAX_NODES) return 0;
    if (!g->nodes[u].exists || !g->nodes[v].exists) return 0;

    int found = 0;
    Edge *curr = g->nodes[u].head;
    while (curr) {
        if (curr->dest == v) {
            curr->blocked = blocked;
            found = 1;
            break;
        }
        curr = curr->next;
    }

    curr = g->nodes[v].head;
    while (curr) {
        if (curr->dest == u) {
            curr->blocked = blocked;
            found = 1;
            break;
        }
        curr = curr->next;
    }

    return found;
}

/**
 * Purpose:        Print ASCII Schematic Map of SRM IST Delhi-NCR Campus layout.
 * Parameters:     none
 * Return:         void
 * Time Complexity: O(1)
 */
void printAsciiMap(void) {
    printf("\n%s========================================================================================%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s                 SRM IST DELHI-NCR CAMPUS SCHEMATIC MAP (MODINAGAR)                    %s\n", COLOR_BOLD, COLOR_RESET);
    printf("%s========================================================================================%s\n", COLOR_CYAN, COLOR_RESET);
    printf(" [Delhi-Meerut Highway NH-58]\n");
    printf("              |\n");
    printf("              v\n");
    printf("       %s[0: Main_Gate]%s ============================= %s[18: Student_Parking]%s\n",
           COLOR_GREEN, COLOR_RESET, COLOR_CYAN, COLOR_RESET);
    printf("              |                                            |\n");
    printf("      %s[4: Admin_Block]%s =========================== %s[16: PNB_ATM]%s\n",
           COLOR_CYAN, COLOR_RESET, COLOR_YELLOW, COLOR_RESET);
    printf("              |\n");
    printf("       +------+-----------------------------+\n");
    printf("       |                                    |\n");
    printf(" %s[1: Academic_A]%s =================== %s[2: Academic_B]%s === %s[19: Temple]%s\n",
           COLOR_GREEN, COLOR_RESET, COLOR_GREEN, COLOR_RESET, COLOR_CYAN, COLOR_RESET);
    printf("   /          \\                             |                  |\n");
    printf("%s[5: Comp_Ctr]%s %s[3: Library]%s          %s[17: Auditorium]%s ----------+\n",
           COLOR_YELLOW, COLOR_RESET, COLOR_CYAN, COLOR_RESET, COLOR_CYAN, COLOR_RESET);
    printf("   |             \\\n");
    printf("%s[6: Workshop]%s == %s[7: Cafeteria]%s ============+\n",
           COLOR_YELLOW, COLOR_RESET, COLOR_YELLOW, COLOR_RESET);
    printf("                        |\n");
    printf("                 %s[8: Nescafe]%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("                  /         \\\n");
    printf("    %s[9: Tagore_Hostel]%s     %s[11: Sarojini_Girls]%s\n",
           COLOR_MAGENTA, COLOR_RESET, COLOR_MAGENTA, COLOR_RESET);
    printf("           |                        |\n");
    printf("    %s[10: Raman_Hostel]%s              |\n",
           COLOR_MAGENTA, COLOR_RESET);
    printf("           |                        |\n");
    printf("    %s[12: Sports_Ground]%s ===== %s[13: Basketball]%s ===== %s[14: Indoor_Arena]%s\n",
           COLOR_GREEN, COLOR_RESET, COLOR_GREEN, COLOR_RESET, COLOR_GREEN, COLOR_RESET);
    printf("           \\                       /                       /\n");
    printf("            \\----- %s[15: Health_Centre]%s -------------------/\n",
           COLOR_RED, COLOR_RESET);
    printf("\n    * %s[20: Future_Expansion_Plot]%s (Isolated sector - No connecting roads)\n",
           COLOR_GRAY, COLOR_RESET);
    printf("%s========================================================================================%s\n", COLOR_CYAN, COLOR_RESET);
}

/**
 * Purpose:        Print the campus graph as an adjacency list.
 * Parameters:     g - pointer to Graph
 * Return:         void
 * Time Complexity: O(V + E)
 */
void printGraph(const Graph *g) {
    if (!g) return;

    printf("\n%s================ SRM IST DELHI-NCR CAMPUS NETWORK MAP ================%s\n", COLOR_BOLD, COLOR_RESET);
    printf("Active Locations: %d\n\n", g->count);

    for (int i = 0; i < MAX_NODES; i++) {
        if (!g->nodes[i].exists) continue;

        printf("  %s[%2d] %-22s%s (%s)\n",
               COLOR_CYAN, g->nodes[i].id, g->nodes[i].name, COLOR_RESET, g->nodes[i].type);

        Edge *curr = g->nodes[i].head;
        if (!curr) {
            printf("       └── %s(Isolated - No roads connected)%s\n", COLOR_GRAY, COLOR_RESET);
            continue;
        }

        while (curr) {
            char *status = curr->blocked ? " [BLOCKED]" : "";
            char *color = curr->blocked ? COLOR_RED : COLOR_GREEN;
            printf("       ├──> %s%-20s%s | Dist: %4dm%s%s%s\n",
                   color,
                   g->nodes[curr->dest].name,
                   COLOR_RESET,
                   curr->weight,
                   curr->blocked ? COLOR_RED : "",
                   status,
                   COLOR_RESET);
            curr = curr->next;
        }
        printf("\n");
    }
    printf("%s============================================================%s\n", COLOR_BOLD, COLOR_RESET);
}

/**
 * Purpose:        Free all dynamically allocated edge nodes in the graph.
 * Parameters:     g - pointer to Graph
 * Return:         void
 * Time Complexity: O(V + E)
 */
void freeGraph(Graph *g) {
    if (!g) return;

    for (int i = 0; i < MAX_NODES; i++) {
        Edge *curr = g->nodes[i].head;
        while (curr) {
            Edge *next = curr->next;
            free(curr);
            curr = next;
        }
        g->nodes[i].head = NULL;
        g->nodes[i].exists = 0;
    }
    g->count = 0;
}
