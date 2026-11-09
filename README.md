# BLACKOUT: ZERO HOUR

An original Nintendo Switch homebrew 3D exploration and rescue prototype.

## Current test build

- Software-rendered 3D scene with perspective projection and depth buffering
- First-person camera with controller movement and look
- Handheld signal scanner prop
- Main menu, explorer selection, and field briefing
- One short objective: locate and scan the disabled relay beacon
- On-screen objective and status HUD

This is an early prototype, not a finished commercial-scale game. It uses simple flat-shaded geometry and has not yet been tested on physical Switch hardware.

## Build

GitHub Actions builds the NRO using devkitPro devkitA64/libnx. Download the `BLACKOUT-ZERO-HOUR-NRO` artifact from the newest successful Actions run.

## Install

Extract the artifact and copy `blackout-zero-hour.nro` to:

`SD:/switch/blackout-zero-hour/blackout-zero-hour.nro`

Launch it from Homebrew Menu on a Switch running Atmosphère.

## Controls

**Menus**
- A: select / confirm
- X: explorer selection
- Y: field briefing
- B: back
- Plus: exit

**Mission**
- Left stick: move
- Right stick: look around
- D-pad: move
- A: scan beacon when close
- B: return to menu
- Plus: exit

This is an unofficial homebrew project and is not affiliated with or endorsed by Nintendo.
