/**
 * ============================================================================
 * history.h - Circular Queue for Recent Searches Interface
 * SRM IST Campus Navigation & Route Planner
 * ============================================================================
 */

#ifndef HISTORY_H
#define HISTORY_H

#include "common.h"

/**
 * Initialize circular queue to empty state.
 */
void initHistory(HistoryQueue *hq);

/**
 * Add a new route search to the circular queue.
 * Automatically overwrites the oldest search when 5 slots are full.
 */
void addHistory(HistoryQueue *hq, const char *src, const char *dest, int dist);

/**
 * Display recent route search history formatted in the terminal.
 */
void printHistory(const HistoryQueue *hq);

/**
 * Reset/clear all stored search history.
 */
void clearHistory(HistoryQueue *hq);

/**
 * Get current count of recorded searches (0 to HISTORY_SIZE).
 */
int getHistoryCount(const HistoryQueue *hq);

#endif /* HISTORY_H */
