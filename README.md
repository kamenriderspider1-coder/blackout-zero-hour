# BLACKOUT: ZERO HOUR

**BLACKOUT: ZERO HOUR is an original, fast-paced first-person shooter (FPS) for macOS, inspired by the pacing, mission structure, and presentation of Call of Duty: Modern Warfare 3.** It is not an exploration-and-rescue game.

The goal is a cinematic, mission-based shooter with responsive first-person controls, enemy encounters, weapon handling, objective-driven combat, and a full campaign. The game should prioritize the FPS experience: moving through combat spaces, engaging hostile AI, completing mission objectives under pressure, and progressing through a connected story.

## Game vision

- **Genre:** First-person shooter.
- **Core loop:** Deploy into a mission, navigate combat arenas, fight hostile AI, complete mission objectives, and reach the mission endpoint.
- **Presentation:** Modern military-action atmosphere, cinematic mission introductions, readable HUD, impactful audio, and detailed 3D environments. The game is an original project and should use original or properly licensed assets.
- **Campaign:** Five connected missions with varied locations, objectives, checkpoints, save progression, dialogue, and cinematic moments.
- **Player systems:** First-person mouse look, responsive movement, sprinting, aiming, reloading, health, ammunition, and weapon switching.
- **Combat systems:** Hostile AI with distinct behaviors, cover-aware movement, hit feedback, encounter pacing, and mission-based enemy waves.
- **Multiplayer goal:** Online-only multiplayer, targeting up to eight players. No local multiplayer, couch co-op, or split-screen mode. Players use separate devices and connect over the internet.

## Current implementation status

The current native prototype in `macos/main3d.c` is an early playable 3D mission prototype built with SDL2 and OpenGL 3.2. It includes a first-person camera, keyboard and controller movement, mouse look, basic wall collision, sprinting, 3D environment props, relay interactions, extraction logic, and a stylized Kamen Rider Zero-One suit model at the extraction area.

**Current playable objective:** Explore the facility, activate three relay stations, then reach the extraction area. The window title reports relay progress and whether the player is near an objective. The controller's A button can interact with relays and extraction.

**Important:** This is still an early prototype, not the completed FPS described above. It does not yet include player weapons, shooting, aiming, reloading, enemy AI, damage/health, a combat HUD, animated character rigs, a multi-mission campaign, cutscenes, or multiplayer. Online multiplayer is a planned feature only; no networking or matchmaking is implemented yet. The Zero-One model is currently assembled from simple 3D shapes and is displayed at the extraction area, not yet a selectable playable character. These are development targets, not completed features.

## Controls

### Current prototype controls
- Return: start / resume
- R: restart the mission at any time
- W, A, S, D: move
- Mouse: look around
- Left / Right arrows: turn
- E: activate a nearby relay; after all three, interact at extraction
- Shift: sprint
- Escape: pause / resume

### Intended FPS controls
- W, A, S, D: move
- Mouse: aim / look
- Left mouse button: fire
- Right mouse button: aim down sights
- Shift: sprint
- R: reload
- Number keys / mouse wheel: switch equipped item
- E: interact with mission objects
- Escape: pause

The intended FPS controls describe the target design; they are not all implemented in the current prototype yet.

## Credits, cosmetics, perks, and future store

The planned progression system will let players earn **in-game credits by playing**. Credits are gameplay rewards, not something required to finish missions.

Planned ways to earn credits:
- Complete missions and optional objectives.
- Finish mission challenges and replayable milestones.
- Earn completion and performance rewards.

Planned ways to spend earned credits:
- Unlock operator and weapon cosmetic skins.
- Unlock fictional weapon options and cosmetic attachment appearances.
- Unlock and customize perk loadouts.
- Save multiple classes and cosmetic presets.

### Future real-money credit purchases

The game may add an **optional** real-money credit purchase feature in a later update. It is not part of the current prototype and must remain disabled until it is deliberately implemented, tested, and approved for the distribution platform.

If added, the store should:
- Keep gameplay credits earnable without paying.
- Clearly show prices and exactly how many credits a purchase provides.
- Use the platform's approved purchase flow and verify transactions securely.
- Require appropriate account-holder/parental approval where applicable.
- Avoid randomized paid loot boxes and pay-to-win advantages.
- Never store payment-card details in the game.

No real-money purchases are available in the current build. The credits economy and store are planned features, not implemented functionality yet.

## Development milestones

1. **Playable FPS foundation:** Replace relay scanning as the main loop with a combat-focused mission; add a visible weapon presentation, firing, aiming, reload/ammo state, player health, hit feedback, and a proper FPS HUD.
2. **Enemy encounters:** Add original hostile AI with patrol, alert, pursuit, cover-seeking, and damage states; build balanced combat spaces and encounter waves.
3. **Campaign:** Build five connected missions with varied environments, mission briefings, objectives, dialogue, cinematic sequences, checkpoints, save progression, and mission completion screens.
4. **Polish:** Replace blockout geometry with authored 3D environments and props; add character animation, sound design, music, effects, accessibility options, graphics settings, and performance optimization.
5. **Online multiplayer only:** Build dedicated/hosted internet sessions, player accounts or guest identities, matchmaking or invite codes, synchronized movement and combat, reconnect handling, and server-authoritative match state. No split-screen or local multiplayer. Online play comes after the core FPS and server infrastructure are stable.

<!-- MACOS-DOWNLOADS-START -->
## Download for macOS

Both installers below were generated and verified by the GitHub Actions release workflow. These builds are not notarized because this project does not have an Apple Developer ID.

- [Apple Silicon (M1/M2/M3/M4)](https://github.com/kamenriderspider1-coder/blackout-zero-hour/releases/download/v1.0.0/BLACKOUT-ZERO-HOUR-macOS-arm64.dmg)
- [Intel Mac](https://github.com/kamenriderspider1-coder/blackout-zero-hour/releases/download/v1.0.0/BLACKOUT-ZERO-HOUR-macOS-x64.dmg)

First launch: macOS may say the app cannot be verified because it is not notarized. Only if you trust this build, open System Settings → Privacy & Security → Open Anyway after the first blocked launch. Do not disable Gatekeeper globally.

See the [full release notes and files](https://github.com/kamenriderspider1-coder/blackout-zero-hour/releases/tag/v1.0.0).
<!-- MACOS-DOWNLOADS-END -->

## Playtest checklist

1. Download the Apple Silicon DMG if your Mac has an M-series chip, or the Intel DMG for an Intel Mac.
2. Open the DMG and drag **BLACKOUT ZERO HOUR.app** into **Applications**.
3. Open the app. If macOS blocks this unsigned test build, use Finder to Control-click the app, choose **Open**, then confirm.
4. Press **Return** to start.
5. Use **W/A/S/D** to move, the mouse to look, **Shift** to sprint, and **E** near a relay to activate it.
6. Activate all three relays, then reach extraction and press **E**.
7. Press **R** at any time to restart. With a controller, use **A** to interact, **Start** to pause, and **Y** to restart.

This is an early gameplay test. It does not yet have shooting, enemies, health, online multiplayer, or a full campaign. Please report crashes, controls that do not respond, graphical glitches, or a relay/extraction that cannot be completed.

## Build

The GitHub Actions macOS workflow builds the native SDL2 + OpenGL application and packages a DMG. A downloadable installer should only be considered available after the workflow completes successfully and the artifact is verified. Builds are currently unsigned and may show a macOS Gatekeeper warning.

## Bluetooth controller support (Version 1)

Connect a Bluetooth game controller to your Mac before launching the game. The game uses SDL's standard game-controller mappings, so common Xbox, PlayStation, and compatible controllers should be detected when macOS exposes them as game controllers.

- Left stick: move
- Right stick: look
- A / Cross: interact with a nearby relay or extraction
- Start / Options: pause
- Y: restart the mission
- F1: open or close controller settings
- In controller settings, press 1 to toggle inverted look, 2 to swap the movement and look sticks, 3 to cycle look sensitivity, and 4 to cycle stick deadzone
- Escape: close settings or pause

Settings are saved in the macOS application preferences folder and reused next time. The game window title displays the current stick layout, look direction, sensitivity, and deadzone. Keyboard movement remains available.
