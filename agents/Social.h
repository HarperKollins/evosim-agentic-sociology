#pragma once
// ============================================================================
// EvoSim — Social Systems (Tribes, Trade, Culture)
// Emergent social structures from agent interactions
// ============================================================================

#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include "../utils/Random.h"

namespace evo {

// ── Cultural Meme ───────────────────────────────────────────────────────────
// A "meme" is a cultural idea that spreads between agents via communication.
// Memes give stat bonuses and mutate slightly when transmitted.
struct Meme {
    std::string name;
    std::string category;  // "technique", "belief", "tradition"
    float influence;       // How strongly it affects behavior (0-10)
    int originTick;        // When it was created
    int spreadCount = 0;   // How many times it's been transmitted

    // Effect on agent stats when held
    float altruismBonus = 0;
    float curiosityBonus = 0;
    float karmaBonus = 0;
};

// ── Tribe ───────────────────────────────────────────────────────────────────
struct Tribe {
    int id;
    std::string name;
    std::vector<int> memberIds;  // Agent IDs
    int foundedTick;
    float collectiveKarma = 0;
    int territory_x = 0, territory_y = 0;  // Center of territory
    
    bool hasMember(int agentId) const {
        return std::find(memberIds.begin(), memberIds.end(), agentId) != memberIds.end();
    }

    void removeMember(int agentId) {
        memberIds.erase(
            std::remove(memberIds.begin(), memberIds.end(), agentId),
            memberIds.end()
        );
    }

    int size() const { return static_cast<int>(memberIds.size()); }
};

// ── Social System ───────────────────────────────────────────────────────────
class SocialSystem {
public:
    std::vector<Tribe> tribes;
    std::vector<Meme> globalMemes;  // All memes in existence
    int nextTribeId = 1;

    static SocialSystem& instance() {
        static SocialSystem inst;
        return inst;
    }

    // ── Tribe Management ────────────────────────────────────────────────────
    int createTribe(const std::string& name, int founderX, int founderY, int tick) {
        Tribe t;
        t.id = nextTribeId++;
        t.name = name;
        t.foundedTick = tick;
        t.territory_x = founderX;
        t.territory_y = founderY;
        tribes.push_back(t);
        return t.id;
    }

    Tribe* getTribe(int tribeId) {
        for (auto& t : tribes) {
            if (t.id == tribeId) return &t;
        }
        return nullptr;
    }

    int findTribeForAgent(int agentId) {
        for (auto& t : tribes) {
            if (t.hasMember(agentId)) return t.id;
        }
        return -1;
    }

    void addToTribe(int tribeId, int agentId) {
        Tribe* t = getTribe(tribeId);
        if (t && !t->hasMember(agentId)) {
            t->memberIds.push_back(agentId);
        }
    }

    void removeFromTribe(int agentId) {
        for (auto& t : tribes) {
            t.removeMember(agentId);
        }
        // Clean up empty tribes
        tribes.erase(
            std::remove_if(tribes.begin(), tribes.end(), 
                [](const Tribe& t) { return t.memberIds.empty(); }),
            tribes.end()
        );
    }

    // ── Meme / Culture ──────────────────────────────────────────────────────
    void createMeme(const std::string& name, const std::string& category, 
                    float influence, int tick) {
        Meme m;
        m.name = name;
        m.category = category;
        m.influence = influence;
        m.originTick = tick;

        // Random bonuses based on category
        auto& rng = Random::instance();
        if (category == "technique") {
            m.curiosityBonus = rng.uniformFloat(0.5f, 2.0f);
        } else if (category == "belief") {
            m.altruismBonus = rng.uniformFloat(-1.0f, 2.0f);
            m.karmaBonus = rng.uniformFloat(0.5f, 1.5f);
        } else if (category == "tradition") {
            m.altruismBonus = rng.uniformFloat(0.5f, 1.5f);
        }

        globalMemes.push_back(m);
    }

    // Get a random meme to spread
    Meme* getRandomMeme() {
        if (globalMemes.empty()) return nullptr;
        int idx = Random::instance().uniformInt(0, static_cast<int>(globalMemes.size()) - 1);
        return &globalMemes[idx];
    }

    // ── Tribe Name Generator ────────────────────────────────────────────────
    static std::string generateTribeName() {
        static const char* prefixes[] = {
            "Sun", "Moon", "Star", "Storm", "River", "Stone", "Fire", "Shadow",
            "Iron", "Golden", "Silver", "Crystal", "Thunder", "Frost", "Blood", "Spirit"
        };
        static const char* suffixes[] = {
            "walkers", "keepers", "seekers", "builders", "hunters", "watchers",
            "born", "forged", "singers", "weavers", "callers", "blades"
        };
        auto& rng = Random::instance();
        int pi = rng.uniformInt(0, 15);
        int si = rng.uniformInt(0, 11);
        return std::string(prefixes[pi]) + suffixes[si];
    }

    // ── Meme Name Generator ─────────────────────────────────────────────────
    static std::string generateMemeName(const std::string& category) {
        auto& rng = Random::instance();
        if (category == "technique") {
            static const char* names[] = {
                "Fire Taming", "Stone Shaping", "Herb Lore", "Star Reading",
                "Water Finding", "Trap Craft", "Bone Tools", "Shelter Weaving",
                "Smoke Signals", "Root Medicine", "Mud Bricks", "Wind Sailing"
            };
            return names[rng.uniformInt(0, 11)];
        } else if (category == "belief") {
            static const char* names[] = {
                "The Great Balance", "Ancestor Spirits", "The Eternal Return",
                "Law of Giving", "The Dream World", "The Silent Observer",
                "Karma Cycle", "Soul Flame", "The Watchers Above", "Void Wisdom"
            };
            return names[rng.uniformInt(0, 9)];
        } else {
            static const char* names[] = {
                "Harvest Dance", "Moon Feast", "Coming of Age", "Elder Council",
                "Peace Circle", "Hunt Song", "Birth Blessing", "Death Vigil"
            };
            return names[rng.uniformInt(0, 7)];
        }
    }

private:
    SocialSystem() = default;
};

} // namespace evo
