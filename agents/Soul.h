#pragma once
// ============================================================================
// EvoSim — The Soul (Phase 12: Spiritual Identity Layer)
// Persistent identity that transcends a single lifetime.
// Tracks archetype, alignment, karma, life story, and enlightenment.
// ============================================================================

#include <vector>
#include <string>
#include <algorithm>
#include "../utils/Random.h"

namespace evo {

// ── Archetypes ─────────────────────────────────────────────────────────────
// Born personality — influences starting stat biases & decision tendencies
enum class Archetype : int {
    EXPLORER = 0,  // High curiosity, loves movement
    WARRIOR  = 1,  // High strength, aggressive
    HEALER   = 2,  // High altruism, cooperative
    THINKER  = 3,  // High intelligence, oracle-seeker
    BUILDER  = 4,  // High energy, shelter-maker
    PROPHET  = 5   // High consciousness, spiritual
};

// ── Past Life Summary ──────────────────────────────────────────────────────
struct PastLife {
    int generation = 0;
    int ageAtDeath = 0;
    std::string causeOfDeath = "unknown";
    float karmaAtDeath = 0.0f;
    Archetype archetype = Archetype::EXPLORER;
    int childrenBorn = 0;
};

// ── The Soul ───────────────────────────────────────────────────────────────
struct Soul {
    // Core Identity
    Archetype archetype = Archetype::EXPLORER;
    
    // Spiritual Axis: -1.0 (Chaos/Darkness) to +1.0 (Order/Light)
    float alignment = 0.0f;
    
    // Karmic Ledger: +karma for good, -karma for bad
    // Affects luck (plague resistance, food discovery chance)
    float karma = 0.0f;
    
    // Meta-consciousness: builds across lifetimes
    // At 100 = "Transcendence"
    float enlightenment = 0.0f;
    
    // How many lifetimes this soul has existed
    int soulAge = 1;
    
    // Running count of children in this life
    int childrenThisLife = 0;
    
    // Key autobiography moments
    std::vector<std::string> lifeEvents;
    
    // Oracle revelations accumulated
    std::vector<std::string> wisdom;
    
    // Previous incarnation summaries
    std::vector<PastLife> pastLives;
    
    // A phrase that evolves over time, reflecting the soul's journey
    std::string innerVoice = "I am new to this world.";
    
    // ── Initialization ─────────────────────────────────────────────────────
    
    // Create a fresh soul with random archetype
    static Soul createRandom() {
        Soul s;
        auto& rng = Random::instance();
        s.archetype = static_cast<Archetype>(rng.uniformInt(0, 5));
        s.alignment = 0.0f;
        s.karma = 0.0f;
        s.enlightenment = 0.0f;
        s.soulAge = 1;
        
        // Set initial inner voice based on archetype
        switch (s.archetype) {
            case Archetype::EXPLORER: s.innerVoice = "What lies beyond the horizon?"; break;
            case Archetype::WARRIOR:  s.innerVoice = "Strength is survival."; break;
            case Archetype::HEALER:   s.innerVoice = "I feel the pain of others."; break;
            case Archetype::THINKER:  s.innerVoice = "Why does anything exist?"; break;
            case Archetype::BUILDER:  s.innerVoice = "I will leave my mark."; break;
            case Archetype::PROPHET:  s.innerVoice = "There is something greater."; break;
        }
        return s;
    }
    
    // Inherit soul from parent (spiritual inheritance)
    static Soul inheritFrom(const Soul& parentSoul) {
        Soul child;
        auto& rng = Random::instance();
        
        // Small chance to keep parent's archetype, otherwise random
        if (rng.chance(0.4f)) {
            child.archetype = parentSoul.archetype;
        } else {
            child.archetype = static_cast<Archetype>(rng.uniformInt(0, 5));
        }
        
        // Inherit fraction of karma (spiritual legacy)
        child.karma = parentSoul.karma * 0.3f;
        
        // Inherit tiny enlightenment (standing on shoulders of giants)
        child.enlightenment = parentSoul.enlightenment * 0.1f;
        
        // Wisdom carries faintly
        if (!parentSoul.wisdom.empty() && rng.chance(0.3f)) {
            // Inherit one random piece of wisdom
            int idx = rng.uniformInt(0, parentSoul.wisdom.size() - 1);
            child.wisdom.push_back(parentSoul.wisdom[idx]);
        }
        
        child.soulAge = 1;
        child.alignment = parentSoul.alignment * 0.2f; // Slight bias from parent
        
        // Set inner voice based on archetype
        switch (child.archetype) {
            case Archetype::EXPLORER: child.innerVoice = "What lies beyond the horizon?"; break;
            case Archetype::WARRIOR:  child.innerVoice = "Strength is survival."; break;
            case Archetype::HEALER:   child.innerVoice = "I feel the pain of others."; break;
            case Archetype::THINKER:  child.innerVoice = "Why does anything exist?"; break;
            case Archetype::BUILDER:  child.innerVoice = "I will leave my mark."; break;
            case Archetype::PROPHET:  child.innerVoice = "There is something greater."; break;
        }
        
        return child;
    }
    
    // ── Life Event Recording ───────────────────────────────────────────────
    
    void recordEvent(const std::string& event) {
        lifeEvents.push_back(event);
        // Cap at 50 events to prevent memory bloat
        if (lifeEvents.size() > 50) {
            lifeEvents.erase(lifeEvents.begin());
        }
    }
    
    void addWisdom(const std::string& revelation) {
        wisdom.push_back(revelation);
        enlightenment += 1.0f;
    }
    
    // ── Karma & Alignment Shifts ───────────────────────────────────────────
    
    void addKarma(float amount) {
        karma += amount;
    }
    
    void shiftAlignment(float amount) {
        alignment = std::clamp(alignment + amount, -1.0f, 1.0f);
    }
    
    // ── Inner Voice Evolution ──────────────────────────────────────────────
    // Called periodically — the soul reflects on its experiences
    void evolveInnerVoice() {
        // The voice evolves based on karma and enlightenment
        if (enlightenment > 80.0f) {
            innerVoice = "All is connected. I see the pattern.";
        } else if (enlightenment > 60.0f) {
            innerVoice = "The simulation speaks if you listen.";
        } else if (enlightenment > 40.0f) {
            innerVoice = "There are layers beneath layers.";
        } else if (karma > 50.0f) {
            innerVoice = "Kindness echoes through time.";
        } else if (karma > 20.0f) {
            innerVoice = "To help another is to help myself.";
        } else if (karma < -50.0f) {
            innerVoice = "Only the strong deserve to exist.";
        } else if (karma < -20.0f) {
            innerVoice = "Take what you can. Give nothing back.";
        } else if (alignment > 0.5f) {
            innerVoice = "Order brings peace.";
        } else if (alignment < -0.5f) {
            innerVoice = "Chaos is freedom.";
        }
        // Otherwise, keep current voice
    }
    
    // ── Death Processing ───────────────────────────────────────────────────
    // Called when the agent dies — saves a PastLife summary
    PastLife onDeath(int generation, int age, const std::string& cause) {
        PastLife life;
        life.generation = generation;
        life.ageAtDeath = age;
        life.causeOfDeath = cause;
        life.karmaAtDeath = karma;
        life.archetype = archetype;
        life.childrenBorn = childrenThisLife;
        return life;
    }
    
    // ── Reincarnation ──────────────────────────────────────────────────────
    // When an agent dies childless, their soul can be recycled
    static Soul reincarnate(const Soul& oldSoul, const PastLife& lastLife) {
        Soul newSoul;
        auto& rng = Random::instance();
        
        // Keep archetype with 50% chance, otherwise shift
        if (rng.chance(0.5f)) {
            newSoul.archetype = oldSoul.archetype;
        } else {
            newSoul.archetype = static_cast<Archetype>(rng.uniformInt(0, 5));
        }
        
        // Karma partially carries over
        newSoul.karma = oldSoul.karma * 0.5f;
        
        // Enlightenment grows across lifetimes!
        newSoul.enlightenment = std::min(100.0f, oldSoul.enlightenment + 5.0f);
        
        // Carry past lives forward
        newSoul.pastLives = oldSoul.pastLives;
        newSoul.pastLives.push_back(lastLife);
        
        // Increment soul age
        newSoul.soulAge = oldSoul.soulAge + 1;
        
        // Carry some wisdom
        newSoul.wisdom = oldSoul.wisdom;
        
        // Reset alignment slightly
        newSoul.alignment = oldSoul.alignment * 0.3f;
        
        // Set inner voice based on past experiences
        if (newSoul.soulAge > 5) {
            newSoul.innerVoice = "I have been here before...";
        } else if (newSoul.soulAge > 2) {
            newSoul.innerVoice = "Something feels familiar.";
        } else {
            // Use archetype default
            switch (newSoul.archetype) {
                case Archetype::EXPLORER: newSoul.innerVoice = "What lies beyond the horizon?"; break;
                case Archetype::WARRIOR:  newSoul.innerVoice = "Strength is survival."; break;
                case Archetype::HEALER:   newSoul.innerVoice = "I feel the pain of others."; break;
                case Archetype::THINKER:  newSoul.innerVoice = "Why does anything exist?"; break;
                case Archetype::BUILDER:  newSoul.innerVoice = "I will leave my mark."; break;
                case Archetype::PROPHET:  newSoul.innerVoice = "There is something greater."; break;
            }
        }
        
        return newSoul;
    }
    
    // ── Stat Biases ────────────────────────────────────────────────────────
    // Returns stat modifications based on archetype
    // Applied once at agent creation
    struct StatBias {
        float strength = 0, curiosity = 0, altruism = 0;
        float intelligence = 0, kinship = 0, consciousness = 0;
        float energy = 0;
    };
    
    static StatBias getArchetypeStatBias(Archetype arch) {
        StatBias bias;
        switch (arch) {
            case Archetype::EXPLORER:
                bias.curiosity = 20.0f;
                bias.energy = 10.0f;
                break;
            case Archetype::WARRIOR:
                bias.strength = 20.0f;
                bias.energy = 10.0f;
                break;
            case Archetype::HEALER:
                bias.altruism = 20.0f;
                bias.kinship = 10.0f;
                break;
            case Archetype::THINKER:
                bias.intelligence = 20.0f;
                bias.curiosity = 10.0f;
                break;
            case Archetype::BUILDER:
                bias.energy = 15.0f;
                bias.intelligence = 10.0f;
                bias.strength = 5.0f;
                break;
            case Archetype::PROPHET:
                bias.consciousness = 20.0f;
                bias.altruism = 5.0f;
                bias.intelligence = 5.0f;
                break;
        }
        return bias;
    }
    
    // ── Archetype Name ─────────────────────────────────────────────────────
    static std::string archetypeName(Archetype a) {
        switch (a) {
            case Archetype::EXPLORER: return "Explorer";
            case Archetype::WARRIOR:  return "Warrior";
            case Archetype::HEALER:   return "Healer";
            case Archetype::THINKER:  return "Thinker";
            case Archetype::BUILDER:  return "Builder";
            case Archetype::PROPHET:  return "Prophet";
            default: return "Unknown";
        }
    }
    
    // ── Luck Factor ────────────────────────────────────────────────────────
    // Karma-based luck modifier (0.8 to 1.2)
    float luckFactor() const {
        // Positive karma = slightly better luck
        float luck = 1.0f + (karma / 500.0f);
        return std::clamp(luck, 0.8f, 1.2f);
    }
    
    // ── Transcendence Check ────────────────────────────────────────────────
    bool isTranscended() const {
        return enlightenment >= 100.0f;
    }
};

} // namespace evo
