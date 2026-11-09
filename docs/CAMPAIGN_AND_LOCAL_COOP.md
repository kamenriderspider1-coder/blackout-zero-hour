# Campaign, local co-op, and online co-op implementation plan

This document defines the planned multiplayer and campaign milestones. It does not claim these modes are already implemented.

## Campaign structure

A continuous campaign with shared mission progression, playable solo or cooperatively:

1. **Nightfall** — tutorial expedition; restore three signal relays and reach the safe zone.
2. **Dead Air** — navigate a storm-damaged district and restore power to a communications hub.
3. **The Floodline** — map safe routes through a flooded transit station and activate evacuation beacons.
4. **Last Transmission** — locate the source of a repeating distress signal and recover its message.
5. **Safe Passage** — coordinate the final evacuation route and bring all objectives online.

Each mission needs its own map, spawn points, objective list, completion trigger, and unlock condition. Campaign progress should be saved locally, with an online profile/save option considered later. Mission objectives are cooperative exploration, navigation, scanning, repair, and rescue tasks.

## Local multiplayer: up to four players

Target: 1–4 players on one Mac, with optional game controllers.

- Player 1: keyboard/mouse or controller 1.
- Players 2–4: SDL game controllers.
- One shared campaign session and shared objective state.
- Each player has an independent position, facing direction, and camera.
- Split-screen layouts: 1 player full screen; 2 players two viewports; 3 players three viewports with one quadrant reserved; 4 players four equal quadrants.
- Each viewport renders from that player's camera.
- Scanning updates shared objectives, completed once for the whole team.
- Player join/leave is allowed from a local lobby before starting a mission.

## Online campaign: up to eight players

Target: 2–8 players joining the same campaign mission over the network.

- Host or dedicated server creates a campaign session with a mission and difficulty setting.
- Friends join through an invite code or session browser.
- The server is authoritative for player positions, mission objectives, mission completion, and shared progression to reduce conflicting game state.
- Each participant renders their own full-screen view; online players do not use local split-screen.
- Late joining should be allowed only at safe checkpoints, with the new player receiving the current mission and objective state.
- If the host disconnects, either transfer host authority safely or end the session with a recoverable checkpoint.
- Use authenticated sessions, encrypted transport (TLS), input/state validation, rate limits, and server-side checks before any public release.
- Keep local co-op and online co-op as separate lobby choices, while both can launch the same campaign missions.

## Required engineering work

1. Replace global player coordinates and angle with a Player array for local players and a network player collection for remote participants.
2. Add SDL game-controller discovery, hot-plug handling, and per-player input mapping.
3. Add a viewport renderer for one to four local cameras.
4. Build a local lobby showing connected controllers and player slots.
5. Add campaign mission data, unlocks, checkpoints, and save/load.
6. Expand the current two-peer networking prototype into an eight-player session protocol with server-authoritative state and reconnect handling.
7. Build online lobby, invite/join flow, session discovery, and secure server hosting.
8. Test local viewport layouts, eight-client network sessions, disconnects, and mission-state synchronization.

## Current status

The game is currently a one-player ray-casting prototype. The local renderer and input loop have not yet been converted to multi-camera, four-player split-screen. Campaign missions 2–5, save progression, controller lobby, and four-player split-screen are not yet implemented. A basic TCP relay/client prototype exists, but it has not been verified as a working multiplayer session, is not publicly hosted, and is not an eight-player campaign service. Online campaign, eight-player sessions, matchmaking/invites, and secure production hosting remain unimplemented.
