#pragma once
// ============================================================================
// EvoSim — Spatial Hashing (Optimization for Scale)
// Replaces O(N^2) neighbor checks with O(1) grid lookups.
// Essential for 7,000+ agents.
// ============================================================================

#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include "../agents/Agent.h"

namespace evo {

class SpatialHash {
public:
    int cellSize;
    int width;
    int height;
    
    // Grid: Cell Coordinate (y * width + x) -> List of Agent IDs
    // Or just (y, x) -> vector<Agent*>
    std::vector<std::vector<Agent*>> grid;
    int cols;
    int rows;

    SpatialHash(int w, int h, int cSize) 
        : width(w), height(h), cellSize(cSize) 
    {
        cols = (width + cellSize - 1) / cellSize; // ceil division
        rows = (height + cellSize - 1) / cellSize;
        grid.resize(cols * rows);
    }

    void clear() {
        for(auto& cell : grid) cell.clear();
    }

    void insert(Agent* agent) {
        int cx = agent->x / cellSize;
        int cy = agent->y / cellSize;
        if (cx >= 0 && cx < cols && cy >= 0 && cy < rows) {
            grid[cy * cols + cx].push_back(agent);
        }
    }

    // Get agents in 3x3 cells around (x, y)
    // Actually, for vision range 1, we just need the local cells.
    // Max vision range is typically small.
    // Returns CANDIDATES. Distance check still needed.
    void query(int x, int y, int range, std::vector<Agent*>& result) {
        int startX = (x - range) / cellSize;
        int endX = (x + range) / cellSize;
        int startY = (y - range) / cellSize;
        int endY = (y + range) / cellSize;

        for (int cy = startY; cy <= endY; cy++) {
            for (int cx = startX; cx <= endX; cx++) {
                int wrappedY = ((cy % rows) + rows) % rows;
                int wrappedX = ((cx % cols) + cols) % cols;
                const auto& cellAgents = grid[wrappedY * cols + wrappedX];
                for (auto* a : cellAgents) {
                    result.push_back(a);
                }
            }
        }
    }
};

} // namespace evo
