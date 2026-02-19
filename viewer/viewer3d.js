// ============================================================================
// EvoSim 3D Viewer — Three.js World Visualization
// Terrain from biomes, 3D archetype meshes, karma auras, orbit camera
// ============================================================================

import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';

// ── Archetype Colors & Geometries ───────────────────────────────────────────
const ARCHETYPE_CONFIG = {
    'Explorer': { color: 0x00e08e, geo: 'tetra' },
    'Warrior': { color: 0xff4466, geo: 'octa' },
    'Healer': { color: 0x44aaff, geo: 'sphere' },
    'Thinker': { color: 0xb366ff, geo: 'box' },
    'Builder': { color: 0xff9922, geo: 'dodeca' },
    'Prophet': { color: 0xffd700, geo: 'icosa' },
    'Unknown': { color: 0x888888, geo: 'sphere' }
};

const BIOME_COLORS = {
    'Ocean': 0x1a4a6e,
    'Plains': 0x3d8c4f,
    'Forest': 0x1a5c2a,
    'Desert': 0xc4a35a,
    'Swamp': 0x4a5c3d,
    'Mountain': 0x6e6e7a,
    'River': 0x2a7ab5,
    'Unknown': 0x333333
};

class EvoSim3D {
    constructor() {
        this.scene = null;
        this.camera = null;
        this.renderer = null;
        this.controls = null;
        this.clock = new THREE.Clock();

        // Simulation data
        this.agentData = {};      // tick -> [agents]
        this.worldData = {};      // tick -> world stats
        this.eventsData = [];     // [{tick, event, details}]
        this.soulData = [];       // soul deaths

        // State
        this.currentTick = 0;
        this.maxTick = 0;
        this.speed = 5;
        this.playing = false;
        this.frameAccum = 0;
        this.gridSize = 30;

        // 3D objects
        this.terrainGroup = new THREE.Group();
        this.agentMeshes = new Map(); // agentId -> mesh
        this.auraMeshes = new Map();
        this.particleSystem = null;
        this.selectedAgentId = null;

        this.init();
    }

    init() {
        const container = document.getElementById('viewport3d');

        // Scene
        this.scene = new THREE.Scene();
        this.scene.background = new THREE.Color(0x050a14);
        this.scene.fog = new THREE.FogExp2(0x050a14, 0.008);

        // Camera
        this.camera = new THREE.PerspectiveCamera(60, window.innerWidth / window.innerHeight, 0.1, 200);
        this.camera.position.set(15, 25, 35);
        this.camera.lookAt(15, 0, 15);

        // Renderer
        this.renderer = new THREE.WebGLRenderer({ antialias: true });
        this.renderer.setSize(window.innerWidth, window.innerHeight);
        this.renderer.setPixelRatio(window.devicePixelRatio);
        this.renderer.shadowMap.enabled = true;
        this.renderer.shadowMap.type = THREE.PCFSoftShadowMap;
        container.appendChild(this.renderer.domElement);

        // Controls
        this.controls = new OrbitControls(this.camera, this.renderer.domElement);
        this.controls.target.set(15, 0, 15);
        this.controls.enableDamping = true;
        this.controls.dampingFactor = 0.05;
        this.controls.maxPolarAngle = Math.PI / 2.1;
        this.controls.update();

        // Lights
        const ambientLight = new THREE.AmbientLight(0x334466, 0.6);
        this.scene.add(ambientLight);

        const dirLight = new THREE.DirectionalLight(0xffeedd, 0.8);
        dirLight.position.set(20, 30, 20);
        dirLight.castShadow = true;
        dirLight.shadow.mapSize.setScalar(1024);
        dirLight.shadow.camera.near = 0.5;
        dirLight.shadow.camera.far = 80;
        dirLight.shadow.camera.left = -40;
        dirLight.shadow.camera.right = 40;
        dirLight.shadow.camera.top = 40;
        dirLight.shadow.camera.bottom = -40;
        this.scene.add(dirLight);

        const hemiLight = new THREE.HemisphereLight(0x446688, 0x222233, 0.3);
        this.scene.add(hemiLight);

        // Grid helper (subtle)
        const gridHelper = new THREE.GridHelper(this.gridSize, this.gridSize, 0x1a2a3a, 0x0d1520);
        gridHelper.position.set(this.gridSize / 2, -0.01, this.gridSize / 2);
        this.scene.add(gridHelper);

        // Stars
        this.createStarfield();

        // Events
        window.addEventListener('resize', () => this.onResize());
        this.renderer.domElement.addEventListener('click', (e) => this.onClick(e));
        this.setupUI();

        // Animation loop
        this.animate();
    }

    createStarfield() {
        const starGeo = new THREE.BufferGeometry();
        const positions = [];
        for (let i = 0; i < 2000; i++) {
            positions.push(
                (Math.random() - 0.5) * 200,
                Math.random() * 80 + 10,
                (Math.random() - 0.5) * 200
            );
        }
        starGeo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        const starMat = new THREE.PointsMaterial({ color: 0xffffff, size: 0.15, transparent: true, opacity: 0.6 });
        this.scene.add(new THREE.Points(starGeo, starMat));
    }

    // ── Terrain ─────────────────────────────────────────────────────────────
    buildTerrain(worldTick) {
        // Remove old terrain
        this.scene.remove(this.terrainGroup);
        this.terrainGroup = new THREE.Group();

        // Parse biome data from the first tick's agents to infer grid size
        // For now, create a flat terrain colored by biome
        // We'll use the agent data to determine biome at each position

        const biomeGrid = {};
        const firstTick = Object.keys(this.agentData).map(Number).sort((a, b) => a - b)[0] || 1;
        const agents = this.agentData[firstTick] || [];

        // Build biome map from agent positions
        for (const a of agents) {
            const key = `${a.x},${a.y}`;
            if (a.biome) biomeGrid[key] = a.biome;
        }

        // Detect grid size from all agents
        let maxX = 0, maxY = 0;
        for (const tick of Object.keys(this.agentData)) {
            for (const a of this.agentData[tick]) {
                maxX = Math.max(maxX, a.x);
                maxY = Math.max(maxY, a.y);
            }
        }
        this.gridSize = Math.max(maxX, maxY, 20) + 1;

        // Create terrain tiles
        const tileGeo = new THREE.BoxGeometry(0.95, 0.2, 0.95);

        // Collect all biome info from all ticks
        for (const tick of Object.keys(this.agentData)) {
            for (const a of this.agentData[tick]) {
                const key = `${a.x},${a.y}`;
                if (a.biome && !biomeGrid[key]) biomeGrid[key] = a.biome;
            }
        }

        for (let x = 0; x < this.gridSize; x++) {
            for (let z = 0; z < this.gridSize; z++) {
                const biome = biomeGrid[`${x},${z}`] || 'Unknown';
                const color = BIOME_COLORS[biome] || BIOME_COLORS['Unknown'];

                let height = 0;
                if (biome === 'Mountain') height = 0.8;
                else if (biome === 'Ocean') height = -0.3;
                else if (biome === 'River') height = -0.1;
                else if (biome === 'Forest') height = 0.3;
                else if (biome === 'Desert') height = 0.1;

                const mat = new THREE.MeshStandardMaterial({
                    color,
                    roughness: 0.8,
                    metalness: 0.1
                });
                const tile = new THREE.Mesh(tileGeo, mat);
                tile.position.set(x, height / 2, z);
                tile.receiveShadow = true;
                this.terrainGroup.add(tile);
            }
        }

        this.scene.add(this.terrainGroup);

        // Re-center camera
        const center = this.gridSize / 2;
        this.controls.target.set(center, 0, center);
        this.camera.position.set(center, this.gridSize * 0.8, center + this.gridSize * 0.7);
        this.controls.update();
    }

    // ── Agent Meshes ────────────────────────────────────────────────────────
    createAgentMesh(agent) {
        const config = ARCHETYPE_CONFIG[agent.archetype] || ARCHETYPE_CONFIG['Unknown'];
        let geo;

        switch (config.geo) {
            case 'tetra': geo = new THREE.TetrahedronGeometry(0.35); break;
            case 'octa': geo = new THREE.OctahedronGeometry(0.35); break;
            case 'sphere': geo = new THREE.SphereGeometry(0.3, 12, 8); break;
            case 'box': geo = new THREE.BoxGeometry(0.45, 0.45, 0.45); break;
            case 'dodeca': geo = new THREE.DodecahedronGeometry(0.35); break;
            case 'icosa': geo = new THREE.IcosahedronGeometry(0.35); break;
            default: geo = new THREE.SphereGeometry(0.3, 8, 6); break;
        }

        const mat = new THREE.MeshStandardMaterial({
            color: config.color,
            emissive: config.color,
            emissiveIntensity: 0.3,
            roughness: 0.4,
            metalness: 0.6
        });

        const mesh = new THREE.Mesh(geo, mat);
        mesh.castShadow = true;
        mesh.userData.agentId = agent.id;

        // Karma aura (point light)
        const karmaColor = agent.karma >= 0 ? 0x00ff88 : 0xff4444;
        const intensity = Math.min(Math.abs(agent.karma) / 50, 2);
        if (intensity > 0.2) {
            const aura = new THREE.PointLight(karmaColor, intensity, 3);
            aura.position.set(0, 0.5, 0);
            mesh.add(aura);
            this.auraMeshes.set(agent.id, aura);
        }

        return mesh;
    }

    updateAgents(tick) {
        const agents = this.agentData[tick];
        if (!agents) return;

        const activeIds = new Set();

        for (const agent of agents) {
            activeIds.add(agent.id);

            let mesh = this.agentMeshes.get(agent.id);

            if (!mesh) {
                mesh = this.createAgentMesh(agent);
                this.scene.add(mesh);
                this.agentMeshes.set(agent.id, mesh);
            }

            // Position (smooth interpolation)
            const targetX = agent.x;
            const targetZ = agent.y;
            const height = this.getTerrainHeight(agent.x, agent.y);

            mesh.position.lerp(new THREE.Vector3(targetX, height + 0.5, targetZ), 0.3);

            // Floating animation
            mesh.position.y += Math.sin(Date.now() * 0.003 + agent.id) * 0.05;

            // Rotation animation
            mesh.rotation.y += 0.02;

            // Update karma aura
            const aura = this.auraMeshes.get(agent.id);
            if (aura) {
                aura.color.setHex(agent.karma >= 0 ? 0x00ff88 : 0xff4444);
                aura.intensity = Math.min(Math.abs(agent.karma) / 50, 2);
            }

            // Highlight selected
            if (agent.id === this.selectedAgentId) {
                mesh.material.emissiveIntensity = 0.8;
                mesh.scale.setScalar(1.3);
            } else {
                mesh.material.emissiveIntensity = 0.3;
                mesh.scale.setScalar(1.0);
            }

            // Tribe indicator: ring around tribemember
            if (agent.tribeId > 0 && !mesh.userData.tribeRing) {
                const ringGeo = new THREE.RingGeometry(0.5, 0.6, 16);
                const ringMat = new THREE.MeshBasicMaterial({
                    color: 0x64ffda,
                    transparent: true,
                    opacity: 0.4,
                    side: THREE.DoubleSide
                });
                const ring = new THREE.Mesh(ringGeo, ringMat);
                ring.rotation.x = -Math.PI / 2;
                ring.position.y = -0.3;
                mesh.add(ring);
                mesh.userData.tribeRing = ring;
            }
        }

        // Remove dead agents
        for (const [id, mesh] of this.agentMeshes) {
            if (!activeIds.has(id)) {
                this.scene.remove(mesh);
                this.agentMeshes.delete(id);
                this.auraMeshes.delete(id);
            }
        }
    }

    getTerrainHeight(x, y) {
        // Match biome heights from buildTerrain
        const agents = this.agentData[Object.keys(this.agentData)[0]] || [];
        for (const a of agents) {
            if (a.x === x && a.y === y && a.biome) {
                if (a.biome === 'Mountain') return 0.8;
                if (a.biome === 'Forest') return 0.3;
                if (a.biome === 'Desert') return 0.1;
                if (a.biome === 'Ocean') return -0.3;
                if (a.biome === 'River') return -0.1;
            }
        }
        return 0;
    }

    // ── CSV Parsing (reused from 2D viewer) ─────────────────────────────────
    parseAgentLog(csv) {
        const lines = csv.trim().split('\n');
        const header = lines[0].split(',');

        for (let i = 1; i < lines.length; i++) {
            const cols = lines[i].split(',');
            if (cols.length < 15) continue;

            const tick = parseInt(cols[1]);
            const agent = {
                id: parseInt(cols[0]),
                x: parseInt(cols[2]),
                y: parseInt(cols[3]),
                health: parseFloat(cols[4]),
                energy: parseFloat(cols[5]),
                satiety: parseFloat(cols[6]),
                kinship: parseFloat(cols[7]),
                status: parseFloat(cols[8]),
                curiosity: parseFloat(cols[9]),
                altruism: parseFloat(cols[10]),
                intel: parseFloat(cols[11]),
                age: parseInt(cols[12]),
                gen: parseInt(cols[13]),
                biome: cols[14],
                archetype: cols[15] || 'Unknown',
                karma: parseFloat(cols[16]) || 0,
                alignment: parseFloat(cols[17]) || 0,
                enlightenment: parseFloat(cols[18]) || 0,
                soulAge: parseInt(cols[19]) || 1,
                tribeId: parseInt(cols[20]) || -1,
                numMemes: parseInt(cols[21]) || 0
            };

            if (!this.agentData[tick]) this.agentData[tick] = [];
            this.agentData[tick].push(agent);
            this.maxTick = Math.max(this.maxTick, tick);
        }
    }

    parseWorldLog(csv) {
        const lines = csv.trim().split('\n');
        for (let i = 1; i < lines.length; i++) {
            const cols = lines[i].split(',');
            if (cols.length < 7) continue;
            const tick = parseInt(cols[0]);
            this.worldData[tick] = {
                pop: parseInt(cols[1]),
                infected: parseInt(cols[2]),
                dead: parseInt(cols[3]),
                avgAlt: parseFloat(cols[4]),
                avgCur: parseFloat(cols[5]),
                avgConsc: parseFloat(cols[6])
            };
        }
    }

    parseEventsLog(csv) {
        const lines = csv.trim().split('\n');
        for (let i = 1; i < lines.length; i++) {
            const cols = lines[i].split(',');
            if (cols.length < 3) continue;
            this.eventsData.push({
                tick: parseInt(cols[0]),
                event: cols[1],
                details: cols.slice(2).join(',')
            });
        }
    }

    parseSoulLog(csv) {
        const lines = csv.trim().split('\n');
        for (let i = 1; i < lines.length; i++) {
            const cols = lines[i].split(',');
            if (cols.length < 8) continue;
            this.soulData.push({
                tick: parseInt(cols[0]),
                agentId: parseInt(cols[1]),
                archetype: cols[2],
                karma: parseFloat(cols[3]),
                cause: cols[7]
            });
        }
    }

    // ── UI ───────────────────────────────────────────────────────────────────
    setupUI() {
        document.getElementById('loadBtn').addEventListener('click', () => {
            document.getElementById('fileInput').click();
        });

        document.getElementById('fileInput').addEventListener('change', (e) => {
            this.loadFiles(e.target.files);
        });

        document.getElementById('loadDemoBtn').addEventListener('click', () => {
            this.autoLoad();
        });

        document.getElementById('playBtn').addEventListener('click', () => this.play());
        document.getElementById('pauseBtn').addEventListener('click', () => this.pause());
        document.getElementById('resetBtn').addEventListener('click', () => this.reset());

        document.getElementById('speedRange').addEventListener('input', (e) => {
            this.speed = parseInt(e.target.value);
            document.getElementById('speedDisplay').innerText = this.speed + 'x';
        });

        document.getElementById('tickRange').addEventListener('input', (e) => {
            this.currentTick = parseInt(e.target.value);
            this.render3D();
        });

        // Camera buttons
        document.getElementById('topViewBtn').addEventListener('click', () => {
            const c = this.gridSize / 2;
            this.camera.position.set(c, this.gridSize, c);
            this.controls.target.set(c, 0, c);
            this.controls.update();
        });

        document.getElementById('sideViewBtn').addEventListener('click', () => {
            const c = this.gridSize / 2;
            this.camera.position.set(c, 5, this.gridSize + 10);
            this.controls.target.set(c, 0, c);
            this.controls.update();
        });

        document.getElementById('orbitBtn').addEventListener('click', () => {
            const c = this.gridSize / 2;
            this.camera.position.set(c, this.gridSize * 0.8, c + this.gridSize * 0.7);
            this.controls.target.set(c, 0, c);
            this.controls.update();
        });
    }

    async loadFiles(fileList) {
        let loaded = 0;
        document.getElementById('fileStatus').innerText = 'Loading...';

        for (const file of fileList) {
            const csv = await file.text();
            if (file.name.includes('simulation_log')) { this.parseAgentLog(csv); loaded++; }
            else if (file.name.includes('world_log')) { this.parseWorldLog(csv); loaded++; }
            else if (file.name.includes('events_log')) { this.parseEventsLog(csv); loaded++; }
            else if (file.name.includes('soul_log')) { this.parseSoulLog(csv); loaded++; }
        }

        this.onDataLoaded(loaded);
    }

    async autoLoad() {
        const files = [
            { url: '../simulation_log.csv', parser: 'parseAgentLog' },
            { url: '../world_log.csv', parser: 'parseWorldLog' },
            { url: '../events_log.csv', parser: 'parseEventsLog' },
            { url: '../soul_log.csv', parser: 'parseSoulLog' }
        ];

        let loaded = 0;
        document.getElementById('fileStatus').innerText = 'Auto-loading...';

        for (const f of files) {
            try {
                const resp = await fetch(f.url);
                if (resp.ok) {
                    const csv = await resp.text();
                    this[f.parser](csv);
                    loaded++;
                }
            } catch (e) { }
        }

        if (loaded > 0) this.onDataLoaded(loaded);
        else document.getElementById('fileStatus').innerText = 'No CSV files found. Use manual load.';
    }

    onDataLoaded(count) {
        document.getElementById('fileStatus').innerText = `✓ Loaded ${count} files · ${this.maxTick} ticks`;
        document.getElementById('maxTickDisplay').innerText = this.maxTick;
        document.getElementById('tickRange').max = this.maxTick;

        this.buildTerrain();
        this.currentTick = 1;
        this.render3D();
    }

    play() { this.playing = true; }
    pause() { this.playing = false; }
    reset() {
        this.currentTick = 0;
        this.playing = false;
        document.getElementById('tickRange').value = 0;
        document.getElementById('tickDisplay').innerText = 0;
        this.render3D();
    }

    // ── Click Selection ─────────────────────────────────────────────────────
    onClick(event) {
        const mouse = new THREE.Vector2(
            (event.clientX / window.innerWidth) * 2 - 1,
            -(event.clientY / window.innerHeight) * 2 + 1
        );

        const raycaster = new THREE.Raycaster();
        raycaster.setFromCamera(mouse, this.camera);

        const meshes = Array.from(this.agentMeshes.values());
        const intersects = raycaster.intersectObjects(meshes, false);

        if (intersects.length > 0) {
            const agentId = intersects[0].object.userData.agentId;
            this.selectedAgentId = agentId;
            this.updateInspector(agentId);
        } else {
            this.selectedAgentId = null;
            document.getElementById('inspectorPanel').style.display = 'none';
        }
    }

    updateInspector(agentId) {
        const agents = this.agentData[this.currentTick] || [];
        const agent = agents.find(a => a.id === agentId);
        if (!agent) return;

        document.getElementById('inspectorPanel').style.display = 'block';
        document.getElementById('inspAgentId').innerText = `#${agent.id}`;
        document.getElementById('inspArchetype').innerText = agent.archetype;
        document.getElementById('inspArchetype').style.color = '#' + (ARCHETYPE_CONFIG[agent.archetype]?.color || 0x888888).toString(16).padStart(6, '0');
        document.getElementById('inspKarma').innerText = agent.karma.toFixed(1);
        document.getElementById('inspAlignment').innerText = agent.alignment.toFixed(2);
        document.getElementById('inspTribe').innerText = agent.tribeId > 0 ? `Tribe #${agent.tribeId}` : 'Lone Wolf';
        document.getElementById('inspHealth').innerText = agent.health.toFixed(0);
        document.getElementById('inspEnergy').innerText = agent.energy.toFixed(0);
        document.getElementById('inspSoulAge').innerText = agent.soulAge;
    }

    updateStats(tick) {
        const agents = this.agentData[tick] || [];
        const world = this.worldData[tick];

        document.getElementById('popCount').innerText = agents.length;

        // Count tribes
        const tribes = new Set(agents.filter(a => a.tribeId > 0).map(a => a.tribeId));
        document.getElementById('tribeCount').innerText = tribes.size;

        // Count memes
        const totalMemes = agents.reduce((sum, a) => sum + (a.numMemes || 0), 0);
        document.getElementById('memeCount').innerText = totalMemes;

        // Average karma
        if (agents.length > 0) {
            const avgKarma = agents.reduce((sum, a) => sum + a.karma, 0) / agents.length;
            document.getElementById('avgKarma').innerText = avgKarma.toFixed(1);
        }

        // Update tick displays
        document.getElementById('tickDisplay').innerText = tick;
        document.getElementById('tickRange').value = tick;
    }

    updateEventFeed(tick) {
        const events = this.soulData.filter(e => e.tick <= tick && e.tick > tick - 20);
        const list = document.getElementById('eventList');
        let html = '';
        for (const e of events.slice(-10).reverse()) {
            const icon = '💀';
            html += `<div class="event-item"><span class="event-tick">Y${e.tick}</span> ${icon} Agent ${e.agentId} (${e.archetype}) — ${e.cause}</div>`;
        }
        list.innerHTML = html;
    }

    // ── Render ───────────────────────────────────────────────────────────────
    render3D() {
        this.updateAgents(this.currentTick);
        this.updateStats(this.currentTick);
        this.updateEventFeed(this.currentTick);

        if (this.selectedAgentId) {
            this.updateInspector(this.selectedAgentId);
        }
    }

    // ── Animation Loop ──────────────────────────────────────────────────────
    animate() {
        requestAnimationFrame(() => this.animate());

        const delta = this.clock.getDelta();
        this.controls.update();

        if (this.playing && this.maxTick > 0) {
            this.frameAccum += delta * this.speed * 3;
            while (this.frameAccum >= 1 && this.currentTick < this.maxTick) {
                this.currentTick++;
                this.frameAccum -= 1;
                this.render3D();
            }
            if (this.currentTick >= this.maxTick) {
                this.playing = false;
            }
        }

        // Animate agent rotations
        for (const mesh of this.agentMeshes.values()) {
            mesh.rotation.y += 0.01;
        }

        this.renderer.render(this.scene, this.camera);
    }

    onResize() {
        this.camera.aspect = window.innerWidth / window.innerHeight;
        this.camera.updateProjectionMatrix();
        this.renderer.setSize(window.innerWidth, window.innerHeight);
    }
}

// ── Boot ────────────────────────────────────────────────────────────────────
window.addEventListener('load', async () => {
    const viewer = new EvoSim3D();

    // Auto-load if served via HTTP
    if (window.location.protocol === 'http:' || window.location.protocol === 'https:') {
        document.getElementById('splashOverlay').style.display = 'none';
        document.getElementById('appContainer').style.display = 'block';
        window.dispatchEvent(new Event('resize'));
        await viewer.autoLoad();
    }
});
