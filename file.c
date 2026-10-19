/**
 * ============================================================================
 * file.c - File Persistence Implementation for Campus Network Map
 * SRM IST Campus Navigation & Route Planner
 * ============================================================================
 */

#include "file.h"

enum ParseSection {
    SECTION_NONE,
    SECTION_NODES,
    SECTION_EDGES
};

/**
 * Purpose:        Load campus graph nodes and roads from text file.
 * Parameters:     g        - pointer to Graph to populate
 *                 ht       - optional pointer to HashTable (can be NULL)
 *                 trie     - optional pointer to Trie (can be NULL)
 *                 filename - path to map data file (e.g., "map.txt")
 * Return:         1 on successful loading, 0 on file open or parse failure
 * Time Complexity: O(V + E)
 */
int loadMap(Graph *g, HashTable *ht, TrieNode *trie, const char *filename) {
    if (!g || !filename) return 0;

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Warning: Could not open map file '%s' for reading.\n", filename);
        return 0;
    }

    initGraph(g);

    char line[256];
    int section = SECTION_NONE;

    while (fgets(line, sizeof(line), fp)) {
        trimNewline(line);

        /* Skip empty lines */
        if (line[0] == '\0') continue;

        /* Check section headers */
        if (strncmp(line, "# nodes", 7) == 0) {
            section = SECTION_NODES;
            continue;
        } else if (strncmp(line, "# edges", 7) == 0) {
            section = SECTION_EDGES;
            continue;
        } else if (line[0] == '#') {
            /* Other comment lines */
            continue;
        }

        if (section == SECTION_NODES) {
            int id = -1;
            char name[NAME_LEN];
            char type[TYPE_LEN];

            if (sscanf(line, "%d %63s %31s", &id, name, type) == 3) {
                if (id >= 0 && id < MAX_NODES) {
                    g->nodes[id].id = id;
                    strncpy(g->nodes[id].name, name, NAME_LEN - 1);
                    g->nodes[id].name[NAME_LEN - 1] = '\0';
                    strncpy(g->nodes[id].type, type, TYPE_LEN - 1);
                    g->nodes[id].type[TYPE_LEN - 1] = '\0';
                    g->nodes[id].exists = 1;
                    g->nodes[id].head = NULL;
                    g->count++;

                    /* Synchronize Hash Table and Trie if provided */
                    if (ht) {
                        insertName(ht, name, id);
                    }
                    if (trie) {
                        insertTrie(trie, name, id);
                    }
                }
            }
        } else if (section == SECTION_EDGES) {
            int u = -1, v = -1, dist = -1;
            if (sscanf(line, "%d %d %d", &u, &v, &dist) == 3) {
                if (dist > 0 && u >= 0 && u < MAX_NODES && v >= 0 && v < MAX_NODES) {
                    addRoad(g, u, v, dist);
                }
            }
        }
    }

    fclose(fp);
    return 1;
}

/**
 * Purpose:        Save campus graph nodes and roads to text file.
 * Parameters:     g        - pointer to Graph
 *                 filename - path to save destination
 * Return:         1 on success, 0 on failure
 * Time Complexity: O(V + E)
 */
int saveMap(const Graph *g, const char *filename) {
    if (!g || !filename) return 0;

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Error: Could not open file '%s' for writing.\n", filename);
        return 0;
    }

    /* Write nodes section */
    fprintf(fp, "# nodes: id name type\n");
    for (int i = 0; i < MAX_NODES; i++) {
        if (g->nodes[i].exists) {
            fprintf(fp, "%d %s %s\n", g->nodes[i].id, g->nodes[i].name, g->nodes[i].type);
        }
    }

    /* Write edges section (only u < dest to avoid duplicating undirected edges) */
    fprintf(fp, "# edges: from to distance\n");
    for (int i = 0; i < MAX_NODES; i++) {
        if (!g->nodes[i].exists) continue;

        Edge *curr = g->nodes[i].head;
        while (curr) {
            if (i < curr->dest && g->nodes[curr->dest].exists) {
                fprintf(fp, "%d %d %d\n", i, curr->dest, curr->weight);
            }
            curr = curr->next;
        }
    }

    fclose(fp);
    return 1;
}
