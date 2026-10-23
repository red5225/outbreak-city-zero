# Roadmap

1. Foundation: SDL3 window, procedural 2D city, input and camera.
2. Map tools: drag-box inspection, wall drawing, zone painting.
3. Simulation: robust state machines, collision and pathfinding.
4. Command: deployable units, evacuation orders, alerts and economy.
5. Content: scenarios, objectives, scoring, saves and campaign.
6. Polish: sprite art, proper UI font, audio, settings and accessibility.
7. Platform: test on Intel macOS 12 and physical iPhone; optimize touch and performance.

## Current prototype limitations
The first version uses simplified steering and coarse wall collision. The nearest-target search is intentionally simple and should be replaced by spatial partitioning before increasing agent counts. Touch controls support drag actions; a production mobile build still needs proper tool buttons, pinch gestures, safe-area layout and device QA. iOS CI produces an unsigned package only unless signing credentials are configured.
