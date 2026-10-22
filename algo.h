/**
 * ============================================================================
 * algo.h - Campus Graph Navigation Algorithms Interface
 * SRM IST Campus Navigation & Route Planner
 * ============================================================================
 */

#ifndef ALGO_H
#define ALGO_H

#include "common.h"
#include "heap.h"

/**
 * Dijkstra's shortest path algorithm using Min-Heap priority queue.
 * Populates dist[] and parent[] arrays for all reachable nodes.
 * Skips blocked edges and utilizes lazy deletion in the min-heap.
 * Returns shortest distance to dest (or INF if unreachable).
 */
int dijkstra(const Graph *g, int src, int dest, int *dist, int *parent);

/**
 * Reconstruct and display shortest path from src to dest using an array stack.
 * Displays step-by-step legs, distances, and walking time at 1.4 m/s.
 */
void printPath(const Graph *g, int src, int dest, const int *parent, const int *dist);

/**
 * Breadth-First Search (BFS) to display all reachable places level-by-level.
 */
void bfs(const Graph *g, int startNode);

/**
 * Depth-First Search (DFS) to explore campus connectivity and reachable paths.
 */
void dfs(const Graph *g, int startNode);

/**
 * Find the nearest place of a specific category (e.g., "canteen", "library").
 * Executes one Dijkstra pass, filters by category, sorts, and prints top choices.
 */
void nearestByType(const Graph *g, int src, const char *type);

/**
 * Calculate an alternate route when a road is under construction / blocked.
 */
void alternateRoute(Graph *g, int src, int dest, int blockU, int blockV);

#endif /* ALGO_H */
