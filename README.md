# OUTBREAK: CITY ZERO

A 2D top-down zombie outbreak strategy prototype for Intel Macs (macOS 12+) and iPhone.

## Core rule: the city starts open

There are no automatic perimeter walls, mandatory gates, or forced quarantine borders. Civilians and zombies can enter through any route that is not physically blocked by a building or something the player has placed. **You decide where to defend.**

## Gameplay in this prototype
- Procedurally generated city blocks, roads, parks, civilians and zombies.
- Pan and zoom the city map; drag-box to inspect groups of agents.
- Optional hand-built walls (key 2).
- Paintable visual zones (key 3); zones do not block movement.
- Player-placed barricades (key 4): cost 12 supplies, block movement, and can be damaged by zombies.
- Player-placed turrets (key 5): cost 35 supplies and automatically target nearby zombies.
- Basic zombie pursuit and infection, pause, and simulation speed controls.
- No automatic gates or invisible perimeter defenses.

## Controls
- Left drag: select area / place active tool.
- Right or middle drag: pan.
- Mouse wheel: zoom.
- 1 select, 2 optional wall, 3 zones, 4 barricade, 5 turret.
- Z cycles zone type, Space pauses, + / - changes speed, Esc cancels placement.

## Build on macOS
Install Xcode Command Line Tools and CMake 3.24+. SDL3 is fetched by CMake, so first configure needs internet.

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0
    cmake --build build --parallel
    ./build/OutbreakCityZero

GitHub Actions packages an Intel macOS ZIP and attempts an unsigned iOS IPA. An unsigned IPA is not ready for ordinary physical-device installation; iOS deployment generally requires signing and provisioning.

## Status
Early playable prototype. Player-placed defenses and open-route design are implemented in the SDL3 prototype; this is not a finished or faithful clone of any existing game. Visuals are procedural and placeholder-quality.