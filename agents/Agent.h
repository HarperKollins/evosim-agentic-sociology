#pragma once
// ============================================================================
// EvoSim — Agent (Expanded for Digital Gnosticism)
// Intelligent agents with neural networks, Sin overrides, and Oracle access
// FULL IMPLEMENTATION OF ALL 16 ACTIONS
// ============================================================================

#include "../nn/NeuralNetwork.h"
#include "AgentStats.h"
#include "Soul.h"
#include "Social.h"
#include "../world/World.h"
#include "../core/Oracle.h"
#include "../utils/Random.h"
#include <vector>
#include <cmath>
#include <memory>
#include <iostream>
#include <algorithm>

namespace evo {

enum class Action : int {
    MOVE_UP = 0, MOVE_DOWN = 1, MOVE_LEFT = 2, MOVE_RIGHT = 3,
    EAT = 4, DRINK = 5, ATTACK = 6, FLEE = 7,
    COOPERATE = 8, TRADE = 9, COMMUNICATE = 10, REST = 11,
    HUNT = 12, BUILD_SHELTER = 13,
    QUERY_ORACLE = 14, PRAY = 15,
    MATE = 16, INVENT = 17, FORM_TRIBE = 18
};

class Agent {
public:
    int id;
    int x, y;
    AgentStats stats;
    NeuralNetwork brain;
    Soul soul;
    float lastReward = 0.0f;
    int lastAction = -1;
    bool wasWarned = false;
    
    // Disease State
    bool isInfected = false;
    int infectionTimer = 0;

    // Social State
    int tribeId = -1;
    std::vector<int> culturalMemes;  // Indices into SocialSystem::globalMemes
    int tradeCount = 0;

    static int getNextId() {
        static int idCounter = 0;
        return ++idCounter;
    }

    // ── Constructors ────────────────────────────────────────────────────────
    Agent(int startX, int startY, int generation)
        : id(getNextId()), x(startX), y(startY),
          brain({17, 24, 19, 19}) // 17 inputs, 19 outputs
    {
        stats.generation = generation;
        stats.age = 0;
        
        // Soul: Assign random archetype and apply stat biases
        soul = Soul::createRandom();
        auto bias = Soul::getArchetypeStatBias(soul.archetype);
        stats.strength += bias.strength;
        stats.curiosity += bias.curiosity;
        stats.altruism += bias.altruism;
        stats.intelligence += bias.intelligence;
        stats.kinship += bias.kinship;
        stats.consciousness += bias.consciousness;
        stats.energy = std::min(100.0f, stats.energy + bias.energy);
        
        soul.recordEvent("Born as " + Soul::archetypeName(soul.archetype) + " (Gen " + std::to_string(generation) + ")");
    }

    Agent(const Agent& parent, int startX, int startY)
        : id(getNextId()), x(startX), y(startY),
          brain(parent.brain)
    {
        stats.generation = parent.stats.generation + 1;
        stats.kinship = parent.stats.kinship;
        stats.intelligence = parent.stats.intelligence;
        brain.mutate(0.05f, 0.1f);
        
        // Soul: Inherit from parent
        soul = Soul::inheritFrom(parent.soul);
        auto bias = Soul::getArchetypeStatBias(soul.archetype);
        stats.strength += bias.strength;
        stats.curiosity += bias.curiosity;
        stats.altruism += bias.altruism;
        stats.intelligence += bias.intelligence;
        stats.kinship += bias.kinship;
        stats.consciousness += bias.consciousness;
        stats.energy = std::min(100.0f, stats.energy + bias.energy);
        
        // Mutate personality/drives
        auto& rng = Random::instance();
        if (rng.chance(0.1f)) stats.kinship += rng.uniformFloat(-5.0f, 5.0f);
        if (rng.chance(0.1f)) stats.intelligence += rng.uniformFloat(-5.0f, 5.0f);
        if (rng.chance(0.1f)) stats.curiosity += rng.uniformFloat(-5.0f, 5.0f);
        if (rng.chance(0.1f)) stats.altruism += rng.uniformFloat(-5.0f, 5.0f);
        
        // Inherit Status (Nepotism?) - small boost
        stats.status = parent.stats.status * 0.2f;

        stats.kinship = std::clamp(stats.kinship, 0.0f, 100.0f);
        stats.intelligence = std::clamp(stats.intelligence, 0.0f, 100.0f);
        stats.curiosity = std::clamp(stats.curiosity, 0.0f, 100.0f);
        stats.altruism = std::clamp(stats.altruism, 0.0f, 100.0f);
        
        soul.recordEvent("Born as " + Soul::archetypeName(soul.archetype) + " (Gen " + std::to_string(stats.generation) + "), child of Agent " + std::to_string(parent.id));
    }

    // ── Sensors (17 Inputs) ─────────────────────────────────────────────────
    Vec sense(const World& world, const std::vector<Agent*>& neighbors) {
        Vec inputs(17);
        // 0-3 Physical
        inputs[0] = stats.health / 100.0f;
        inputs[1] = stats.satiety / 100.0f; // Inverted hunger
        inputs[2] = stats.energy / 100.0f;
        inputs[3] = stats.strength / 100.0f;
        // 4-7 Env
        const auto& cell = world.grid[y][x];
        inputs[4] = (cell.type == CellType::FOOD) ? 1.0f : 0.0f;
        inputs[5] = (cell.type == CellType::WATER) ? 1.0f : 0.0f;
        inputs[6] = (cell.type == CellType::TREE_OF_KNOWLEDGE) ? 1.0f : 0.0f; // Keep as "Ancient Relic"?
        inputs[7] = (cell.type == CellType::PREDATOR) ? 1.0f : 0.0f;
        // 8-10 Social / Tribe
        float nearestDist = findNearbyAgent(x, y, neighbors);
        inputs[8] = std::clamp(nearestDist / 20.0f, 0.0f, 1.0f);
        inputs[9] = stats.kinship / 100.0f;
        inputs[10] = stats.status / 100.0f; 
        // 11-13 World Context
        inputs[11] = world.isNight ? 1.0f : 0.0f;
        inputs[12] = static_cast<float>(cell.biome) / 5.0f;
        inputs[13] = cell.temperature / 50.0f;
        // 14-16 Psychological Drives
        inputs[14] = stats.curiosity / 100.0f;
        inputs[15] = stats.altruism / 100.0f;
        inputs[16] = stats.consciousness / 100.0f; // Self-awareness
        return inputs;
    }

    // ── Action Execution ────────────────────────────────────────────────────
    void executeAction(Action act, World& world, const std::vector<Agent*>& neighbors) {
        auto& rng = Random::instance();
        
        lastAction = static_cast<int>(act);

        // Movement Logic (Unchanged for now)
        int dx = 0, dy = 0;
        switch (act) {
            case Action::MOVE_UP:    dy = -1; break;
            case Action::MOVE_DOWN:  dy = 1; break;
            case Action::MOVE_LEFT:  dx = -1; break;
            case Action::MOVE_RIGHT: dx = 1; break;
            default: break;
        }

        if (dx != 0 || dy != 0) {
            int nx = x + dx;
            int ny = y + dy;
            world.clamp(nx, ny);
            float moveCost = 1.0f;
            Biome b = world.grid[ny][nx].biome;
            if (b == Biome::OCEAN) return; // Cannot walk on water (yet)
            
            if (b == Biome::MOUNTAIN) moveCost *= 2.0f;
            if (b == Biome::SWAMP) moveCost *= 1.5f; 
            if (b == Biome::RIVER) moveCost *= 2.0f; // Swimming is hard
            if (world.isNight) moveCost *= 1.2f;

            if (stats.energy >= moveCost) {
                x = nx; y = ny;
                stats.energy -= moveCost * 0.5f;
            }
            return;
        }

        Cell& currentCell = world.grid[y][x];

        // Simplified Action Logic for Project HUMAN
        switch (act) {
            case Action::EAT:
                if (currentCell.type == CellType::FOOD) {
                    stats.satiety = std::min(100.0f, stats.satiety + 30.0f);
                    stats.energy = std::min(100.0f, stats.energy + 10.0f);
                    currentCell.type = CellType::EMPTY;
                } else if (currentCell.type == CellType::BERRY_BUSH) {
                    stats.satiety = std::min(100.0f, stats.satiety + 15.0f);
                }
                break;

            case Action::DRINK:
                if (currentCell.type == CellType::WATER) {
                    stats.satiety = std::min(100.0f, stats.satiety + 10.0f); // Water helps satiety slightly
                }
                break;

            case Action::ATTACK: // Violence lowers Altruism, raises Status (if successful)
                stats.energy -= 5.0f;
                stats.altruism -= 2.0f;
                soul.shiftAlignment(-0.05f); // Violence pulls toward chaos
                soul.addKarma(-5.0f);
                for (auto* other : neighbors) {
                    if (other->id != id && other->x == x && other->y == y && other->isAlive()) {
                        other->stats.health -= 20.0f;
                        stats.status += 1.0f; // Dominance
                        if (other->stats.health <= 0.0f) {
                            soul.recordEvent("Killed Agent " + std::to_string(other->id));
                            soul.addKarma(-10.0f);
                        }
                        break; 
                    }
                }
                break;

            case Action::REST:
                stats.energy = std::min(100.0f, stats.energy + 10.0f);
                if (currentCell.type == CellType::SHELTER) {
                    stats.energy += 5.0f;
                    stats.health += 1.0f;
                }
                break;
            
            // Research / Pray / Meditate
            case Action::QUERY_ORACLE: 
            case Action::PRAY:
                stats.consciousness += 0.5f;
                stats.energy -= 1.0f;
                soul.shiftAlignment(0.02f); // Seeking knowledge → order
                soul.enlightenment += 0.2f;
                break;

            case Action::MATE:
                stats.kinship += 2.0f;
                stats.energy -= 10.0f;
                stats.libido = 0.0f; // Satisfaction
                stats.happiness += 5.0f; // Reward
                soul.addKarma(2.0f); // Creating life is good karma
                break;

            case Action::INVENT:
                stats.curiosity += 2.0f;
                stats.energy -= 5.0f;
                stats.satiety -= 2.0f; // Thinking makes you hungry
                if (stats.curiosity > 80.0f && Random::instance().chance(0.01f)) {
                    stats.status += 5.0f; 
                }
                break;

            case Action::FORM_TRIBE: {
                stats.kinship += 5.0f;
                stats.energy -= 5.0f;
                stats.altruism += 1.0f;
                auto& social = SocialSystem::instance();
                // If not in a tribe, try to form one with nearby agents
                if (tribeId == -1) {
                    int nearbyCount = 0;
                    for (auto* other : neighbors) {
                        if (other->id != id && other->isAlive()) {
                            float d = std::sqrt(std::pow(x - other->x, 2) + std::pow(y - other->y, 2));
                            if (d <= 5.0f) nearbyCount++;
                        }
                    }
                    if (nearbyCount >= 2) {
                        // Create tribe!
                        std::string tribeName = SocialSystem::generateTribeName();
                        int newTribeId = social.createTribe(tribeName, x, y, 0);
                        tribeId = newTribeId;
                        social.addToTribe(newTribeId, id);
                        soul.recordEvent("Founded tribe '" + tribeName + "'");
                        soul.addKarma(5.0f);
                        // Recruit nearby agents
                        for (auto* other : neighbors) {
                            if (other->id != id && other->isAlive() && other->tribeId == -1) {
                                float d = std::sqrt(std::pow(x - other->x, 2) + std::pow(y - other->y, 2));
                                if (d <= 5.0f) {
                                    other->tribeId = newTribeId;
                                    social.addToTribe(newTribeId, other->id);
                                }
                            }
                        }
                    }
                } else {
                    // Already in tribe — recruit nearby agents
                    for (auto* other : neighbors) {
                        if (other->id != id && other->isAlive() && other->tribeId == -1) {
                            float d = std::sqrt(std::pow(x - other->x, 2) + std::pow(y - other->y, 2));
                            if (d <= 3.0f && other->stats.kinship > 30.0f) {
                                other->tribeId = tribeId;
                                social.addToTribe(tribeId, other->id);
                                soul.addKarma(2.0f);
                            }
                        }
                    }
                }
                break;
            }

            case Action::COOPERATE: // Grooming / Helping
                stats.energy -= 5.0f;
                stats.altruism += 1.0f;
                stats.kinship += 1.0f;
                soul.shiftAlignment(0.03f); // Cooperation → order
                soul.addKarma(3.0f);
                for (auto* other : neighbors) {
                    if (other->id != id && other->x == x && other->y == y && other->isAlive()) {
                        other->stats.health += 5.0f;
                        other->stats.kinship += 1.0f;
                        break;
                    }
                }
                break;
            
            // Communicate / Teach — now spreads cultural memes
            case Action::COMMUNICATE: {
                stats.energy -= 2.0f;
                stats.kinship += 0.5f;
                auto& social = SocialSystem::instance();
                
                // Chance to create a new meme if agent is intelligent/curious
                if (social.globalMemes.size() < 50 && stats.curiosity > 50.0f && Random::instance().chance(0.05f)) {
                    std::string cat;
                    float roll = Random::instance().uniformFloat(0, 1);
                    if (roll < 0.4f) cat = "technique";
                    else if (roll < 0.7f) cat = "belief";
                    else cat = "tradition";
                    std::string memeName = SocialSystem::generateMemeName(cat);
                    social.createMeme(memeName, cat, stats.curiosity / 10.0f, 0);
                    int memeIdx = static_cast<int>(social.globalMemes.size()) - 1;
                    culturalMemes.push_back(memeIdx);
                    soul.recordEvent("Created meme '" + memeName + "'");
                }
                
                // Spread a meme to neighbors
                if (!culturalMemes.empty()) {
                    int memeIdx = culturalMemes[Random::instance().uniformInt(0, static_cast<int>(culturalMemes.size()) - 1)];
                    for (auto* other : neighbors) {
                        if (other->id != id && other->isAlive()) {
                            float d = std::sqrt(std::pow(x - other->x, 2) + std::pow(y - other->y, 2));
                            if (d <= 3.0f) {
                                // Check if they already have this meme
                                bool has = false;
                                for (int m : other->culturalMemes) {
                                    if (m == memeIdx) { has = true; break; }
                                }
                                if (!has && memeIdx < (int)social.globalMemes.size()) {
                                    other->culturalMemes.push_back(memeIdx);
                                    social.globalMemes[memeIdx].spreadCount++;
                                    // Apply meme bonuses
                                    other->stats.altruism += social.globalMemes[memeIdx].altruismBonus;
                                    other->stats.curiosity += social.globalMemes[memeIdx].curiosityBonus;
                                    other->soul.addKarma(social.globalMemes[memeIdx].karmaBonus);
                                }
                            }
                        }
                    }
                }
                break;
            }

            case Action::FLEE: { // Run away from nearest agent
                stats.energy -= 3.0f;
                // Move in opposite direction from nearest neighbor
                float nearX = 0, nearY = 0;
                float minD = 9999.0f;
                for (auto* other : neighbors) {
                    if (other->id != id && other->isAlive()) {
                        float d = std::sqrt(std::pow(x - other->x, 2) + std::pow(y - other->y, 2));
                        if (d < minD) { minD = d; nearX = other->x; nearY = other->y; }
                    }
                }
                if (minD < 9999.0f) {
                    int fx = x + (x > nearX ? 1 : -1);
                    int fy = y + (y > nearY ? 1 : -1);
                    world.clamp(fx, fy);
                    if (world.grid[fy][fx].biome != Biome::OCEAN) { x = fx; y = fy; }
                }
                break;
            }

            case Action::TRADE: { // Exchange resources with neighbor
                stats.energy -= 3.0f;
                for (auto* other : neighbors) {
                    if (other->id != id && other->x == x && other->y == y && other->isAlive()) {
                        // Give some energy, gain kinship
                        float gift = std::min(10.0f, stats.energy);
                        stats.energy -= gift * 0.5f;
                        other->stats.energy += gift * 0.5f;
                        stats.kinship += 3.0f;
                        other->stats.kinship += 3.0f;
                        stats.status += 1.0f;
                        tradeCount++;
                        soul.addKarma(2.0f);
                        
                        // Tribe bonus: trading with tribemate gives extra kinship
                        if (tribeId != -1 && tribeId == other->tribeId) {
                            stats.kinship += 2.0f;
                            other->stats.kinship += 2.0f;
                            // Contribute to collective tribe karma
                            auto* tribe = SocialSystem::instance().getTribe(tribeId);
                            if (tribe) tribe->collectiveKarma += 1.0f;
                        }
                        break;
                    }
                }
                break;
            }

            case Action::HUNT: // Kill prey on current cell
                stats.energy -= 5.0f;
                if (currentCell.type == CellType::PREDATOR) {
                    // Brave attempt to fight a predator
                    if (stats.strength > 60.0f && rng.chance(0.3f)) {
                        currentCell.type = CellType::FOOD;
                        stats.status += 5.0f;
                        stats.satiety += 20.0f;
                    } else {
                        stats.health -= 15.0f; // Predator fights back
                    }
                } else {
                    // Forage-hunt: small chance to find food
                    if (rng.chance(0.2f)) {
                        stats.satiety += 15.0f;
                        stats.strength += 0.5f;
                    }
                }
                break;

            case Action::BUILD_SHELTER: // Needs Energy + Intelligence
                if (stats.energy > 30.0f && currentCell.type == CellType::EMPTY) {
                     stats.energy -= 30.0f;
                     currentCell.type = CellType::SHELTER;
                     stats.status += 2.0f; // Builder status
                     soul.recordEvent("Built a shelter");
                     soul.addKarma(5.0f);
                }
                break;
        }
    }

    void onEatForbiddenFruit(World& world) {
        // Legacy or transform into "Monolith" discovery?
        // Acts as "Instant Consciousness Boost"
        stats.consciousness += 50.0f;
        stats.curiosity += 50.0f;
        soul.recordEvent("Discovered the Monolith!");
        soul.enlightenment += 20.0f;
        soul.shiftAlignment(-0.2f); // Forbidden knowledge → chaos
        soul.addKarma(-10.0f); // Disobedience
        if (!world.treeOfKnowledgeEaten) {
            world.treeOfKnowledgeEaten = true;
            std::cout << "\n[!!!] AGENT " << id << " DISCOVERED THE MONOLITH! [!!!]\n";
        }
    }

    void updateStats(const World& world) {
        float metabolicRate = 0.5f; 
        
        stats.age++;
        stats.satiety -= metabolicRate;
        stats.energy -= 0.1f; // Living costs energy
        
        // Decay/Growth of Drives
        stats.curiosity += 0.1f; // Humans get bored
        stats.status -= 0.05f;   // Fame is fleeting
        stats.kinship -= 0.1f;   // Relationships degrade if ignored

        // Biological Imperative (The "Horny" Update)
        if (stats.age > 15) {
            stats.libido += 4.0f; // Hormones kick in HARD
        }
        
        // Frustration (Blue Balls mechanic)
        if (stats.libido > 80.0f) {
            stats.energy -= 0.5f; // Sexual frustration drains energy/focus
            stats.altruism -= 0.1f; // Get grumpy
        }

        // Health effects
        if (stats.satiety <= 0.0f) stats.health -= 5.0f;
        if (stats.energy <= 0.0f) stats.health -= 5.0f; // Exhaustion damage
        if (stats.age > 80) stats.health -= (stats.age - 80) * 0.5f;
        
        // Clamping
        stats.satiety = std::clamp(stats.satiety, 0.0f, 100.0f);
        stats.health = std::clamp(stats.health, 0.0f, 100.0f);
        stats.energy = std::clamp(stats.energy, 0.0f, 100.0f);
        
        stats.kinship = std::clamp(stats.kinship, 0.0f, 100.0f);
        stats.status = std::clamp(stats.status, 0.0f, 100.0f);
        stats.curiosity = std::clamp(stats.curiosity, 0.0f, 100.0f);
        stats.altruism = std::clamp(stats.altruism, 0.0f, 100.0f);
        stats.consciousness = std::clamp(stats.consciousness, 0.0f, 100.0f);
        stats.libido = std::clamp(stats.libido, 0.0f, 100.0f);
        stats.happiness = std::clamp(stats.happiness, 0.0f, 100.0f);
    }

    bool isAlive() const { return stats.health > 0.0f; }

    bool canReproduce() const {
        float energyReq = 60.0f;
        if (stats.kinship > 80.0f) energyReq = 30.0f; 
        return stats.energy > energyReq && stats.health > 30.0f && stats.age > 10;
    }

    std::unique_ptr<Agent> reproduce(World& world) {
        stats.energy -= 30.0f;
        // stat penalty/cost
        return std::make_unique<Agent>(*this, x, y);
    }
    
    std::string causeOfDeath() const {
        if (stats.health <= 0.0f) return "injury";
        if (stats.satiety <= 0.0f) return "starvation";
        if (stats.age > 1000) return "old_age";
        return "unknown";
    }

private:
    float findNearbyAgent(int cx, int cy, const std::vector<Agent*>& neighbors) {
        float minDist = 9999.0f;
        for (const auto* other : neighbors) {
             // Skip self (check by pointer or ID)
             // But 'this' vs 'other' pointer check is safe
            if (other == this) continue;
            // Also skip dead? The spatial hash usually stores active ones, but double check
            if (!other->isAlive()) continue;

            float dist = std::sqrt(std::pow(cx - other->x, 2) + std::pow(cy - other->y, 2));
            if (dist < minDist) minDist = dist;
        }
        return minDist;
    }
};

} // namespace evo
