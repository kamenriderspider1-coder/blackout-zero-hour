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
- **Co-op goal:** Four-player local split-screen and up-to-eight-player online co-op, developed after the single-player FPS foundation is stable.

## Current implementation status

The current native prototype in `macos/main3d.c` is an early 3D blockout. It has an SDL2 + OpenGL 3.2 window, a first-person camera, basic movement and collision, simple polygonal geometry, and placeholder relay/extraction interactions.

**Important:** This prototype is not yet the intended FPS. It does not yet have a complete combat loop, weapon systems, hostile AI, campaign missions, cinematic presentation, or multiplayer. Those are development targets, not completed features. The next development work must replace the exploration-style relay loop with an actual FPS mission and combat loop.

## Controls

### Current prototype controls
- Return: start / resume
- W, A, S, D: move
- Mouse: look around
- Left / Right arrows: turn
- E: interact with a nearby placeholder relay
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
5. **Multiplayer:** Add four-player local split-screen and up-to-eight-player online co-op only after the single-player FPS and networking foundations are stable.

## Build

The GitHub Actions macOS workflow builds the native SDL2 + OpenGL application and packages a DMG. A downloadable installer should only be considered available after the workflow completes successfully and the artifact is verified. Builds are currently unsigned and may show a macOS Gatekeeper warning.

## Bluetooth controller support (Version 1)

Connect a Bluetooth game controller to your Mac before launching the game. The game uses SDL's standard game-controller mappings, so common Xbox, PlayStation, and compatible controllers should be detected when macOS exposes them as game controllers.

- Left stick: move
- Right stick: look
- A / Cross: interact with a nearby relay
- Start / Options: pause
- F1: open or close controller settings
- In controller settings, press 1 to toggle inverted look, 2 to swap the movement and look sticks, 3 to cycle look sensitivity, and 4 to cycle stick deadzone
- Escape: close settings or pause

Settings are saved in the macOS application preferences folder and reused next time. The game window title displays the current stick layout, look direction, sensitivity, and deadzone. Keyboard movement remains available.
