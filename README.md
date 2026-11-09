# BLACKOUT: ZERO HOUR

An original Nintendo Switch homebrew exploration and rescue game project.

## Current test build

- Software-rendered 3D scene with perspective projection and depth buffering
- First-person camera with controller movement and look
- Handheld signal scanner prop
- Main operations menu with Campaign, Multiplayer, Co-op Operations, Survival, and Extras
- Multiplayer and co-op menu shells with an 8-player target
- Campaign and survival entry points, explorer selection, and field briefing
- A visible relay beacon in the 3D environment
- Complete mission loop: approach and scan the relay, then return to the extraction point
- Mission-complete screen with replay and main-menu options
- On-screen objective and status HUD

The current build is an early playable offline foundation with simple flat-shaded geometry and a software renderer. The multiplayer and co-op screens are menu/lobby placeholders only: there is no matchmaking, network session, server, or player synchronization yet. Real 8-player internet play requires a separately hosted service, a network protocol, connection handling, and testing on multiple consoles. The source changes have not yet been rebuilt or tested on physical Switch hardware.

## Build

GitHub Actions builds the NRO using devkitPro devkitA64/libnx. Download the `BLACKOUT-ZERO-HOUR-NRO` artifact from the newest successful Actions run.

## Install

Extract the artifact and copy `blackout-zero-hour.nro` to:

`SD:/switch/blackout-zero-hour/blackout-zero-hour.nro`

Launch it from Homebrew Menu on a Switch running Atmosphère.

## Controls

**Menus**
- Up / Down: choose Campaign, Multiplayer, Co-op Operations, Survival, or Extras
- A: open selection
- B: back
- Campaign: A starts the mission, X selects explorer, Y opens briefing
- B: back
- Plus: exit

**Mission**
- Left stick: move
- Right stick: look around
- D-pad: move
- A: scan beacon when close, then replay after mission completion
- B: return to menu, or return to menu after mission completion
- Plus: exit

Multiplayer status: planned, not yet functional. The lobby screen intentionally reports that no server is configured.

This is an unofficial homebrew project and is not affiliated with or endorsed by Nintendo.
