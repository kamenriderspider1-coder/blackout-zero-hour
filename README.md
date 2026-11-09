# BLACKOUT: ZERO HOUR

An original Nintendo Switch homebrew 3D exploration and rescue prototype.

## Current test build

- Software-rendered 3D scene with perspective projection and depth buffering
- First-person camera with controller movement and look
- Handheld signal scanner prop
- Main menu, explorer selection, and field briefing
- A visible relay beacon in the 3D environment
- Complete mission loop: approach and scan the relay, then return to the extraction point
- Mission-complete screen with replay and main-menu options
- On-screen objective and status HUD

This is a small, self-contained homebrew V1, not a commercial-scale production. It uses simple flat-shaded geometry and a software renderer. The source builds into an NRO, but runtime behavior still needs confirmation on physical Switch hardware.

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
- A: scan beacon when close, then replay after mission completion
- B: return to menu, or return to menu after mission completion
- Plus: exit

This is an unofficial homebrew project and is not affiliated with or endorsed by Nintendo.
