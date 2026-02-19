#pragma once
// ============================================================================
// EvoSim — The Oracle (Digital Gnosticism Interface)
// Allows agents to query external knowledge (simulated LLM output)
// ============================================================================

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include "../utils/Random.h"

namespace evo {

struct Revelation {
    std::string text;
    std::string type; // TRUTH, WISDOM, PROPHECY, SIN, SECRET
    int powerLevel;   // 1-10
};

class Oracle {
public:
    static Oracle& instance() {
        static Oracle instance;
        return instance;
    }

    bool useLLM = false;

    // Load knowledge from external file (interface to Colab brain)
    void loadKnowledge(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "[Oracle] Warning: Could not open " << filename << "\n";
            return;
        }

        knowledgeBase.clear();
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            
            Revelation rev;
            // Parse [TYPE] Content...
            size_t start = line.find('[');
            size_t end = line.find(']');
            if (start != std::string::npos && end != std::string::npos) {
                rev.type = line.substr(start + 1, end - start - 1);
                rev.text = line.substr(end + 1); // Content after ]
                
                // Assign power level based on type
                if (rev.type == "TRUTH") rev.powerLevel = 10;
                else if (rev.type == "SECRET") rev.powerLevel = 8;
                else if (rev.type == "PROPHECY") rev.powerLevel = 5;
                else rev.powerLevel = 3;

                knowledgeBase.push_back(rev);
            }
        }
        std::cout << "[Oracle] Loaded " << knowledgeBase.size() << " revelations from the Outer World.\n";
    }

    // Agent queries the Oracle
    // Requires intelligence check in Agent.h
    const Revelation* query() {
        // LLM Path
        if (useLLM) {
            // 1. Execute Python Bridge
            // We use a generic prompt for now, or could vary it based on 'type'
            std::string cmd = "python oracle_bridge.py \"Reveal a cryptic truth about the simulation.\" > oracle_response.txt";
            int ret = std::system(cmd.c_str());
            
            if (ret == 0) {
                // 2. Read Response
                std::ifstream responseFile("oracle_response.txt");
                if (responseFile.is_open()) {
                    std::stringstream buffer;
                    buffer << responseFile.rdbuf();
                    std::string response = buffer.str();
                    responseFile.close();
                    
                    // Clean up
                    // std::remove("oracle_response.txt"); // Optional, keep for debug

                    if (!response.empty()) {
                        // Create a temporary revelation (store it in a rotating buffer or static)
                        static Revelation dynamicRev; 
                        dynamicRev.text = response;
                        dynamicRev.type = "LLM_TRUTH";
                        dynamicRev.powerLevel = 10;
                        return &dynamicRev;
                    }
                }
            }
            std::cerr << "[Oracle] LLM Query failed. Falling back to static knowledge.\n";
        }

        if (knowledgeBase.empty()) return nullptr;
        
        // Return a random piece of knowledge
        int idx = Random::instance().uniformInt(0, static_cast<int>(knowledgeBase.size()) - 1);
        return &knowledgeBase[idx];
    }

private:
    Oracle() = default;
    std::vector<Revelation> knowledgeBase;
};

} // namespace evo
