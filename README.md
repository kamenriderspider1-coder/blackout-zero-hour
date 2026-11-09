# BLACKOUT: ZERO HOUR

An original Nintendo Switch homebrew first-person shooter prototype.

## 3D graphics prototype

The previous build used the libnx text console. That was not the requested game, so the project is being moved to a custom polygon-rendered first-person engine.

Current graphics work in this branch:
- Full-screen framebuffer rendering instead of the terminal console
- Perspective-projected 3D polygon geometry and depth buffer
- First-person camera with analog-stick look and movement
- A visible in-game weapon model built from 3D geometry
- 3D environment blocks, floor perspective, crosshair, and HUD
- Operations, loadout, and gunsmith screens
- Toggleable fictional optic, barrel, and grip models

## Current limitations

This is an early software-rendered 3D engine proof of concept, not a finished AAA-style shooter. The renderer currently uses simple flat-shaded geometry. It does not yet have textured environments, skeletal animation, full enemy AI, campaign missions, multiplayer, online services, or a complete progression system. The build must pass GitHub Actions and be tested on the actual Switch before it can be called playable on hardware.

## Build

GitHub Actions builds the NRO using devkitPro devkitA64/libnx. Every push to `main` triggers a build. Download the `BLACKOUT-ZERO-HOUR-NRO` artifact from the newest successful Actions run.

## Install

Extract the downloaded artifact and copy the NRO to:

`SD:/switch/blackout-zero-hour/blackout-zero-hour.nro`

Launch it from Homebrew Menu on a Switch running Atmosphère.

## Controls

**Operations**
- A: deploy
- X: loadout
- Y: gunsmith
- Plus: exit

**Loadout**
- Left/right: select weapon
- A: return
- B: back

**Gunsmith**
- Up/down: choose attachment
- A: toggle attachment
- B: back

**3D mission**
- Left stick or D-pad: move
- Right stick: look around
- B: return to operations
- Plus: exit

All names, code, maps, and planned content are original. This unofficial homebrew project is not affiliated with or endorsed by Nintendo or any commercial game publisher.
