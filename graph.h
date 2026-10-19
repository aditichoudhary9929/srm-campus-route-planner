/**
 * ============================================================================
 * graph.h - Graph Adjacency List Interface
 * SRM IST Campus Navigation & Route Planner
 * ============================================================================
 */

#ifndef GRAPH_H
#define GRAPH_H

#include "common.h"

/**
 * Initialize graph memory and reset node list.
 */
void initGraph(Graph *g);

/**
 * Add a new campus location.
 * Returns assigned node ID (0 to MAX_NODES-1) or -1 on error/overflow.
 */
int addLocation(Graph *g, const char *name, const char *type);

/**
 * Add an undirected weighted road between node u and node v.
 * Returns 1 on success, 0 on invalid parameters.
 */
int addRoad(Graph *g, int u, int v, int dist);

/**
 * Remove the road between node u and node v (in both directions).
 * Returns 1 on success, 0 if edge didn't exist.
 */
int removeRoad(Graph *g, int u, int v);

/**
 * Remove a location from the graph, removing all incident edges.
 * Returns 1 on success, 0 if node was already absent.
 */
int removeLocation(Graph *g, int u);

/**
 * Mark a road as blocked (construction/event) or unblocked.
 * Returns 1 on success, 0 if road not found.
 */
int setRoadBlocked(Graph *g, int u, int v, int blocked);

/**
 * Display the campus network as an adjacency list.
 */
void printGraph(const Graph *g);

/**
 * Free all dynamically allocated edge nodes in the adjacency list.
 */
void freeGraph(Graph *g);

#endif /* GRAPH_H */
