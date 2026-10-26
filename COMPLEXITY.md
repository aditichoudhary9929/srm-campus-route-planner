# Time & Space Complexity Analysis
## SRM IST Delhi-NCR Campus Navigation & Route Planner

This document provides a rigorous theoretical and empirical complexity analysis of every data structure and algorithm utilized in the project.

---

## 📊 Comprehensive Complexity Table

| Feature / Operation | Algorithm / Data Structure | Time Complexity (Best) | Time Complexity (Average) | Time Complexity (Worst) | Space Complexity |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Shortest Path** | Dijkstra with Binary Min-Heap | $O(E \log V)$ | $O((V + E) \log V)$ | $O((V + E) \log V)$ | $O(V + E)$ |
| **Priority Queue Push** | Min-Heap `heapifyUp` | $O(1)$ | $O(\log N)$ | $O(\log N)$ | $O(1)$ amortized |
| **Priority Queue Pop** | Min-Heap `heapifyDown` | $O(\log N)$ | $O(\log N)$ | $O(\log N)$ | $O(1)$ |
| **Path Reconstruction** | Array-based Stack | $O(1)$ | $O(K)$ *(path length)* | $O(V)$ | $O(V)$ |
| **Place Name Lookup** | Hash Table (`djb2` + Chaining) | $O(1)$ | $O(1)$ | $O(N)$ *(collision)* | $O(M + N)$ |
| **Prefix Autocomplete** | 128-Way Trie | $O(P)$ *(prefix length)* | $O(P + K)$ *(matches)* | $O(P + \Sigma \cdot L)$ | $O(\Sigma \cdot L \cdot N)$ |
| **Reachability Walk** | Breadth-First Search (Queue) | $O(V + E)$ | $O(V + E)$ | $O(V + E)$ | $O(V)$ |
| **Connectivity Walk** | Depth-First Search (Recursive) | $O(V + E)$ | $O(V + E)$ | $O(V + E)$ | $O(V)$ *(call stack)* |
| **Nearest by Category** | Single Dijkstra + Selection Sort | $O((V + E) \log V)$ | $O((V + E) \log V + C^2)$ | $O((V + E) \log V + V^2)$ | $O(V)$ |
| **Road Block / Detour** | Edge flag toggle + Dijkstra | $O((V + E) \log V)$ | $O((V + E) \log V)$ | $O((V + E) \log V)$ | $O(V + E)$ |
| **Recent History Entry** | Circular Queue (Modulo Math) | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ *(fixed 5)* |
| **Recent History View** | Circular Queue Traversal | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ *(fixed 5)* |
| **Graph Map Load/Save** | Formatted Sequential File I/O | $O(V + E)$ | $O(V + E)$ | $O(V + E)$ | $O(V + E)$ |

---

## 🧠 Detailed Theoretical Justifications

### 1. Dijkstra’s Algorithm: Min-Heap vs. Adjacency Matrix
- **Naive Implementation (Linear Scan):** Uses an array to find the minimum distance node in each step.
  $$\text{Time} = O(V^2 + E)$$
  For our sparse campus graph where $V \approx 16$ and $E \approx 22$, $V^2 = 256$.
- **Min-Heap Implementation (Our Project):**
  $$\text{Time} = O((V + E) \log V)$$
  Extracting the minimum element takes $O(\log V)$, and relaxing an edge takes $O(\log V)$ when pushed onto the heap. With lazy deletion, at most $E$ nodes enter the heap:
  $$O(E \log V + V \log V) = O((V + E) \log V)$$
  For our campus map: $(16 + 22) \cdot \log_2(16) \approx 38 \cdot 4 = 152$ operations.

### 2. Graph Storage: Adjacency List vs. Adjacency Matrix
- **Adjacency Matrix:** Requires a 2D array of size $V \times V$. Space is strictly $\Theta(V^2)$, wasting $O(V^2 - 2E)$ memory on empty cells ($0$).
- **Adjacency List (Our Project):** Stores an array of $V$ nodes, each pointing to a linked list of edges. Total memory is:
  $$\text{Space} = O(V + 2E)$$
  For a sparse walkway network ($E \ll V^2$), adjacency lists save over $85\%$ of graph memory footprint while allowing $O(\text{degree})$ neighbor iterations during Dijkstra relaxations.

### 3. Hash Table: djb2 Algorithm & Load Factor ($\alpha$)
The `djb2` hash function was designed by Dan Bernstein:
$$\text{hash}_{i} = (\text{hash}_{i-1} \times 33) + c = ((\text{hash}_{i-1} \ll 5) + \text{hash}_{i-1}) + c$$
- **Load Factor ($\alpha$):**
  $$\alpha = \frac{N}{M} = \frac{16\text{ locations}}{101\text{ buckets}} \approx 0.158$$
- Because the load factor $\alpha \ll 0.75$ and $101$ is a prime number, hash collisions are negligible ($0$ collisions observed in our tests), guaranteeing true $O(1)$ average time complexity for place name lookups.

### 4. Trie Autocomplete vs. Linear Substring Search
- **Linear String Search:** Scanning all $V$ locations for a prefix of length $P$:
  $$\text{Time} = O(V \cdot P)$$
- **Trie (Our Project):** Navigates down the prefix path character-by-character in $O(P)$ time, independent of graph size. The subsequent subtree traversal visits only nodes that actually match the prefix:
  $$\text{Time} = O(P + K)$$
  where $K$ is the number of descendants in the matching branch.

### 5. Circular Queue vs. Linear Array for Recent Searches
- **Linear Array Eviction:** Adding a 6th search requires evicting index $0$ and shifting elements $1 \dots 4$ left by one position:
  $$\text{Time} = O(\text{HISTORY\_SIZE})$$
- **Circular Queue (Our Project):** Maintains `front` and `count`. Eviction and insertion use modulo arithmetic:
  $$\text{insertIdx} = \text{front}; \quad \text{front} = (\text{front} + 1) \pmod 5$$
  $$\text{Time} = O(1)$$
  Zero memory reallocation, zero data shifting.
