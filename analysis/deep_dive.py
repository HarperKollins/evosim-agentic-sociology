import pandas as pd
import numpy as np
import glob
import os

def load_all_data():
    all_files = glob.glob("data/demo/*.csv") + glob.glob("experiments/*.csv") + glob.glob("*.csv")
    sim_files = [f for f in all_files if "sim" in f or "simulation" in f]
    world_files = [f for f in all_files if "world" in f]
    soul_files = [f for f in all_files if "soul" in f]
    
    print(f"Found {len(sim_files)} simulation logs, {len(world_files)} world logs, {len(soul_files)} soul logs")
    
    # Load and concatenate
    sim_dfs = []
    for f in sim_files:
        try:
            df = pd.read_csv(f)
            df['Source'] = os.path.basename(f)
            sim_dfs.append(df)
        except Exception as e:
            print(f"Skipping {f}: {e}")
            
    if not sim_dfs:
        print("No simulation data found!")
        return None, None, None
        
    full_sim = pd.concat(sim_dfs, ignore_index=True)
    
    # Load souls
    soul_dfs = []
    for f in soul_files:
        try:
            df = pd.read_csv(f)
            df['Source'] = os.path.basename(f)
            soul_dfs.append(df)
        except:
            pass
    full_souls = pd.concat(soul_dfs, ignore_index=True) if soul_dfs else pd.DataFrame()

    return full_sim, full_souls

def analyze_tribal_cohesion(df):
    # Does being in a tribe increase survival?
    # We approximate survival by "Max Age" per agent
    # Note: "Age" column exists
    
    if 'TribeId' not in df.columns: return "No Tribe Data"
    
    tribal = df[df['TribeId'] != -1]
    lone = df[df['TribeId'] == -1]
    
    avg_age_tribe = tribal['Age'].mean()
    avg_age_lone = lone['Age'].mean()
    
    return {
        "Tribal_Avg_Age": avg_age_tribe,
        "Lone_Avg_Age": avg_age_lone,
        "Benefit": (avg_age_tribe - avg_age_lone) / avg_age_lone * 100 if avg_age_lone > 0 else 0
    }

def analyze_karma_paradox(souls):
    if souls.empty or 'Karma' not in souls.columns: return "No Soul Data"
    
    # Correlation between Karma and Age at death
    corr = souls['Karma'].corr(souls['Age'])
    
    # Outlier detection: High Karma (>50) survival rate vs Low Karma (<-50)
    saints = souls[souls['Karma'] > 20]
    sinners = souls[souls['Karma'] < -20]
    
    return {
        "Karma_Life_Correlation": corr,
        "Saint_Avg_Life": saints['Age'].mean() if not saints.empty else 0,
        "Sinner_Avg_Life": sinners['Age'].mean() if not sinners.empty else 0
    }

def analyze_meme_toxicity(df):
    if 'NumMemes' not in df.columns: return "No Meme Data"
    
    # Does having more memes killing you?
    corr = df['NumMemes'].corr(df['Age'])
    return {"Meme_Count_Life_Correlation": corr}

def main():
    print("Loading massive dataset...")
    sim, souls = load_all_data()
    
    if sim is not None:
        print("\n--- Tribal Cohesion Analysis ---")
        print(analyze_tribal_cohesion(sim))
        
        print("\n--- Meme Toxicity Scan ---")
        print(analyze_meme_toxicity(sim))
        
    if not souls.empty:
        print("\n--- Karma Paradox Scan ---")
        print(analyze_karma_paradox(souls))

    print("\nDeep analysis complete.")

if __name__ == "__main__":
    main()
