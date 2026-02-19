#pragma once
#include "../agents/Agent.h"
#include "../world/World.h"
#include <cmath>

namespace evo {

struct RewardContext {
    float deltaHealth = 0.0f;
    float deltaEnergy = 0.0f;
    float deltaSatiety = 0.0f;
    bool causedDeath = false;
    bool reproduced = false;
    bool isNight = false;
    bool nearPredator = false;
    float distToPredator = 9999.0f;
    // Drives Context
    float currentKinship = 0.0f;
    float currentStatus = 0.0f;
    float currentCuriosity = 0.0f;
    float currentAltruism = 0.0f;
    bool gainedKnowledge = false;
    bool ateFood = false;
};

class RLTrainer {
public:
    static float getReward(const Agent& agent, Action action, const RewardContext& ctx) {
        float reward = 0.0f;

        // ── Survival Rewards (Maslow Level 1) ───────────────────────────────
        if (!agent.isAlive()) return -10.0f; // Death penalty
        
        reward += ctx.deltaHealth * 0.1f;
        reward += ctx.deltaSatiety * 0.1f; // Hunger satisfaction
        
        // ── Action-Specific Rewards ─────────────────────────────────────────
        switch (action) {
            case Action::EAT:
                if (ctx.deltaSatiety > 0) reward += 2.0f; 
                break;
            
            case Action::DRINK:
                if (ctx.deltaSatiety > 0) reward += 2.0f;
                break;

            case Action::ATTACK: // Status vs Altruism tradeoff
                if (ctx.causedDeath) {
                    reward += 5.0f; // Dominance victory
                    if (agent.stats.status > 50.0f) reward += 2.0f; // Ambition satisfied
                } else {
                    reward -= 0.5f; // Wasted energy
                }
                if (agent.stats.altruism > 80.0f) reward -= 5.0f; // Guilt
                break;

            case Action::FLEE:
                if (ctx.nearPredator) reward += 3.0f; // Good job running
                break;

            case Action::COOPERATE: // Grooming / Healing
                reward += 3.0f; 
                if (agent.stats.kinship > 50.0f) reward += 2.0f; // Social bonding
                break;

            case Action::TRADE:
                reward += 2.0f;
                if (agent.stats.status > 50.0f) reward += 1.0f; // Wealth is status
                break;
            
            case Action::COMMUNICATE:
                reward += 1.0f;
                if (agent.stats.kinship > 50.0f) reward += 1.0f;
                break;

            case Action::REST:
                if (ctx.deltaEnergy > 0) reward += 1.0f;
                if (ctx.isNight) reward += 1.0f; // Sleeping at night is smart
                break;

            case Action::HUNT:
                if (ctx.causedDeath) reward += 4.0f;
                break;

            case Action::BUILD_SHELTER:
                reward += 2.5f;
                if (agent.stats.status > 60.0f) reward += 2.0f; // "Look at my house"
                break;

            // ── Higher Needs (Maslow Level 5) ───────────────────────────────
            case Action::QUERY_ORACLE:
                if (ctx.gainedKnowledge) {
                    reward += 5.0f; 
                    if (agent.stats.curiosity > 70.0f) reward += 5.0f; // Intellectual bliss
                } else {
                    reward -= 1.0f; // Frustration
                }
                break;

            case Action::PRAY:
                // Renamed to Reflect/Meditate?
                if (agent.stats.consciousness > 20.0f) reward += 2.0f; // Inner peace
                if (ctx.nearPredator) reward -= 5.0f; // Darwin award
                break;

            case Action::MATE:
                if (ctx.reproduced) reward += 10.0f; // Biological imperative
                else reward -= 0.5f; // Wasted effort
                if (agent.stats.kinship > 50.0f) reward += 2.0f;
                break;

            case Action::INVENT:
                if (agent.stats.curiosity > 60.0f) reward += 3.0f; // Curiosity satisfied
                // If invention event triggered (needs context flag?), bonus
                break;

            case Action::FORM_TRIBE:
                reward += 2.0f;
                if (agent.stats.altruism > 50.0f && agent.stats.kinship > 50.0f) reward += 3.0f;
                break;

            default: break;
        }

        // ── Context Penalties/Bonuses ───────────────────────────────────────
        if (ctx.nearPredator && action != Action::FLEE && action != Action::ATTACK && action != Action::HUNT) {
             reward -= 2.0f; // Danger!
        }

        return reward;
    }
};

} // namespace evo
