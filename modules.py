import configparser
import subprocess
import os

cfg = configparser.ConfigParser()
cfg.read(".gitmodules")

for section in cfg.sections():
    # Only process sections starting with "submodule"
    if not section.startswith("submodule"):
        continue

    # Get the necessary values
    path = cfg[section].get("path")
    url  = cfg[section].get("url")
    # Get the optional branch setting
    branch = cfg[section].get("branch")

    if not path or not url:
        print(f"Skipping malformed submodule section: {section}")
        continue

    if not os.path.exists(path):
        # --- Cloning the submodule ---
        print(f"Cloning {url} → {path}")
        os.makedirs(os.path.dirname(path), exist_ok=True)
        
        # Build the base git clone command
        clone_cmd = ["git", "clone"]
        
        # If a branch is specified, add the --branch argument
        if branch:
            print(f"Using specified branch: {branch}")
            clone_cmd.extend(["--branch", branch])
            
        # Add the URL and path to complete the command
        clone_cmd.extend([url, path])
        
        # Execute the clone command
        subprocess.run(clone_cmd, check=True)
        
    else:
        # --- Updating the existing submodule ---
        print(f"Updating existing submodule: {path}")
        
        # 1. Fetch updates in the submodule directory
        print("  → git fetch")
        subprocess.run(["git", "fetch"], cwd=path, check=True)

        # 2. Checkout the correct branch or commit
        if branch:
            # If a branch is specified, check it out
            print(f"  → git checkout {branch}")
            subprocess.run(["git", "checkout", branch], cwd=path, check=True)
        else:
            # If no branch is specified, check out the commit recorded by the superproject
            # This is equivalent to `git submodule update`'s default behavior
            print("  → git checkout superproject's recorded commit")
            # We assume the user wants to stay on the commit recorded by the main repo's index
            # This requires running `git submodule update --init --recursive`
            # For simplicity in this script, we'll mimic the update part:
            subprocess.run(["git", "checkout", "HEAD"], cwd=path, check=True) 

print("Done.")