# SRM IST Campus Route Planner - Viva Voce Preparation Guide
## 15 High-Yield Questions & Model Answers for College Examiners

This guide contains the 15 most likely viva voce questions asked by Data Structures and Algorithms evaluators at SRM IST, paired with crisp, high-scoring answers.

---

### Q1. Why did you choose an Adjacency List over an Adjacency Matrix for representing the campus?
**Model Answer:**
> "A campus walkway network is a **sparse graph** where the number of roads ($E$) is much smaller than $V^2$. An adjacency matrix would consume $O(V^2)$ memory, wasting space on zeros. An adjacency list consumes only $O(V + E)$ memory and allows Dijkstra's algorithm to iterate directly over actual adjacent neighbors in $O(\text{degree})$ time rather than scanning all $V$ cells."

---

### Q2. What is the time complexity of Dijkstra's algorithm in your implementation?
**Model Answer:**
> "The time complexity is **$O((V + E) \log V)$**. Each vertex is extracted from the binary min-heap at most once ($V \log V$), and each road edge is relaxed at most once ($E \log V$). This is significantly faster than the naive $O(V^2)$ implementation."

---

### Q3. What is "Lazy Deletion" in your Min-Heap, and why did you use it?
**Model Answer:**
> "In standard priority queues, updating an existing node's distance requires a `decreaseKey` operation, which requires an auxiliary lookup map to locate the element's position inside the heap. 
> With **lazy deletion**, whenever a shorter path to node $v$ is found, we simply push a new pair $(v, \text{newDist})$ into the heap. When `popHeap()` extracts an element, we check if `d > dist[u]`. If it is, the entry is outdated (stale) and we discard it immediately. This keeps heap operations simple, robust, and $O(\log V)$."

---

### Q4. How does the `djb2` hash function work, and why does it multiply by 33?
**Model Answer:**
> "`djb2` is Dan Bernstein's classic hash function. It initializes `hash = 5381` and for every character computes `hash = ((hash << 5) + hash) + c`, which is equivalent to `hash * 33 + c`. 
> The number 33 has excellent bit-diffusion properties that minimize collisions for English strings and place names, and multiplying by 33 can be performed efficiently in hardware using a bitwise shift (`<< 5`) and addition."

---

### Q5. How does your Hash Table resolve collisions, and what is its load factor?
**Model Answer:**
> "We resolve collisions using **Separate Chaining**: each bucket in the array of size 101 points to a singly-linked list of `HEntry` structs. 
> With 16 campus locations and 101 prime buckets, our load factor is:
> $$\alpha = \frac{N}{M} = \frac{16}{101} \approx 0.158$$
> Because $\alpha \ll 0.75$, the probability of clustering is extremely low, giving us average $O(1)$ search and insertion."

---

### Q6. How does your Trie implement prefix autocomplete, and what is its time complexity?
**Model Answer:**
> "Each `TrieNode` has 128 child pointers corresponding to ASCII characters. To find places starting with a prefix of length $P$, we walk down $P$ steps from the root in $O(P)$ time. 
> Once at the prefix node, we perform a Depth-First Search (DFS) on its subtree to collect all descendant nodes with `isEnd == 1`. The total time is **$O(P + K)$**, where $K$ is the number of matching completions."

---

### Q7. How do you support case-insensitive lookups in the Trie and Hash Table?
**Model Answer:**
> "Before inserting or hashing, every character is converted to lowercase using `tolower((unsigned char)c)`. 
> For example, `'Main_Gate'`, `'main_gate'`, and `'MAIN_GATE'` produce the exact same `djb2` hash bucket and follow the exact same path down the Trie."

---

### Q8. Why did you use an array-based Stack to print the shortest path?
**Model Answer:**
> "Dijkstra's `parent[]` array stores predecessor pointers from the destination back to the source ($dest \to \dots \to src$). 
> To display turn-by-turn navigation in the natural forward direction ($src \to \dots \to dest$), we push the nodes onto a stack and then pop them. Because a stack is **Last-In, First-Out (LIFO)**, popping reverses the traversal order to forward chronological order in $O(K)$ time."

---

### Q9. How do you simulate a blocked road without destroying graph topology?
**Model Answer:**
> "Each `Edge` struct has an integer flag `int blocked;`. When construction occurs, `setRoadBlocked(g, u, v, 1)` sets this flag to 1 on both $(u \to v)$ and $(v \to u)$. 
> Traversal algorithms (Dijkstra, BFS, DFS) simply evaluate `if (edge->blocked) continue;` and skip the edge. This avoids deleting edges from the adjacency list and allows instantaneous unblocking."

---

### Q10. What happens if a destination is disconnected or completely unreachable?
**Model Answer:**
> "All distances in Dijkstra are initialized to `INF = 10^9`. If a node is in a disconnected component (such as `Extension_Site`), its distance remains `INF` after heap exhaustion. 
> `printPath()` checks `if (dist[dest] == INF)` and immediately prints:
> `No route exists between 'src' and 'dest' (Disconnected or road blocked)` 
> without crashing or causing an infinite loop."

---

### Q11. Why is a Circular Queue preferred over a normal linear queue for search history?
**Model Answer:**
> "In a normal array or linear queue, evicting the oldest search requires shifting the remaining 4 elements to the left, which takes $O(N)$ time. 
> In a **Circular Queue**, we maintain `front` and `count`. When the queue reaches capacity (5 items), we overwrite the slot at `front` and advance `front = (front + 1) % 5` in **$O(1)$ time** with zero memory shifting."

---

### Q12. How do you compute the estimated walking time?
**Model Answer:**
> "We use the standard pedestrian walking velocity constant of **$1.4\text{ m/s}$** ($\approx 5\text{ km/h}$). 
> Total seconds is calculated as `totalSeconds = distance / 1.4`. We then convert to minutes and seconds via integer division:
> `mins = totalSeconds / 60; secs = totalSeconds % 60;`"

---

### Q13. Why did you avoid `scanf()` and how is input sanitized?
**Model Answer:**
> "`scanf("%s", buffer)` does not perform bounds checking and causes buffer overflow vulnerabilities if input exceeds buffer capacity. Furthermore, residual newline characters (`\n`) remain in `stdin`, causing subsequent prompts to be skipped. 
> We implemented `promptString()` using `fgets()` with bounded buffers, followed by `trimNewline()`. This guarantees safe memory bounds and rejects empty inputs."

---

### Q14. How do you prove your program has zero memory leaks?
**Model Answer:**
> "Every module that allocates dynamic memory has a dedicated destructor:
> - `freeGraph()` traverses all adjacency lists and frees every `Edge`.
> - `freeHashTable()` iterates over 101 buckets and frees every `HEntry`.
> - `freeTrie()` performs a post-order recursive traversal freeing every `TrieNode`.
> - `freeHeap()` frees the dynamic heap buffer and structure.
> 
> All destructors are invoked on menu exit (Option 11) and inside test suites. We verify zero leaks using `-fsanitize=address,undefined`."

---

### Q15. How are the Graph, Hash Table, and Trie kept synchronized when adding or deleting locations?
**Model Answer:**
> "When a location is added, `addLocation()` assigns an ID, followed by `insertName()` into the Hash Table and `insertTrie()` into the Trie. 
> When a location is deleted:
> 1. `removeFromTrie()` unmarks the word in the Trie.
> 2. `removeName()` unlinks the entry from the Hash Table bucket.
> 3. `removeLocation()` frees all connected edges and sets `exists = 0` in the Graph.
> This guarantees that all three data structures remain 100% consistent."
