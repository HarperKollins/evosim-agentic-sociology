#pragma once
#include <string>
#include "../utils/Random.h"

namespace evo {

struct Disease {
    std::string name;
    float lethality;    // 0.0 - 1.0 (Chance to reduce health massively)
    float infectivity;  // 0.0 - 1.0 (Chance to spread on contact)
    int duration;       // Ticks it lasts
    
    // Static factory for randomized diseases
    static Disease createRandom() {
        auto& rng = Random::instance();
        Disease d;
        d.name = "Plague-" + std::to_string(rng.uniformInt(0, 999));
        d.lethality = rng.uniformFloat(0.05f, 0.15f);    // 5-15% damage per tick logic
        d.infectivity = rng.uniformFloat(0.2f, 0.7f);
        d.duration = rng.uniformInt(10, 30);
        return d;
    }
};

}
