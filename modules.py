import configparser
import subprocess
import os

cfg = configparser.ConfigParser()
cfg.read(".gitmodules")

for section in cfg.sections():
    if not section.startswith("submodule"):
        continue

    path = cfg[section].get("path")
    url  = cfg[section].get("url")

    if not path or not url:
        continue

    if not os.path.exists(path):
        print(f"Cloning {url} → {path}")
        os.makedirs(os.path.dirname(path), exist_ok=True)
        subprocess.run(["git", "clone", url, path])
    else:
        print(f"Skipping existing {path}")

print("Done.")
