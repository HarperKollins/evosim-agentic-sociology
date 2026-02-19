# EvoSim — Research Guide

A guide for researchers using EvoSim to study emergent behavior in agent-based systems.

## Theoretical Framework

### Digital Gnosticism

EvoSim's design draws from **Gnostic cosmology** as a metaphor for artificial life:

| Gnostic Concept | EvoSim Implementation |
|-----------------|----------------------|
| The Demiurge (creator god) | The simulation engine / Higher Power Oracle |
| Sparks of divine light | Agent souls with karma and enlightenment |
| The material world (Kenoma) | The grid world with biomes and resources |
| Gnosis (salvific knowledge) | Enlightenment stat, Tree of Knowledge |
| Archons (rulers) | Environmental pressures, disease, predators |
| Reincarnation | Soul persistence across agent deaths |

This is not a theological claim — it's a **design pattern** that naturally produces interesting emergent behaviors by giving agents both material needs (food, health) and spiritual dimensions (karma, enlightenment).

### Emergent Sociology

The core hypothesis: **complex social structures can emerge from agents with no hardcoded social rules.**

Agents don't "know" about tribes, religion, or culture. They have:
- A neural network that maps sensors to actions
- A reward signal based on survival and stat changes
- The *option* to perform social actions (TRADE, COOPERATE, FORM_TRIBE, COMMUNICATE)

Any social behavior that emerges is **learned**, not programmed.

## Designing Experiments

### Experiment Template

```bash
# 1. Set a specific seed for reproducibility
./EvoSim.exe --ticks 500 --agents 30 --grid 30 --seed 42 --silent

# 2. Run multiple trials
for i in {1..10}; do
    ./EvoSim.exe --ticks 500 --agents 30 --grid 30 --seed $i --silent
    mv simulation_log.csv "data/trial_${i}_sim.csv"
    mv world_log.csv "data/trial_${i}_world.csv"
    mv soul_log.csv "data/trial_${i}_soul.csv"
done
```

### Variables You Can Control

| Variable | How to Modify |
|----------|---------------|
| Population size | `--agents N` |
| World size & density | `--grid N` (smaller = denser) |
| Simulation length | `--ticks N` |
| Resource availability | Modify `World::seasonCycle()` in `World.h` |
| Disease frequency | Modify spawn chance in `Disease.h` |
| Neural network architecture | Change layer sizes in `Agent.h` constructor |
| Reward weights | Modify `calculateReward()` in `Agent.h` |
| Oracle availability | `--llm` flag or modify knowledge file |
| Inheritance mode | `--lamarck` for Lamarckian vs. Darwinian |

### Measurable Outcomes

From `simulation_log.csv`:
- **Average lifespan** per generation
- **Tribe formation rate** (% of agents in tribes over time)
- **Meme diversity** (unique memes per tick)
- **Karma distribution** (population-level moral tendency)
- **Archetype evolution** (which archetypes dominate over time)
- **Spatial clustering** (do agents form settlements?)

From `soul_log.csv`:
- **Cause of death distribution** (starvation vs. combat vs. disease)
- **Karma at death** (do "good" agents live longer?)
- **Enlightenment at death** (does spiritual pursuit help survival?)
- **Soul age distribution** (how many reincarnation cycles?)

## Open Research Questions

### 1. Does Altruism Evolve?
**Hypothesis**: In dense environments with frequent interactions, agents should evolve higher altruism because cooperation provides survival advantages.

**Test**: Compare `--grid 20 --agents 30` (dense) vs. `--grid 50 --agents 30` (sparse) and measure average altruism over 500 ticks.

### 2. Religion as Survival Strategy
**Hypothesis**: Agents who pray and query the Oracle gain enlightenment bonuses that improve survival, creating evolutionary pressure for religious behavior.

**Test**: Compare simulations with and without Oracle access. Measure average lifespan and population stability.

### 3. Tribal vs. Individual Selection
**Hypothesis**: Tribes should outperform lone wolves in harsh environments (small grids, many predators) but not in resource-rich environments.

**Test**: Vary predator spawn rates and measure survival of tribal vs. non-tribal agents.

### 4. Cultural Meme Fitness
**Hypothesis**: Some meme categories (technique, belief, tradition) provide different survival advantages. "Technique" memes should spread faster in resource-scarce environments.

**Test**: Track meme spread rates and agent survival by meme type across different world configurations.

### 5. Lamarckian vs. Darwinian Evolution
**Hypothesis**: Lamarckian inheritance (passing learned weights to offspring) accelerates adaptation but may reduce genetic diversity.

**Test**: Compare `--lamarck` vs. no Lamarckian inheritance over 1000 ticks. Measure average fitness and population diversity.

### 6. Karma and Natural Selection
**Hypothesis**: If karma correlates with cooperative behavior, and cooperation improves group survival, then high-karma agents should accumulate over generations.

**Test**: Track karma distribution across generations. Check if mean karma increases, decreases, or stabilizes.

## Data Analysis Tips

### Python Quick Start

```python
import pandas as pd
import matplotlib.pyplot as plt

# Load simulation data
sim = pd.read_csv('simulation_log.csv')
world = pd.read_csv('world_log.csv')
souls = pd.read_csv('soul_log.csv')

# Population over time
world.plot(x='Tick', y='Pop', title='Population Over Time')
plt.show()

# Karma distribution at final tick
final = sim[sim['Tick'] == sim['Tick'].max()]
final['Karma'].hist(bins=30, title='Karma Distribution')
plt.show()

# Archetype survival
soul_stats = souls.groupby('Archetype')['Age'].mean()
soul_stats.plot(kind='bar', title='Average Lifespan by Archetype')
plt.show()

# Tribe membership over time
tribal = sim.groupby('Tick').apply(lambda x: (x['TribeId'] > 0).mean())
tribal.plot(title='% of Agents in Tribes Over Time')
plt.show()
```

## Publishing Results

If you discover interesting emergent behaviors, consider:

1. **Documenting your experiment** as a markdown file in `docs/experiments/`
2. **Sharing your configuration** (CLI flags, any code modifications)
3. **Including visualizations** (screenshots, charts from analysis)
4. **Opening a GitHub discussion** to share findings with the community

We encourage open science — share your data, methods, and conclusions.
