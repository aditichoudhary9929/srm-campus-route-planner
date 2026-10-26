# SRM IST Delhi-NCR Campus Navigation & Route Planner

An interactive campus navigation system implemented in **pure C (C11 standard library only, zero external dependencies)** for **SRM Institute of Science and Technology, Delhi-NCR Campus (Modinagar)**.

The project models the campus walkway and road network as an undirected weighted graph, integrating multiple core Data Structures and Algorithms (DSA) into a cohesive, production-grade terminal application.

---

## 🌟 Features

1. **Shortest Path Navigation:** Dijkstra's algorithm powered by a binary Min-Heap priority queue ($O((V + E) \log V)$), displaying step-by-step turn-by-turn directions, distances, and estimated walking times at $1.4\text{ m/s}$.
2. **Real-time Autocomplete Search:** 128-way Trie prefix tree providing fast prefix completion for campus buildings, canteens, and hostels.
3. **Instant Place Lookup:** Dan Bernstein's `djb2` hash function with 101 prime buckets and separate chaining for average $O(1)$ case-insensitive name-to-ID lookups.
4. **Nearest Facility Locator:** Single-pass Dijkstra filtering to find and rank the closest canteens, libraries, labs, hostels, or ATMs from any location on campus.
5. **Road Blockage & Detour Simulation:** Mark roads as closed for construction or events; algorithms automatically compute optimal detours without corrupting graph topology.
6. **Campus Connectivity Exploration:** Breadth-First Search (BFS) and Depth-First Search (DFS) traversals to analyze reachability and identify isolated campus sectors.
7. **Recent Search History:** Fixed-footprint Circular Queue recording the last 5 searched routes with $O(1)$ FIFO overwrite behavior.
8. **Multi-Stop Route Planning:** Chain multiple campus destinations ($A \to B \to C$) into a single itinerary.
9. **Interactive Map Editor:** Add or delete roads and locations live with automatic three-way synchronization across Graph, Hash Table, and Trie.
10. **File Persistence:** Complete map serialization to and from `map.txt`.

---

## 📐 Architecture & Module Design

```
                     +---------------------------------------+
                     |                main.c                 |
                     |   Interactive Menu & Safe Input (CLI) |
                     +---------------------------------------+
                                         |
         +-------------------------------+-------------------------------+
         |                               |                               |
+------------------+           +-------------------+           +-------------------+
|      algo.c      |           |      hash.c       |           |      trie.c       |
| Dijkstra, BFS,   |           |  djb2 Hash Table  |           | 128-Way Prefix    |
| DFS, Nearest     |           |  O(1) Name Lookup |           | Tree Autocomplete |
+------------------+           +-------------------+           +-------------------+
         |                               |                               |
         +---------------+---------------+-------------------------------+
                         |
+-------------------------------------------------+     +-------------------------+
|                    graph.c                      |     |        history.c        |
|  Adjacency List (Nodes & Bidirectional Edges)   |     | Circular Queue (Last 5) |
+-------------------------------------------------+     +-------------------------+
         |                                                       |
+-------------------------------------------------+              |
|                     file.c                      |              |
|  Disk Persistence (Load/Save from map.txt)      |              |
+-------------------------------------------------+              |
                         |                                       |
                         +-------------------+-------------------+
                                             |
                                  +---------------------+
                                  |      common.h       |
                                  | Core Structs & Defs |
                                  +---------------------+
```

---

## 🌐 Interactive Web Application (Localhost :3000)

The project includes an interactive, dark-mode visual Web Application that renders an SVG campus map with animated Dijkstra paths, live BFS/DFS wave visualizers, real-time road detour toggles, and Trie autocomplete search.

### Quick Start (Web on Localhost)
```powershell
# Option 1: Direct Double-click or terminal
.\start.bat

# Option 2: Native Node.js standard server (Zero npm dependencies needed)
node server.js
```
Open **[http://localhost:3000](http://localhost:3000)** in your browser.

- **Port:** `3000` (auto-fallbacks if occupied)
- **Live Graph API:** `http://localhost:3000/api/map`
- **Admin PIN:** `srm123`

---

## 🛠️ Build and Run Instructions (C CLI)

The project includes both a Windows batch script (`build.bat`) and a standard `Makefile`.

### 1. Build and Run the Main Application
```powershell
# Using build.bat (No make required)
.\build.bat
.\campus_nav.exe

# Or using GCC directly
gcc -std=c11 -Wall -Wextra -Wpedantic -g graph.c heap.c trie.c hash.c history.c algo.c file.c main.c -o campus_nav.exe
.\campus_nav.exe
```

### 2. Run the Automated Test Runner
```powershell
# Using build.bat test
.\build.bat test

# Or using GCC directly
gcc -std=c11 -Wall -Wextra -Wpedantic -I. -o test_runner.exe graph.c heap.c trie.c hash.c history.c algo.c file.c tests/test_main.c
.\test_runner.exe
```

### 3. Clean Build Artifacts
```powershell
.\build.bat clean
```

---

## 📄 File Format Specification (`map.txt`)

`map.txt` uses plain-text sections with comments (`#`):

```text
# nodes: id name type
0 Main_Gate gate
1 Academic_Block academic
2 Central_Library library
...

# edges: from to distance
0 1 180
1 2 80
...
```

- **Node IDs:** Contiguous integers from `0` to `MAX_NODES - 1`.
- **Names:** Alphanumeric strings without spaces (words separated by underscores `_`).
- **Types:** Category identifiers (e.g., `academic`, `canteen`, `hostel`, `lab`, `sports`, `admin`, `facility`, `parking`).
- **Edges:** Undirected roads specified by source, destination, and distance in metres. Edges are stored symmetrically in memory.

---

## 👥 4-Member Team Work-Split Breakdown

| Member | Primary Responsibility | Modules Owned | Key DSA Deliverables |
| :--- | :--- | :--- | :--- |
| **Member 1** | Graph Engine & File I/O | `graph.c`, `graph.h`, `file.c`, `file.h`, `map.txt` | Adjacency list representation, memory allocation/cleanup, undirected edge symmetry, file serialization |
| **Member 2** | Shortest Path & Detour Engine | `heap.c`, `heap.h`, `algo.c` (Dijkstra, Detours) | Array-based binary Min-Heap priority queue, lazy deletion relaxation, stack path reconstruction |
| **Member 3** | Fast Lookups & Autocomplete | `hash.c`, `hash.h`, `trie.c`, `trie.h` | `djb2` hash function with chaining, 128-way Trie prefix search with recursive DFS collection |
| **Member 4** | UI, Traversals & Test Automation | `history.c`, `history.h`, `main.c`, `algo.c` (BFS/DFS), `tests/test_main.c` | Circular queue search buffer, safe `fgets` CLI menu, BFS/DFS connectivity algorithms, assert test runner |
