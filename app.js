/**
 * ============================================================================
 * app.js - Interactive Campus Navigator Application Logic
 * SRM IST Delhi-NCR Campus Route Planner
 * ============================================================================
 */

document.addEventListener('DOMContentLoaded', () => {
    // ------------------------------------------------------------------------
    // State & Graph Initialization
    // ------------------------------------------------------------------------
    const graph = new CampusGraph();
    const historyQueue = new SearchHistoryQueue(10);
    historyQueue.load();

    let startNodeId = null;
    let endNodeId = null;
    let multiStops = [];
    let isMultiMode = false;
    let activeRouteData = null;
    let currentAdminUnlocked = false;
    let traversalTimer = null;

    // Map Viewport Pan & Zoom State
    let zoomLevel = 1;
    let panX = 0;
    let panY = 0;
    let isPanning = false;
    let startMouseX = 0;
    let startMouseY = 0;

    // Layer Visibilities
    let showNames = true;
    let showDistances = true;
    let showZones = true;

    // DOM Elements
    const svg = document.getElementById('campusSvg');
    const layerZones = document.getElementById('layerZones');
    const layerRoads = document.getElementById('layerRoads');
    const layerActiveRoute = document.getElementById('layerActiveRoute');
    const layerNodes = document.getElementById('layerNodes');
    const mapViewport = document.getElementById('mapViewport');

    const inputStart = document.getElementById('inputStart');
    const inputEnd = document.getElementById('inputEnd');
    const dropdownStart = document.getElementById('dropdownStart');
    const dropdownEnd = document.getElementById('dropdownEnd');
    const btnFindRoute = document.getElementById('btnFindRoute');
    const checkAvoidBlocked = document.getElementById('checkAvoidBlocked');
    const btnSwapPoints = document.getElementById('btnSwapPoints');
    const btnRandomRoute = document.getElementById('btnRandomRoute');
    const btnResetView = document.getElementById('btnResetView');

    const nodePopover = document.getElementById('nodePopover');
    let selectedPopoverNodeId = null;

    // ------------------------------------------------------------------------
    // Load Initial Campus Data
    // ------------------------------------------------------------------------
    function initializeGraphData() {
        DEFAULT_NODES.forEach(n => {
            graph.addNode(n.id, n.name, n.type, n.x, n.y);
            // Attach additional metadata
            const nodeObj = graph.nodes.get(n.id);
            nodeObj.label = n.label;
            nodeObj.icon = n.icon;
            nodeObj.desc = n.desc;
        });

        DEFAULT_EDGES.forEach(e => {
            graph.addEdge(e.from, e.to, e.weight);
        });

        populateSelectOptions();
        renderMap();
        renderRoadBlockList();
        renderHistoryList();
        updateTopologySpecs();
    }

    // ------------------------------------------------------------------------
    // Render SVG Map Components
    // ------------------------------------------------------------------------
    function renderMap() {
        renderZones();
        renderRoads();
        renderNodes();
    }

    // 1. Campus Zones
    function renderZones() {
        layerZones.innerHTML = '';
        if (!showZones) return;

        CAMPUS_ZONES.forEach(z => {
            const rect = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
            rect.setAttribute('x', z.x);
            rect.setAttribute('y', z.y);
            rect.setAttribute('width', z.w);
            rect.setAttribute('height', z.h);
            rect.setAttribute('fill', z.color);
            rect.setAttribute('stroke', z.border);
            rect.setAttribute('stroke-width', '1.5');
            rect.setAttribute('stroke-dasharray', '4 4');
            rect.setAttribute('class', 'svg-zone');
            layerZones.appendChild(rect);

            const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
            text.setAttribute('x', z.x + 14);
            text.setAttribute('y', z.y + 24);
            text.setAttribute('class', 'svg-zone-label');
            text.textContent = z.name;
            layerZones.appendChild(text);
        });
    }

    // 2. Roads (Edges)
    function renderRoads() {
        layerRoads.innerHTML = '';

        graph.edges.forEach(edge => {
            const u = graph.nodes.get(edge.from);
            const v = graph.nodes.get(edge.to);
            if (!u || !v || !u.active || !v.active) return;

            const g = document.createElementNS('http://www.w3.org/2000/svg', 'g');
            g.setAttribute('class', 'road-group');
            g.setAttribute('data-from', edge.from);
            g.setAttribute('data-to', edge.to);

            // Road Line
            const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
            line.setAttribute('x1', u.x);
            line.setAttribute('y1', u.y);
            line.setAttribute('x2', v.x);
            line.setAttribute('y2', v.y);
            line.setAttribute('class', `svg-road ${edge.blocked ? 'blocked' : ''}`);
            line.setAttribute('id', `road_${edge.from}_${edge.to}`);

            // Click road to toggle block in detour mode
            line.addEventListener('click', (ev) => {
                ev.stopPropagation();
                toggleRoadBlockState(edge.from, edge.to);
            });

            g.appendChild(line);

            // Distance Label
            if (showDistances) {
                const midX = (u.x + v.x) / 2;
                const midY = (u.y + v.y) / 2;

                const bgRect = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
                bgRect.setAttribute('x', midX - 16);
                bgRect.setAttribute('y', midY - 9);
                bgRect.setAttribute('width', 32);
                bgRect.setAttribute('height', 16);
                bgRect.setAttribute('rx', 4);
                bgRect.setAttribute('fill', 'rgba(11, 15, 25, 0.85)');
                bgRect.setAttribute('stroke', 'rgba(255, 255, 255, 0.1)');
                bgRect.setAttribute('stroke-width', '1');
                g.appendChild(bgRect);

                const label = document.createElementNS('http://www.w3.org/2000/svg', 'text');
                label.setAttribute('x', midX);
                label.setAttribute('y', midY + 3);
                label.setAttribute('class', 'svg-road-label');
                label.textContent = `${edge.weight}m`;
                g.appendChild(label);
            }

            layerRoads.appendChild(g);
        });
    }

    // 3. Landmark Nodes
    function renderNodes() {
        layerNodes.innerHTML = '';

        graph.nodes.forEach(node => {
            if (!node.active) return;

            const category = CAMPUS_CONFIG.categories[node.type] || CAMPUS_CONFIG.categories.facility;
            const g = document.createElementNS('http://www.w3.org/2000/svg', 'g');
            g.setAttribute('class', 'svg-node-group');
            g.setAttribute('id', `node_group_${node.id}`);
            g.setAttribute('transform', `translate(${node.x}, ${node.y})`);

            // Check highlight classes
            if (node.id === startNodeId) g.classList.add('is-start');
            if (node.id === endNodeId) g.classList.add('is-end');
            if (multiStops.includes(node.id)) g.classList.add('is-waypoint');

            // Outer Glow Ring
            const ring = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
            ring.setAttribute('r', '22');
            ring.setAttribute('class', 'svg-node-ring');
            ring.setAttribute('stroke', category.color);
            g.appendChild(ring);

            // Base Filled Circle
            const base = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
            base.setAttribute('r', '17');
            base.setAttribute('class', 'svg-node-base');
            base.setAttribute('fill', '#111827');
            base.setAttribute('stroke', category.color);
            base.setAttribute('stroke-width', '2');
            g.appendChild(base);

            // Icon
            const icon = document.createElementNS('http://www.w3.org/2000/svg', 'text');
            icon.setAttribute('class', 'svg-node-icon');
            icon.textContent = node.icon || category.icon || '📍';
            g.appendChild(icon);

            // Name Label
            if (showNames) {
                const label = document.createElementNS('http://www.w3.org/2000/svg', 'text');
                label.setAttribute('y', '32');
                label.setAttribute('class', 'svg-node-label');
                label.textContent = node.label || node.name.replace(/_/g, ' ');
                g.appendChild(label);
            }

            // Click Handler for Popover
            g.addEventListener('click', (ev) => {
                ev.stopPropagation();
                openNodePopover(node, ev);
            });

            layerNodes.appendChild(g);
        });
    }

    // ------------------------------------------------------------------------
    // Active Route Visual Path Overlay
    // ------------------------------------------------------------------------
    function drawActiveRoute(path) {
        layerActiveRoute.innerHTML = '';
        if (!path || path.length < 2) return;

        // Construct SVG path string through all route coordinates
        let d = '';
        for (let i = 0; i < path.length; i++) {
            const node = graph.nodes.get(path[i]);
            if (!node) continue;
            d += (i === 0 ? `M ${node.x} ${node.y} ` : `L ${node.x} ${node.y} `);
        }

        // Glowing Background Route Path
        const pathGlow = document.createElementNS('http://www.w3.org/2000/svg', 'path');
        pathGlow.setAttribute('d', d);
        pathGlow.setAttribute('fill', 'none');
        pathGlow.setAttribute('stroke', 'rgba(56, 189, 248, 0.35)');
        pathGlow.setAttribute('stroke-width', '14');
        pathGlow.setAttribute('stroke-linecap', 'round');
        pathGlow.setAttribute('stroke-linejoin', 'round');
        layerActiveRoute.appendChild(pathGlow);

        // Core Glowing Route Line
        const pathLine = document.createElementNS('http://www.w3.org/2000/svg', 'path');
        pathLine.setAttribute('d', d);
        pathLine.setAttribute('fill', 'none');
        pathLine.setAttribute('class', 'svg-road active-path');
        pathLine.setAttribute('stroke-linejoin', 'round');
        layerActiveRoute.appendChild(pathLine);

        // Update node highlight classes
        document.querySelectorAll('.svg-node-group').forEach(el => el.classList.remove('in-path'));
        path.forEach(id => {
            const el = document.getElementById(`node_group_${id}`);
            if (el) el.classList.add('in-path');
        });
    }

    // ------------------------------------------------------------------------
    // Route Calculation (Dijkstra Execution)
    // ------------------------------------------------------------------------
    function calculateRoute() {
        const avoidBlocked = checkAvoidBlocked.checked;

        if (isMultiMode) {
            if (multiStops.length < 2) {
                alert('Please select at least 2 stops for a multi-stop route.');
                return;
            }
            const res = graph.multiStopRoute(multiStops, { avoidBlocked });
            displayRouteResult(res);
        } else {
            if (startNodeId === null || endNodeId === null) {
                alert('Please select both a Start and Destination landmark.');
                return;
            }
            if (startNodeId === endNodeId) {
                alert('Start and destination landmarks are the same!');
                return;
            }
            const res = graph.dijkstra(startNodeId, endNodeId, { avoidBlocked });
            displayRouteResult(res);
        }
    }

    function displayRouteResult(res) {
        const panel = document.getElementById('routeSummaryPanel');
        const valDist = document.getElementById('valDistance');
        const valTime = document.getElementById('valTime');
        const valCal = document.getElementById('valCalories');
        const valExplored = document.getElementById('valExploredNodes');
        const directionsList = document.getElementById('directionsList');

        if (!res.found) {
            panel.classList.add('active');
            layerActiveRoute.innerHTML = '';
            valDist.textContent = 'Blocked';
            valTime.textContent = '--';
            valCal.textContent = '--';
            valExplored.textContent = res.exploredCount || 0;
            directionsList.innerHTML = `
                <div style="color: #F87171; padding: 12px; font-size: 0.8rem; background: rgba(239, 68, 68, 0.1); border-radius: 8px;">
                    ⚠️ ${res.message || 'No accessible path found! Check if roads along this corridor are blocked.'}
                </div>
            `;
            return;
        }

        activeRouteData = res;
        drawActiveRoute(res.path);

        // Display Metrics
        valDist.textContent = `${res.distance} m`;
        const mins = Math.floor(res.walkSeconds / 60);
        const secs = res.walkSeconds % 60;
        valTime.textContent = mins > 0 ? `${mins}m ${secs}s` : `${secs}s`;
        valCal.textContent = `${res.calories} kcal`;
        valExplored.textContent = `${res.exploredCount || res.path.length} nodes`;

        // Generate Turn-by-Turn Directions
        directionsList.innerHTML = '';
        for (let i = 0; i < res.path.length; i++) {
            const curr = graph.nodes.get(res.path[i]);
            const stepDiv = document.createElement('div');
            stepDiv.className = 'direction-step';

            if (i === 0) {
                stepDiv.innerHTML = `
                    <span class="step-num">🟢</span>
                    <div>
                        <div style="font-weight: 600; color: #F1F5F9;">Start at ${curr.label || curr.name}</div>
                        <div style="color: var(--text-muted); font-size: 0.72rem;">Begin navigation journey</div>
                    </div>
                `;
            } else if (i === res.path.length - 1) {
                stepDiv.innerHTML = `
                    <span class="step-num">🏁</span>
                    <div>
                        <div style="font-weight: 600; color: #34D399;">Arrive at ${curr.label || curr.name}</div>
                        <div style="color: var(--text-muted); font-size: 0.72rem;">Destination reached (${res.distance}m total)</div>
                    </div>
                `;
            } else {
                const prev = graph.nodes.get(res.path[i - 1]);
                const edge = graph.edges.find(e => (e.from === prev.id && e.to === curr.id) || (e.from === curr.id && e.to === prev.id));
                const legDist = edge ? edge.weight : 0;

                stepDiv.innerHTML = `
                    <span class="step-num">${i}.</span>
                    <div>
                        <div style="font-weight: 600; color: #F1F5F9;">Walk towards ${curr.label || curr.name}</div>
                        <div style="color: var(--text-muted); font-size: 0.72rem;">Follow path for ${legDist} meters</div>
                    </div>
                `;
            }
            directionsList.appendChild(stepDiv);
        }

        panel.classList.add('active');

        // Add to Circular Search History Queue
        const startName = graph.nodes.get(res.path[0])?.label || 'Origin';
        const endName = graph.nodes.get(res.path[res.path.length - 1])?.label || 'Dest';
        historyQueue.enqueue({
            fromName: startName,
            toName: endName,
            fromId: res.path[0],
            toId: res.path[res.path.length - 1],
            distance: res.distance,
            path: res.path
        });
        renderHistoryList();
    }

    // ------------------------------------------------------------------------
    // Trie Autocomplete & Dropdowns
    // ------------------------------------------------------------------------
    function setupAutocomplete(inputEl, dropdownEl, onSelect) {
        inputEl.addEventListener('input', () => {
            const query = inputEl.value.trim();
            if (!query) {
                dropdownEl.classList.remove('open');
                dropdownEl.innerHTML = '';
                return;
            }

            // Combine Trie suggestions and substring matches
            const trieMatches = graph.trie.suggest(query);
            const matchesSet = new Set(trieMatches);

            // Fallback substring search across all active nodes
            graph.nodes.forEach(n => {
                if (!n.active) return;
                const cleanQuery = query.toLowerCase();
                if (n.name.toLowerCase().includes(cleanQuery) || 
                    (n.label && n.label.toLowerCase().includes(cleanQuery)) ||
                    n.id.toString() === cleanQuery) {
                    matchesSet.add(n.id);
                }
            });

            const results = Array.from(matchesSet);
            if (results.length === 0) {
                dropdownEl.innerHTML = `<div style="padding: 10px; font-size: 0.78rem; color: var(--text-subtle); text-align: center;">No matching campus location</div>`;
                dropdownEl.classList.add('open');
                return;
            }

            dropdownEl.innerHTML = '';
            results.slice(0, 7).forEach(id => {
                const node = graph.nodes.get(id);
                if (!node) return;

                const item = document.createElement('div');
                item.className = 'autocomplete-item';
                item.innerHTML = `
                    <div style="display: flex; align-items: center; gap: 8px;">
                        <span>${node.icon || '📍'}</span>
                        <span style="font-weight: 500;">${node.label || node.name}</span>
                    </div>
                    <span class="type-tag">${node.type}</span>
                `;

                item.addEventListener('click', () => {
                    inputEl.value = node.label || node.name;
                    dropdownEl.classList.remove('open');
                    onSelect(node.id);
                });

                dropdownEl.appendChild(item);
            });

            dropdownEl.classList.add('open');
        });

        // Close dropdown when clicking outside
        document.addEventListener('click', (e) => {
            if (!inputEl.contains(e.target) && !dropdownEl.contains(e.target)) {
                dropdownEl.classList.remove('open');
            }
        });
    }

    setupAutocomplete(inputStart, dropdownStart, (id) => {
        startNodeId = id;
        renderNodes();
    });

    setupAutocomplete(inputEnd, dropdownEnd, (id) => {
        endNodeId = id;
        renderNodes();
    });

    // Global Search Autocomplete
    const inputGlobalSearch = document.getElementById('inputGlobalSearch');
    const dropdownGlobalSearch = document.getElementById('dropdownGlobalSearch');
    const searchResultCard = document.getElementById('searchResultCard');
    let selectedSearchResultId = null;

    setupAutocomplete(inputGlobalSearch, dropdownGlobalSearch, (id) => {
        selectedSearchResultId = id;
        const node = graph.nodes.get(id);
        if (node) {
            document.getElementById('searchResultName').textContent = `${node.icon || '📍'} ${node.label || node.name}`;
            document.getElementById('searchResultDesc').textContent = node.desc || `Campus Landmark (${node.type})`;
            searchResultCard.style.display = 'block';
            centerOnNode(node.x, node.y);
        }
    });

    document.getElementById('btnSearchSetStart').addEventListener('click', () => {
        if (selectedSearchResultId !== null) {
            setStartNode(selectedSearchResultId);
            switchToTab('tab-route');
        }
    });

    document.getElementById('btnSearchSetEnd').addEventListener('click', () => {
        if (selectedSearchResultId !== null) {
            setEndNode(selectedSearchResultId);
            switchToTab('tab-route');
        }
    });

    // ------------------------------------------------------------------------
    // Node Popover Card
    // ------------------------------------------------------------------------
    function openNodePopover(node, ev) {
        selectedPopoverNodeId = node.id;
        const category = CAMPUS_CONFIG.categories[node.type] || CAMPUS_CONFIG.categories.facility;

        document.getElementById('popoverIcon').textContent = node.icon || category.icon || '📍';
        document.getElementById('popoverTitle').textContent = node.label || node.name;
        document.getElementById('popoverBadge').textContent = category.label || node.type;
        document.getElementById('popoverBadge').style.background = category.bg;
        document.getElementById('popoverBadge').style.color = category.color;
        document.getElementById('popoverDesc').textContent = node.desc || `SRM IST campus landmark`;

        // Position popover relative to map viewport
        const rect = mapViewport.getBoundingClientRect();
        const pt = svg.createSVGPoint();
        pt.x = node.x;
        pt.y = node.y;
        const screenPt = pt.matrixTransform(svg.getScreenCTM());

        nodePopover.style.left = `${screenPt.x - rect.left}px`;
        nodePopover.style.top = `${screenPt.y - rect.top}px`;
        nodePopover.classList.add('open');
    }

    document.addEventListener('click', (e) => {
        if (!nodePopover.contains(e.target)) {
            nodePopover.classList.remove('open');
        }
    });

    document.getElementById('btnPopoverSetStart').addEventListener('click', () => {
        if (selectedPopoverNodeId !== null) {
            setStartNode(selectedPopoverNodeId);
            nodePopover.classList.remove('open');
            switchToTab('tab-route');
        }
    });

    document.getElementById('btnPopoverSetEnd').addEventListener('click', () => {
        if (selectedPopoverNodeId !== null) {
            setEndNode(selectedPopoverNodeId);
            nodePopover.classList.remove('open');
            switchToTab('tab-route');
        }
    });

    document.getElementById('btnPopoverAddWaypoint').addEventListener('click', () => {
        if (selectedPopoverNodeId !== null) {
            addMultiStop(selectedPopoverNodeId);
            nodePopover.classList.remove('open');
            switchToTab('tab-route');
            setRouteMode(true);
        }
    });

    function setStartNode(id) {
        startNodeId = id;
        const n = graph.nodes.get(id);
        inputStart.value = n ? (n.label || n.name) : '';
        renderNodes();
    }

    function setEndNode(id) {
        endNodeId = id;
        const n = graph.nodes.get(id);
        inputEnd.value = n ? (n.label || n.name) : '';
        renderNodes();
    }

    // ------------------------------------------------------------------------
    // Nearest Place Locator
    // ------------------------------------------------------------------------
    const btnFindNearest = document.getElementById('btnFindNearest');
    const selectNearestFrom = document.getElementById('selectNearestFrom');
    const selectNearestType = document.getElementById('selectNearestType');
    const nearestResultBox = document.getElementById('nearestResultBox');
    let currentNearestResult = null;

    btnFindNearest.addEventListener('click', () => {
        const fromId = parseInt(selectNearestFrom.value, 10);
        const targetType = selectNearestType.value;
        const res = graph.findNearestByType(fromId, targetType, { avoidBlocked: checkAvoidBlocked.checked });

        if (!res) {
            alert(`No reachable ${targetType} found from selected location.`);
            nearestResultBox.style.display = 'none';
            return;
        }

        currentNearestResult = res;
        document.getElementById('nearestResultName').textContent = `${res.node.icon || '📍'} ${res.node.label || res.node.name}`;
        document.getElementById('nearestResultDist').textContent = `Distance: ${res.distance}m (~${Math.round(res.walkSeconds / 60)} min walk)`;
        nearestResultBox.style.display = 'block';

        // Draw temporary path to nearest
        drawActiveRoute(res.path);
    });

    document.getElementById('btnNavigateToNearest').addEventListener('click', () => {
        if (currentNearestResult) {
            setStartNode(parseInt(selectNearestFrom.value, 10));
            setEndNode(currentNearestResult.node.id);
            switchToTab('tab-route');
            calculateRoute();
        }
    });

    // ------------------------------------------------------------------------
    // Graph Traversals (BFS / DFS Animation)
    // ------------------------------------------------------------------------
    let traversalType = 'bfs';
    const btnChooseBFS = document.getElementById('btnChooseBFS');
    const btnChooseDFS = document.getElementById('btnChooseDFS');
    const btnStartTraversal = document.getElementById('btnStartTraversal');
    const btnStopTraversal = document.getElementById('btnStopTraversal');
    const selectTraversalStart = document.getElementById('selectTraversalStart');
    const traversalProgressBox = document.getElementById('traversalProgressBox');
    const traversalStepBadge = document.getElementById('traversalStepBadge');
    const traversalSequenceChips = document.getElementById('traversalSequenceChips');

    btnChooseBFS.addEventListener('click', () => {
        traversalType = 'bfs';
        btnChooseBFS.style.background = 'rgba(139, 92, 246, 0.2)';
        btnChooseBFS.style.borderColor = 'rgba(139, 92, 246, 0.4)';
        btnChooseDFS.style.background = '';
        btnChooseDFS.style.borderColor = '';
    });

    btnChooseDFS.addEventListener('click', () => {
        traversalType = 'dfs';
        btnChooseDFS.style.background = 'rgba(139, 92, 246, 0.2)';
        btnChooseDFS.style.borderColor = 'rgba(139, 92, 246, 0.4)';
        btnChooseBFS.style.background = '';
        btnChooseBFS.style.borderColor = '';
    });

    btnStartTraversal.addEventListener('click', () => {
        clearInterval(traversalTimer);
        const startId = parseInt(selectTraversalStart.value, 10);
        const traversalResult = traversalType === 'bfs' ? graph.bfs(startId).order : graph.dfs(startId);

        // Reset visual state
        document.querySelectorAll('.svg-node-group').forEach(el => el.classList.remove('bfs-visited', 'in-path'));
        layerActiveRoute.innerHTML = '';
        traversalProgressBox.style.display = 'block';
        traversalSequenceChips.innerHTML = '';

        let step = 0;
        traversalTimer = setInterval(() => {
            if (step >= traversalResult.length) {
                clearInterval(traversalTimer);
                return;
            }

            const item = traversalResult[step];
            const node = graph.nodes.get(item.id);
            if (node) {
                const el = document.getElementById(`node_group_${node.id}`);
                if (el) el.classList.add('bfs-visited');

                const chip = document.createElement('span');
                chip.style.cssText = `
                    font-size: 0.72rem;
                    padding: 3px 8px;
                    border-radius: 999px;
                    background: rgba(168, 85, 247, 0.2);
                    border: 1px solid rgba(168, 85, 247, 0.4);
                    color: #C084FC;
                    font-weight: 500;
                `;
                chip.textContent = `${step + 1}. ${node.label || node.name}`;
                traversalSequenceChips.appendChild(chip);
            }

            step++;
            traversalStepBadge.textContent = `Step ${step}/${traversalResult.length}`;
        }, 350);
    });

    btnStopTraversal.addEventListener('click', () => {
        clearInterval(traversalTimer);
        document.querySelectorAll('.svg-node-group').forEach(el => el.classList.remove('bfs-visited'));
    });

    // ------------------------------------------------------------------------
    // Road Blockades & Detours
    // ------------------------------------------------------------------------
    function toggleRoadBlockState(from, to) {
        const isBlocked = graph.toggleEdgeBlock(from, to);
        renderRoads();
        renderRoadBlockList();

        // Update badge
        const blockedCount = graph.edges.filter(e => e.blocked).length;
        document.getElementById('blockedCountBadge').textContent = `${blockedCount} Blocked`;

        // If active route passes through this edge, automatically recalculate detour!
        if (activeRouteData && activeRouteData.found) {
            const path = activeRouteData.path;
            for (let i = 0; i < path.length - 1; i++) {
                if ((path[i] === from && path[i + 1] === to) || (path[i] === to && path[i + 1] === from)) {
                    calculateRoute();
                    break;
                }
            }
        }
    }

    function renderRoadBlockList() {
        const container = document.getElementById('roadBlockListContainer');
        container.innerHTML = '';

        graph.edges.forEach(edge => {
            const u = graph.nodes.get(edge.from);
            const v = graph.nodes.get(edge.to);
            if (!u || !v) return;

            const row = document.createElement('div');
            row.style.cssText = `
                display: flex;
                align-items: center;
                justify-content: space-between;
                padding: 7px 10px;
                background: rgba(15, 23, 42, 0.4);
                border: 1px solid var(--border-glass);
                border-radius: var(--radius-sm);
                font-size: 0.78rem;
            `;

            row.innerHTML = `
                <div>
                    <span style="font-weight: 500;">${u.name} &harr; ${v.name}</span>
                    <span style="color: var(--text-subtle); margin-left: 6px;">(${edge.weight}m)</span>
                </div>
                <button class="btn-header" style="padding: 2px 8px; font-size: 0.72rem; ${edge.blocked ? 'background: rgba(244, 63, 94, 0.2); border-color: rgba(244, 63, 94, 0.4); color: #FB7185;' : ''}">
                    ${edge.blocked ? '🚧 Blocked' : 'Open'}
                </button>
            `;

            row.querySelector('button').addEventListener('click', () => {
                toggleRoadBlockState(edge.from, edge.to);
            });

            container.appendChild(row);
        });
    }

    document.getElementById('btnClearAllBlocks').addEventListener('click', () => {
        graph.edges.forEach(e => {
            if (e.blocked) graph.toggleEdgeBlock(e.from, e.to);
        });
        renderRoads();
        renderRoadBlockList();
        document.getElementById('blockedCountBadge').textContent = '0 Blocked';
        if (startNodeId !== null && endNodeId !== null) calculateRoute();
    });

    document.getElementById('btnBlockDemoRoad').addEventListener('click', () => {
        // Block road between Academic Block A (1) and Academic Block B (2)
        toggleRoadBlockState(1, 2);
    });

    // ------------------------------------------------------------------------
    // Search History List (Circular Queue)
    // ------------------------------------------------------------------------
    function renderHistoryList() {
        const container = document.getElementById('historyListContainer');
        const items = historyQueue.getAll();

        if (items.length === 0) {
            container.innerHTML = `<div style="text-align: center; color: var(--text-subtle); font-size: 0.8rem; padding: 20px;">No recent searches yet</div>`;
            return;
        }

        container.innerHTML = '';
        items.forEach(item => {
            const card = document.createElement('div');
            card.style.cssText = `
                padding: 10px;
                background: rgba(15, 23, 42, 0.5);
                border: 1px solid var(--border-glass);
                border-radius: var(--radius-sm);
                cursor: pointer;
                transition: all 0.2s ease;
            `;

            card.innerHTML = `
                <div style="display: flex; justify-content: space-between; font-size: 0.72rem; color: var(--text-subtle); margin-bottom: 4px;">
                    <span>Route Query</span>
                    <span>${item.timestamp}</span>
                </div>
                <div style="font-weight: 600; font-size: 0.84rem; color: #F1F5F9;">
                    ${item.fromName} &rarr; ${item.toName}
                </div>
                <div style="font-size: 0.74rem; color: #38BDF8; margin-top: 4px;">
                    ${item.distance} meters &bull; Click to reload
                </div>
            `;

            card.addEventListener('click', () => {
                setStartNode(item.fromId);
                setEndNode(item.toId);
                switchToTab('tab-route');
                calculateRoute();
            });

            container.appendChild(card);
        });
    }

    document.getElementById('btnClearHistory').addEventListener('click', () => {
        historyQueue.clear();
        renderHistoryList();
    });

    // ------------------------------------------------------------------------
    // Admin Panel (PIN: srm123)
    // ------------------------------------------------------------------------
    const btnAdminUnlock = document.getElementById('btnAdminUnlock');
    const inputAdminPin = document.getElementById('inputAdminPin');
    const btnSubmitAdminPin = document.getElementById('btnSubmitAdminPin');
    const adminLockedState = document.getElementById('adminLockedState');
    const adminUnlockedState = document.getElementById('adminUnlockedState');
    const btnAdminLockSession = document.getElementById('btnAdminLockSession');

    btnAdminUnlock.addEventListener('click', () => {
        switchToTab('tab-admin');
    });

    btnSubmitAdminPin.addEventListener('click', () => {
        const pin = inputAdminPin.value.trim();
        if (pin === 'srm123') {
            currentAdminUnlocked = true;
            adminLockedState.style.display = 'none';
            adminUnlockedState.style.display = 'block';
            btnAdminUnlock.textContent = '🔓 Admin Mode';
            populateEdgeSelects();
        } else {
            alert('Incorrect PIN! Access denied.');
        }
    });

    btnAdminLockSession.addEventListener('click', () => {
        currentAdminUnlocked = false;
        adminLockedState.style.display = 'block';
        adminUnlockedState.style.display = 'none';
        btnAdminUnlock.innerHTML = '<span>🔒</span> Admin Access';
        inputAdminPin.value = '';
    });

    // Add Node
    document.getElementById('btnAddNodeSubmit').addEventListener('click', () => {
        const name = document.getElementById('inputNewNodeName').value.trim();
        const type = document.getElementById('selectNewNodeType').value;
        if (!name) {
            alert('Please enter a location name');
            return;
        }

        const newId = Math.max(...Array.from(graph.nodes.keys())) + 1;
        // Place in campus layout area
        const x = 500 + Math.floor(Math.random() * 200 - 100);
        const y = 450 + Math.floor(Math.random() * 200 - 100);

        graph.addNode(newId, name, type, x, y);
        const nodeObj = graph.nodes.get(newId);
        nodeObj.label = name.replace(/_/g, ' ');

        document.getElementById('inputNewNodeName').value = '';
        populateSelectOptions();
        renderNodes();
        updateTopologySpecs();
        alert(`Landmark "${name}" added with ID ${newId}!`);
    });

    // Add Road
    document.getElementById('btnAddEdgeSubmit').addEventListener('click', () => {
        const from = parseInt(document.getElementById('selectEdgeFrom').value, 10);
        const to = parseInt(document.getElementById('selectEdgeTo').value, 10);
        const dist = parseInt(document.getElementById('inputEdgeWeight').value, 10);

        if (from === to) {
            alert('Cannot connect road to the same location');
            return;
        }
        if (isNaN(dist) || dist <= 0) {
            alert('Please enter a positive distance');
            return;
        }

        graph.addEdge(from, to, dist);
        renderRoads();
        renderRoadBlockList();
        updateTopologySpecs();
        alert(`Road connected between ${graph.nodes.get(from).name} and ${graph.nodes.get(to).name} (${dist}m)!`);
    });

    function populateEdgeSelects() {
        const fromSel = document.getElementById('selectEdgeFrom');
        const toSel = document.getElementById('selectEdgeTo');
        fromSel.innerHTML = '';
        toSel.innerHTML = '';

        graph.nodes.forEach(n => {
            if (!n.active) return;
            const opt1 = document.createElement('option');
            opt1.value = n.id;
            opt1.textContent = n.label || n.name;
            fromSel.appendChild(opt1);

            const opt2 = document.createElement('option');
            opt2.value = n.id;
            opt2.textContent = n.label || n.name;
            toSel.appendChild(opt2);
        });
    }

    // ------------------------------------------------------------------------
    // Multi-Stop Routing Mode
    // ------------------------------------------------------------------------
    const btnModeSingle = document.getElementById('btnModeSingle');
    const btnModeMulti = document.getElementById('btnModeMulti');
    const singleRouteContainer = document.getElementById('singleRouteContainer');
    const multiRouteContainer = document.getElementById('multiRouteContainer');
    const inputAddStop = document.getElementById('inputAddStop');
    const dropdownAddStop = document.getElementById('dropdownAddStop');
    const stopsListContainer = document.getElementById('stopsListContainer');

    function setRouteMode(multi) {
        isMultiMode = multi;
        if (multi) {
            btnModeMulti.style.background = 'rgba(59, 130, 246, 0.2)';
            btnModeMulti.style.borderColor = 'rgba(59, 130, 246, 0.4)';
            btnModeSingle.style.background = '';
            btnModeSingle.style.borderColor = '';
            singleRouteContainer.style.display = 'none';
            multiRouteContainer.style.display = 'block';
            document.getElementById('routeModeBadge').textContent = 'Multi-Stop Dijkstra';

            // Seed with current start/end if available
            if (multiStops.length === 0) {
                if (startNodeId !== null) multiStops.push(startNodeId);
                if (endNodeId !== null) multiStops.push(endNodeId);
                renderStopsList();
            }
        } else {
            btnModeSingle.style.background = 'rgba(59, 130, 246, 0.2)';
            btnModeSingle.style.borderColor = 'rgba(59, 130, 246, 0.4)';
            btnModeMulti.style.background = '';
            btnModeMulti.style.borderColor = '';
            singleRouteContainer.style.display = 'block';
            multiRouteContainer.style.display = 'none';
            document.getElementById('routeModeBadge').textContent = 'Dijkstra + MinHeap';
        }
    }

    btnModeSingle.addEventListener('click', () => setRouteMode(false));
    btnModeMulti.addEventListener('click', () => setRouteMode(true));

    function addMultiStop(id) {
        if (!multiStops.includes(id)) {
            multiStops.push(id);
            renderStopsList();
            renderNodes();
        }
    }

    setupAutocomplete(inputAddStop, dropdownAddStop, (id) => {
        addMultiStop(id);
        inputAddStop.value = '';
    });

    function renderStopsList() {
        stopsListContainer.innerHTML = '';
        multiStops.forEach((id, idx) => {
            const node = graph.nodes.get(id);
            if (!node) return;

            const item = document.createElement('div');
            item.className = 'stop-item';
            item.innerHTML = `
                <span class="stop-badge">${idx + 1}</span>
                <span style="flex:1; font-size: 0.82rem; font-weight: 500;">${node.label || node.name}</span>
                <button class="btn-remove-stop" data-idx="${idx}">&times;</button>
            `;

            item.querySelector('.btn-remove-stop').addEventListener('click', () => {
                multiStops.splice(idx, 1);
                renderStopsList();
                renderNodes();
            });

            stopsListContainer.appendChild(item);
        });
    }

    // ------------------------------------------------------------------------
    // UI Helpers & Pan / Zoom Controls
    // ------------------------------------------------------------------------
    function populateSelectOptions() {
        selectNearestFrom.innerHTML = '';
        selectTraversalStart.innerHTML = '';

        graph.nodes.forEach(n => {
            if (!n.active) return;
            const opt1 = document.createElement('option');
            opt1.value = n.id;
            opt1.textContent = n.label || n.name;
            selectNearestFrom.appendChild(opt1);

            const opt2 = document.createElement('option');
            opt2.value = n.id;
            opt2.textContent = n.label || n.name;
            selectTraversalStart.appendChild(opt2);
        });
    }

    function updateTopologySpecs() {
        const stats = graph.getStats();
        const elN = document.getElementById('specTotalNodes');
        const elE = document.getElementById('specTotalEdges');
        if (elN) elN.textContent = stats.totalNodes;
        if (elE) elE.textContent = stats.totalEdges;
    }

    function switchToTab(tabId) {
        document.querySelectorAll('.tab-btn').forEach(btn => {
            btn.classList.toggle('active', btn.dataset.tab === tabId);
        });
        document.querySelectorAll('.tab-pane').forEach(pane => {
            pane.classList.toggle('active', pane.id === tabId);
        });
    }

    document.querySelectorAll('.sidebar-tabs .tab-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            switchToTab(btn.dataset.tab);
        });
    });

    // Swap Start and End
    btnSwapPoints.addEventListener('click', () => {
        const tempId = startNodeId;
        const tempVal = inputStart.value;

        setStartNode(endNodeId);
        inputStart.value = inputEnd.value;

        setEndNode(tempId);
        inputEnd.value = tempVal;

        if (startNodeId !== null && endNodeId !== null) calculateRoute();
    });

    // Random Route
    btnRandomRoute.addEventListener('click', () => {
        const nodeIds = Array.from(graph.nodes.keys()).filter(id => graph.nodes.get(id).active);
        if (nodeIds.length < 2) return;

        const randStart = nodeIds[Math.floor(Math.random() * nodeIds.length)];
        let randEnd = nodeIds[Math.floor(Math.random() * nodeIds.length)];
        while (randEnd === randStart) {
            randEnd = nodeIds[Math.floor(Math.random() * nodeIds.length)];
        }

        setStartNode(randStart);
        setEndNode(randEnd);
        switchToTab('tab-route');
        setRouteMode(false);
        calculateRoute();
    });

    btnFindRoute.addEventListener('click', calculateRoute);

    // Layer Toggles
    document.getElementById('toggleLayerNames').addEventListener('click', (e) => {
        showNames = !showNames;
        e.currentTarget.classList.toggle('active', showNames);
        renderNodes();
    });

    document.getElementById('toggleLayerDistances').addEventListener('click', (e) => {
        showDistances = !showDistances;
        e.currentTarget.classList.toggle('active', showDistances);
        renderRoads();
    });

    document.getElementById('toggleLayerZones').addEventListener('click', (e) => {
        showZones = !showZones;
        e.currentTarget.classList.toggle('active', showZones);
        renderZones();
    });

    // Zoom & Pan Functions
    function applyTransform() {
        svg.style.transform = `translate(${panX}px, ${panY}px) scale(${zoomLevel})`;
    }

    function centerOnNode(nx, ny) {
        zoomLevel = 1.3;
        panX = 0;
        panY = 0;
        applyTransform();
    }

    document.getElementById('btnZoomIn').addEventListener('click', () => {
        zoomLevel = Math.min(zoomLevel + 0.2, 2.5);
        applyTransform();
    });

    document.getElementById('btnZoomOut').addEventListener('click', () => {
        zoomLevel = Math.max(zoomLevel - 0.2, 0.6);
        applyTransform();
    });

    btnResetView.addEventListener('click', () => {
        zoomLevel = 1;
        panX = 0;
        panY = 0;
        applyTransform();
    });

    document.getElementById('btnCenterMap').addEventListener('click', () => {
        zoomLevel = 1;
        panX = 0;
        panY = 0;
        applyTransform();
    });

    // Pan with mouse drag
    mapViewport.addEventListener('mousedown', (e) => {
        if (e.target.closest('.control-btn') || e.target.closest('.layer-chip') || e.target.closest('.node-popover')) return;
        isPanning = true;
        startMouseX = e.clientX - panX;
        startMouseY = e.clientY - panY;
        mapViewport.classList.add('panning');
    });

    window.addEventListener('mousemove', (e) => {
        if (!isPanning) return;
        panX = e.clientX - startMouseX;
        panY = e.clientY - startMouseY;
        applyTransform();
    });

    window.addEventListener('mouseup', () => {
        isPanning = false;
        mapViewport.classList.remove('panning');
    });

    mapViewport.addEventListener('wheel', (e) => {
        e.preventDefault();
        const delta = e.deltaY < 0 ? 0.1 : -0.1;
        zoomLevel = Math.min(Math.max(0.6, zoomLevel + delta), 2.5);
        applyTransform();
    }, { passive: false });

    // ------------------------------------------------------------------------
    // Toast Notification & UI Enhancements
    // ------------------------------------------------------------------------
    const toastEl = document.getElementById('toastNotification');
    let toastTimeout = null;

    function showToast(message) {
        if (!toastEl) return;
        toastEl.textContent = message;
        toastEl.classList.add('show');
        clearTimeout(toastTimeout);
        toastTimeout = setTimeout(() => {
            toastEl.classList.remove('show');
        }, 3000);
    }

    // Share Route via Link
    const btnShareRoute = document.getElementById('btnShareRoute');
    if (btnShareRoute) {
        btnShareRoute.addEventListener('click', () => {
            let hash = '';
            if (isMultiMode && multiStops.length > 1) {
                hash = `#stops=${multiStops.join(',')}`;
            } else if (startNodeId !== null && endNodeId !== null) {
                hash = `#start=${startNodeId}&end=${endNodeId}`;
            }
            if (!hash) {
                showToast('Calculate a route first to share!');
                return;
            }
            const fullUrl = window.location.origin + window.location.pathname + hash;
            navigator.clipboard.writeText(fullUrl).then(() => {
                showToast('📋 Route link copied to clipboard!');
            }).catch(() => {
                showToast('Route: ' + fullUrl);
            });
        });
    }

    // Print Directions
    const btnPrintRoute = document.getElementById('btnPrintRoute');
    if (btnPrintRoute) {
        btnPrintRoute.addEventListener('click', () => {
            window.print();
        });
    }

    // Quick Search Hotkey (Ctrl + K / Cmd + K)
    const btnQuickSearch = document.getElementById('btnQuickSearch');
    function triggerQuickSearch() {
        switchToTab('tab-search');
        const inputGlobal = document.getElementById('inputGlobalSearch');
        if (inputGlobal) {
            inputGlobal.focus();
            inputGlobal.select();
        }
    }
    if (btnQuickSearch) btnQuickSearch.addEventListener('click', triggerQuickSearch);

    window.addEventListener('keydown', (e) => {
        if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'k') {
            e.preventDefault();
            triggerQuickSearch();
        }
    });

    // ------------------------------------------------------------------------
    // Animated Route Walk Simulation
    // ------------------------------------------------------------------------
    const layerWalker = document.getElementById('layerWalker');
    const simStatusText = document.getElementById('simStatusText');
    const simStatusIcon = document.getElementById('simStatusIcon');
    const btnSimulateWalk = document.getElementById('btnSimulateWalk');
    const btnPauseSimulate = document.getElementById('btnPauseSimulate');
    const btnResetSimulate = document.getElementById('btnResetSimulate');

    let isSimulating = false;
    let simStepIndex = 0;
    let simProgress = 0; // 0.0 to 1.0 along current segment
    let simAnimId = null;

    function startWalkingSimulation() {
        if (!activeRouteData || !activeRouteData.path || activeRouteData.path.length < 2) {
            showToast('Please compute a route first!');
            return;
        }

        isSimulating = true;
        layerWalker.style.display = 'block';
        btnSimulateWalk.style.display = 'none';
        btnPauseSimulate.style.display = 'inline-flex';
        simStatusText.textContent = 'Navigating...';
        simStatusIcon.textContent = '🏃';

        function stepAnimation() {
            if (!isSimulating) return;

            const path = activeRouteData.path;
            if (simStepIndex >= path.length - 1) {
                // Completed walk!
                isSimulating = false;
                btnSimulateWalk.style.display = 'inline-flex';
                btnPauseSimulate.style.display = 'none';
                simStatusText.textContent = 'Arrived at Destination!';
                simStatusIcon.textContent = '🏁';
                showToast('🎉 You have arrived at your destination!');
                return;
            }

            const uNode = graph.nodes.get(path[simStepIndex]);
            const vNode = graph.nodes.get(path[simStepIndex + 1]);

            if (uNode && vNode) {
                simProgress += 0.02; // speed factor
                if (simProgress >= 1.0) {
                    simProgress = 0;
                    simStepIndex++;
                }

                const currX = uNode.x + (vNode.x - uNode.x) * simProgress;
                const currY = uNode.y + (vNode.y - uNode.y) * simProgress;

                layerWalker.setAttribute('transform', `translate(${currX}, ${currY})`);

                const targetLabel = vNode.label || vNode.name;
                simStatusText.textContent = `Walking to ${targetLabel}...`;
            } else {
                simStepIndex++;
            }

            simAnimId = requestAnimationFrame(stepAnimation);
        }

        simAnimId = requestAnimationFrame(stepAnimation);
    }

    function pauseWalkingSimulation() {
        isSimulating = false;
        cancelAnimationFrame(simAnimId);
        btnSimulateWalk.style.display = 'inline-flex';
        btnPauseSimulate.style.display = 'none';
        simStatusText.textContent = 'Paused';
        simStatusIcon.textContent = '⏸️';
    }

    function resetWalkingSimulation() {
        pauseWalkingSimulation();
        simStepIndex = 0;
        simProgress = 0;
        if (activeRouteData && activeRouteData.path && activeRouteData.path.length > 0) {
            const firstNode = graph.nodes.get(activeRouteData.path[0]);
            if (firstNode) {
                layerWalker.setAttribute('transform', `translate(${firstNode.x}, ${firstNode.y})`);
            }
        }
        simStatusText.textContent = 'Ready to Simulate';
        simStatusIcon.textContent = '🚶';
    }

    if (btnSimulateWalk) btnSimulateWalk.addEventListener('click', startWalkingSimulation);
    if (btnPauseSimulate) btnPauseSimulate.addEventListener('click', pauseWalkingSimulation);
    if (btnResetSimulate) btnResetSimulate.addEventListener('click', resetWalkingSimulation);

    // ------------------------------------------------------------------------
    // URL Hash Routing Support (#start=0&end=3 or #stops=0,1,3)
    // ------------------------------------------------------------------------
    function parseUrlHashParams() {
        const hash = window.location.hash.substring(1);
        if (!hash) return false;

        const params = new URLSearchParams(hash);
        if (params.has('stops')) {
            const stopIds = params.get('stops').split(',').map(s => parseInt(s.trim(), 10)).filter(n => !isNaN(n) && graph.nodes.has(n));
            if (stopIds.length >= 2) {
                multiStops = stopIds;
                setRouteMode(true);
                renderStopsList();
                calculateRoute();
                return true;
            }
        } else if (params.has('start') && params.has('end')) {
            const s = parseInt(params.get('start'), 10);
            const e = parseInt(params.get('end'), 10);
            if (graph.nodes.has(s) && graph.nodes.has(e)) {
                setStartNode(s);
                setEndNode(e);
                setRouteMode(false);
                calculateRoute();
                return true;
            }
        }
        return false;
    }

    // ------------------------------------------------------------------------
    // Kickoff Application
    // ------------------------------------------------------------------------
    initializeGraphData();

    // Check Host Environment
    const hostPill = document.getElementById('hostStatusPill');
    if (hostPill) {
        if (window.location.hostname.includes('netlify.app')) {
            hostPill.textContent = 'Netlify Production';
        } else if (window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1') {
            hostPill.textContent = 'Localhost :3000 Active';
        } else {
            hostPill.textContent = 'Live Web App';
        }
    }

    // Apply URL Hash if present, otherwise load default pleasant route
    if (!parseUrlHashParams()) {
        setStartNode(0);  // Main Gate
        setEndNode(3);    // Central Library
        calculateRoute();
    }
});
