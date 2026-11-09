# BLACKOUT: ZERO HOUR

An original Nintendo Switch homebrew game project.

## Current milestone

The current build is a small playable libnx prototype:
- D-pad movement on a tile map
- Wall collision
- Three intel pickups
- Extraction objective
- Nintendo Switch Homebrew Menu NRO packaging

This is a foundation, not yet a 3D first-person shooter. A first-person renderer, original 3D assets, campaign, bots, and multiplayer features are not implemented yet.

## Build

The GitHub Actions workflow builds with the devkitPro devkitA64/libnx toolchain. Push to main or run the workflow manually, then download the BLACKOUT-ZERO-HOUR-NRO artifact from the Actions run.

## Install

Copy blackout-zero-hour.nro to:

SD:/switch/blackout-zero-hour/blackout-zero-hour.nro

Launch it from Homebrew Menu on a Switch running Atmosphère.

## Controls

- D-pad: move
- Plus: quit

All names, maps, code, and planned game content are original. This is an unofficial homebrew project and is not affiliated with or endorsed by Nintendo or any commercial game publisher.

## ChatGPT development link

This repository is connected to the BLACKOUT development conversation. You can describe a feature or bug in that chat, and ChatGPT can update the actual source files here through the authorized GitHub connection. Each accepted change is committed to this repository and can trigger a fresh Switch build through GitHub Actions.

This is a developer-to-repository bridge, not a live ChatGPT service inside the Switch game. After a successful build, download the `BLACKOUT-ZERO-HOUR-NRO` artifact from the Actions run and install it on the Switch. See [the full workflow](docs/CHATGPT_DEV_LINK.md).
