/**
 * ============================================================================
 * dsa.js - Core Data Structures & Algorithms Implementation
 * SRM IST Delhi-NCR Campus Route Planner
 * 
 * 1. MinHeap (Priority Queue for Dijkstra)
 * 2. Trie (Prefix-tree for instant place autocomplete & search)
 * 3. Graph (Adjacency List, Dijkstra, BFS, DFS, Nearest-by-Type, Detours)
 * 4. CircularQueue (Recent Route Searches history buffer)
 * ============================================================================
 */

// ============================================================================
// 1. Min-Heap Priority Queue (O(log V) push and pop)
// ============================================================================
class MinHeap {
    constructor() {
        this.heap = [];
    }

    push(nodeId, priority) {
        this.heap.push({ nodeId, priority });
        this._siftUp(this.heap.length - 1);
    }

    pop() {
        if (this.isEmpty()) return null;
        const root = this.heap[0];
        const last = this.heap.pop();
        if (this.heap.length > 0) {
            this.heap[0] = last;
            this._siftDown(0);
        }
        return root;
    }

    isEmpty() {
        return this.heap.length === 0;
    }

    _siftUp(idx) {
        while (idx > 0) {
            const parentIdx = Math.floor((idx - 1) / 2);
            if (this.heap[idx].priority < this.heap[parentIdx].priority) {
                [this.heap[idx], this.heap[parentIdx]] = [this.heap[parentIdx], this.heap[idx]];
                idx = parentIdx;
            } else {
                break;
            }
        }
    }

    _siftDown(idx) {
        const length = this.heap.length;
        while (true) {
            let left = 2 * idx + 1;
            let right = 2 * idx + 2;
            let smallest = idx;

            if (left < length && this.heap[left].priority < this.heap[smallest].priority) {
                smallest = left;
            }
            if (right < length && this.heap[right].priority < this.heap[smallest].priority) {
                smallest = right;
            }
            if (smallest !== idx) {
                [this.heap[idx], this.heap[smallest]] = [this.heap[smallest], this.heap[idx]];
                idx = smallest;
            } else {
                break;
            }
        }
    }
}

// ============================================================================
// 2. Trie Prefix Tree (O(L) Auto-complete search)
// ============================================================================
class TrieNode {
    constructor() {
        this.children = {};
        this.isEndOfWord = false;
        this.nodeIds = []; // Stores nodeIds that match or contain this word/prefix
    }
}

class Trie {
    constructor() {
        this.root = new TrieNode();
    }

    // Insert normalized word and map to node
    insert(word, nodeId) {
        let current = this.root;
        const clean = word.toLowerCase().replace(/[^a-z0-9]/g, '');
        for (const char of clean) {
            if (!current.children[char]) {
                current.children[char] = new TrieNode();
            }
            current = current.children[char];
            if (!current.nodeIds.includes(nodeId)) {
                current.nodeIds.push(nodeId);
            }
        }
        current.isEndOfWord = true;
    }

    // Autocomplete search: returns matching nodeIds
    suggest(prefix) {
        let current = this.root;
        const clean = prefix.toLowerCase().replace(/[^a-z0-9]/g, '');
        if (!clean) return [];

        for (const char of clean) {
            if (!current.children[char]) {
                return [];
            }
            current = current.children[char];
        }
        return current.nodeIds;
    }
}

// ============================================================================
// 3. Graph Data Structure & Algorithms
// ============================================================================
class CampusGraph {
    constructor() {
        this.nodes = new Map(); // id -> { id, name, type, x, y, active }
        this.adj = new Map();   // id -> Array<{ to, weight, blocked, id }>
        this.edges = [];        // Array<{ id, from, to, weight, blocked }>
        this.trie = new Trie();
        this.nextEdgeId = 1;
    }

    addNode(id, name, type, x, y) {
        this.nodes.set(id, {
            id,
            name,
            type: type || 'facility',
            x: x !== undefined ? x : 500,
            y: y !== undefined ? y : 400,
            active: true
        });

        if (!this.adj.has(id)) {
            this.adj.set(id, []);
        }

        // Index in Trie for fast autocomplete
        this.trie.insert(name, id);
        // Also index parts of name (e.g. "Library", "Hostel", "Sports")
        const tokens = name.split(/[_\s-]+/);
        for (const token of tokens) {
            if (token.length > 1) {
                this.trie.insert(token, id);
            }
        }
    }

    addEdge(from, to, weight) {
        if (!this.nodes.has(from) || !this.nodes.has(to)) return null;

        // Check if edge already exists
        const existing = this.edges.find(e => 
            (e.from === from && e.to === to) || (e.from === to && e.to === from)
        );
        if (existing) {
            existing.weight = weight;
            return existing;
        }

        const edgeId = `e_${from}_${to}`;
        const edgeObj = { id: edgeId, from, to, weight, blocked: false };
        this.edges.push(edgeObj);

        this.adj.get(from).push({ to, weight, blocked: false, edgeId });
        this.adj.get(to).push({ to: from, weight, blocked: false, edgeId });
        return edgeObj;
    }

    toggleEdgeBlock(from, to) {
        const edge = this.edges.find(e => 
            (e.from === from && e.to === to) || (e.from === to && e.to === from)
        );
        if (!edge) return false;

        edge.blocked = !edge.blocked;

        // Update in adjacency lists
        const listA = this.adj.get(from) || [];
        for (const neighbor of listA) {
            if (neighbor.to === to) neighbor.blocked = edge.blocked;
        }
        const listB = this.adj.get(to) || [];
        for (const neighbor of listB) {
            if (neighbor.to === from) neighbor.blocked = edge.blocked;
        }
        return edge.blocked;
    }

    isEdgeBlocked(from, to) {
        const edge = this.edges.find(e => 
            (e.from === from && e.to === to) || (e.from === to && e.to === from)
        );
        return edge ? edge.blocked : false;
    }

    // ========================================================================
    // Dijkstra's Shortest Path Algorithm with Min-Heap
    // Time: O((V + E) log V), Space: O(V)
    // ========================================================================
    dijkstra(startId, endId, options = { avoidBlocked: true }) {
        if (!this.nodes.has(startId) || !this.nodes.has(endId)) {
            return { found: false, error: 'Start or end location not found' };
        }

        const dist = new Map();
        const prev = new Map();
        const visited = new Set();
        const exploredOrder = [];
        const pq = new MinHeap();

        for (const [id] of this.nodes) {
            dist.set(id, Infinity);
        }

        dist.set(startId, 0);
        pq.push(startId, 0);

        let ops = 0;

        while (!pq.isEmpty()) {
            ops++;
            const { nodeId: u, priority: d } = pq.pop();

            if (visited.has(u)) continue;
            visited.add(u);
            exploredOrder.push(u);

            if (u === endId) break; // Destination reached!

            const neighbors = this.adj.get(u) || [];
            for (const edge of neighbors) {
                const v = edge.to;
                if (!this.nodes.get(v)?.active) continue;
                if (options.avoidBlocked && edge.blocked) continue; // Skip blocked roads
                if (visited.has(v)) continue;

                const alt = d + edge.weight;
                if (alt < dist.get(v)) {
                    dist.set(v, alt);
                    prev.set(v, u);
                    pq.push(v, alt);
                }
            }
        }

        if (dist.get(endId) === Infinity) {
            return {
                found: false,
                startId,
                endId,
                exploredCount: visited.size,
                operations: ops,
                message: 'No accessible route found between selected points (roads may be blocked).'
            };
        }

        // Reconstruct path
        const path = [];
        let curr = endId;
        while (curr !== undefined) {
            path.unshift(curr);
            curr = prev.get(curr);
        }

        const totalDist = dist.get(endId);
        // Average walking speed ~1.25 m/s (~4.5 km/h)
        const walkSeconds = Math.round(totalDist / 1.25);
        // Calories ~ 0.05 kcal per meter for an average adult
        const calories = Math.round(totalDist * 0.048);

        return {
            found: true,
            startId,
            endId,
            path,
            distance: totalDist,
            walkSeconds,
            calories,
            exploredCount: visited.size,
            operations: ops,
            exploredOrder
        };
    }

    // ========================================================================
    // Multi-stop Route Planning (A -> B -> C -> ...)
    // ========================================================================
    multiStopRoute(stopIds, options = { avoidBlocked: true }) {
        if (!stopIds || stopIds.length < 2) {
            return { found: false, error: 'At least 2 locations required for route planning' };
        }

        const legs = [];
        let combinedPath = [];
        let totalDistance = 0;
        let totalSeconds = 0;
        let totalCalories = 0;
        let totalExplored = 0;

        for (let i = 0; i < stopIds.length - 1; i++) {
            const legStart = stopIds[i];
            const legEnd = stopIds[i + 1];
            const res = this.dijkstra(legStart, legEnd, options);

            if (!res.found) {
                return {
                    found: false,
                    failedLeg: { from: legStart, to: legEnd },
                    message: `Route blocked or disconnected between leg ${i + 1} (${this.nodes.get(legStart)?.name} -> ${this.nodes.get(legEnd)?.name})`
                };
            }

            legs.push({
                from: legStart,
                to: legEnd,
                path: res.path,
                distance: res.distance,
                walkSeconds: res.walkSeconds,
                calories: res.calories
            });

            totalDistance += res.distance;
            totalSeconds += res.walkSeconds;
            totalCalories += res.calories;
            totalExplored += res.exploredCount;

            // Combine path nodes without repeating junction node
            if (i === 0) {
                combinedPath.push(...res.path);
            } else {
                combinedPath.push(...res.path.slice(1));
            }
        }

        return {
            found: true,
            isMultiStop: true,
            stops: stopIds,
            path: combinedPath,
            legs,
            distance: totalDistance,
            walkSeconds: totalSeconds,
            calories: totalCalories,
            exploredCount: totalExplored
        };
    }

    // ========================================================================
    // Find Nearest Place by Category
    // ========================================================================
    findNearestByType(startId, targetType, options = { avoidBlocked: true }) {
        if (!this.nodes.has(startId)) return null;

        const dist = new Map();
        const prev = new Map();
        const visited = new Set();
        const pq = new MinHeap();

        for (const [id] of this.nodes) dist.set(id, Infinity);
        dist.set(startId, 0);
        pq.push(startId, 0);

        let nearestNode = null;

        while (!pq.isEmpty()) {
            const { nodeId: u, priority: d } = pq.pop();
            if (visited.has(u)) continue;
            visited.add(u);

            const nodeObj = this.nodes.get(u);
            if (u !== startId && nodeObj && nodeObj.type.toLowerCase() === targetType.toLowerCase()) {
                nearestNode = nodeObj;
                break;
            }

            const neighbors = this.adj.get(u) || [];
            for (const edge of neighbors) {
                const v = edge.to;
                if (!this.nodes.get(v)?.active) continue;
                if (options.avoidBlocked && edge.blocked) continue;
                if (visited.has(v)) continue;

                const alt = d + edge.weight;
                if (alt < dist.get(v)) {
                    dist.set(v, alt);
                    prev.set(v, u);
                    pq.push(v, alt);
                }
            }
        }

        if (!nearestNode) return null;

        // Path to nearest node
        const path = [];
        let curr = nearestNode.id;
        while (curr !== undefined) {
            path.unshift(curr);
            curr = prev.get(curr);
        }

        const totalDist = dist.get(nearestNode.id);
        return {
            node: nearestNode,
            path,
            distance: totalDist,
            walkSeconds: Math.round(totalDist / 1.25),
            calories: Math.round(totalDist * 0.048)
        };
    }

    // ========================================================================
    // BFS - Breadth-First Search Traversal (Wave / Level-order)
    // ========================================================================
    bfs(startId) {
        if (!this.nodes.has(startId)) return [];

        const visited = new Set([startId]);
        const queue = [{ id: startId, level: 0 }];
        const result = [];
        const levels = new Map();

        while (queue.length > 0) {
            const { id: u, level } = queue.shift();
            result.push({ id: u, level });

            if (!levels.has(level)) levels.set(level, []);
            levels.get(level).push(u);

            const neighbors = (this.adj.get(u) || []).sort((a, b) => a.weight - b.weight);
            for (const edge of neighbors) {
                const v = edge.to;
                if (this.nodes.get(v)?.active && !visited.has(v)) {
                    visited.add(v);
                    queue.push({ id: v, level: level + 1 });
                }
            }
        }

        return { order: result, levels: Array.from(levels.entries()) };
    }

    // ========================================================================
    // DFS - Depth-First Search Traversal
    // ========================================================================
    dfs(startId) {
        if (!this.nodes.has(startId)) return [];

        const visited = new Set();
        const result = [];

        const traverse = (u, depth) => {
            visited.add(u);
            result.push({ id: u, depth });

            const neighbors = (this.adj.get(u) || []).sort((a, b) => a.weight - b.weight);
            for (const edge of neighbors) {
                const v = edge.to;
                if (this.nodes.get(v)?.active && !visited.has(v)) {
                    traverse(v, depth + 1);
                }
            }
        };

        traverse(startId, 0);
        return result;
    }

    // Get graph summary
    getStats() {
        return {
            totalNodes: this.nodes.size,
            totalEdges: this.edges.length,
            blockedEdges: this.edges.filter(e => e.blocked).length
        };
    }
}

// ============================================================================
// 4. Circular Queue for Recent Searches
// ============================================================================
class SearchHistoryQueue {
    constructor(capacity = 10) {
        this.capacity = capacity;
        this.buffer = new Array(capacity);
        this.head = 0;
        this.tail = 0;
        this.count = 0;
    }

    enqueue(item) {
        const entry = {
            ...item,
            timestamp: new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' })
        };

        if (this.count === this.capacity) {
            this.head = (this.head + 1) % this.capacity; // Evict oldest
        } else {
            this.count++;
        }

        this.buffer[this.tail] = entry;
        this.tail = (this.tail + 1) % this.capacity;
        this._persist();
    }

    getAll() {
        const items = [];
        for (let i = 0; i < this.count; i++) {
            const idx = (this.head + i) % this.capacity;
            items.push(this.buffer[idx]);
        }
        return items.reverse(); // Most recent first
    }

    clear() {
        this.head = 0;
        this.tail = 0;
        this.count = 0;
        this.buffer = new Array(this.capacity);
        this._persist();
    }

    _persist() {
        try {
            if (typeof localStorage !== 'undefined') {
                localStorage.setItem('srm_nav_history', JSON.stringify(this.getAll()));
            }
        } catch (e) {
            // Ignore storage errors in non-browser env
        }
    }

    load() {
        try {
            if (typeof localStorage !== 'undefined') {
                const data = localStorage.getItem('srm_nav_history');
                if (data) {
                    const parsed = JSON.parse(data);
                    // Add in reverse to maintain order
                    parsed.reverse().forEach(item => this.enqueue(item));
                }
            }
        } catch (e) {
            // Ignore parse errors
        }
    }
}

// Export for node or browser
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { MinHeap, Trie, CampusGraph, SearchHistoryQueue };
}
