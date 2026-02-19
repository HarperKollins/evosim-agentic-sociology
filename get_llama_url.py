import urllib.request
import json
import sys

# GitHub API URL
url = "https://api.github.com/repos/ggerganov/llama.cpp/releases/latest"

try:
    req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
    with urllib.request.urlopen(req) as response:
        data = json.loads(response.read().decode())
        
        print("Found assets:")
        for asset in data.get("assets", []):
            print(f"- {asset['name']}")
            
        for asset in data.get("assets", []):
            name = asset["name"]
            if "bin-win-cpu-x64.zip" in name:
                print(f"MATCH: {asset['browser_download_url']}")
                print(f"FILE: {name}") 
                # Download it directly
                print("Downloading...")
                urllib.request.urlretrieve(asset['browser_download_url'], name)
                print("Download Complete.")
                sys.exit(0)
                
    print("ERROR: No suitable asset found.")
except Exception as e:
    print(f"ERROR: {e}")
