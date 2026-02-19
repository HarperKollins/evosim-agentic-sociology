# Contributing to EvoSim

Thank you for your interest in contributing to EvoSim! This project explores emergent sociology in agent-based simulations, and we welcome contributions of all kinds.

## Getting Started

### Development Setup

1. **Clone the repo**
   ```bash
   git clone https://github.com/HarperKollins/evosim-agentic-sociology.git
   cd evosim-agentic-sociology
   ```

2. **Get a C++17 compiler**
   - **Windows**: Download [w64devkit](https://github.com/skeeto/w64devkit/releases) and add its `bin/` to your PATH
   - **Linux/macOS**: `sudo apt install g++` or `brew install gcc`

3. **Build**
   ```bash
   g++ -std=c++17 -O2 -o EvoSim.exe main.cpp
   ```

4. **Run a test simulation**
   ```bash
   ./EvoSim.exe --ticks 100 --agents 20 --grid 20 --silent
   ```

5. **View results**
   ```bash
   python -m http.server 9000
   # Open http://localhost:9000/viewer/index.html
   ```

### Project Structure

| Directory | Purpose |
|-----------|---------|
| `agents/` | Agent brains, stats, souls, and social systems |
| `core/` | Simulation engine, Oracle, disease mechanics |
| `nn/` | Neural network implementation (from scratch) |
| `rl/` | Reinforcement learning (Q-learning) |
| `world/` | Grid world, biomes, spatial hashing |
| `utils/` | Logging and RNG utilities |
| `viewer/` | Browser-based 2D and 3D visualizations |
| `docs/` | Technical documentation |

## Code Style

- **Header-only C++17** — all code lives in `.h` files for simplicity
- **Namespace**: everything is in `namespace evo {}`
- Use `camelCase` for functions and variables, `PascalCase` for types
- Prefer `float` over `double` for agent stats (performance)
- Keep header files self-contained — include what you use

## How to Contribute

### Adding a New Agent Action

1. Add the action to the `Action` enum in `agents/Agent.h`
2. Implement the behavior in `Agent::executeAction()` (the main switch block)
3. Add sensory inputs if needed in `Agent::buildInputVector()`
4. Update the neural network output count in `NeuralNetwork.h` if needed

### Adding a New Biome

1. Add the biome to the `Biome` enum in `world/World.h`
2. Update `World::generate()` with terrain generation rules
3. Update `World::biomeEffects()` with stat modifiers
4. Add the biome color to `viewer/script.js` and `viewer/viewer3d.js`

### Adding a New Soul Archetype

1. Add the archetype to the `Archetype` enum in `agents/Soul.h`
2. Update `Soul::determineArchetype()` with classification logic
3. Update `Soul::archetypeName()` with the display name
4. Add archetype visuals in `viewer/script.js` (shape + color)

### Adding a New Cultural Meme

1. Add the meme name to `SocialSystem::generateMemeName()` in `agents/Social.h`
2. Optionally add a new category with custom stat bonuses in `SocialSystem::createMeme()`

## Pull Request Process

1. **Fork the repo** and create a feature branch (`git checkout -b feature/my-feature`)
2. **Write clean code** following the style guide above
3. **Test your changes** — run a simulation and verify no crashes
4. **Update documentation** if you've added new features, flags, or data columns
5. **Submit a PR** with a clear description of what changed and why

### Issue Labels

| Label | Description |
|-------|-------------|
| `bug` | Something isn't working |
| `enhancement` | New feature or improvement |
| `research` | Related to research questions or experiments |
| `documentation` | Docs improvements |
| `good first issue` | Beginner-friendly tasks |
| `performance` | Speed or memory optimization |

## Research Contributions

If you're using EvoSim for research, we'd love to hear about it! Consider:

- Sharing your experiment configurations and results
- Writing up findings as a docs/experiments/ markdown file
- Publishing analysis notebooks for the CSV output data
- Proposing new research questions as GitHub issues

## Questions?

Open an issue or start a discussion on the GitHub repo. All contributions are valued — from fixing typos to implementing entirely new systems.
