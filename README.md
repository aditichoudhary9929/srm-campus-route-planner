# SRM IST Delhi-NCR Campus Route Planner & Navigator

[![Live Demo on Netlify](https://img.shields.io/badge/Live%20Demo-srmrouteplanner.netlify.app-00C7B7?style=for-the-badge&logo=netlify&logoColor=white)](https://srmrouteplanner.netlify.app/)
[![GitHub Pages Mirror](https://img.shields.io/badge/GitHub%20Pages-Live%20Mirror-blue?style=for-the-badge&logo=github)](https://aditichoudhary9929.github.io/srm-campus-route-planner/)
[![Language: C11](https://img.shields.io/badge/Language-C11%20Standard-blue.svg?style=for-the-badge&logo=c)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg?style=for-the-badge)](LICENSE)

An interactive campus navigation system and graph algorithm visualizer built for **SRM Institute of Science and Technology, Delhi-NCR Campus (Modinagar)**. 

The project models the campus walkway and road network as an undirected, weighted graph, combining core Data Structures & Algorithms into both an interactive **modern web application** and a **pure C11 terminal application** with zero external runtime dependencies.

---

## 🌐 Live Deployments & Access

| Environment | URL | Details |
| :--- | :--- | :--- |
| 🚀 **Production (Netlify)** | **[https://srmrouteplanner.netlify.app/](https://srmrouteplanner.netlify.app/)** | Primary live deployment on global CDN with fast edge caching and SPA routing |
| 🌐 **Mirror (GitHub Pages)** | **[https://aditichoudhary9929.github.io/srm-campus-route-planner/](https://aditichoudhary9929.github.io/srm-campus-route-planner/)** | Secondary static backup deployment |
| 💻 **Localhost (Dev)** | `http://localhost:3000` | Native Node.js HTTP server (`node server.js` or `.\start.bat`) |
| 🖥️ **Desktop CLI** | `.\campus_nav.exe` | Compiled C11 interactive console interface |

---

## 🌟 Key Features

1. **Shortest Path Navigation (Dijkstra's Algorithm):** Powered by an array-based binary Min-Heap priority queue ($O((V + E) \log V)$). Calculates metric distance in meters, estimated walking time at $1.25\text{ m/s}$, calories burned, and step-by-step turn directions.
2. **Real-time Autocomplete Search (Trie):** Multi-word prefix tree ($O(L)$) offering instant search suggestions for campus buildings, hostels, canteens, and sports fields.
3. **Constant-Time Place Lookup (Hash Table):** Dan Bernstein's `djb2` hash algorithm with separate chaining across prime-sized buckets for average $O(1)$ case-insensitive name-to-ID lookups.
4. **Nearest Facility Locator:** Single-pass Dijkstra exploration that filters and ranks the closest amenities (canteen, hospital, hostel, sports, lab, academic block, ATM) from any starting point.
5. **Road Blockage & Detour Simulation:** Mark roads as closed for maintenance, events, or weather. The engine immediately recalculates and visually draws alternate detours in real time.
6. **Live Route Walking Simulation:** An animated pedestrian marker (`🚶`) navigates the path from origin to destination at realistic walking speeds with Play, Pause, and Reset controls.
7. **Graph Reachability (BFS & DFS):** Animated Breadth-First Search (Queue / wave) and Depth-First Search (Stack / backtrack) algorithms to visualize connected zones and radius reachability.
8. **Recent Search History (Circular Queue):** Fixed-footprint FIFO circular queue (capacity 10) for instant route recall without memory fragmentation.
9. **Multi-Stop Route Planning:** Chain multiple destinations ($A \to B \to C \to D$) into a continuous itinerary with segmented leg metrics.
10. **Shareable Route Links (URL Hash):** Instant URL hash encoding (`#start=0&end=3` or `#stops=0,1,3`) for one-click route sharing and bookmarking.
11. **Admin Campus Topology Editor:** Protected by security passcode (`srm123`) to add new landmarks, connect roads, or modify existing paths.
12. **Persistent Graph Storage:** Serialization to and from plain-text `map.txt` for synchronization across C and web interfaces.

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

## 📊 Algorithmic Complexity

| Module / Operation | Algorithm / Data Structure | Time Complexity | Space Complexity |
| :--- | :--- | :--- | :--- |
| **Shortest Path** | Dijkstra with Min-Heap | $O((V + E) \log V)$ | $O(V)$ |
| **Name Autocomplete** | 128-way Trie Prefix Tree | $O(L)$ ($L = \text{prefix length}$) | $O(\Sigma \cdot L \cdot N)$ |
| **Name Lookup** | `djb2` Hash Table + Chaining | Average $O(1)$, Worst $O(N)$ | $O(V)$ |
| **Connectivity Search** | Breadth-First Search (BFS) | $O(V + E)$ | $O(V)$ |
| **Zone Traversal** | Depth-First Search (DFS) | $O(V + E)$ | $O(V)$ |
| **Search History** | Circular Queue (Ring Buffer) | $O(1)$ Enqueue / Dequeue | $O(K)$ ($K = 10$) |
| **Nearest by Category** | Priority-Queue Dijkstra Filter | $O((V + E) \log V)$ | $O(V)$ |

*See [COMPLEXITY.md](COMPLEXITY.md) for full mathematical derivations and Big-O proofs.*

---

## 🚀 How to Use the Application

### 1. Using the Web Application ([https://srmrouteplanner.netlify.app/](https://srmrouteplanner.netlify.app/))

- **Find Shortest Route:**
  1. Select or type a **Starting Landmark** (e.g., `Main_Gate`).
  2. Select or type a **Destination Landmark** (e.g., `Central_Library`).
  3. Click **Calculate Optimal Route** (or press Enter).
  4. The route will be traced in glowing cyan on the SVG map with distance, walking time, calories burned, and turn-by-turn directions.
- **Simulate Route Walk:**
  - Click **▶️ Play** under the route statistics to watch a walking marker (`🚶`) navigate the route in real time.
- **Multi-Stop Itinerary:**
  - Switch to **Multi-Stop (A → B → C)** mode in the Route tab to add intermediate waypoints.
- **Test Road Detours:**
  - Open the **Detour** tab or click directly on any road line on the map to toggle it as **Blocked/Under Maintenance**. The engine will immediately recalculate the optimal alternative path.
- **Nearest Amenities:**
  - Open the **Search** tab, pick your current location and desired category (e.g., `🍔 Canteen` or `🏥 Hospital`), and click **Locate Nearest Facility**.
- **Share Route:**
  - Click **🔗 Share Route Link** to copy a direct URL containing encoded parameters (e.g. `#start=0&end=3`).

---

### 2. Admin Security & Passcode

Administrative features allow modifications to the campus graph topology without recompilation:

- **Default Passcode:** `srm123`
- **Capabilities Unlocked:**
  - **Add Landmark:** Add a new campus building with custom ID, name, category, and coordinates.
  - **Connect Road:** Create a new road edge between any two locations with custom distance in meters.
  - **Delete Landmark / Road:** Remove decommissioned walkways or buildings.
  - **Save & Synchronize:** Automatically serialize updates back to the graph dataset.

To access in the Web UI: Click **Admin Access** in the top navigation bar and enter `srm123`.  
To access in the C CLI: Choose Menu Option `1` (Add Location) or `2` (Add Road) and enter `srm123` when prompted.

---

### 3. Running Locally via Terminal

#### Web Server (Node.js)
```powershell
# Double-click start.bat OR run from terminal:
node server.js
```
Open [http://localhost:3000](http://localhost:3000) in your browser.

#### C CLI Application
```powershell
# Using the Windows batch script:
.\build.bat
.\campus_nav.exe

# Or compile manually with GCC:
gcc -std=c11 -Wall -Wextra -Wpedantic -g graph.c heap.c trie.c hash.c history.c algo.c file.c main.c -o campus_nav.exe
.\campus_nav.exe
```

#### Running Automated Tests
```powershell
.\build.bat test
.\test_runner.exe
```

---

## 🔒 Privacy & Security Notice

- **Zero Credentials in Repository:** This repository contains **zero private API keys, Personal Access Tokens, or private user data**. All tokens used during configuration were scrubbed before committing.
- **Client-Side First:** All graph routing, Trie autocomplete, and simulation algorithms run directly inside the user's browser, ensuring speed and data privacy.
- **Security Headers:** The web application is deployed with production security headers (`X-Frame-Options: DENY`, `X-Content-Type-Options: nosniff`, `Referrer-Policy: strict-origin-when-cross-origin`).

---

## 👥 4-Member Team Work-Split Breakdown

| Member | Primary Responsibility | Modules Owned | Key DSA Deliverables |
| :--- | :--- | :--- | :--- |
| **Member 1** | Graph Engine & File I/O | `graph.c`, `graph.h`, `file.c`, `file.h`, `map.txt` | Adjacency list representation, memory allocation/cleanup, undirected edge symmetry, file serialization |
| **Member 2** | Shortest Path & Detour Engine | `heap.c`, `heap.h`, `algo.c` (Dijkstra, Detours) | Array-based binary Min-Heap priority queue, lazy deletion relaxation, stack path reconstruction |
| **Member 3** | Fast Lookups & Autocomplete | `hash.c`, `hash.h`, `trie.c`, `trie.h` | `djb2` hash function with chaining, 128-way Trie prefix search with recursive DFS collection |
| **Member 4** | UI, Traversals & Web Platform | `history.c`, `main.c`, `algo.c` (BFS/DFS), `public/` web engine | Circular queue search buffer, safe `fgets` CLI menu, BFS/DFS reachability visualizer, SVG map renderer |

---

## 📚 Academic Documents

- [COMPLEXITY.md](COMPLEXITY.md) — Comprehensive asymptotic time and space complexity proofs.
- [VIVA.md](VIVA.md) — 25 anticipated Viva questions with detailed technical answers for oral evaluation.
