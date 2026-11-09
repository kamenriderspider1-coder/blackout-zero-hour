# BLACKOUT: ZERO HOUR

An original first-person exploration and rescue game for macOS.

## Current 3D prototype

The macOS build target is now the native SDL2 + OpenGL 3.2 core prototype in `macos/main3d.c`. It uses polygonal cube geometry, a perspective projection, a first-person camera, depth testing, basic directional shading, wall collision, three relay objectives, and an extraction marker. It is an early blockout prototype, not a finished AAA-quality game: it does not yet include detailed art assets, character animations, cinematic sequences, campaign progression, or working multiplayer.

### Controls
- Return: start / resume
- W, A, S, D: move
- Mouse: look around
- Left / Right arrows: turn
- E: scan a nearby relay
- Escape: pause / resume

### Planned next milestones
- Replace blockout geometry with authored 3D environment and prop assets.
- Add mission UI, audio, dialogue, checkpoint/save progression, and the five-mission campaign.
- Add four-player local split-screen and up-to-eight-player online co-op after the core 3D game is stable.

## Build

The GitHub Actions macOS workflow builds the native app and packages a DMG using SDL2 and Apple's OpenGL framework. A downloadable installer should only be considered available after the workflow completes successfully and the artifact is verified.
