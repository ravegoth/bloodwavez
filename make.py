import os
import json
import subprocess
import shutil

CONFIG_FILE = "build_config.json"
OUTPUT_EXE = "bin/bloodwavez.exe"

def ask_path(prompt):
    while True:
        path = input(prompt).strip('" ')
        if os.path.exists(path):
            return path
        print("Invalid path. Try again.")

def gpp_in_path():
    return shutil.which("g++") is not None

def load_config():
    if os.path.exists(CONFIG_FILE):
        with open(CONFIG_FILE, "r") as f:
            return json.load(f)
    return None

def save_config(config):
    with open(CONFIG_FILE, "w") as f:
        json.dump(config, f, indent=4)

def main():
    config = load_config()

    if not config or not gpp_in_path():
        print("g++ is NOT in PATH." if not gpp_in_path() else "No config found.")
        gpp_dir = ask_path("Enter path to g++ (e.g., C:\\mingw\\bin\\g++.exe or its folder): ")
        if os.path.isfile(gpp_dir):
            gpp_path = gpp_dir
        else:
            gpp_path = os.path.join(gpp_dir, "g++.exe")
        if not os.path.exists(gpp_path):
            print("g++.exe not found at the given location.")
            return
    else:
        gpp_path = "g++"

    if not config:
        include = ask_path("Enter SFML include folder path: ")
        lib = ask_path("Enter SFML lib folder path: ")

        config = {
            "gpp_path": gpp_path,
            "sfml_include": include,
            "sfml_lib": lib
        }
        save_config(config)

    compile_cmd = [
        config["gpp_path"],
        "src/main.cpp",
        "-o", OUTPUT_EXE,
        "-I", config["sfml_include"],
        "-L", config["sfml_lib"],
        "-lsfml-graphics", "-lsfml-window", "-lsfml-system", "-lsfml-audio", "-lsfml-network",
        "-mwindows",
        "-static-libgcc", "-static-libstdc++"
    ]

    print("\nCompiling...")
    result = subprocess.run(compile_cmd)

    if result.returncode == 0 and os.path.exists(OUTPUT_EXE):
        print(f"\n? Build successful: {OUTPUT_EXE}")
        save_config(config)
    else:
        print("\n? Build failed.")

if __name__ == "__main__":
    main()
