# BLACKOUT: ZERO HOUR

An original Nintendo Switch homebrew action prototype.

## Playtest build 0.2

This build expands the original movement proof-of-concept into a playable top-down tactical mission with a console-rendered interface.

Implemented:
- Operations menu
- Three selectable fictional primary weapon profiles
- Gunsmith menu with three toggleable cosmetic/configuration slots
- One campaign-style mission, Operation Nightfall
- Grid movement and wall collision
- Three intel caches and extraction objective
- Three simulated target drones
- Directional pulse mechanic, ammo counter, and recharge
- Mission completion screen and score

This is a playable early prototype, **not a finished 3D first-person shooter**. It does not yet include 3D weapon models, animated graphics, campaign chapters, enemy AI, online/local multiplayer, split-screen, voice, or a full progression system. The current interface uses the Switch homebrew console renderer.

## Build

GitHub Actions builds the NRO using devkitPro devkitA64/libnx. Every push to `main` triggers a build. Download the `BLACKOUT-ZERO-HOUR-NRO` artifact from the newest successful Actions run.

## Install

Extract the downloaded artifact and copy the NRO to:

`SD:/switch/blackout-zero-hour/blackout-zero-hour.nro`

Launch it from Homebrew Menu on a Switch running Atmosphère.

## Controls

**Operations menu**
- D-pad up/down: navigate
- A: select
- Plus: quit

**Loadout**
- Left/right: choose weapon
- A: return to operations
- B: back

**Gunsmith**
- Up/down: choose configuration slot
- A: toggle slot
- B: back

**Mission**
- D-pad: move
- A: collect intel / use extraction
- X: directional pulse
- Y: recharge ammo
- B: return to operations
- Plus: quit

Collect all three intel caches, then reach the E extraction marker. The pulse travels horizontally in the direction of your last horizontal movement.

## ChatGPT development link

The connected GitHub repository is the development bridge. Feature requests in the BLACKOUT project conversation can be implemented in these source files, committed here, and compiled by Actions. This is not a live ChatGPT service inside the Switch game. See [docs/CHATGPT_DEV_LINK.md](docs/CHATGPT_DEV_LINK.md).

All names, code, maps, and planned content are original. This unofficial homebrew project is not affiliated with or endorsed by Nintendo or any commercial game publisher.
