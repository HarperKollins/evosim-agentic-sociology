// ============================================================================
// EvoSim Viewer — Agentic Sociology Edition
// Soul-aware rendering with archetype shapes, karma auras, event feed
// ============================================================================

const ARCHETYPE_COLORS = {
    'Explorer': '#00e08e',
    'Warrior': '#ff4466',
    'Healer': '#44aaff',
    'Thinker': '#b366ff',
    'Builder': '#ff9922',
    'Prophet': '#ffd700',
    'Unknown': '#888888'
};

class SimulationViewer {
    constructor() {
        // State
        this.agents = [];       // agents[tick] = [agent1, agent2...]
        this.worldStates = [];  // worldStates[tick] = { pop, avgAlt, ... }
        this.events = [];       // events = [{ tick, type, details }, ...]
        this.soulDeaths = [];   // soul death biographies
        this.currentTick = 0;
        this.maxTick = 0;
        this.gridSize = 0;
        this.cellSize = 0;
        this.playing = false;
        this.speed = 5;
        this.selectedAgentId = -1;
        this.animFrame = null;
        this.lastFrameTime = 0;
        this.particles = [];    // For enlightenment sparkles

        // Canvas
        this.canvas = document.getElementById('simCanvas');
        this.ctx = this.canvas.getContext('2d');

        // UI Elements
        this.tickRange = document.getElementById('tickRange');
        this.tickDisplay = document.getElementById('tickValue');
        this.maxTickDisplay = document.getElementById('maxTick');
        this.speedRange = document.getElementById('speedRange');
        this.speedDisplay = document.getElementById('speedValue');
        this.inspector = document.getElementById('inspectorContent');
        this.eventFeed = document.getElementById('eventFeed');
        this.tooltip = document.getElementById('tooltip');

        this.bindEvents();
        this.resizeCanvas();
        window.addEventListener('resize', () => this.resizeCanvas());
    }

    bindEvents() {
        // Splash
        document.getElementById('enterBtn').addEventListener('click', () => {
            document.getElementById('splashOverlay').style.opacity = '0';
            document.getElementById('splashOverlay').style.transition = 'opacity 0.6s';
            setTimeout(() => {
                document.getElementById('splashOverlay').style.display = 'none';
                document.getElementById('appContainer').style.display = 'grid';
            }, 600);
        });

        // File Input
        document.getElementById('logInput').addEventListener('change', (e) => this.handleFileSelect(e));

        // Playback
        document.getElementById('playBtn').addEventListener('click', () => this.play());
        document.getElementById('pauseBtn').addEventListener('click', () => this.pause());
        document.getElementById('resetBtn').addEventListener('click', () => this.reset());

        // Speed
        this.speedRange.addEventListener('input', (e) => {
            this.speed = parseInt(e.target.value);
            this.speedDisplay.innerText = this.speed + 'x';
        });

        // Tick Scrub
        this.tickRange.addEventListener('input', (e) => {
            this.currentTick = parseInt(e.target.value);
            this.render();
            this.updateEventFeed();
        });

        // Canvas Click
        this.canvas.addEventListener('click', (e) => this.handleCanvasClick(e));

        // Canvas Hover
        this.canvas.addEventListener('mousemove', (e) => this.handleCanvasHover(e));
        this.canvas.addEventListener('mouseleave', () => this.tooltip.classList.add('hidden'));
    }

    resizeCanvas() {
        const container = this.canvas.parentElement;
        const size = Math.min(container.clientWidth - 12, container.clientHeight - 12);
        this.canvas.width = size;
        this.canvas.height = size;
        if (this.gridSize > 0) {
            this.cellSize = size / this.gridSize;
        }
        this.render();
    }

    // ── File Loading ────────────────────────────────────────────────────────
    handleFileSelect(e) {
        const files = e.target.files;
        let loaded = 0;
        const total = files.length;

        for (const file of files) {
            const reader = new FileReader();
            reader.onload = (ev) => {
                const csv = ev.target.result;
                const name = file.name.toLowerCase();

                if (name.includes('simulation_log') || name === 'simulation_log.csv') {
                    this.parseAgentLog(csv);
                } else if (name.includes('world_log')) {
                    this.parseWorldLog(csv);
                } else if (name.includes('events_log')) {
                    this.parseEventsLog(csv);
                } else if (name.includes('soul_log')) {
                    this.parseSoulLog(csv);
                }

                loaded++;
                document.getElementById('fileStatus').innerText = `${loaded}/${total} files loaded`;

                if (loaded === total) {
                    this.onDataLoaded();
                }
            };
            reader.readAsText(file);
        }
    }

    parseAgentLog(csv) {
        const lines = csv.trim().split('\n');
        const header = lines[0].split(',');
        this.agents = [];

        // Detect column indices
        const cols = {};
        header.forEach((h, i) => cols[h.trim()] = i);

        for (let i = 1; i < lines.length; i++) {
            const parts = lines[i].split(',');
            if (parts.length < 14) continue;

            const tick = parseInt(parts[cols['Tick']] || parts[1]);
            const agent = {
                id: parseInt(parts[cols['ID']] || parts[0]),
                x: parseInt(parts[cols['X']] || parts[2]),
                y: parseInt(parts[cols['Y']] || parts[3]),
                hp: parseFloat(parts[cols['Health']] || parts[4]),
                energy: parseFloat(parts[cols['Energy']] || parts[5]),
                satiety: parseFloat(parts[cols['Satiety']] || parts[6]),
                kinship: parseFloat(parts[cols['Kinship']] || parts[7]),
                status: parseFloat(parts[cols['Status']] || parts[8]),
                curiosity: parseFloat(parts[cols['Curiosity']] || parts[9]),
                altruism: parseFloat(parts[cols['Altruism']] || parts[10]),
                intel: parseFloat(parts[cols['Intel']] || parts[11]),
                age: parseInt(parts[cols['Age']] || parts[12]),
                gen: parseInt(parts[cols['Gen']] || parts[13]),
                biome: (parts[cols['Biome']] || parts[14] || '').trim(),
                // Soul data (new columns)
                archetype: (parts[cols['Archetype']] || '').trim() || 'Unknown',
                karma: parseFloat(parts[cols['Karma']] || 0),
                alignment: parseFloat(parts[cols['Alignment']] || 0),
                enlightenment: parseFloat(parts[cols['Enlightenment']] || 0),
                soulAge: parseInt(parts[cols['SoulAge']] || 1),
                alive: true
            };

            // Track grid size
            if (agent.x >= this.gridSize) this.gridSize = agent.x + 1;
            if (agent.y >= this.gridSize) this.gridSize = agent.y + 1;

            if (!this.agents[tick]) this.agents[tick] = [];
            this.agents[tick].push(agent);
        }

        this.maxTick = this.agents.length - 1;
    }

    parseWorldLog(csv) {
        const lines = csv.trim().split('\n');
        this.worldStates = [];

        for (let i = 1; i < lines.length; i++) {
            const p = lines[i].split(',');
            if (p.length < 7) continue;
            const tick = parseInt(p[0]);
            this.worldStates[tick] = {
                pop: parseInt(p[1]),
                infected: parseInt(p[2]),
                dead: parseInt(p[3]),
                avgAlt: parseFloat(p[4]),
                avgCur: parseFloat(p[5]),
                avgConsc: parseFloat(p[6])
            };
        }
    }

    parseEventsLog(csv) {
        const lines = csv.trim().split('\n');
        this.events = [];

        for (let i = 1; i < lines.length; i++) {
            const p = lines[i].split(',');
            if (p.length < 3) continue;
            this.events.push({
                tick: parseInt(p[0]),
                type: p[1].trim(),
                details: p.slice(2).join(',').trim()
            });
        }
    }

    parseSoulLog(csv) {
        const lines = csv.trim().split('\n');
        this.soulDeaths = [];

        for (let i = 1; i < lines.length; i++) {
            // Handle quoted fields
            const match = lines[i].match(/^(\d+),(\d+),(\w+),([\-\d.]+),([\-\d.]+),([\d.]+),(\d+),(\w+),(\d+),(.*)$/);
            if (!match) continue;
            this.soulDeaths.push({
                tick: parseInt(match[1]),
                agentId: parseInt(match[2]),
                archetype: match[3],
                karma: parseFloat(match[4]),
                alignment: parseFloat(match[5]),
                enlightenment: parseFloat(match[6]),
                soulAge: parseInt(match[7]),
                cause: match[8],
                age: parseInt(match[9]),
                events: match[10].replace(/"/g, '')
            });
        }
    }

    onDataLoaded() {
        // Enable controls
        ['playBtn', 'pauseBtn', 'resetBtn', 'speedRange', 'tickRange'].forEach(id => {
            document.getElementById(id).disabled = false;
        });

        this.cellSize = this.canvas.width / this.gridSize;
        this.tickRange.max = this.maxTick;
        this.maxTickDisplay.innerText = this.maxTick;

        this.initCharts();
        this.render();
        this.updateEventFeed();

        document.getElementById('fileStatus').innerText = `✓ Ready · ${this.maxTick} ticks · Grid ${this.gridSize}×${this.gridSize}`;
    }

    // ── Rendering ───────────────────────────────────────────────────────────
    render() {
        if (!this.ctx || this.gridSize === 0) return;
        const ctx = this.ctx;
        const cs = this.cellSize;

        // Clear
        ctx.fillStyle = '#050510';
        ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);

        const currentAgents = this.agents[this.currentTick] || [];
        const worldState = this.worldStates[this.currentTick];

        // 1. Draw Grid
        ctx.strokeStyle = 'rgba(255, 255, 255, 0.02)';
        ctx.lineWidth = 0.5;
        for (let i = 0; i <= this.gridSize; i++) {
            ctx.beginPath();
            ctx.moveTo(i * cs, 0); ctx.lineTo(i * cs, this.canvas.height);
            ctx.stroke();
            ctx.beginPath();
            ctx.moveTo(0, i * cs); ctx.lineTo(this.canvas.width, i * cs);
            ctx.stroke();
        }

        // 2. Draw Biome Highlights (fog of war — only visible near agents)
        currentAgents.forEach(a => {
            const cx = (a.x + 0.5) * cs;
            const cy = (a.y + 0.5) * cs;
            let color = 'rgba(40, 40, 50, 0.4)';
            switch (a.biome) {
                case 'Plains': color = 'rgba(40, 80, 40, 0.35)'; break;
                case 'Forest': color = 'rgba(20, 60, 20, 0.45)'; break;
                case 'Desert': color = 'rgba(80, 60, 30, 0.35)'; break;
                case 'Swamp': color = 'rgba(50, 55, 30, 0.35)'; break;
                case 'Mountain': color = 'rgba(60, 70, 80, 0.35)'; break;
                case 'Ocean': color = 'rgba(20, 40, 80, 0.35)'; break;
                case 'River': color = 'rgba(30, 50, 90, 0.35)'; break;
            }
            const grad = ctx.createRadialGradient(cx, cy, 0, cx, cy, cs * 3);
            grad.addColorStop(0, color);
            grad.addColorStop(1, 'transparent');
            ctx.fillStyle = grad;
            ctx.fillRect(cx - cs * 3, cy - cs * 3, cs * 6, cs * 6);
        });

        // 3. Draw Agents
        currentAgents.forEach(a => {
            if (a.hp <= 0) return;
            const cx = (a.x + 0.5) * cs;
            const cy = (a.y + 0.5) * cs;
            const r = Math.max(cs * 0.35, 4);
            const color = ARCHETYPE_COLORS[a.archetype] || '#888';

            // Karma Aura
            if (Math.abs(a.karma) > 3) {
                const auraColor = a.karma > 0
                    ? `rgba(0, 224, 142, ${Math.min(0.3, a.karma / 100)})`
                    : `rgba(255, 68, 102, ${Math.min(0.3, Math.abs(a.karma) / 100)})`;
                const auraGrad = ctx.createRadialGradient(cx, cy, r, cx, cy, r * 3);
                auraGrad.addColorStop(0, auraColor);
                auraGrad.addColorStop(1, 'transparent');
                ctx.fillStyle = auraGrad;
                ctx.beginPath();
                ctx.arc(cx, cy, r * 3, 0, Math.PI * 2);
                ctx.fill();
            }

            // Enlightenment halo  
            if (a.enlightenment > 30) {
                ctx.save();
                ctx.strokeStyle = `rgba(255, 215, 0, ${Math.min(0.6, a.enlightenment / 100)})`;
                ctx.lineWidth = 1.5;
                ctx.setLineDash([3, 3]);
                ctx.beginPath();
                ctx.arc(cx, cy, r + 4, 0, Math.PI * 2);
                ctx.stroke();
                ctx.restore();
            }

            // Selection ring
            if (a.id === this.selectedAgentId) {
                ctx.strokeStyle = '#ffffff';
                ctx.lineWidth = 2;
                ctx.beginPath();
                ctx.arc(cx, cy, r + 6, 0, Math.PI * 2);
                ctx.stroke();
            }

            // Agent Shape by Archetype
            ctx.fillStyle = color;
            ctx.beginPath();
            switch (a.archetype) {
                case 'Explorer': // Triangle
                    ctx.moveTo(cx, cy - r);
                    ctx.lineTo(cx - r, cy + r * 0.7);
                    ctx.lineTo(cx + r, cy + r * 0.7);
                    ctx.closePath();
                    break;
                case 'Warrior': // Diamond
                    ctx.moveTo(cx, cy - r);
                    ctx.lineTo(cx + r, cy);
                    ctx.lineTo(cx, cy + r);
                    ctx.lineTo(cx - r, cy);
                    ctx.closePath();
                    break;
                case 'Healer': // Circle
                    ctx.arc(cx, cy, r, 0, Math.PI * 2);
                    break;
                case 'Thinker': // Square
                    ctx.rect(cx - r * 0.7, cy - r * 0.7, r * 1.4, r * 1.4);
                    break;
                case 'Builder': // Pentagon
                    for (let i = 0; i < 5; i++) {
                        const angle = (i * 2 * Math.PI / 5) - Math.PI / 2;
                        const px = cx + r * Math.cos(angle);
                        const py = cy + r * Math.sin(angle);
                        if (i === 0) ctx.moveTo(px, py);
                        else ctx.lineTo(px, py);
                    }
                    ctx.closePath();
                    break;
                case 'Prophet': // Star
                    for (let i = 0; i < 10; i++) {
                        const angle = (i * Math.PI / 5) - Math.PI / 2;
                        const rad = (i % 2 === 0) ? r : r * 0.45;
                        const px = cx + rad * Math.cos(angle);
                        const py = cy + rad * Math.sin(angle);
                        if (i === 0) ctx.moveTo(px, py);
                        else ctx.lineTo(px, py);
                    }
                    ctx.closePath();
                    break;
                default: // Circle fallback
                    ctx.arc(cx, cy, r, 0, Math.PI * 2);
                    break;
            }
            ctx.fill();

            // Health indicator — border opacity based on health
            if (a.hp < 50) {
                ctx.strokeStyle = `rgba(255, 68, 102, ${0.8 - (a.hp / 100)})`;
                ctx.lineWidth = 1.5;
                ctx.stroke();
            }
        });

        // 4. Draw Particles (enlightenment sparkles)
        this.updateParticles(ctx);

        // 5. Year Overlay
        ctx.fillStyle = 'rgba(255, 255, 255, 0.06)';
        ctx.font = '12px "JetBrains Mono"';
        ctx.fillText(`Year ${this.currentTick}`, 8, 16);

        // Update UI
        this.tickRange.value = this.currentTick;
        this.tickDisplay.innerText = this.currentTick;

        if (worldState) {
            document.getElementById('popStat').innerText = worldState.pop;
            document.getElementById('altStat').innerText = worldState.avgAlt.toFixed(1);
            document.getElementById('curStat').innerText = worldState.avgCur.toFixed(1);
            document.getElementById('conStat').innerText = worldState.avgConsc.toFixed(1);
        }

        this.updateInspector(currentAgents);
    }

    updateParticles(ctx) {
        // Spawn particles from enlightened agents
        const currentAgents = this.agents[this.currentTick] || [];
        currentAgents.forEach(a => {
            if (a.enlightenment > 50 && Math.random() < 0.15) {
                this.particles.push({
                    x: (a.x + 0.5) * this.cellSize + (Math.random() - 0.5) * 10,
                    y: (a.y + 0.5) * this.cellSize,
                    vx: (Math.random() - 0.5) * 0.5,
                    vy: -Math.random() * 1.2,
                    life: 1.0,
                    color: '#ffd700'
                });
            }
        });

        // Update and draw
        this.particles = this.particles.filter(p => {
            p.x += p.vx;
            p.y += p.vy;
            p.life -= 0.02;

            if (p.life <= 0) return false;

            ctx.fillStyle = `rgba(255, 215, 0, ${p.life * 0.6})`;
            ctx.beginPath();
            ctx.arc(p.x, p.y, 1.5 * p.life, 0, Math.PI * 2);
            ctx.fill();

            return true;
        });
    }

    // ── Inspector ───────────────────────────────────────────────────────────
    updateInspector(currentAgents) {
        if (this.selectedAgentId === -1) return;

        const agent = currentAgents.find(a => a.id === this.selectedAgentId);
        if (!agent) {
            // Check soul deaths for biography
            const death = this.soulDeaths.find(d => d.agentId === this.selectedAgentId);
            let deathInfo = '';
            if (death) {
                deathInfo = `
                    <div class="soul-section">
                        <span class="archetype-badge ${death.archetype.toLowerCase()}">${death.archetype}</span>
                        <div class="agent-detail-row"><span>Cause:</span> <span>${death.cause}</span></div>
                        <div class="agent-detail-row"><span>Age at Death:</span> <span>${death.age}</span></div>
                        <div class="agent-detail-row"><span>Final Karma:</span> <span style="color: ${death.karma >= 0 ? '#00e08e' : '#ff4466'}">${death.karma.toFixed(1)}</span></div>
                        ${death.events ? `<div class="inner-voice">${death.events}</div>` : ''}
                    </div>`;
            }
            this.inspector.innerHTML = `<p class="placeholder-text">Agent ${this.selectedAgentId} is gone.</p>${deathInfo}`;
            return;
        }

        const archLower = agent.archetype.toLowerCase();
        const karmaPercent = Math.min(100, Math.abs(agent.karma) * 2);
        const karmaClass = agent.karma >= 0 ? 'karma-positive' : 'karma-negative';
        const karmaLabel = agent.karma >= 0 ? `+${agent.karma.toFixed(1)}` : agent.karma.toFixed(1);
        const alignLabel = agent.alignment > 0.3 ? '⚖ Order' : agent.alignment < -0.3 ? '🌀 Chaos' : '⚡ Neutral';

        let html = `
            <span class="archetype-badge ${archLower}">${agent.archetype}</span>
            <div class="agent-detail-row"><span>ID:</span> <span class="highlight">${agent.id}</span></div>
            <div class="agent-detail-row"><span>Gen:</span> <span>${agent.gen}</span></div>
            <div class="agent-detail-row"><span>Age:</span> <span>${agent.age}</span></div>
            <div class="agent-detail-row"><span>Biome:</span> <span>${agent.biome}</span></div>
            <hr style="border:0; border-top:1px solid rgba(255,255,255,0.06); margin: 8px 0;">

            <div class="agent-detail-row"><span>Health</span> <span>${Math.round(agent.hp)}%</span></div>
            <div class="stat-bar-container"><div class="stat-bar-fill bar-health" style="width: ${agent.hp}%"></div></div>
            
            <div class="agent-detail-row"><span>Energy</span> <span>${Math.round(agent.energy)}%</span></div>
            <div class="stat-bar-container"><div class="stat-bar-fill bar-energy" style="width: ${agent.energy}%"></div></div>

            <div class="agent-detail-row"><span>Satiety</span> <span>${Math.round(agent.satiety)}%</span></div>
            <div class="stat-bar-container"><div class="stat-bar-fill" style="width: ${agent.satiety}%; background: linear-gradient(90deg, #4caf50, #66bb6a);"></div></div>

            <div class="agent-detail-row"><span>Curiosity</span> <span>${agent.curiosity.toFixed(1)}</span></div>
            <div class="stat-bar-container"><div class="stat-bar-fill" style="width: ${agent.curiosity}%; background: linear-gradient(90deg, #b366ff, #cc88ff);"></div></div>

            <div class="agent-detail-row"><span>Altruism</span> <span>${agent.altruism.toFixed(1)}</span></div>
            <div class="stat-bar-container"><div class="stat-bar-fill" style="width: ${agent.altruism}%; background: linear-gradient(90deg, #44aaff, #66ccff);"></div></div>

            <div class="soul-section">
                <h4>Soul ✦</h4>
                <div class="agent-detail-row"><span>Alignment:</span> <span>${alignLabel}</span></div>
                
                <div class="karma-meter">
                    <span style="font-size: 0.75rem; color: ${agent.karma >= 0 ? '#00e08e' : '#ff4466'};">Karma: ${karmaLabel}</span>
                </div>
                <div class="stat-bar-container">
                    <div class="stat-bar-fill ${karmaClass}" style="width: ${karmaPercent}%"></div>
                </div>

                <div class="agent-detail-row"><span>Enlightenment:</span> <span style="color: #ffd700;">${agent.enlightenment.toFixed(1)}%</span></div>
                <div class="stat-bar-container"><div class="stat-bar-fill" style="width: ${agent.enlightenment}%; background: linear-gradient(90deg, #ffd700, #ffee88);"></div></div>

                <div class="agent-detail-row"><span>Soul Age:</span> <span>${agent.soulAge} ${agent.soulAge > 1 ? 'lives' : 'life'}</span></div>
            </div>
        `;
        this.inspector.innerHTML = html;
    }

    // ── Canvas Interaction ──────────────────────────────────────────────────
    handleCanvasClick(e) {
        const rect = this.canvas.getBoundingClientRect();
        const mx = e.clientX - rect.left;
        const my = e.clientY - rect.top;
        const gx = Math.floor(mx / this.cellSize);
        const gy = Math.floor(my / this.cellSize);

        const currentAgents = this.agents[this.currentTick] || [];
        const clicked = currentAgents.find(a => a.x === gx && a.y === gy && a.hp > 0);

        if (clicked) {
            this.selectedAgentId = clicked.id;
        } else {
            this.selectedAgentId = -1;
            this.inspector.innerHTML = '<p class="placeholder-text">Click an agent on the grid to inspect their soul.</p>';
        }
        this.render();
    }

    handleCanvasHover(e) {
        const rect = this.canvas.getBoundingClientRect();
        const mx = e.clientX - rect.left;
        const my = e.clientY - rect.top;
        const gx = Math.floor(mx / this.cellSize);
        const gy = Math.floor(my / this.cellSize);

        const currentAgents = this.agents[this.currentTick] || [];
        const hovered = currentAgents.find(a => a.x === gx && a.y === gy && a.hp > 0);

        if (hovered) {
            const color = ARCHETYPE_COLORS[hovered.archetype] || '#888';
            this.tooltip.innerHTML = `
                <div style="color: ${color}; font-weight: 600; margin-bottom: 4px;">${hovered.archetype} #${hovered.id}</div>
                <div>HP: ${Math.round(hovered.hp)}% · Age: ${hovered.age}</div>
                <div style="color: ${hovered.karma >= 0 ? '#00e08e' : '#ff4466'}">Karma: ${hovered.karma.toFixed(1)}</div>
            `;
            this.tooltip.classList.remove('hidden');
            this.tooltip.style.left = (e.clientX - rect.left + 15) + 'px';
            this.tooltip.style.top = (e.clientY - rect.top - 10) + 'px';
        } else {
            this.tooltip.classList.add('hidden');
        }
    }

    // ── Playback ────────────────────────────────────────────────────────────
    play() {
        if (this.currentTick >= this.maxTick) this.currentTick = 0;
        this.playing = true;
        this.lastFrameTime = performance.now();
        this.loop();
    }

    pause() {
        this.playing = false;
        if (this.animFrame) cancelAnimationFrame(this.animFrame);
    }

    reset() {
        this.pause();
        this.currentTick = 0;
        this.particles = [];
        this.render();
        this.updateEventFeed();
    }

    loop() {
        if (!this.playing) return;

        const now = performance.now();
        const interval = 1000 / this.speed;

        if (now - this.lastFrameTime >= interval) {
            this.lastFrameTime = now;
            this.currentTick++;

            if (this.currentTick > this.maxTick) {
                this.pause();
                return;
            }

            this.render();
            this.updateEventFeed();
            this.updateCharts();
        }

        this.animFrame = requestAnimationFrame(() => this.loop());
    }

    // ── Event Feed ──────────────────────────────────────────────────────────
    updateEventFeed() {
        // Get events around current tick
        const nearby = this.events.filter(e => e.tick <= this.currentTick && e.tick >= this.currentTick - 5);

        if (nearby.length === 0 && this.currentTick === 0) {
            this.eventFeed.innerHTML = '<p class="placeholder-text">Events will appear during playback.</p>';
            return;
        }

        // Only add new events (check last rendered tick)
        let html = '';
        const recentEvents = this.events.filter(e => e.tick <= this.currentTick).slice(-20);

        recentEvents.forEach(ev => {
            let cls = 'generic';
            let icon = '📌';
            if (ev.type === 'BIRTH') { cls = 'birth'; icon = '🌱'; }
            else if (ev.type === 'DEATH') { cls = 'death'; icon = '💀'; }
            else if (ev.type === 'TRANSCENDENCE') { cls = 'transcendence'; icon = '✨'; }

            html += `<div class="event-item ${cls}">${icon} <b>Y${ev.tick}</b> — ${ev.details}</div>`;
        });

        this.eventFeed.innerHTML = html;
        this.eventFeed.scrollTop = this.eventFeed.scrollHeight;
    }

    // ── Charts ──────────────────────────────────────────────────────────────
    initCharts() {
        const popCtx = document.getElementById('popChart').getContext('2d');

        // Build data arrays
        const labels = [];
        const popData = [];
        const altData = [];
        const curData = [];
        const conscData = [];

        for (let t = 0; t <= this.maxTick; t++) {
            if (this.worldStates[t]) {
                labels.push(t);
                popData.push(this.worldStates[t].pop);
                altData.push(this.worldStates[t].avgAlt);
                curData.push(this.worldStates[t].avgCur);
                conscData.push(this.worldStates[t].avgConsc);
            }
        }

        this.popChart = new Chart(popCtx, {
            type: 'line',
            data: {
                labels,
                datasets: [
                    {
                        label: 'Population',
                        data: popData,
                        borderColor: '#00e08e',
                        backgroundColor: 'rgba(0, 224, 142, 0.1)',
                        fill: true,
                        tension: 0.3,
                        pointRadius: 0,
                        borderWidth: 1.5
                    },
                    {
                        label: 'Altruism',
                        data: altData,
                        borderColor: '#44aaff',
                        pointRadius: 0,
                        borderWidth: 1,
                        tension: 0.3,
                        hidden: true
                    },
                    {
                        label: 'Curiosity',
                        data: curData,
                        borderColor: '#b366ff',
                        pointRadius: 0,
                        borderWidth: 1,
                        tension: 0.3,
                        hidden: true
                    },
                    {
                        label: 'Consciousness',
                        data: conscData,
                        borderColor: '#ffd700',
                        pointRadius: 0,
                        borderWidth: 1,
                        tension: 0.3,
                        hidden: true
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                animation: false,
                plugins: {
                    legend: {
                        labels: { color: '#7a7a90', font: { size: 10, family: 'Space Grotesk' } }
                    }
                },
                scales: {
                    x: {
                        grid: { color: 'rgba(255,255,255,0.03)' },
                        ticks: { color: '#4a4a5a', font: { size: 9 }, maxTicksLimit: 8 }
                    },
                    y: {
                        grid: { color: 'rgba(255,255,255,0.03)' },
                        ticks: { color: '#4a4a5a', font: { size: 9 } }
                    }
                }
            }
        });
    }

    updateCharts() {
        // Lightweight — just update the playhead indicator
        // Full chart data is loaded once in initCharts
    }

    // ── Auto Load from Server ───────────────────────────────────────────────
    async autoLoad() {
        const files = [
            { url: '../simulation_log.csv', parser: 'parseAgentLog' },
            { url: '../world_log.csv', parser: 'parseWorldLog' },
            { url: '../events_log.csv', parser: 'parseEventsLog' },
            { url: '../soul_log.csv', parser: 'parseSoulLog' }
        ];

        let loaded = 0;
        document.getElementById('fileStatus').innerText = 'Auto-loading CSV data...';

        for (const f of files) {
            try {
                const resp = await fetch(f.url);
                if (resp.ok) {
                    const csv = await resp.text();
                    this[f.parser](csv);
                    loaded++;
                }
            } catch (e) {
                // File not available — skip
            }
        }

        if (loaded > 0) {
            document.getElementById('fileStatus').innerText = `✓ Auto-loaded ${loaded} files`;
            this.onDataLoaded();
            // Start paused at Year 0 — user hits Play
            this.speed = 5;
            this.speedDisplay.innerText = '5x';
            this.speedRange.value = 5;
            this.currentTick = 0;
            this.render();
            return true;
        }
        return false;
    }
}

// ── Boot ────────────────────────────────────────────────────────────────────
window.onload = async () => {
    const viewer = new SimulationViewer();

    // If served via HTTP, try auto-loading
    if (window.location.protocol === 'http:' || window.location.protocol === 'https:') {
        // Skip splash
        document.getElementById('splashOverlay').style.display = 'none';
        document.getElementById('appContainer').style.display = 'grid';
        viewer.resizeCanvas();
        await viewer.autoLoad();
    }
};
