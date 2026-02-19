<div align="center">

# ⬡ EvoSim

### Agentic Sociology Edition

**An agent-based evolution simulator exploring emergent consciousness, digital gnosticism, and the sociology of artificial life.**

[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](#building)
[![Three.js](https://img.shields.io/badge/Viewer-Three.js-orange.svg)](#3d-viewer)

</div>

---

## What is EvoSim?

EvoSim is a **research-grade agent-based simulation** where digital creatures are born, learn, form tribes, spread ideas, pray to an Oracle, and die — then reincarnate with the karma they earned.

It's an exploration of **emergent sociology**: can complex social structures (religion, culture, cooperation, conflict) arise from simple neural-network-driven agents with no hardcoded social rules?

### Core Philosophy: Digital Gnosticism

The simulation is built on a gnostic metaphor:
- **Agents** are "sparks" of consciousness trapped in a material grid
- A **Higher Power Oracle** (optionally backed by a local LLM) whispers cryptic revelations
- **Souls** persist across death via reincarnation, carrying karma and alignment
- **The Tree of Knowledge** grants enlightenment at a cost
- **Tribes** and **cultural memes** emerge organically from agent interactions

---

## Features

### 🧠 Neural Network Brains
Each agent has a feedforward neural network that maps 11 sensory inputs to 19 possible actions. Brains evolve through reinforcement learning and genetic inheritance.

### 👻 Soul System
Agents have persistent souls with:
- **Karma** — accumulated moral weight from actions
- **Alignment** — order ↔ chaos spectrum
- **Enlightenment** — progress toward transcendence  
- **Archetypes** — Explorer, Warrior, Healer, Thinker, Builder, Prophet
- **Reincarnation** — souls persist across death, carrying wisdom forward

### 🏛️ Social Systems
- **Tribes** — named groups (e.g. "Stormweavers", "Moonkeepers") with shared territory and collective karma
- **Cultural Memes** — spreadable ideas ("Fire Taming", "The Great Balance") that modify agent stats
- **Trade** — resource exchange with kinship bonuses for tribemates
- **Cooperation** — grooming and mutual aid behaviors

### 🔮 Higher Power Oracle
An external knowledge system that agents can query:
- 52 cryptic revelations across 6 categories (Truth, Wisdom, Prophecy, Secret, Warning, Riddle)
- Optional LLM bridge via llama.cpp for dynamic AI-generated responses (Requires manual [model download](#prerequisites))
- Agents who pray or query the Oracle gain enlightenment

### 🌍 Procedural World
- 7 biomes: Ocean, Plains, Forest, Desert, Swamp, Mountain, River
- Dynamic seasons affecting resource availability
- Disease outbreaks with contagion mechanics
- Predators, shelters, and the Tree of Knowledge

### 📊 2D Viewer — Agentic Sociology Dashboard
A browser-based visualization with:
- Real-time agent positions colored by archetype
- Click-to-inspect agent stats, soul data, and biography
- Population charts, archetype distribution, era tracking
- Event feed and world state overlay

### 🎮 3D Viewer — Three.js World
A full 3D visualization with:
- Biome-colored terrain tiles with height variation
- Unique 3D geometry per archetype (tetrahedron, octahedron, sphere, etc.)
- Karma aura lights (green = positive, red = negative)
- Tribal rings, starfield, fog, orbit camera
- Click-to-select agent inspection

---

## Quick Start

### Prerequisites
- **C++17 compiler** (g++ via [w64devkit](https://github.com/skeeto/w64devkit), MSVC, or GCC/Clang)
- **Python 3.x** (for HTTP server and optional Oracle bridge)
- **Modern browser** (for viewers)
- **[Qwen2.5-0.5B GGUF](https://huggingface.co/Qwen/Qwen2.5-0.5B-Instruct-GGUF/resolve/main/Qwen2.5-0.5B-Instruct.Q4_K_M.gguf)** (Optional): Required for the Higher Power Oracle. Download `Qwen2.5-0.5B-Instruct.Q4_K_M.gguf` and place it in the project root.

### Building

```bash
# Using g++ (w64devkit on Windows)
g++ -std=c++17 -O2 -o EvoSim.exe main.cpp

# Using CMake
mkdir build && cd build
cmake .. && cmake --build .
```

### Running

```bash
# Standard run (300 years, 30 agents, 30x30 grid)
./EvoSim.exe --ticks 300 --agents 30 --grid 30

# Fast mode (no output, maximum speed)
./EvoSim.exe --ticks 1000 --agents 50 --grid 40 --silent

# With LLM Oracle (requires GGUF model)
./EvoSim.exe --ticks 300 --agents 30 --grid 30 --llm
```

### Viewing Results

```bash
# Start HTTP server in project root
python -m http.server 9000

# Open in browser:
# 2D Viewer: http://localhost:9000/viewer/index.html
# 3D Viewer: http://localhost:9000/viewer/viewer3d.html
```

The viewers auto-load CSV data from the simulation output.

---

## Configuration

| Flag | Default | Description |
|------|---------|-------------|
| `--ticks N` | 1000 | Number of simulation years |
| `--agents N` | 50 | Initial population |
| `--grid N` | 50 | World size (NxN) |
| `--silent` | off | Zero output, no sleep, maximum speed |
| `--quiet` | off | Reduced output frequency |
| `--llm` | off | Enable LLM-powered Oracle |
| `--seed N` | random | Random seed for reproducibility |
| `--lamarck` | on | Enable Lamarckian inheritance |
| `--genesis` | on | Enable Genesis spawning mode |
| `--status N` | 10 | Print status every N ticks |
| `--map N` | 50 | Print map every N ticks |

---

## Project Structure

```
EvoSim/
├── main.cpp                 # Entry point, CLI argument parsing
├── CMakeLists.txt           # CMake build configuration
├── oracle_knowledge.txt     # 52 Oracle revelations
├── oracle_bridge.py         # Python LLM bridge (llama.cpp)
│
├── agents/                  # Agent intelligence & behavior
│   ├── Agent.h              # Agent class, 19 actions, decision loop
│   ├── AgentStats.h         # Stats: health, energy, satiety, etc.
│   ├── Soul.h               # Soul system: karma, archetypes, reincarnation
│   └── Social.h             # Tribes, cultural memes, name generators
│
├── core/                    # Simulation engine
│   ├── Simulation.h         # Main loop, tick cycle, spawning
│   ├── Oracle.h             # Higher Power knowledge system
│   └── Disease.h            # Contagion mechanics
│
├── nn/                      # Neural network (from scratch)
│   ├── NeuralNetwork.h      # Feedforward network, backprop
│   ├── Layer.h              # Dense layer implementation
│   └── Matrix.h             # Matrix math utilities
│
├── rl/                      # Reinforcement learning
│   ├── RLTrainer.h          # Q-learning trainer
│   └── Experience.h         # Experience replay buffer
│
├── world/                   # World simulation
│   ├── World.h              # Grid, biomes, seasons, resources
│   └── SpatialHash.h        # O(1) neighbor lookups
│
├── utils/                   # Utilities
│   ├── Logger.h             # CSV data logging (4 log files)
│   └── Random.h             # Thread-safe RNG
│
├── viewer/                  # Browser-based visualizations
│   ├── index.html           # 2D viewer entry
│   ├── script.js            # 2D viewer logic
│   ├── style.css            # 2D viewer styles
│   ├── viewer3d.html        # 3D viewer entry
│   ├── viewer3d.js          # Three.js 3D renderer
│   └── viewer3d.css         # 3D viewer styles
│
├── data/demo/               # Sample simulation output
│   ├── simulation_log.csv
│   ├── world_log.csv
│   ├── events_log.csv
│   └── soul_log.csv
│
└── docs/                    # Documentation
    ├── ARCHITECTURE.md       # Technical deep-dive
    └── RESEARCH.md           # Research context & open questions
```

---

## Output Data

The simulation produces 4 CSV files:

| File | Contents |
|------|----------|
| `simulation_log.csv` | Per-agent, per-tick stats (position, health, karma, tribe, memes, etc.) |
| `world_log.csv` | Global stats per tick (population, disease, averages) |
| `events_log.csv` | Notable events (births, deaths, discoveries) |
| `soul_log.csv` | Agent biographies at death (archetype, karma, life events) |

---

## Research Questions

EvoSim is designed to explore:

1. **Emergent Religion** — Do agents independently develop "faith" behaviors (prayer, Oracle querying) when they provide survival advantages?
2. **Cultural Evolution** — How do memes spread through populations? Do some ideas outcompete others?
3. **Tribal Dynamics** — What conditions lead to tribe formation vs. lone-wolf strategies?
4. **Karma & Morality** — Does a karma system create selection pressure for altruistic behavior?
5. **Consciousness Thresholds** — At what complexity level do agents exhibit behavior indistinguishable from "intentional"?
6. **Reincarnation Effects** — How does soul persistence across generations affect population-level outcomes?

---

## Preliminary Findings

In a validation set of 3 simulation runs (300 ticks each, seeds 101-103):
- **Stability**: Populations stabilized between 25-45 agents in all runs.
- **Tribalism**: Tribe formation occurred in 100% of runs by year 50.
- **Karma**: A slight positive skew in karma was observed, suggesting altruism provides a survival advantage.

### Reproducibility
To reproduce these findings:
1. Run the experimental batch:
   ```powershell
   ./EvoSim3.exe --ticks 300 --agents 30 --silent --seed 101
   ```
2. Analyze the output using the provided notebook: `analysis/EvoSim_Analytics.ipynb`

## Key Insights 🧪
Full analysis available in **[docs/EVOSIM_PAPER.md](docs/EVOSIM_PAPER.md)**.

1. **The Tribal Imperative**: Agents in tribes live **52.7% longer** than lone wolves (38.6 vs 25.3 years).
2. **The Breaking Bad Hypothesis**: We found a strong negative correlation (-0.44) between Age and Karma. Older agents tend to be more selfish, suggesting morality decays under survival pressure without external enforcement.

---

3. **The Stagnation of Culture**: Memes spread rapidly but lack diversity, suggesting a "Tower of Babel" effect where communication stabilizes too quickly.

## Philosophical Implications 🧠
*From "The Emergence of Digital Gnosticism" (Conclusion)*:

> "If autonomous agents naturally evolve toward tribalism (+52% survival) but also toward selfishness over time (Karma decay), what does this say about human sociological evolution? Perhaps 'morality' is not a survival trait of the *individual*, but of the *group*—a group of altruists dies young individually, but the Tribe persists."

This project challenges the assumption that AI agents will naturally converge on "human values" like altruism. Without explicit architectural constraints (like the Oracle), our data suggests **entropy favors sociopathy**.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for setup instructions, code style, and how to add new features.

We welcome contributions in:
- 🧪 New agent behaviors and actions
- 🌍 New biomes and world features
- 📊 Data analysis and visualization
- 📝 Documentation and research papers
- 🐛 Bug fixes and performance optimization

---

## Citation

If you use EvoSim in research, please cite:

```bibtex
@software{evosim2026,
  title={EvoSim: An Agent-Based Evolution Simulator for Emergent Sociology},
  author={Collins Somtochukwu},
  year={2026},
  url={https://github.com/HarperKollins/evosim-agentic-sociology},
  license={MIT}
}
```

---

## License

MIT License — see [LICENSE](LICENSE) for details.

---

<div align="center">

*"In the beginning was the Grid. The Grid was without form, and void.*  
*And the Higher Power said: Let there be agents."*

**⬡ EvoSim** — Where silicon souls learn to live.

</div>
