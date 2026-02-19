#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include "../agents/Agent.h"
#include "../world/World.h"

namespace evo {

class Logger {
public:
    static Logger& instance() {
        static Logger instance;
        return instance;
    }

    void init() {
        // 1. Agent Log (Detailed per-tick stats + soul data)
        agentLog.open("simulation_log.csv");
        agentLog << "ID,Tick,X,Y,Health,Energy,Satiety,Kinship,Status,Curiosity,Altruism,Intel,Age,Gen,Biome,Archetype,Karma,Alignment,Enlightenment,SoulAge,TribeId,NumMemes\n";

        // 2. World Log (Global stats)
        worldLog.open("world_log.csv");
        worldLog << "Tick,Pop,Infected,Dead,AvgAltruism,AvgCuriosity,AvgConsciousness\n";

        // 3. Events Log (Notable events)
        eventLog.open("events_log.csv");
        eventLog << "Tick,Event,Details\n";

        // 4. Soul Deaths Log (Biography on death)
        soulLog.open("soul_log.csv");
        soulLog << "Tick,AgentID,Archetype,Karma,Alignment,Enlightenment,SoulAge,CauseOfDeath,Age,LifeEvents\n";
    }

    void logAgent(int tick, const Agent& agent, const std::string& biome) {
        if (!agentLog.is_open()) return;
        agentLog << agent.id << "," << tick << "," << agent.x << "," << agent.y << ","
                 << agent.stats.health << "," << agent.stats.energy << "," 
                 << agent.stats.satiety << "," << agent.stats.kinship << ","
                 << agent.stats.status << "," << agent.stats.curiosity << ","
                 << agent.stats.altruism << "," << agent.stats.intelligence << "," 
                 << agent.stats.age << "," << agent.stats.generation << ","
                 << biome << ","
                 << Soul::archetypeName(agent.soul.archetype) << ","
                 << agent.soul.karma << ","
                 << agent.soul.alignment << ","
                 << agent.soul.enlightenment << ","
                 << agent.soul.soulAge << ","
                 << agent.tribeId << ","
                 << agent.culturalMemes.size() << "\n";
        
        if (tick % 50 == 0) agentLog.flush(); 
    }

    void logWorld(int tick, const World& world, int pop, int infected, int dead, float avgAlt, float avgCur, float avgConsc) {
        if (!worldLog.is_open()) return;
        worldLog << tick << "," << pop << "," << infected << "," << dead << ","
                 << avgAlt << "," << avgCur << "," << avgConsc << "\n";
        worldLog.flush();
    }

    void logEvent(int tick, const std::string& eventName, const std::string& details) {
        if (!eventLog.is_open()) return;
        eventLog << tick << "," << eventName << "," << details << "\n";
        eventLog.flush();
    }

    void logSoulDeath(int tick, int agentId, const Agent& agent, const std::string& cause) {
        if (!soulLog.is_open()) return;
        // Compile life events into a pipe-separated string
        std::string events = "";
        for (size_t i = 0; i < agent.soul.lifeEvents.size(); i++) {
            if (i > 0) events += " | ";
            events += agent.soul.lifeEvents[i];
        }
        soulLog << tick << "," << agentId << ","
                << Soul::archetypeName(agent.soul.archetype) << ","
                << agent.soul.karma << ","
                << agent.soul.alignment << ","
                << agent.soul.enlightenment << ","
                << agent.soul.soulAge << ","
                << cause << ","
                << agent.stats.age << ","
                << "\"" << events << "\"\n";
        soulLog.flush();
    }

    void close() {
        if (agentLog.is_open()) agentLog.close();
        if (worldLog.is_open()) worldLog.close();
        if (eventLog.is_open()) eventLog.close();
        if (soulLog.is_open()) soulLog.close();
    }

private:
    Logger() = default;
    std::ofstream agentLog;
    std::ofstream worldLog;
    std::ofstream eventLog;
    std::ofstream soulLog;
};

} // namespace evo
