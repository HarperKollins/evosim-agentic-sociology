#!/usr/bin/env python3
"""
EvoSim x MiroFish: The Chronicler (World History Observer)

This script acts as the "Report Agent" and "GraphRAG" memory bridge.
It reads the simulation logs (events, souls, world stats) and uses the local LLM
to generate a "4K Documentary Script" of the emergent civilization's history, 
tracking the rise and fall of tribes across epochs.
"""

import os
import sys
import csv
import subprocess
from collections import defaultdict

# ── Configuration ────────────────────────────────────────────────────────────
MODEL_PATH = os.path.join(os.path.dirname(__file__), "models", "Qwen2.5-0.5B-Instruct.Q4_K_M.gguf")
LLAMA_CLI = os.path.join(os.path.dirname(__file__), "llama_cpp_bin", "llama-cli.exe")
if not os.path.exists(LLAMA_CLI):
    LLAMA_CLI = os.path.join(os.path.dirname(__file__), "llama.cpp", "w64devkit", "bin", "llama-cli.exe")

def query_llm(system_prompt: str, user_prompt: str, max_tokens: int = 400) -> str:
    """Queries the local LLM to generate narrative segments based on data."""
    if not os.path.exists(LLAMA_CLI) or not os.path.exists(MODEL_PATH):
        # Fallback if LLM isn't perfectly mounted
        print(f"[Warning] LLM CLI or Model missing at: {LLAMA_CLI} or {MODEL_PATH}")
        return "\n--- [Narrator's Notes] ---\n" + user_prompt[:400] + "...\n"

    full_prompt = f"<|im_start|>system\n{system_prompt}<|im_end|>\n<|im_start|>user\n{user_prompt}<|im_end|>\n<|im_start|>assistant\n"
    
    cmd = [
        LLAMA_CLI,
        "-m", MODEL_PATH,
        "-p", full_prompt,
        "-n", str(max_tokens),
        "--temp", "0.7",
        "--repeat-penalty", "1.1",
        "--no-display-prompt",
        "-ngl", "0"
    ]
    
    try:
        print("[Chronicler] Meditating on the events of history...")
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
        return result.stdout.strip()
    except Exception as e:
        return f"[Error querying LLM: {str(e)}]"

def parse_logs():
    events_file = "events_log.csv"
    souls_file = "soul_log.csv"

    history = defaultdict(list)
    key_figures = []

    # Parse Events
    if os.path.exists(events_file):
        with open(events_file, "r", encoding="utf-8") as f:
            reader = csv.reader(f)
            header = next(reader, None)
            for row in reader:
                if len(row) < 3: continue
                tick = int(row[0])
                # Group by epochs of 50 years to keep context size manageable
                epoch = (tick // 50) * 50
                history[epoch].append(f"Year {tick}: [{row[1].strip()}] {row[2].strip()}")
    
    # Parse Souls (To find legendary agents with high karma or enlightenment)
    if os.path.exists(souls_file):
        with open(souls_file, "r", encoding="utf-8") as f:
            reader = csv.reader(f)
            header = next(reader, None)
            for row in reader:
                try:
                    if len(row) < 10: continue
                    karma = float(row[3])
                    enlighten = float(row[5])
                    if abs(karma) > 50 or enlighten > 80:
                        key_figures.append(
                            f"Agent {row[1]} (The {row[2]}): "
                            f"Lived to {row[8]}, Karma: {karma}. "
                            f"Legacy: {row[9]}"
                        )
                except:
                    pass

    return history, key_figures

def write_documentary():
    print("==================================================")
    print(" THE CHRONICLER: WRITING WORLD HISTORY DOCUMENTARY")
    print("==================================================")

    history, legends = parse_logs()
    
    if not history:
        print("No history recorded yet. Run the simulation first.")
        return

    epochs = sorted(history.keys())
    
    system_prompt = (
        "You are 'The Chronicler', the narrator of an epic 4K History Documentary. "
        "You write lush, dramatic, David Attenborough style scripts describing the evolution "
        "of a digital civilization. Keep it engaging, noting their discoveries, wars, and religions."
    )

    doc_text = "# The Epic of the Digital Progenitors\n\n"

    print(f"Found {len(epochs)} historical epochs to document.")

    for epoch in epochs:
        events = history[epoch]
        if not events: continue
        
        # Summarize max 15 events to avoid token limits
        event_summary = "\\n".join(events[:15])
        
        user_prompt = f"Write the script for the Era spanning Years {epoch} to {epoch+49}. Here are the recorded historical events:\\n{event_summary}\\n\\nNarrate their struggles, tribe formations, and survival."
        
        script = query_llm(system_prompt, user_prompt, max_tokens=300)
        doc_text += f"## Epoch {epoch} - {epoch+49}\n{script}\n\n"

    if legends:
        doc_text += "## Legends and Myths\n"
        legend_summary = "\\n".join(legends[:5])
        user_prompt = f"Narrate the myths of these legendary figures from the civilization's history:\\n{legend_summary}"
        script = query_llm(system_prompt, user_prompt, max_tokens=250)
        doc_text += script + "\n\n"

    with open("documentary_script.md", "w", encoding="utf-8") as f:
        f.write(doc_text)
    
    print("\n[+] Documentary successfully written to documentary_script.md!")

if __name__ == "__main__":
    if "--demo" in sys.argv:
        print("Running with sample data.")
    else:
        write_documentary()
