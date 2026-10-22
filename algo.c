/**
 * ============================================================================
 * algo.c - Graph Navigation Algorithms (Dijkstra, BFS, DFS, Nearest)
 * SRM IST Campus Navigation & Route Planner
 * ============================================================================
 */

#include "algo.h"

/**
 * Purpose:        Dijkstra's shortest path algorithm using a Min-Heap.
 * Parameters:     g      - pointer to Graph
 *                 src    - source node ID
 *                 dest   - destination node ID (-1 for all nodes)
 *                 dist   - output array of shortest distances from src
 *                 parent - output array of predecessor nodes in shortest path
 * Return:         Shortest distance to dest (or INF if unreachable)
 * Time Complexity: O((V + E) log V)
 */
int dijkstra(const Graph *g, int src, int dest, int *dist, int *parent) {
    if (!g || !dist || !parent) return INF;
    if (src < 0 || src >= MAX_NODES || !g->nodes[src].exists) return INF;

    /* Initialize distance and parent arrays */
    for (int i = 0; i < MAX_NODES; i++) {
        dist[i] = INF;
        parent[i] = -1;
    }

    dist[src] = 0;

    MinHeap *h = createMinHeap(MAX_NODES * 4);
    if (!h) return INF;

    pushHeap(h, src, 0);

    while (!isHeapEmpty(h)) {
        HeapNode current = popHeap(h);
        int u = current.node;
        int d = current.dist;

        /* Lazy deletion: skip outdated heap entries */
        if (d > dist[u]) {
            continue;
        }

        /* If single-pair target reached, we can stop relaxation early */
        if (dest != -1 && u == dest) {
            break;
        }

        /* Relax all outgoing edges from u */
        Edge *edge = g->nodes[u].head;
        while (edge) {
            /* Skip blocked roads */
            if (!edge->blocked) {
                int v = edge->dest;
                if (g->nodes[v].exists) {
                    int weight = edge->weight;
                    if (dist[u] + weight < dist[v]) {
                        dist[v] = dist[u] + weight;
                        parent[v] = u;
                        pushHeap(h, v, dist[v]);
                    }
                }
            }
            edge = edge->next;
        }
    }

    freeHeap(h);
    return (dest >= 0 && dest < MAX_NODES) ? dist[dest] : 0;
}

/**
 * Purpose:        Reconstruct and print shortest route from src to dest using stack.
 * Parameters:     g      - pointer to Graph
 *                 src    - start node ID
 *                 dest   - destination node ID
 *                 parent - array of predecessor pointers from Dijkstra
 *                 dist   - array of shortest distances from Dijkstra
 * Return:         void
 * Time Complexity: O(Path Length)
 */
void printPath(const Graph *g, int src, int dest, const int *parent, const int *dist) {
    if (!g || !parent || !dist) return;

    if (src < 0 || src >= MAX_NODES || !g->nodes[src].exists ||
        dest < 0 || dest >= MAX_NODES || !g->nodes[dest].exists) {
        printf("%sError: Invalid start or destination location ID.%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    /* Edge Case: Source is same as destination */
    if (src == dest) {
        printf("\n%sYou are already at %s!%s (Distance: 0m, 0 secs)\n",
               COLOR_YELLOW, g->nodes[src].name, COLOR_RESET);
        return;
    }

    /* Edge Case: Destination unreachable */
    if (dist[dest] == INF) {
        printf("\n%sNo route exists between '%s' and '%s'.%s (Disconnected or road blocked)\n",
               COLOR_RED, g->nodes[src].name, g->nodes[dest].name, COLOR_RESET);
        return;
    }

    /* Array-based Stack to reverse path from dest -> src */
    int stack[MAX_NODES];
    int top = -1;
    int curr = dest;

    while (curr != -1) {
        stack[++top] = curr;
        if (curr == src) break;
        curr = parent[curr];
    }

    if (stack[top] != src) {
        printf("\n%sNo valid path could be traced from '%s' to '%s'.%s\n",
               COLOR_RED, g->nodes[src].name, g->nodes[dest].name, COLOR_RESET);
        return;
    }

    /* Print Visual Route */
    printf("\n%s=== RECOMMENDED CAMPUS ROUTE ===%s\n", COLOR_BOLD, COLOR_RESET);
    printf("From: %s%s%s [%s]\n", COLOR_CYAN, g->nodes[src].name, COLOR_RESET, g->nodes[src].type);
    printf("To:   %s%s%s [%s]\n\n", COLOR_GREEN, g->nodes[dest].name, COLOR_RESET, g->nodes[dest].type);

    printf("Turn-by-turn Navigation:\n");
    int totalHops = top;
    for (int i = top; i > 0; i--) {
        int fromNode = stack[i];
        int toNode = stack[i - 1];

        /* Find leg distance */
        int legDist = 0;
        Edge *e = g->nodes[fromNode].head;
        while (e) {
            if (e->dest == toNode) {
                legDist = e->weight;
                break;
            }
            e = e->next;
        }

        double legSecs = legDist / WALKING_SPEED;
        printf("  Leg %d: %s%-20s%s --(%3dm, ~%2.0fs)--> %s%-20s%s\n",
               (top - i + 1),
               COLOR_CYAN, g->nodes[fromNode].name, COLOR_RESET,
               legDist, legSecs,
               COLOR_GREEN, g->nodes[toNode].name, COLOR_RESET);
    }

    /* Route overview */
    printf("\nOverview: ");
    for (int i = top; i >= 0; i--) {
        printf("%s%s%s", COLOR_BOLD, g->nodes[stack[i]].name, COLOR_RESET);
        if (i > 0) printf(" -> ");
    }
    printf("\n");

    /* Walking time calculation: speed = 1.4 m/s */
    int totalDist = dist[dest];
    double totalSeconds = totalDist / WALKING_SPEED;
    int mins = (int)(totalSeconds / 60);
    int secs = (int)totalSeconds % 60;

    printf("\nTotal Distance:   %s%d metres%s (across %d road segments)\n",
           COLOR_YELLOW, totalDist, COLOR_RESET, totalHops);
    printf("Est. Walk Time:   %s%d min(s) %d sec(s)%s (at student walking pace of 1.4 m/s)\n",
           COLOR_YELLOW, mins, secs, COLOR_RESET);
    printf("%s================================%s\n", COLOR_BOLD, COLOR_RESET);
}

/**
 * Purpose:        Breadth-First Search (BFS) for reachable campus locations.
 * Parameters:     g         - pointer to Graph
 *                 startNode - starting place ID
 * Return:         void
 * Time Complexity: O(V + E)
 */
void bfs(const Graph *g, int startNode) {
    if (!g || startNode < 0 || startNode >= MAX_NODES || !g->nodes[startNode].exists) {
        printf("%sError: Invalid start node for BFS.%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    int visited[MAX_NODES] = {0};
    int queue[MAX_NODES];
    int front = 0, rear = 0;

    printf("\n%s=== BFS REACHABLE PLACES FROM %s ===%s\n",
           COLOR_BOLD, g->nodes[startNode].name, COLOR_RESET);

    visited[startNode] = 1;
    queue[rear++] = startNode;
    int reachableCount = 0;

    while (front < rear) {
        int u = queue[front++];
        reachableCount++;
        printf("  [%2d] %-22s (%s)\n",
               g->nodes[u].id, g->nodes[u].name, g->nodes[u].type);

        Edge *curr = g->nodes[u].head;
        while (curr) {
            int v = curr->dest;
            if (!curr->blocked && g->nodes[v].exists && !visited[v]) {
                visited[v] = 1;
                queue[rear++] = v;
            }
            curr = curr->next;
        }
    }

    printf("\nTotal places reachable: %d / %d active locations\n", reachableCount, g->count);

    /* Report unreachable locations */
    if (reachableCount < g->count) {
        printf("%sUnreachable places from here:%s\n", COLOR_YELLOW, COLOR_RESET);
        for (int i = 0; i < MAX_NODES; i++) {
            if (g->nodes[i].exists && !visited[i]) {
                printf("  - %s (ID: %d)\n", g->nodes[i].name, i);
            }
        }
    }
    printf("%s================================================%s\n", COLOR_BOLD, COLOR_RESET);
}

/**
 * Purpose:        Recursive helper for Depth-First Search.
 * Parameters:     g       - pointer to Graph
 *                 u       - current node ID
 *                 visited - visited status array
 *                 count   - counter for reachable nodes
 * Return:         void
 * Time Complexity: O(V + E)
 */
static void dfsHelper(const Graph *g, int u, int *visited, int *count) {
    visited[u] = 1;
    (*count)++;
    printf("  -> %s [%s]\n", g->nodes[u].name, g->nodes[u].type);

    Edge *curr = g->nodes[u].head;
    while (curr) {
        int v = curr->dest;
        if (!curr->blocked && g->nodes[v].exists && !visited[v]) {
            dfsHelper(g, v, visited, count);
        }
        curr = curr->next;
    }
}

/**
 * Purpose:        Depth-First Search (DFS) traversal of campus network.
 * Parameters:     g         - pointer to Graph
 *                 startNode - starting place ID
 * Return:         void
 * Time Complexity: O(V + E)
 */
void dfs(const Graph *g, int startNode) {
    if (!g || startNode < 0 || startNode >= MAX_NODES || !g->nodes[startNode].exists) {
        printf("%sError: Invalid start node for DFS.%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    int visited[MAX_NODES] = {0};
    int count = 0;

    printf("\n%s=== DFS CAMPUS CONNECTIVITY EXPLORATION ===%s\n", COLOR_BOLD, COLOR_RESET);
    printf("Starting Point: %s\n", g->nodes[startNode].name);

    dfsHelper(g, startNode, visited, &count);

    printf("\nDFS Traversal completed: %d places reached.\n", count);
    printf("%s================================================%s\n", COLOR_BOLD, COLOR_RESET);
}

/**
 * Purpose:        Find nearest places of a specified category using Dijkstra.
 * Parameters:     g    - pointer to Graph
 *                 src  - student's current location ID
 *                 type - category to find (e.g., "canteen", "library", "hostel")
 * Return:         void
 * Time Complexity: O((V + E) log V + V log V)
 */
void nearestByType(const Graph *g, int src, const char *type) {
    if (!g || !type || src < 0 || src >= MAX_NODES || !g->nodes[src].exists) {
        printf("%sError: Invalid parameters for nearest place search.%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    int dist[MAX_NODES];
    int parent[MAX_NODES];

    /* Single Dijkstra pass computes shortest paths to ALL places on campus */
    dijkstra(g, src, -1, dist, parent);

    /* Gather all places matching the requested type */
    typedef struct {
        int id;
        int distance;
    } ResultItem;

    ResultItem results[MAX_NODES];
    int matchCount = 0;

    char targetType[TYPE_LEN];
    toLowerCase(targetType, type);

    for (int i = 0; i < MAX_NODES; i++) {
        if (g->nodes[i].exists && i != src && dist[i] != INF) {
            char currType[TYPE_LEN];
            toLowerCase(currType, g->nodes[i].type);
            if (strcmp(currType, targetType) == 0) {
                results[matchCount].id = i;
                results[matchCount].distance = dist[i];
                matchCount++;
            }
        }
    }

    if (matchCount == 0) {
        printf("\n%sNo reachable '%s' places found from %s.%s\n",
               COLOR_YELLOW, type, g->nodes[src].name, COLOR_RESET);
        return;
    }

    /* Sort matches ascending by distance (Selection Sort) */
    for (int i = 0; i < matchCount - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < matchCount; j++) {
            if (results[j].distance < results[minIdx].distance) {
                minIdx = j;
            }
        }
        if (minIdx != i) {
            ResultItem tmp = results[i];
            results[i] = results[minIdx];
            results[minIdx] = tmp;
        }
    }

    printf("\n%s=== NEAREST '%s' LOCATIONS FROM %s ===%s\n",
           COLOR_BOLD, type, g->nodes[src].name, COLOR_RESET);

    for (int i = 0; i < matchCount; i++) {
        int destId = results[i].id;
        int d = results[i].distance;
        double secs = d / WALKING_SPEED;
        int mins = (int)(secs / 60);

        printf("  %s#%d: %-22s%s | Dist: %s%4dm%s | Walk: ~%d min(s)\n",
               (i == 0 ? COLOR_GREEN : COLOR_CYAN),
               i + 1,
               g->nodes[destId].name,
               COLOR_RESET,
               COLOR_YELLOW, d, COLOR_RESET,
               mins);
    }
    printf("%s=================================================%s\n", COLOR_BOLD, COLOR_RESET);
}

/**
 * Purpose:        Calculate an alternate detour route when a road is blocked.
 * Parameters:     g              - pointer to Graph
 *                 src            - origin location ID
 *                 dest           - destination location ID
 *                 blockU, blockV - road endpoints to block
 * Return:         void
 * Time Complexity: O((V + E) log V)
 */
void alternateRoute(Graph *g, int src, int dest, int blockU, int blockV) {
    if (!g || src < 0 || src >= MAX_NODES || dest < 0 || dest >= MAX_NODES) return;

    printf("\n%s[ROAD CLOSURE SIMULATION]%s\n", COLOR_BOLD, COLOR_RESET);
    printf("Simulating closure on road: %s <---> %s\n",
           g->nodes[blockU].name, g->nodes[blockV].name);

    /* Temporarily mark road as blocked */
    setRoadBlocked(g, blockU, blockV, 1);

    int dist[MAX_NODES];
    int parent[MAX_NODES];

    dijkstra(g, src, dest, dist, parent);

    printf("\nAlternate detour route calculated:\n");
    printPath(g, src, dest, parent, dist);

    /* Restore road to open state */
    setRoadBlocked(g, blockU, blockV, 0);
    printf("\n%s[NOTE]%s Road %s <-> %s reopened after calculation.\n",
           COLOR_GREEN, COLOR_RESET, g->nodes[blockU].name, g->nodes[blockV].name);
}
