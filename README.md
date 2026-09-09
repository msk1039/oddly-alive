# Procedural Koi

A small C++20 and Raylib experiment that renders procedurally animated koi as
black-and-white pixel art.

Read [how the animation works](docs/how-it-works.md) for the full explanation.



https://github.com/user-attachments/assets/67b8d7ad-1521-45d1-aa88-6d4cd5aee727



## Requirements

- CMake 3.20 or newer
- Raylib
- pkg-config

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/procedural-koi
```

## Controls

- Left click: call the fish
- Space: scatter the fish
- `[` / `]`: change the fish count
- `D`: show the procedural spines
- `H`: hide the interface
- `R`: reset
- `F11`: toggle fullscreen
- Escape: quit
