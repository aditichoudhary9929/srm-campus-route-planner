/**
 * ============================================================================
 * server.js - Native Node.js HTTP Server & Campus API
 * SRM IST Delhi-NCR Campus Route Planner
 * Zero External Dependencies (Runs purely on Node.js standard library)
 * ============================================================================
 */

const http = require('http');
const fs = require('fs');
const path = require('path');
const url = require('url');

const { CampusGraph } = require('./public/dsa.js');
const { DEFAULT_NODES, DEFAULT_EDGES } = require('./public/map-data.js');

const PORT = process.env.PORT || 3000;
const PUBLIC_DIR = path.join(__dirname, 'public');

// MIME type dictionary
const MIME_TYPES = {
    '.html': 'text/html; charset=UTF-8',
    '.css': 'text/css; charset=UTF-8',
    '.js': 'application/javascript; charset=UTF-8',
    '.json': 'application/json; charset=UTF-8',
    '.png': 'image/png',
    '.jpg': 'image/jpeg',
    '.svg': 'image/svg+xml',
    '.ico': 'image/x-icon',
    '.txt': 'text/plain; charset=UTF-8'
};

// Initialize server-side Graph instance from map.txt if present
const serverGraph = new CampusGraph();

function loadServerMap() {
    try {
        const mapFilePath = path.join(__dirname, 'map.txt');
        if (fs.existsSync(mapFilePath)) {
            const content = fs.readFileSync(mapFilePath, 'utf8');
            const lines = content.split(/\r?\n/);
            let section = '';

            for (const line of lines) {
                const trimmed = line.trim();
                if (!trimmed) continue;
                if (trimmed.startsWith('# nodes')) {
                    section = 'nodes';
                    continue;
                }
                if (trimmed.startsWith('# edges')) {
                    section = 'edges';
                    continue;
                }

                if (section === 'nodes') {
                    const parts = trimmed.split(/\s+/);
                    if (parts.length >= 3) {
                        const id = parseInt(parts[0], 10);
                        const name = parts[1];
                        const type = parts[2];
                        const defaultRef = DEFAULT_NODES.find(n => n.id === id);
                        const x = defaultRef ? defaultRef.x : 500;
                        const y = defaultRef ? defaultRef.y : 400;
                        serverGraph.addNode(id, name, type, x, y);
                    }
                } else if (section === 'edges') {
                    const parts = trimmed.split(/\s+/);
                    if (parts.length >= 3) {
                        const u = parseInt(parts[0], 10);
                        const v = parseInt(parts[1], 10);
                        const w = parseInt(parts[2], 10);
                        serverGraph.addEdge(u, v, w);
                    }
                }
            }
            console.log(`[DSA Server] Loaded ${serverGraph.nodes.size} nodes and ${serverGraph.edges.length} edges from map.txt`);
        }
    } catch (err) {
        console.warn('[DSA Server] Falling back to default campus dataset:', err.message);
        DEFAULT_NODES.forEach(n => serverGraph.addNode(n.id, n.name, n.type, n.x, n.y));
        DEFAULT_EDGES.forEach(e => serverGraph.addEdge(e.from, e.to, e.weight));
    }
}

loadServerMap();

// Request Handler
const server = http.createServer((req, res) => {
    const parsedUrl = url.parse(req.url, true);
    const pathname = parsedUrl.pathname;

    // Enable CORS for API routes
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
    res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

    if (req.method === 'OPTIONS') {
        res.writeHead(204);
        res.end();
        return;
    }

    // ------------------------------------------------------------------------
    // API Endpoints
    // ------------------------------------------------------------------------
    if (pathname === '/api/health') {
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ status: 'ok', serverTime: new Date().toISOString() }));
        return;
    }

    if (pathname === '/api/map') {
        const nodesList = [];
        serverGraph.nodes.forEach(n => nodesList.push(n));
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({
            nodes: nodesList,
            edges: serverGraph.edges,
            stats: serverGraph.getStats()
        }));
        return;
    }

    if (pathname === '/api/stats') {
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify(serverGraph.getStats()));
        return;
    }

    // ------------------------------------------------------------------------
    // Static File Serving
    // ------------------------------------------------------------------------
    let safePath = path.normalize(pathname).replace(/^(\.\.[\/\\])+/, '');
    if (safePath === '/' || safePath === '\\') {
        safePath = '/index.html';
    }

    let filePath = path.join(PUBLIC_DIR, safePath);

    // If requesting map.txt directly
    if (safePath === '/map.txt') {
        filePath = path.join(__dirname, 'map.txt');
    }

    fs.stat(filePath, (err, stats) => {
        if (err || !stats.isFile()) {
            // Fallback to index.html for Single Page Navigation
            const indexPath = path.join(PUBLIC_DIR, 'index.html');
            fs.readFile(indexPath, (err2, content) => {
                if (err2) {
                    res.writeHead(404, { 'Content-Type': 'text/plain' });
                    res.end('404 Not Found');
                } else {
                    res.writeHead(200, { 'Content-Type': 'text/html; charset=UTF-8' });
                    res.end(content);
                }
            });
            return;
        }

        const ext = path.extname(filePath).toLowerCase();
        const contentType = MIME_TYPES[ext] || 'application/octet-stream';

        fs.readFile(filePath, (readErr, content) => {
            if (readErr) {
                res.writeHead(500, { 'Content-Type': 'text/plain' });
                res.end(`Internal Server Error: ${readErr.code}`);
            } else {
                res.writeHead(200, { 'Content-Type': contentType });
                res.end(content);
            }
        });
    });
});

// Start Server with fallback port handling
server.listen(PORT, () => {
    console.log('\n' + '='.repeat(68));
    console.log('   SRM INSTITUTE OF SCIENCE AND TECHNOLOGY, DELHI-NCR CAMPUS');
    console.log('            Campus Route Planner & Navigator v2.0');
    console.log('='.repeat(68));
    console.log(`\n  🚀 Web Application running at:  http://localhost:${PORT}`);
    console.log(`  🌐 Network Access URL:         http://127.0.0.1:${PORT}`);
    console.log(`  📡 Graph Data API:             http://localhost:${PORT}/api/map`);
    console.log(`  🗺️ Source Graph File:          ${path.join(__dirname, 'map.txt')}`);
    console.log(`  ⚙️ Admin Passcode:             srm123\n`);
    console.log('='.repeat(68) + '\n');
});

server.on('error', (err) => {
    if (err.code === 'EADDRINUSE') {
        console.log(`Port ${PORT} is busy, retrying on port ${Number(PORT) + 1}...`);
        server.listen(Number(PORT) + 1);
    } else {
        console.error('Server error:', err);
    }
});
