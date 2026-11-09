# Campaign and 4-player local co-op implementation plan

This document defines the next playable milestone. It does not claim these modes are already implemented.

## Campaign structure

A continuous single-player or local co-op campaign with shared mission progression:

1. **Nightfall** — tutorial expedition; restore three signal relays and reach the safe zone.
2. **Dead Air** — navigate a storm-damaged district and restore power to a communications hub.
3. **The Floodline** — map safe routes through a flooded transit station and activate evacuation beacons.
4. **Last Transmission** — locate the source of a repeating distress signal and recover its message.
5. **Safe Passage** — coordinate the final evacuation route and bring all objectives online.

Each mission needs its own map, spawn points, objective list, completion trigger, and unlock condition. Campaign progress should be saved locally in a versioned save file. Mission objectives are cooperative exploration, navigation, scanning, repair, and rescue tasks.

## Local multiplayer: up to four players

Target: 1–4 players on one Mac, with optional game controllers.

- Player 1: keyboard/mouse or controller 1.
- Players 2–4: SDL game controllers.
- One shared campaign session and shared objective state.
- Each player has an independent position, facing direction, and camera.
- Split-screen layouts: 1 player full screen; 2 players two viewports; 3 players three viewports with one quadrant reserved; 4 players four equal quadrants.
- Each viewport uses SDL renderer viewports/clipping and renders from that player's camera.
- Scanning updates shared objectives, completed once for the whole team.
- Player join/leave is allowed from a local lobby before starting a mission.

## Required engineering work

1. Replace global player coordinates and angle with a Player array of four entries.
2. Add SDL game-controller discovery, hot-plug handling, and per-player input mapping.
3. Add a viewport renderer that takes a player camera and destination rectangle.
4. Draw a lobby screen showing connected controllers and player slots.
5. Add campaign mission data and a save/load layer.
6. Test all viewport layouts and controller disconnect/reconnect.

## Current status

The existing game remains a one-player ray-casting prototype. Its renderer and input loop have not yet been converted to multi-camera, four-player split-screen. Campaign missions 2–5, save progression, controller lobby, and four-player split-screen are not yet implemented.
