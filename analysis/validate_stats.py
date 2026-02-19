import pandas as pd
import numpy as np
from scipy import stats
import glob
import os

def load_data():
    all_files = glob.glob("experiments/*.csv") + glob.glob("*.csv")
    
    # Sim data (Tribalism)
    sim_files = [f for f in all_files if "sim" in f or "simulation" in f]
    sim_dfs = []
    for f in sim_files:
        try:
            df = pd.read_csv(f)
            # We only care about agents who died or reached max tick (to avoid 0 age bias of new agents)
            # Actually, sim log is per-tick. We need unique agents final age.
            # Simulation log has AgentId, Age, TribeId. 
            # We need to group by AgentId and take the max Age.
            sim_dfs.append(df)
        except: pass
    
    if not sim_dfs: return None, None
    full_sim = pd.concat(sim_dfs, ignore_index=True)
    
    # Process Sim Data for Survival Analysis
    # Group by AgentId to get final state
    agents = full_sim.sort_values('Tick').groupby('AgentId').last().reset_index()
    
    # Soul data (Karma)
    soul_files = [f for f in all_files if "soul" in f]
    soul_dfs = []
    for f in soul_files:
        try: soul_dfs.append(pd.read_csv(f))
        except: pass
    full_souls = pd.concat(soul_dfs, ignore_index=True) if soul_dfs else pd.DataFrame()

    return agents, full_souls

def validate():
    agents, souls = load_data()
    results = {}
    
    print(f"Loaded {len(agents)} unique agents and {len(souls)} soul records.")

    # 1. Validate Tribal Benefit (T-Test)
    if not agents.empty and 'TribeId' in agents.columns:
        tribal = agents[agents['TribeId'] != -1]['Age']
        lone = agents[agents['TribeId'] == -1]['Age']
        
        t_stat, p_val = stats.ttest_ind(tribal, lone, equal_var=False)
        results['Tribal_Test'] = {
            'Tribal_Mean': tribal.mean(),
            'Lone_Mean': lone.mean(),
            'Difference_Pct': (tribal.mean() - lone.mean()) / lone.mean() * 100,
            'T_Statistic': t_stat,
            'P_Value': p_val
        }

    # 2. Validate Breaking Bad (Pearson Correlation)
    if not souls.empty and 'Karma' in souls.columns and 'Age' in souls.columns:
        # Filter out age 0 to avoid noise
        valid_souls = souls[souls['Age'] > 0]
        corr, p_val = stats.pearsonr(valid_souls['Age'], valid_souls['Karma'])
        results['Karma_Test'] = {
            'Correlation': corr,
            'P_Value': p_val,
            'Sample_Size': len(valid_souls)
        }

    return results

if __name__ == "__main__":
    res = validate()
    for k, v in res.items():
        print(f"\n--- {k} ---")
        for metric, val in v.items():
            print(f"{metric}: {val}")
