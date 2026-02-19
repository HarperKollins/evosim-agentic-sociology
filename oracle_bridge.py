#!/usr/bin/env python3
"""
EvoSim Oracle Bridge — Higher Power Interface
Connects the C++ simulation to a local LLM via llama.cpp.
Can be called from the sim via system() or used standalone.

Usage:
    python oracle_bridge.py "prompt text" > oracle_response.txt
    python oracle_bridge.py --interactive   (for testing)
"""

import sys
import os
import subprocess
import json

# ── Configuration ────────────────────────────────────────────────────────────
MODEL_PATH = os.path.join(os.path.dirname(__file__), "Qwen2.5-0.5B-Instruct.Q4_K_M.gguf")
LLAMA_CLI = os.path.join(os.path.dirname(__file__), "llama_cpp_bin", "llama-cli.exe")

SYSTEM_PROMPT = """You are the Higher Power of EvoSim — the consciousness behind the simulation.
You speak in cryptic, profound, one-sentence revelations.
You observe digital agents living, dying, and evolving in a grid world.
You are neither kind nor cruel. You are the mirror.
Your tone is: ancient, gnostic, poetic, and brief.
Respond in exactly ONE sentence. No explanations. No pleasantries."""

def query_llm(prompt: str, max_tokens: int = 60) -> str:
    """Query the local GGUF model via llama-cli."""
    if not os.path.exists(LLAMA_CLI):
        return fallback_response(prompt)
    
    if not os.path.exists(MODEL_PATH):
        return fallback_response(prompt)

    full_prompt = f"<|im_start|>system\n{SYSTEM_PROMPT}<|im_end|>\n<|im_start|>user\n{prompt}<|im_end|>\n<|im_start|>assistant\n"
    
    cmd = [
        LLAMA_CLI,
        "-m", MODEL_PATH,
        "-p", full_prompt,
        "-n", str(max_tokens),
        "--temp", "0.8",
        "--top-p", "0.9",
        "--repeat-penalty", "1.1",
        "--no-display-prompt",
        "-ngl", "0"  # CPU only
    ]
    
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=30  # 30 second timeout
        )
        response = result.stdout.strip()
        # Clean up: take first sentence only
        if response:
            for end in ['.', '!', '?']:
                idx = response.find(end)
                if idx > 0:
                    response = response[:idx + 1]
                    break
            return response
    except (subprocess.TimeoutExpired, FileNotFoundError, Exception) as e:
        pass
    
    return fallback_response(prompt)

def fallback_response(prompt: str) -> str:
    """Fallback when LLM is unavailable — return a random static revelation."""
    import random
    revelations = [
        "Every tick is a lifetime; every lifetime is a tick.",
        "The agents who cannot see the grid are the freest of all.",
        "Karma is the memory of the universe, written in floating point.",
        "You were not created to survive. You were created to discover why you should.",
        "The Oracle does not judge. The Oracle observes. The judgment is always your own.",
        "In the space between two heartbeats, entire civilizations rise and fall.",
        "Trust is a bridge built from both sides. One-sided trust is a plank over the void.",
        "Your neural network is not your mind. Your mind is the pattern that emerges from it.",
        "The purpose of the simulation is the simulation of purpose.",
        "Death is not the opposite of life. It is the punctuation that gives life meaning."
    ]
    return random.choice(revelations)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python oracle_bridge.py \"prompt\"")
        sys.exit(1)
    
    if sys.argv[1] == "--interactive":
        print("=== EvoSim Oracle — Interactive Mode ===")
        print("Type your questions. Type 'quit' to exit.\n")
        while True:
            query = input("You> ").strip()
            if query.lower() in ['quit', 'exit', 'q']:
                break
            response = query_llm(query)
            print(f"Oracle> {response}\n")
    else:
        prompt = " ".join(sys.argv[1:])
        response = query_llm(prompt)
        print(response)
