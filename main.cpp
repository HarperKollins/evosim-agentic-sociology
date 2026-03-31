// ============================================================================
// EvoSim: Artificial Life Intelligence Sandbox
// Genesis Edition v2.0 -- Expanded World with Biomes, Predators, Cooperation
//
// "In the beginning, there were agents in a garden.
//  They were told not to eat from the Tree of Knowledge.
//  They had free will. Whatever choice they make determines the future."
//
// Author: Collins Universe
// Built with: C++17, custom neural networks, Q-learning, Gaussian evolution
// ============================================================================

#include "core/Simulation.h"
#include <iostream>
#include <string>
#include <cstring>

void printHelp() {
    std::cout << R"(
EvoSim: Artificial Life Intelligence Sandbox -- Genesis Edition v2.0

Usage: EvoSim [options]

Options:
  --ticks N        Max simulation ticks (default: 20000)
  --infinite       Run infinitely until agents go extinct
  --agents N       Initial agent count (default: 20)
  --grid N         Grid size NxN (default: 40)
  --maxpop N       Max population cap (default: 50)
  --lamarckian     Inherited learned weights (default)
  --darwinian      Reset child brain, only mutate structure
  --no-genesis     Disable Tree of Knowledge mechanic
  --llm            Enable LLM Oracle (requires Qwen model & python)
  --seed N         Random seed for reproducibility
  --quiet          Less verbose output
  --help           Show this help

Features:
  - 7 Biomes (Ocean, Plains, Forest, Desert, Swamp, Mountain, River)
  - 4 Seasons (Spring, Summer, Autumn, Winter)
  - Day/Night cycle with behavioral effects
  - Complex Human Drives (Kinship, Status, Curiosity, Altruism, Consciousness)
  - Project H.U.M.A.N. Mechanics: Emergent Culture & Technology

Examples:
  EvoSim                           # Default Genesis run
  EvoSim --ticks 5000 --agents 2   # Longer run, Adam & Eve style
  EvoSim --darwinian --no-genesis  # Pure evolution, no Genesis lore
  EvoSim --seed 42 --ticks 1000   # Reproducible run
)";
}

int main(int argc, char* argv[]) {
    evo::SimConfig config;

    // ── Parse CLI Arguments ─────────────────────────────────────────────────
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            printHelp();
            return 0;
        }
        else if (arg == "--ticks" && i + 1 < argc) {
            config.maxTicks = std::stoi(argv[++i]);
        }
        else if (arg == "--infinite") {
            config.maxTicks = -1;
        }
        else if (arg == "--agents" && i + 1 < argc) {
            config.initialAgents = std::stoi(argv[++i]);
        }
        else if (arg == "--grid" && i + 1 < argc) {
            int size = std::stoi(argv[++i]);
            config.width = size;
            config.height = size;
        }
        else if (arg == "--maxpop" && i + 1 < argc) {
            config.maxPopulation = std::stoi(argv[++i]);
        }
        else if (arg == "--lamarckian") {
            config.lamarckian = true;
        }
        else if (arg == "--darwinian") {
            config.lamarckian = false;
        }
        else if (arg == "--no-genesis") {
            config.genesisMode = false;
        }
        else if (arg == "--llm") {
            config.useLLM = true;
            std::cout << "[CONFIG] LLM Oracle ENABLED. Expect slower simulation speeds during queries.\n";
        }
        else if (arg == "--seed" && i + 1 < argc) {
            config.seed = std::stoi(argv[++i]);
        }
        else if (arg == "--quiet") {
            config.statusEveryN = 100;
            config.mapEveryN = 1000;
        }
        else if (arg == "--silent") {
            config.silent = true;
        }
        else {
            std::cerr << "Unknown argument: " << arg << "\n";
            std::cerr << "Use --help to see available options.\n";
            return 1;
        }
    }

    // ── Safety checks for low-end hardware ──────────────────────────────────
    if (config.maxPopulation > 10000) {
        std::cout << "[!] Warning: maxPopulation > 10000 may be heavy on your system.\n";
        std::cout << "   Capping at 10000 for safety.\n";
        config.maxPopulation = 10000;
    }
    if (config.width > 100 || config.height > 100) {
        std::cout << "[!] Warning: Grid > 100x100 may be slow. Capping at 100.\n";
        config.width = std::min(config.width, 100);
        config.height = std::min(config.height, 100);
    }

    // ── Create and Run Simulation ───────────────────────────────────────────
    evo::Simulation sim(config);
    sim.run();

    return 0;
}
