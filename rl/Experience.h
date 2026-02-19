#pragma once
// ============================================================================
// EvoSim — Experience Tuple
// State-Action-Reward-NextState for reinforcement learning
// ============================================================================

#include "../nn/Matrix.h"

namespace evo {

struct Experience {
    Vec state;       // s  — sensor input at time t
    int action;      // a  — action taken
    float reward;    // r  — reward received
    Vec nextState;   // s' — sensor input at time t+1
    bool done;       // terminal state? (agent died)

    Experience() : action(0), reward(0.0f), done(false) {}

    Experience(const Vec& s, int a, float r, const Vec& ns, bool d)
        : state(s), action(a), reward(r), nextState(ns), done(d) {}
};

} // namespace evo
