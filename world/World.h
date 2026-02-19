#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include "../utils/Random.h"
#include "../agents/AgentStats.h" // For Biome enum if moved, or Cell struct

namespace evo {

enum class CellType {
    EMPTY,
    FOOD,
    WATER,
    WALL,
    AGENT,
    PREDATOR,     // New: Predators (Demons in Phase 5?)
    SHELTER,      // New: Shelter/Temple
    BERRY_BUSH,   // New: Small food source
    TREE_OF_KNOWLEDGE // Genesis
};

enum class Biome {
    OCEAN = 0,
    PLAINS = 1,
    FOREST = 2,
    DESERT = 3,
    SWAMP = 4,
    MOUNTAIN = 5,
    RIVER = 6
};

struct Cell {
    CellType type = CellType::EMPTY;
    int agentId = -1; 
    float foodValue = 0.0f;
    Biome biome = Biome::OCEAN;
    float temperature = 20.0f; 
    float height = 0.0f; // Heightmap value for 3D logic later?
};

class World {
public:
    int width, height;
    std::vector<std::vector<Cell>> grid;
    
    // Day/Night Cycle
    bool isNight = false;
    int dayCount = 0;
    
    // Seasons
    enum Season { SPRING, SUMMER, AUTUMN, WINTER };
    Season currentSeason = SPRING;
    int ticksInSeason = 0;

    // Genesis State
    bool treeOfKnowledgeEaten = false;
    int treeEatenAtTick = -1;
    int treeEatenByAgent = -1;

    // Project HUMAN: Cultural State
    // No hardcoded ProphetId. We track "Memes" or "Philosophies" in Simulation.h
    
    World(int w, int h) : width(w), height(h) {
        grid.resize(height, std::vector<Cell>(width));
        generateTerrain();
    }

    void generateTerrain() {
        auto& rng = Random::instance();
        // Improved "Continents" using Simplex-ish noise
        float scale = 0.1f;
        for(int y=0; y<height; y++) {
            for(int x=0; x<width; x++) {
                // Layered noise for continents
                float nx = x * scale;
                float ny = y * scale;
                float noise = std::sin(nx) + std::cos(ny) + std::sin(nx * 2.0f + ny) * 0.5f;
                // Height: -2 to 2 roughly
                
                grid[y][x].height = noise;

                if (noise < -0.2f) {
                    grid[y][x].biome = Biome::OCEAN;
                    grid[y][x].type = CellType::WATER; // Ocean is water
                }
                else if (noise < 0.0f) grid[y][x].biome = Biome::SWAMP; // Coast / Marsh
                else if (noise < 0.5f) grid[y][x].biome = Biome::PLAINS;
                else if (noise < 1.0f) grid[y][x].biome = Biome::FOREST;
                else grid[y][x].biome = Biome::MOUNTAIN;

                // River Generation (Simple)
                if (std::abs(std::sin(x * 0.2f + y * 0.1f)) < 0.05f && grid[y][x].biome != Biome::OCEAN) {
                     grid[y][x].biome = Biome::RIVER;
                     grid[y][x].type = CellType::WATER;
                }

                // Temp
                grid[y][x].temperature = 20.0f - (noise * 5.0f); // Higher = Colder? 
                if (grid[y][x].biome == Biome::DESERT) grid[y][x].temperature = 40.0f; // Override
            }
        }
    }

    void clamp(int& x, int& y) const {
        if (x < 0) x = 0;
        if (x >= width) x = width - 1;
        if (y < 0) y = 0;
        if (y >= height) y = height - 1;
    }

    std::string getBiomeName(Biome b) const {
        switch(b) {
            case Biome::OCEAN: return "Ocean";
            case Biome::PLAINS: return "Plains";
            case Biome::FOREST: return "Forest";
            case Biome::DESERT: return "Desert";
            case Biome::SWAMP: return "Swamp";
            case Biome::MOUNTAIN: return "Mountain";
            case Biome::RIVER: return "River";
            default: return "Unknown";
        }
    }
    
    std::string getSeasonName() const {
        switch(currentSeason) {
            case SPRING: return "Spring";
            case SUMMER: return "Summer";
            case AUTUMN: return "Autumn";
            case WINTER: return "Winter";
            default: return "";
        }
    }
};

} // namespace evo
