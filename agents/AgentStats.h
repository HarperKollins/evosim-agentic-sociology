#pragma once
// ============================================================================
// EvoSim — Agent Stats (Expanded for Digital Gnosticism)
// Holds all vital statistics including Sin, Belief, and Money for Phase 5
// ============================================================================

struct AgentStats {
    int generation = 0;
    int age = 0;

    // Physical Needs (0-100)
    float health = 100.0f;
    float satiety = 100.0f; // Replaces Hunger/Thirst (Abstracted)
    float energy = 100.0f;  // Stamina for actions

    // Psychological Drives (0-100)
    // "Maslow's Hierarchy"
    float kinship = 50.0f;    // Need for family/tribe (was Social)
    float status = 0.0f;      // Need for dominance/respect (was Greed)
    float curiosity = 10.0f;  // Need for knowledge/exploration (was Lust/Intel)
    float altruism = 50.0f;   // Need to help others (was Sin axis)
    float libido = 0.0f;      // Reproductive drive (New)
    float happiness = 50.0f;  // General well-being (New)
    
    // Higher Consciousness
    float consciousness = 0.0f; // Awareness of self/world (emergent)

    // Legacy/Core
    float intelligence = 50.0f; // Base processing power
    float strength = 50.0f;     // Physical power

    // ── Behavioral Tracking ─────────────────────────────────────────────────
    int killCount = 0;
    int cooperateCount = 0;
    int tradeCount = 0;
    int sheltersBuilt = 0;
    int preyHunted = 0;
    int oracleQueries = 0;     // New: Number of times consulted the Oracle

    // ── Genesis State ───────────────────────────────────────────────────────
    bool hasEatenForbiddenFruit = false;

    // ── Memory (Short-term spatial awareness) ──────────────────────────────
    static const int MEMORY_SIZE = 5;
    int memoryX[MEMORY_SIZE];
    int memoryY[MEMORY_SIZE];
    int memoryIndex = 0;

    AgentStats() {
        for(int i=0; i<MEMORY_SIZE; i++) { memoryX[i]=-1; memoryY[i]=-1; }
    }

    void recordPosition(int x, int y) {
        memoryX[memoryIndex] = x;
        memoryY[memoryIndex] = y;
        memoryIndex = (memoryIndex + 1) % MEMORY_SIZE;
    }

    bool hasVisitedRecently(int x, int y) const {
        for(int i=0; i<MEMORY_SIZE; i++) {
            if (memoryX[i] == x && memoryY[i] == y) return true;
        }
        return false;
    }
};
