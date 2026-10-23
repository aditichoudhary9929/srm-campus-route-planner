/**
 * ============================================================================
 * history.c - Circular Queue Implementation for Recent Route Searches
 * SRM IST Campus Navigation & Route Planner
 * ============================================================================
 */

#include "history.h"

/**
 * Purpose:        Initialize the circular queue to an empty state.
 * Parameters:     hq - pointer to HistoryQueue
 * Return:         void
 * Time Complexity: O(HISTORY_SIZE)
 */
void initHistory(HistoryQueue *hq) {
    if (!hq) return;
    hq->front = 0;
    hq->count = 0;
    for (int i = 0; i < HISTORY_SIZE; i++) {
        hq->items[i].srcName[0] = '\0';
        hq->items[i].destName[0] = '\0';
        hq->items[i].distance = 0;
    }
}

/**
 * Purpose:        Add a search record into the circular queue with FIFO overwrite.
 * Parameters:     hq   - pointer to HistoryQueue
 *                 src  - origin place name
 *                 dest - destination place name
 *                 dist - total route distance in metres
 * Return:         void
 * Time Complexity: O(1)
 */
void addHistory(HistoryQueue *hq, const char *src, const char *dest, int dist) {
    if (!hq || !src || !dest) return;

    int insertIdx;

    if (hq->count < HISTORY_SIZE) {
        /* Queue not yet full: append at end */
        insertIdx = (hq->front + hq->count) % HISTORY_SIZE;
        hq->count++;
    } else {
        /* Queue full: overwrite oldest item at front and advance front pointer */
        insertIdx = hq->front;
        hq->front = (hq->front + 1) % HISTORY_SIZE;
    }

    strncpy(hq->items[insertIdx].srcName, src, NAME_LEN - 1);
    hq->items[insertIdx].srcName[NAME_LEN - 1] = '\0';

    strncpy(hq->items[insertIdx].destName, dest, NAME_LEN - 1);
    hq->items[insertIdx].destName[NAME_LEN - 1] = '\0';

    hq->items[insertIdx].distance = dist;
}

/**
 * Purpose:        Display the last 5 searches from oldest to most recent.
 * Parameters:     hq - pointer to HistoryQueue
 * Return:         void
 * Time Complexity: O(HISTORY_SIZE) = O(1)
 */
void printHistory(const HistoryQueue *hq) {
    if (!hq) return;

    printf("\n%s=== RECENT ROUTE SEARCH HISTORY (LAST %d) ===%s\n",
           COLOR_BOLD, HISTORY_SIZE, COLOR_RESET);

    if (hq->count == 0) {
        printf("  %s(No route searches performed yet)%s\n", COLOR_GRAY, COLOR_RESET);
        printf("%s================================================%s\n", COLOR_BOLD, COLOR_RESET);
        return;
    }

    for (int i = 0; i < hq->count; i++) {
        int idx = (hq->front + i) % HISTORY_SIZE;
        int d = hq->items[idx].distance;
        double secs = d / WALKING_SPEED;
        int mins = (int)(secs / 60);

        printf("  %s[%d]%s %-18s -> %-18s | %s%4dm%s (~%d min)\n",
               COLOR_CYAN, i + 1, COLOR_RESET,
               hq->items[idx].srcName,
               hq->items[idx].destName,
               COLOR_YELLOW, d, COLOR_RESET,
               mins);
    }
    printf("%s================================================%s\n", COLOR_BOLD, COLOR_RESET);
}

/**
 * Purpose:        Reset history queue contents.
 * Parameters:     hq - pointer to HistoryQueue
 * Return:         void
 * Time Complexity: O(1)
 */
void clearHistory(HistoryQueue *hq) {
    if (!hq) return;
    hq->front = 0;
    hq->count = 0;
}

/**
 * Purpose:        Return the number of stored search records.
 * Parameters:     hq - pointer to HistoryQueue
 * Return:         Integer count between 0 and HISTORY_SIZE
 * Time Complexity: O(1)
 */
int getHistoryCount(const HistoryQueue *hq) {
    return (hq ? hq->count : 0);
}
