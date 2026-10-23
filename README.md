# OUTBREAK: CITY ZERO

A 2D top-down zombie outbreak strategy prototype for Intel Macs (macOS 12+) and iPhone. Built with C++20, SDL3, and CMake.

## Current slice
- Procedural city map, roads, blocks and buildings
- Camera pan/zoom
- Drag-box inspection
- Wall construction by dragging, with supply costs
- Paintable Safe, Evacuation and Containment zones
- Moving civilians/zombies and basic infection simulation
- Pause and simulation speed controls
- Mouse, keyboard and basic touch input

This is an original genre-inspired implementation using procedural visuals, not copied source code or assets.

## Build on Mac
Install Xcode Command Line Tools and CMake 3.24+. SDL3 is fetched by CMake, so the first configure needs internet access.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0
cmake --build build --parallel
./build/OutbreakCityZero
```

## Controls
- Left drag: select / build wall / paint zone depending on active tool
- Right or middle drag: pan
- Mouse wheel: zoom
- 1 Select, 2 Walls, 3 Zones, Z cycle zone, Space pause, + / - speed, Esc cancel

## Releases
GitHub Actions will publish a macOS Intel ZIP artifact and attempt an unsigned iOS IPA build. An unsigned IPA is not ready for ordinary physical-device installation; iPhone deployment normally requires Apple signing and provisioning credentials. No signing secrets are stored in this repository.

Status: early prototype. Advanced pathfinding, campaign missions, production art/audio, and real-device QA remain future work.
