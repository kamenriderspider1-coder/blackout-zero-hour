# BLACKOUT: ZERO HOUR

An original Nintendo Switch homebrew exploration and rescue game prototype.

## Current menu structure

- **Campaign:** Operation Nightfall story mission, explorer selection, and field briefing
- **Multiplayer:** public matchmaking and private-room menu, designed around 12 player slots
- **Co-op Operations:** team expedition and private team-room menu, designed around up to 12 players
- **Survival:** relay recovery challenge
- **Extras:** explorer selection, field briefing, and online status

## Online multiplayer status

**12 players is the design target, not a working online feature yet.** The current NRO does not connect to a matchmaking or game server. Wi-Fi connectivity alone is not enough: online play still needs a hosted service for account/session handling, lobby discovery, invites, and synchronized game state. The lobby screen currently reports that the online service is not configured. No players can join yet.

The next engineering milestone is a small dedicated session service plus a Switch network client, followed by testing with multiple consoles. The server endpoint and deployment environment must be selected and configured before real online play can be enabled.

## Current prototype

- Software-rendered 3D scene with perspective projection and depth buffering
- First-person camera movement and look
- Handheld signal scanner and visible relay beacon
- One complete offline mission loop: locate and scan the relay, then return to extraction
- Mission-complete screen with replay and main-menu options

This remains an early prototype, not a commercial-scale game. Several menu destinations are scaffolding and do not yet contain full game modes. It uses simple flat-shaded geometry and a software renderer. Runtime behavior still needs confirmation on physical Switch hardware.

## Build and install

GitHub Actions builds the NRO using devkitPro devkitA64/libnx. Download the `BLACKOUT-ZERO-HOUR-NRO` artifact from the newest successful Actions run.

Copy `blackout-zero-hour.nro` to:

`SD:/switch/blackout-zero-hour/blackout-zero-hour.nro`

Launch it from Homebrew Menu on a Switch running Atmosphère.

## Controls

- Menus: D-pad Up/Down to navigate, A to select, B to go back, Plus to exit
- Mission: left stick to move, right stick to look, D-pad to move, A to scan the beacon when close, B to return to menu

This is an unofficial homebrew project and is not affiliated with or endorsed by Nintendo.
