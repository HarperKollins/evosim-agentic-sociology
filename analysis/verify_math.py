import pandas as pd
import numpy as np
import glob
import os

def calculate_stats():
    # Debug prints
    print(f"Propagating search in {os.getcwd()}")
    files = glob.glob('experiments/*.csv')
    print(f"Found {len(files)} files in experiments/: {files}")
    
    sim_dfs = []
    for f in files:
        if 'sim' in f or 'simulation' in f: # Filter only simulation logs
            try: 
                df = pd.read_csv(f)
                df['Source_File'] = os.path.basename(f)
                sim_dfs.append(df)
                print(f"Loaded {len(df)} rows from {f}")
            except Exception as e:
                print(f"Error loading {f}: {e}")
    
    if not sim_dfs:
        print("No simulation data loaded.")
        return

    full_sim = pd.concat(sim_dfs, ignore_index=True)
    id_col = 'ID' if 'ID' in full_sim.columns else 'AgentId'
    print(f"Using ID column: {id_col}")
    
    # ... rest of logic
    # Group by (Source_File, ID) to get unique agents' final state
    agents = full_sim.sort_values('Tick').groupby(['Source_File', id_col]).last().reset_index()
    
    print(f"Total Unique Agents Analyzed: {len(agents)}")
    
    if 'TribeId' in agents.columns:
        tribal_ages = agents[agents['TribeId'] != -1]['Age'].values
        lone_ages = agents[agents['TribeId'] == -1]['Age'].values
        
        n1 = len(tribal_ages)
        n2 = len(lone_ages)
        
        if n1 > 1 and n2 > 1:
            m1 = np.mean(tribal_ages)
            m2 = np.mean(lone_ages)
            v1 = np.var(tribal_ages, ddof=1)
            v2 = np.var(lone_ages, ddof=1)
            
            se = np.sqrt(v1/n1 + v2/n2)
            t_stat = (m1 - m2) / se
            
            print(f"--- TRIBAL STATS ---")
            print(f"N_Tribal: {n1}, N_Lone: {n2}")
            print(f"Mean_Tribal: {m1:.2f}, Mean_Lone: {m2:.2f}")
            print(f"Difference: +{((m1-m2)/m2)*100:.1f}%")
            print(f"T-Statistic: {t_stat:.4f}")
            print(f"Is Significant (p<0.001)? {abs(t_stat) > 3.291}")

    if 'Karma' in agents.columns:
        valid = agents[agents['Age'] > 5]
        x = valid['Age'].values
        y = valid['Karma'].values
        n = len(x)
        
        if n > 2:
            r_num = np.sum((x - np.mean(x)) * (y - np.mean(y)))
            r_den = np.sqrt(np.sum((x - np.mean(x))**2) * np.sum((y - np.mean(y))**2))
            r = r_num / r_den if r_den != 0 else 0
            t_corr = r * np.sqrt(n-2) / np.sqrt(1 - r**2) if abs(r) < 1 else 0
            
            print(f"\n--- KARMA STATS ---")
            print(f"N: {n}")
            print(f"Correlation (r): {r:.4f}")
            print(f"T-Statistic (for r): {t_corr:.4f}")
            print(f"Is Significant (p<0.001)? {abs(t_corr) > 3.291}")

if __name__ == "__main__":
    calculate_stats()
