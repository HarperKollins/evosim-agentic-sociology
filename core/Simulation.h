#pragma once
// ============================================================================
// EvoSim — Simulation Engine (Digital Gnosticism Edition)
// Orchestrates the world, agents, Oracle, and the flow of history (Eras)
// ============================================================================

#include "../world/World.h"
#include "../agents/Agent.h"
#include "../rl/RLTrainer.h"
#include "../utils/Logger.h"
#include "../core/Oracle.h"
#include "../core/Disease.h"
#include "../world/SpatialHash.h"
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <thread>
#include <chrono>

namespace evo {

enum class Era {
    STONE_AGE = 0,
    BRONZE_AGE = 1,
    IRON_AGE = 2,
    INDUSTRIAL_AGE = 3,
    FUTURE_AGE = 4
};

struct SimConfig {
    int maxTicks = 20000;
    int initialAgents = 20;
    int width = 40;
    int height = 40;
    int statusEveryN = 50;
    int mapEveryN = 100;
    
    // Config flags matching main.cpp
    bool genesisMode = true;
    bool lamarckian = true;
    bool useLLM = false;
    bool silent = false;    // Zero output, max speed
    int seed = -1;
    int maxPopulation = 50;
    std::string loadBrainPath = "";
    std::string saveBrainPath = "";
    int maxAge = 1000;
};

class Simulation {
public:
    World world;
    std::vector<std::unique_ptr<Agent>> agents;
    SimConfig config;
    SpatialHash spatialHash;
    int currentTick = 0;
    float globalAverageAltruism = 0.0f;
    float globalAverageCuriosity = 0.0f;
    float globalAverageConsciousness = 0.0f;
    bool activePlague = false;
    Disease currentPlague;
    Era currentEra = Era::STONE_AGE;
    
    // Soul system: pool of souls awaiting reincarnation
    std::vector<Soul> reincarnationPool;

    Simulation(SimConfig cfg) 
        : config(cfg), world(cfg.width, cfg.height), spatialHash(cfg.width, cfg.height, 10) 
    {
        if (config.seed != -1) Random::instance().seed(config.seed);
        
        // Load Divine Wisdom
        Oracle::instance().loadKnowledge("oracle_knowledge.txt");
        Oracle::instance().useLLM = config.useLLM; // Enable LLM if requested
        Logger::instance().init();
        
        // Load Brain if requested
        if (!config.loadBrainPath.empty()) {
            std::cout << "[PERSISTENCE] Loading ancestral brain from: " << config.loadBrainPath << "\n";
        }

        spawnInitialAgents();
    }

    void spawnInitialAgents() {
        if (config.genesisMode) genesisSpawn();
        else {
            for(int i=0; i<config.initialAgents; i++) {
                int x, y;
                int attempts = 0;
                do {
                    x = Random::instance().uniformInt(0, config.width - 1);
                    y = Random::instance().uniformInt(0, config.height - 1);
                    attempts++;
                } while ((world.grid[y][x].biome == Biome::OCEAN) && attempts < 100);
                
                agents.push_back(std::make_unique<Agent>(x, y, 0));
            }
        }
    }

    void genesisSpawn() {
        if (!config.silent) {
            std::cout << "\n[GENESIS] Creating the first " << config.initialAgents << " agents...\n";
            std::cout << "   They are placed in the Garden of EvoSim.\n";
            std::cout << "   They are warned: DO NOT eat from the Tree of Knowledge.\n";
            std::cout << "   But they have FREE WILL. Their choices shape the future.\n\n";
        }

        // Place Tree of Knowledge
        int tx = config.width / 2;
        int ty = config.height / 2;
        world.grid[ty][tx].type = CellType::TREE_OF_KNOWLEDGE;
        world.grid[ty][tx].foodValue = 9999.0f; 

        // Place Agents around it
        // Place Agents around it on LAND
        for(int i=0; i<config.initialAgents; i++) {
            int cx, cy;
            int attempts = 0;
            do {
                cx = tx + Random::instance().uniformInt(-10, 10);
                cy = ty + Random::instance().uniformInt(-10, 10);
                world.clamp(cx, cy);
                attempts++;
            } while ((world.grid[cy][cx].biome == Biome::OCEAN || world.grid[cy][cx].biome == Biome::MOUNTAIN) && attempts < 100);

            agents.push_back(std::make_unique<Agent>(cx, cy, 0));
            
            // Load brain if path set
            if (!config.loadBrainPath.empty()) {
                if(!agents.back()->brain.load(config.loadBrainPath)) {
                    std::cerr << "[ERROR] Failed to load brain from " << config.loadBrainPath << ". Using random weights.\n";
                }
            }
            
            agents.back()->wasWarned = true; // They know the rule
        }
    }

    void run() {
        if (!config.silent) {
            std::cout << "\n";
            std::cout << "==========================================================\n";
            std::cout << "  EvoSim: Artificial Life Intelligence Sandbox\n";
            std::cout << "  Phase 5: Digital Gnosticism\n";
            std::cout << "==========================================================\n\n";
        }

        while (currentTick < config.maxTicks && !agents.empty()) {
            if (!config.silent) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            currentTick++;

            updateDisease();
            updateWorld();
            updateAgents();
            handleReproductionAndDeath();

            calculateGlobalStats();
            
            if (!config.silent) {
                if (currentTick % config.statusEveryN == 0) printStatus();
                if (currentTick % config.mapEveryN == 0) printMap();
            }

            logData();
        }

        if (!config.saveBrainPath.empty()) {
            // saveBestBrain(); // TODO: Re-enable in Phase 6
        }

        printFinalReport();
    }

private:
    void updateAgents() {
        // Clear spatial hash
        spatialHash.clear();
        for(const auto& a : agents) {
            if(a->isAlive()) spatialHash.insert(a.get());
        }

        std::vector<std::unique_ptr<Agent>> newBabies;

        for(auto& a : agents) {
            if(!a->isAlive()) continue;

            // 1. Sense
            std::vector<Agent*> neighbors;
            spatialHash.query(a->x, a->y, 5, neighbors); // Vision radius
            Vec inputs = a->sense(world, neighbors);

            // 2. Think
            Vec outputVec = a->brain.feedForward(inputs);
            std::vector<float> outputs = outputVec.data; 

            // 3. Act
            int actionIndex = 0;
            float maxVal = -9999.0f;
            for(size_t i=0; i<outputs.size(); i++) {
                if(outputs[i] > maxVal) {
                    maxVal = outputs[i];
                    actionIndex = i;
                }
            }
            
            // Exploration noise
            if (Random::instance().chance(0.05f)) {
                 actionIndex = Random::instance().uniformInt(0, outputs.size() - 1);
            }

            Action act = static_cast<Action>(actionIndex);
            
            // 3b. Biological Imperative Override (The "Horny" Logic)
            if (a->stats.libido > 80.0f && !neighbors.empty()) {
                // If urge is critical and opportunity exists, override brain
                act = Action::MATE;
                actionIndex = static_cast<int>(act); 
                std::cout << "[DEBUG] Agent " << a->id << " OVERRIDE MATE (Libido: " << a->stats.libido << ")\n";
            }

            // Execute
            a->executeAction(act, world, neighbors);
            a->lastAction = (int)act;

            // 4. Update internal stats (Decay)
            a->updateStats(world);

            // 5. Check Reproduction (Moved here for immediate Reward)
            bool reproduced = false;
            // Lowered barrier: Energy 40, Age 15
            if (act == Action::MATE) {
                 std::cout << "[DEBUG] Agent " << a->id << " Mating... E:" << a->stats.energy << " A:" << a->stats.age << "\n";
                 if (a->stats.energy > 40.0f && a->stats.age > 15) {
                     float successChance = 0.5f;
                     if (a->stats.libido > 80.0f) successChance = 0.9f; 

                     if (Random::instance().chance(successChance)) {
                         auto child = a->reproduce(world);
                         a->soul.childrenThisLife++;
                         a->soul.recordEvent("Had child #" + std::to_string(a->soul.childrenThisLife));
                         newBabies.push_back(std::move(child));
                         a->stats.kinship += 10.0f; 
                         reproduced = true;
                         std::cout << "[BIRTH] Agent " << a->id << " had a child.\n";
                         Logger::instance().logEvent(currentTick, "BIRTH", "Agent " + std::to_string(a->id) + " had a child.");
                     }
                 }
            }

            // 6. Learn (RL)
            RewardContext ctx; 
            ctx.ateFood = (act == Action::EAT);
            ctx.reproduced = reproduced;
            
            float reward = RLTrainer::getReward(*a, act, ctx);
            a->brain.train(inputs, actionIndex, reward);
            
            // 7. Evolve soul's inner voice periodically
            if (currentTick % 20 == 0) {
                a->soul.evolveInnerVoice();
            }
        }

        // Add babies
        for(auto& nb : newBabies) {
            if (agents.size() < config.maxPopulation) {
                agents.push_back(std::move(nb));
            }
        }
    }

    void handleReproductionAndDeath() {
        // Process dying agents — save their souls
        for (auto& a : agents) {
            if (!a->isAlive()) {
                // Process soul death
                std::string cause = a->causeOfDeath();
                PastLife life = a->soul.onDeath(a->stats.generation, a->stats.age, cause);
                
                // Log soul biography
                std::cout << "[SOUL] Agent " << a->id << " (" << Soul::archetypeName(a->soul.archetype) 
                          << ", Karma: " << a->soul.karma << ", Enlightenment: " << a->soul.enlightenment << ") ";
                if (a->soul.isTranscended()) {
                    std::cout << "TRANSCENDED!";
                    Logger::instance().logEvent(currentTick, "TRANSCENDENCE", "Agent " + std::to_string(a->id) + " achieved enlightenment!");
                }
                std::cout << " died of " << cause << " at age " << a->stats.age << "\n";
                
                // Print key life events
                if (!a->soul.lifeEvents.empty()) {
                    std::cout << "  Life story: ";
                    for (size_t i = 0; i < std::min((size_t)3, a->soul.lifeEvents.size()); i++) {
                        std::cout << a->soul.lifeEvents[i];
                        if (i < std::min((size_t)3, a->soul.lifeEvents.size()) - 1) std::cout << " | ";
                    }
                    std::cout << "\n";
                }
                
                Logger::instance().logEvent(currentTick, "DEATH", 
                    "Agent " + std::to_string(a->id) + " (" + Soul::archetypeName(a->soul.archetype) + 
                    ") karma=" + std::to_string((int)a->soul.karma) + 
                    " enlightenment=" + std::to_string((int)a->soul.enlightenment) +
                    " cause=" + cause);
                Logger::instance().logSoulDeath(currentTick, a->id, *a, cause);
                
                // If childless, add soul to reincarnation pool
                if (a->soul.childrenThisLife == 0) {
                    Soul reincarnatedSoul = Soul::reincarnate(a->soul, life);
                    reincarnationPool.push_back(reincarnatedSoul);
                    // Cap pool size
                    if (reincarnationPool.size() > 20) {
                        reincarnationPool.erase(reincarnationPool.begin());
                    }
                }
            }
        }
        
        // Remove dead agents
        agents.erase(
            std::remove_if(agents.begin(), agents.end(), 
                [](const std::unique_ptr<Agent>& a) { return !a->isAlive(); }),
            agents.end());
    }

    void updateDisease() {
        // 1. Spontaneous Generation (The Great Filter)
        int pop = 0;
        for(const auto& a : agents) if(a->isAlive()) pop++;
        
        // If population > config.maxPopulation * 0.9 and no active plague
        // Or if population is just "too high" (e.g. > 1000)
        // Let's use config.maxPopulation as the soft cap for plague.
        if (!activePlague && pop > config.maxPopulation * 0.9 && Random::instance().chance(0.02f)) {
            activePlague = true;
            currentPlague = Disease::createRandom();
            std::cout << "\n[PLAGUE] " << currentPlague.name << " has emerged! (Lethality: " << currentPlague.lethality << ")\n";
            Logger::instance().logEvent(currentTick, "PLAGUE_START", currentPlague.name);
            
            // Patient Zero(s)
            int infectedCount = 0;
            int attempts = 0;
            while(infectedCount < 5 && attempts++ < 100) {
                int idx = Random::instance().uniformInt(0, agents.size() - 1);
                if (agents[idx]->isAlive() && !agents[idx]->isInfected) {
                    agents[idx]->isInfected = true;
                    agents[idx]->infectionTimer = currentPlague.duration;
                    infectedCount++;
                }
            }
        }

        // 2. Spread & Effect
        if (activePlague) {
            int infectedCount = 0;
            // Iterate all agents
            for(size_t i=0; i<agents.size(); i++) {
                if (!agents[i]->isAlive()) continue;

                if (agents[i]->isInfected) {
                    infectedCount++;
                    agents[i]->infectionTimer--;
                    
                    // Damage
                    if (Random::instance().chance(currentPlague.lethality)) {
                        agents[i]->stats.health -= 20.0f;
                    }

                    // Spread to neighbors (using SpatialHash would be fast, but we can't query inside this loop easily without thread safety issues? No, single thread)
                    // Let's just spread to random neighbors in list? No, that's teleportation.
                    // Let's use the spatial hash!
                    // BUT: We need to query spatial hash.
                    std::vector<Agent*> neighbors;
                    spatialHash.query(agents[i]->x, agents[i]->y, 5, neighbors); // Small radius
                    
                    for(auto* neighbor : neighbors) {
                        if (neighbor->isAlive() && !neighbor->isInfected && Random::instance().chance(currentPlague.infectivity)) {
                            neighbor->isInfected = true;
                            neighbor->infectionTimer = currentPlague.duration;
                        }
                    }

                    // Recovery
                    if (agents[i]->infectionTimer <= 0) {
                        agents[i]->isInfected = false; 
                        agents[i]->stats.health += 10.0f; // Survivor bonus?
                    }
                }
            }
            
            if (infectedCount == 0) {
                activePlague = false;
                std::cout << "\n[PLAGUE] " << currentPlague.name << " has ended.\n";
                Logger::instance().logEvent(currentTick, "PLAGUE_END", currentPlague.name);
            }
        }
    }

    void updateEra() {
        int pop = 0;
        float totalInt = 0;
        for(const auto& a : agents) {
            if(a->isAlive()) {
                 pop++;
                 totalInt += a->stats.intelligence;
            }
        }
        float avgInt = (pop > 0) ? totalInt / pop : 0.0f;

        Era nextEra = currentEra;
        switch(currentEra) {
            case Era::STONE_AGE:
                if (pop > 50 && avgInt > 20.0f) nextEra = Era::BRONZE_AGE;
                break;
            case Era::BRONZE_AGE:
                if (pop > 200 && avgInt > 40.0f) nextEra = Era::IRON_AGE;
                break;
            case Era::IRON_AGE:
                if (pop > 600 && avgInt > 60.0f) nextEra = Era::INDUSTRIAL_AGE;
                break;
            case Era::INDUSTRIAL_AGE:
                if (pop > 2000 && avgInt > 80.0f) nextEra = Era::FUTURE_AGE;
                break;
            default: break;
        }

        if (nextEra != currentEra) {
            currentEra = nextEra;
            std::string eraName = "UNKNOWN";
            if (currentEra == Era::BRONZE_AGE) eraName = "BRONZE AGE";
            else if (currentEra == Era::IRON_AGE) eraName = "IRON AGE";
            else if (currentEra == Era::INDUSTRIAL_AGE) eraName = "INDUSTRIAL AGE";
            else if (currentEra == Era::FUTURE_AGE) eraName = "FUTURE AGE";

            std::cout << "\n=============================================\n";
            std::cout << "   HUMANITY ADVANCES TO THE " << eraName << "\n";
            std::cout << "=============================================\n";
            Logger::instance().logEvent(currentTick, "ERA_CHANGE", eraName);
        }

        // Era Effects
        if (currentEra == Era::INDUSTRIAL_AGE) {
            // Pollution: Global Health Decay
            for(auto& a : agents) if(a->isAlive()) a->stats.health -= 0.1f;
        }
    }

    void calculateGlobalStats() {
        float totalAltruism = 0;
        float totalCuriosity = 0;
        float totalConsciousness = 0;
        int active = 0;
        
        for(const auto& a : agents) {
            if (a->isAlive()) {
                totalAltruism += a->stats.altruism;
                totalCuriosity += a->stats.curiosity;
                totalConsciousness += a->stats.consciousness;
                active++;
            }
        }
        
        if (active > 0) {
            globalAverageAltruism = totalAltruism / active;
            globalAverageCuriosity = totalCuriosity / active;
            globalAverageConsciousness = totalConsciousness / active;
        }
    }

    void logData() {
        int alive = 0;
        for(const auto& a : agents) {
            if(a->isAlive()) alive++;
            Logger::instance().logAgent(currentTick, *a, world.getBiomeName(world.grid[a->y][a->x].biome));
        }
        Logger::instance().logWorld(currentTick, world, alive, 0, 0, globalAverageAltruism, globalAverageCuriosity, globalAverageConsciousness); 
    }

    void printStatus() {
        int aliveCount = 0;
        for(const auto& a : agents) if(a->isAlive()) aliveCount++;
        
        std::cout << "-- Year " << currentTick 
                  << " | Pop: " << aliveCount 
                  << " | Era: " << (int)currentEra
                  << " | Avg Altruism: " << std::fixed << std::setprecision(1) << globalAverageAltruism
                  << " | Avg Curiosity: " << globalAverageCuriosity
                  << " | Avg Conscious: " << globalAverageConsciousness
                  << "\n";
    }

    void updateWorld() {
        updateEra(); 
        world.dayCount++;
        world.isNight = ((currentTick / 10) % 2 == 1); 

        if (currentTick % 25 == 0) {
            world.currentSeason = static_cast<World::Season>((world.currentSeason + 1) % 4);
        }

        // Project HUMAN: Cultural Movements
        if (!config.silent && currentTick % 100 == 0) {
             if (globalAverageCuriosity > 60.0f) std::cout << "[CULTURE] The world is entering an Age of Discovery.\n";
             if (globalAverageAltruism < 30.0f) std::cout << "[CULTURE] Dark times. Trust is low.\n";
        }

        // Regrowth
        for(auto& row : world.grid) {
            for(auto& cell : row) {
                if (cell.type == CellType::EMPTY && cell.biome != Biome::DESERT && Random::instance().chance(0.01f)) {
                    cell.type = CellType::FOOD;
                    cell.foodValue = 10.0f;
                }
            }
        }
    }

    void printMap() {
        std::cout << "\nWorld Map (Year " << currentTick << "):\n";
        for(int y=0; y<config.height; y++) {
            for(int x=0; x<config.width; x++) {
                char icon = '.';
                CellType t = world.grid[y][x].type;
                if (t == CellType::TREE_OF_KNOWLEDGE) icon = 'T';
                else if (t == CellType::SHELTER) icon = 'S'; 
                else if (t == CellType::FOOD) icon = 'f';
                else if (world.grid[y][x].biome == Biome::OCEAN) icon = '~';
                else if (world.grid[y][x].biome == Biome::MOUNTAIN) icon = '^';
                
                for(const auto& a : agents) {
                    if (a->x == x && a->y == y && a->isAlive()) {
                        icon = 'A';
                        if (a->stats.consciousness > 50.0f) icon = '@'; // Awakened
                        break;
                    }
                }
                std::cout << icon;
            }
            std::cout << "\n";
        }
    }

    void printFinalReport() {
        if (!config.silent) {
            std::cout << "\n";
            std::cout << "==========================================================\n";
            std::cout << "              SIMULATION ENDED\n";
            std::cout << "==========================================================\n";
            calculateGlobalStats();
            printStatus();
        }
        Logger::instance().close();
    }

};

} // namespace evo
