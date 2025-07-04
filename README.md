# bloodwavez

**bloodwavez** is a 2D top-down pixel art game where you play as a warrior progressing through increasingly difficult levels filled with enemies and bosses. Defeat them using various weapons, collect gold and XP, unlock new skills through a skill tree, and improve your character via meta-progression. The game features smart enemy AI powered by neural networks, a dynamic inventory system, pixel-perfect sprite work, and immersive background music and sound effects.

[View video demo](https://drive.google.com/file/d/1cHuSf3AdqzctMaZXP4qXpxJKE00FMNnx/view?usp=sharing)

## Controls

- `W A S D` – move
- `E` – pickup items
- `I` – inventory
- `1` & `2` – switch weapons
- `P` – toggle skill tree view
- `Left Click` – use equipped weapon
- `Space` – dash (has cooldown)
- `ESC` – exit

## Build Instructions (Windows – SFML 3.0.0)

1. Make sure you have a C++ compiler installed (e.g. `g++`) and added to your PATH. ([this](https://github.com/brechtsanders/winlibs_mingw/releases/download/14.2.0posix-19.1.1-12.0.0-ucrt-r2/winlibs-x86_64-posix-seh-gcc-14.2.0-mingw-w64ucrt-12.0.0-r2.7z)) (and python3 too)
2. Download and extract [SFML 3.0.0](https://www.sfml-dev.org/download.php). For the release version just set `RELEASE = true`.
3. Run `make.py` and add the paths. Then run the game from the terminal but from the main dir, not /bin (`.\bin\bloodwavez.exe`)
